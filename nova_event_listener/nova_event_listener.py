#!/usr/bin/env python
# -*- encoding: utf-8 -*-
#
# Copyright 2013 Telefónica I+D
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#


__version__         = '1.0.0'
__version_info__    = tuple([int(num) for num in __version__.split('.')])
__description__     = 'OpenStack Nova Event Listener'


from celery import Celery
from ceilometer.openstack.common import rpc
from ConfigParser import SafeConfigParser
from optparse import OptionParser
from urllib2 import Request, URLError, urlopen
import logging.config
import os.path
import httplib
import random
import sys


"""NGSI entity type for OpenStack's servers"""
SERVER_ENTITY_TYPE = 'vm'


"""Default attributes for NGSI entities.

A dictionary whose keys are the different entity types. Values are lists of valid entity attributes for that type. The
set of types and/or attributes may be modified or extended via configuration file (see below).
"""
ENTITY_ATTRIBUTES = {
    'vm': [                                             # entityType='vm'
        'cpuLoadPct',                                   # percentage of CPU load
        'freeSpacePct',                                 # percentage of free physical or virtual host disk
        'usedMemPct',                                   # percentage of RAM memory in use
        'procs',                                        # number of running processes
        'users'                                         # number of users logged in
    ]
}


"""Default configuration.

The configuration `cfg_defaults` can be superseded with that read from `cfg_filename` (at path `conf/<progname>.cfg`),
if file exists.
"""
name = os.path.splitext(os.path.basename(__file__))[0]
cfg_filename = os.path.join(os.path.dirname(__file__), 'conf', '%s.cfg' % name)
cfg_defaults = {
    'brokerUrl':            'http://127.0.0.1:1338/',   # context broker URL
    'registerAppUrl':       'http://fiware/vm/',        # NGSI register providing application (ignored)
    'registerDuration':     'P99Y',                     # NGSI register duration (99 years ~ infinity)
    'registerAttributes':   str(ENTITY_ATTRIBUTES),     # NGSI register entity attributes
    'retries':              2,                          # number of retries (exponential backoff)
    'factor':               2,                          # factor for exponential backoff
    'randomize':            False,                      # enable randomization for exponential backoff
    'minRetryTime':         1000,                       # minimum time for exponential backoff (millis)
    'maxRetryTime':         sys.maxint,                 # maximum time for exponential backoff (millis)
    'logLevel':             'INFO',
    'logFormat':            '%(asctime)s [%(level)s] %(message)s'
}


"""Celery application initialization.

Uses local RabbitMQ server as message broker and result backend.
"""
app = Celery(name, broker='amqp://', backend='amqp')


def get_register_context_xml(entity_id, entity_type, register_app_url, register_duration, register_attributes):
    """Generate a XML `registerContext` request body.

    :param string entity_id:            the identifier of the entity being registered.
    :param string entity_type:          the type of the entity being registered.
    :param string register_app_url:     the providing application URL.
    :param string register_duration:    the duration of the registration.
    :param dict   register_attributes:  the attribute list of each type of entity being registered.
    :returns:                           the body in XML format for the registerContext NGSI9 request.
    """
    format_args = locals()
    format_args.update({
        'register_attrs_xml': '\n'.join(['''
            <contextRegistrationAttribute>
                <name>{0}</name>
                <type>string</type>
                <isDomain>false</isDomain>
            </contextRegistrationAttribute>
        '''.format(attr) for attr in register_attributes.get(entity_type)])
    })
    result = '''
        <?xml version="1.0"?>
        <registerContextRequest>
            <contextRegistrationList>
                <contextRegistration>
                    <entityIdList>
                        <entityId type="{entity_type}" isPattern="false">
                            <id>{entity_id}</id>
                        </entityId>
                    </entityIdList>
                    <contextRegistrationAttributeList>
                        {register_attrs_xml}
                    </contextRegistrationAttributeList>
                    <providingApplication>{register_app_url}</providingApplication>
                </contextRegistration>
            </contextRegistrationList>
            <duration>{register_duration}</duration>
        </registerContextRequest>
    '''.format(**format_args)
    return result;


def listen(connection, config):
    """Listen a RPC connection for events.

    When an event of instance creation or deletion is received, this function schedules an asynchronous, retriable task
    consisting of sending a NSGI registration request to Context Broker.

    :param connection:                  the RPC connection to listen to.
    :param config:                      the `ConfigParser` object with the configuration parameters.
    """
    topic = 'notifications.info'
    def process_event(msg):
        event_type = msg.get('event_type')
        event_creation = False
        event_deletion = False
        if event_type == 'compute.instance.create.end':
            event_creation = True
        elif event_type == 'compute.instance.delete.end':
            event_deletion = True
        elif event_type == 'compute.instance.update':
            state = msg.get('payload').get('state')
            transition = msg.get('payload').get('state_description')
            if transition == '':
                if state == 'active':
                    event_creation = True
                elif state == 'deleted':
                    event_deletion = True
        # schedule asynchronous processing of creation events
        if event_creation:
            logging.debug('Instance %s created (%s)', msg.get('payload').get('instance_id'), event_type)
            ngsi_request.delay(event_msg=msg,
                               broker_url=config.get('common', 'brokerUrl'),
                               max_retries=int(eval(config.get('common', 'retries'))),
                               factor=int(eval(config.get('common', 'factor'))),
                               randomize=bool(eval(config.get('common', 'randomize'))),
                               min_time_millis=int(eval(config.get('common', 'minRetryTime'))),
                               max_time_millis=int(eval(config.get('common', 'maxRetryTime'))),
                               register_app_url=config.get('common', 'registerAppUrl'),
                               register_duration=config.get('common', 'registerDuration'),
                               register_attributes=config.get('common', 'registerAttributes'))
        elif event_deletion:
            logging.debug('Instance %s deleted (%s)', msg.get('payload').get('instance_id'), event_type)

    connection.declare_topic_consumer(topic, process_event)
    try:
        connection.consume()
    except KeyboardInterrupt:
        pass


@app.task(bind=True)
def ngsi_request(self, event_msg,
                 broker_url=cfg_defaults['brokerUrl'],
                 max_retries=cfg_defaults['retries'],
                 factor=cfg_defaults['factor'],
                 randomize=cfg_defaults['randomize'],
                 min_time_millis=cfg_defaults['minRetryTime'],
                 max_time_millis=cfg_defaults['maxRetryTime'],
                 register_app_url=cfg_defaults['registerAppUrl'],
                 register_duration=cfg_defaults['registerDuration'],
                 register_attributes=cfg_defaults['registerAttributes']):

    """Process an instance creation event message (asynchronously) by sending a NGSI9 request to Context Broker.

    :param task    self:                the asynchronous task being processed.
    :param dict    event_msg:           the event message that triggered the task.
    :param string  broker_url:          the URL of the Context Broker to send requests to.
    :param integer max_retries:         the maximum number of retries (in case the request fails).
    :param integer factor:              the factor of the exponential backoff retrial policy.
    :param boolean randomize:           enable randomization in exponential backoff retrial policy.
    :param integer min_time_millis:     the minimum delay of the exponential backoff retrial policy.
    :param integer max_time_millis:     the maximum delay of the exponential backoff retrial policy.
    :param string  register_app_url:    the providing application URL.
    :param string  register_duration:   the duration of the registration.
    :param string  register_attributes: the stringified dictionary of entity types and their attribute list.
    :raises URLError:                   when an error occurs and maximum number of retries is exceeded.
    """
    def count_exponential_backoff(attempts):
        rnd = random.uniform(1,2) if randomize else 1
        return min(int(rnd * min_time_millis/1000 * (factor**attempts)), max_time_millis/1000)

    try:
        region = event_msg.get('_context_service_catalog')[0].get('endpoints')[0].get('region')
        instance_id = event_msg.get('payload').get('instance_id')
        hostname = event_msg.get('payload').get('hostname')
        tenant_id = event_msg.get('_context_tenant')
        project = event_msg.get('_context_project_name')
        user = event_msg.get('_context_user_name')
        logging.debug('New instance %s', {
            'region':       region,
            'instance_id':  instance_id,
            'hostname':     hostname,
            'tenant_id':    tenant_id,
            'project':      project,
            'user':         user
        })
        entity_id = '{0}:{1}'.format(region, instance_id)
        request_data = get_register_context_xml(
            entity_id=entity_id,
            entity_type=SERVER_ENTITY_TYPE,
            register_app_url=register_app_url,
            register_duration=register_duration,
            register_attributes=eval(register_attributes))
        response = urlopen(Request(
            url=broker_url,
            data=request_data,
            headers={
                'Content-Type': 'application/xml; charset=utf8',
                'Content-Length': len(request_data)
            }
        ))
        status = response.getcode()
        logging.info('Registration of %s into %s: %s %s', entity_id, broker_url, status, httplib.responses[status])
    except (URLError) as exc:
        error = RuntimeError('urlopen({0}): {1}'.format(broker_url, exc.reason.strerror))
        raise self.retry(exc=error, countdown=count_exponential_backoff(self.request.retries), max_retries=max_retries)


def main():
    # process configuration file (if exists) and setup logging
    config = SafeConfigParser(cfg_defaults)
    config.add_section('common')
    for key, value in cfg_defaults.items(): config.set('common', key, str(value))
    if config.read(cfg_filename):
        logging.config.fileConfig(cfg_filename)
    else:
        logging.basicConfig(stream=sys.stdout, level=cfg_defaults['logLevel'], format=cfg_defaults['logFormat'])

    # process command line arguments
    parser = OptionParser(version='{0} {1}'.format(__description__, __version__), description=__description__)
    parser.add_option('-b', '--brokerUrl', dest='brokerUrl', metavar='URL', type='string',
                      default=config.get('common', 'brokerUrl'), help='context broker URL [default=%default]'),
    parser.add_option('-l', '--logLevel', dest='logLevel', metavar='LEVEL',
                      choices=[level for level in logging._levelNames.keys() if isinstance(level, str)],
                      default=config.get('common', 'logLevel'), help='logging level [default=%default]')
    (opts, args) = parser.parse_args()  # @UnusedVariable
    config.set('common', 'brokerUrl', opts.brokerUrl)
    config.set('common', 'logLevel', opts.logLevel)
    logging.root.setLevel(opts.logLevel)

    # rpc connection
    connection = rpc.create_connection()
    try:
        logging.info('Context Broker URL: %s', config.get('common', 'brokerUrl'))
        listen(connection, config)
    finally:
        connection.close()
    return 0


if __name__ == '__main__':
    main()
