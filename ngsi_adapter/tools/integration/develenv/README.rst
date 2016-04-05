=================================================
 Continuous Integration with Develenv Jenkins CI
=================================================

Here you can find configuration files to setup the jobs needed in the continuous
integration of this component using Develenv Jenkins CI. Files are named after
their corresponding job: ``{jobname}_config.xml``

`Jenkins Remote Access API`_ may be used to create/update jobs programmatically:

.. code::

   # Create a new job via posting a local configuration file
   $ curl "http://user:password@jenkins/createItem?name=jobname" \
     -X POST -H "Content-Type: text/xml" \
     --data-binary "@jobname_config.xml"

   # Update an existing job via posting a local configuration file
   $ curl "http://user:password@jenkins/job/jobname/config.xml" \
     -X POST -H "Content-Type: text/xml" \
     --data-binary "@jobname_config.xml"

Please note some of these jobs require the definition of environment variables
and slave nodes for distributed builds.

.. _Jenkins Remote Access API: https://wiki.jenkins-ci.org/display/JENKINS/Remote+access+API
