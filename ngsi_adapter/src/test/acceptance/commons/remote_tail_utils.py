# -*- coding: utf-8 -*-

# Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
#
# This file is part of FIWARE project.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# You may obtain a copy of the License at:
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the License for the specific language governing permissions and
# limitations under the License.
#
# For those usages not covered by the Apache version 2.0 License please
# contact with opensource@tid.es

__author__ = 'jfernandez'


from sshtail import SSHTailer, load_dss_key
import time
import threading
from logger_utils import get_logger


logger = get_logger("remote_tail_utils")

# Delay period just after starting remote tailers
TIMER_DELAY_PERIOD = 3

# Grace period when stopping thread. 3 seconds by default
TIMER_GRACE_PERIOD = 3

# Global flag
_tail_terminate_flag = False


class RemoteTail:

    def __init__(self, remote_host_ip, remote_host_user, remote_log_path, remote_log_file_name, local_log_target,
                 private_key):
        """
        Inits RemoteTail class
        :param remote_host_ip: Remote Host IP
        :param remote_host_user: Remote host User name
        :param remote_log_path: Remote log path location
        :param remote_log_file_name: Remote log filename to be tailed
        :param local_log_target: Local path where remote logs will be captured
        :param private_key: Private key to use in the SSH connection.
        If no path's specified for the private key file name, it automatically prepends /home/<current_user>/.ssh/
        and for RSA keys, import load_rsa_key instead.
        :param service_name: Output log file naming (optional)
        :return: None
        """

        self.tailer = None
        self.tail_terminate_flag = False
        self.thread = None
        self.local_capture_file_descriptor = None

        self.remote_host_ip = remote_host_ip
        self.remote_host_user = remote_host_user
        self.remote_log_path = remote_log_path
        self.remote_log_file_name = remote_log_file_name
        self.local_log_target = local_log_target
        self.private_key = private_key

    def init_tailer_connection(self):
        """
        Creates ssh connection to host and init tail on the file.
        :return: None
        """

        private_key_loaded = load_dss_key(self.private_key)
        connection_host = self.remote_host_user + '@' + self.remote_host_ip
        target_log_path = self.remote_log_path + self.remote_log_file_name
        logger.info("Remote Tailer: Connecting to remote host [host: %s, path: %s", connection_host,
                    target_log_path)
        self.tailer = SSHTailer(connection_host, target_log_path, private_key_loaded)

        # Open local output file
        local_capture_path = self.local_log_target + self.remote_log_file_name
        logger.debug("Remote Tailer: Opening local file to save the captured logs")
        self.local_capture_file_descriptor = open(local_capture_path, 'w')

    def start_tailer(self):
        """
        This method starts a new thread for execute a tailing on the remote log file
        :return: None
        """

        logger.debug("Remote Tailer: Launching thread to capture logs")
        self.thread = threading.Thread(target=_read_tailer, args=[self.tailer, self.local_capture_file_descriptor])
        self.thread.start()
        logger.debug("Delay timer before starting: " + str(TIMER_DELAY_PERIOD))
        time.sleep(TIMER_DELAY_PERIOD)

    def stop_tailer(self):
        """
        This method will stop the tailer process after a grace time period
        :return: None
        """

        logger.info("Remote Tailer: Stopping tailers")
        global _tail_terminate_flag
        logger.debug("Grace period after stopping: " + str(TIMER_GRACE_PERIOD))
        time.sleep(TIMER_GRACE_PERIOD)
        _tail_terminate_flag = True


def _read_tailer(tailer, local_capture_file_descriptor):
    """
    Execute a 'tail' on remote log file until tail_terminate_flag will be True
    :param tailer: Created and initialized sshtail connection
    :param local_capture_file_descriptor: Opened descriptor to local file where remote logs will be captured
    :return: None
    """

    global _tail_terminate_flag
    _tail_terminate_flag = False

    try:
        while not _tail_terminate_flag:
            for line in tailer.tail():
                local_capture_file_descriptor.writelines(line + "\n")
                local_capture_file_descriptor.flush()

            # wait a bit
            time.sleep(0.5)

        logger.debug("Remote Tailer: Remote capture finished")
    except:
        logger.error("Remote Tailer: Error when reading remote log lines")

    logger.debug("Remote Tailer: Closing connections and file descriptors")
    tailer.disconnect()
    local_capture_file_descriptor.close()
