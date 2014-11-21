FROM ubuntu:lucid
MAINTAINER Arkadi Shishlov <arkadi.shishlov@gmail.com>
RUN apt-get update \
    && apt-get install -y gcc g++ libncurses5-dev uuid-dev libxml2-dev libsqlite3-dev wget make file \
    && cd /tmp \
    && echo Fetching Asterisk sources... \
    && wget -nv -O - http://downloads.asterisk.org/pub/telephony/asterisk/asterisk-13-current.tar.gz | tar xz \
    && wget -nv -O - http://www.digip.org/jansson/releases/jansson-2.7.tar.bz2 | tar xj
RUN cd /tmp/jansson-* \
    && ./configure \
    && make -j4 \
    && make install \
    && ldconfig
ADD mkast.sh /tmp/
RUN cd /tmp/asterisk-* \
    && /tmp/mkast.sh /opt/asterisk-13
VOLUME /opt/intel2
VOLUME /opt/asterisk-g72x
