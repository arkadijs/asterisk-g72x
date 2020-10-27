#!/bin/sh
exec docker run -ti --rm -v $(pwd)/../..:/opt/asterisk-g72x arkadi/asterisk-32bit:18 /bin/sh -c \
    'cp -v /opt/asterisk-g72x/bin/codec_g72?-ast180-gcc4-glibc-barcelona.so /opt/asterisk/lib/asterisk/modules/ \
    && /opt/asterisk/sbin/asterisk -f & sleep 5 && /opt/asterisk/sbin/asterisk -x "core show translation"'
