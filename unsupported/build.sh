#!/bin/sh -xe

t=arkadi/asterisk-32bit:13
docker build -t $t .
docker run --rm -v /opt/intel2:/opt/intel2 -v $(pwd)/..:/opt/asterisk-g72x $t /bin/sh -c 'cd /opt/asterisk-g72x && unsupported/g72x-build.sh'
