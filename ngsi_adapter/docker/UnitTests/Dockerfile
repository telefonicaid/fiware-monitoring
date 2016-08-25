# How to execute: 
# $ docker build -t fiware-monitoring-ngsi-adapter-build .
# $ docker run --name fiware-monitoring-ngsi-adapter-build fiware-monitoring-ngsi-adapter-build

FROM centos:6.6
RUN curl --silent --location https://rpm.nodesource.com/setup_4.x | bash
RUN yum -y install git gcc-c++ make nodejs
RUN git clone https://github.com/telefonicaid/fiware-monitoring /root/fiware-monitoring
WORKDIR /root/fiware-monitoring/ngsi_adapter
CMD docker/UnitTests/build.sh
