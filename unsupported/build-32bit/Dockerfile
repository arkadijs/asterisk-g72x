FROM 32bit/ubuntu:14.04
LABEL maintainer="Arkadi Shishlov <arkadi.shishlov@gmail.com>"
RUN apt-get update
RUN apt-get install -y gcc g++ libncurses5-dev uuid-dev libxml2-dev libsqlite3-dev libssl-dev libedit-dev ca-certificates wget make file bzip2 patch
RUN cd /tmp \
    && wget -nv -O - http://downloads.asterisk.org/pub/telephony/asterisk/asterisk-16-current.tar.gz | tar xz
ADD mkast.sh /tmp/
RUN cd /tmp/asterisk-* \
    && /tmp/mkast.sh /opt/asterisk
VOLUME /opt/intel2
VOLUME /opt/asterisk-g72x
