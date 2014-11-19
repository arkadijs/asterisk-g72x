#!/bin/sh
exec docker run -ti --rm -v $(pwd)/..:/opt/asterisk-g72x arkadi/asterisk-32bit:13 /bin/sh -c \
    'cp -v /opt/asterisk-g72x/bin/codec_g72?-ast130-gcc4-glibc-barcelona.so /opt/asterisk-13/lib/asterisk/modules/ \
    && /opt/asterisk-13/sbin/asterisk -f & sleep 5 && /opt/asterisk-13/sbin/asterisk -x "core show translation"'
