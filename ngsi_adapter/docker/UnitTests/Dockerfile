# How to execute: 
# $ docker build -t fiware-monitoring-ngsi-adapter .
# $ docker run --name fiware-monitoring-ngsi-adapter fiware-monitoring-ngsi-adapter
#
# To build from a specific branch
# $ docker build --build-arg BRANCH={name} -t fiware-monitoring-ngsi-adapter .

FROM centos:6.6
ARG BRANCH=develop
RUN curl --silent --location https://rpm.nodesource.com/setup_4.x | bash
RUN yum -y install git gcc-c++ make nodejs
RUN git clone https://github.com/telefonicaid/fiware-monitoring /root/fiware-monitoring
WORKDIR /root/fiware-monitoring/ngsi_adapter
RUN git checkout ${BRANCH}; git status
CMD docker/UnitTests/build.sh
