FROM 32bit/ubuntu:14.04
MAINTAINER Arkadi Shishlov <arkadi.shishlov@gmail.com>
RUN apt-get update \
    && apt-get install -y gcc g++ libncurses5-dev uuid-dev libjansson-dev libxml2-dev libsqlite3-dev wget make \
    && cd /tmp \
    && echo Fetching Asterisk sources...
    && wget -nv -O - http://downloads.asterisk.org/pub/telephony/asterisk/asterisk-13-current.tar.gz | tar xz
ADD mkast.sh /tmp/
RUN cd /tmp/asterisk-* \
    && /tmp/mkast.sh /opt/asterisk-13
VOLUME /opt/intel2
VOLUME /opt/asterisk-g72x
