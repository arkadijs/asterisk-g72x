#!/bin/sh -xe

cp -p ../mkast.sh .
t=asterisk-32bit:20
docker build -t $t .
docker run --rm -v /opt/intel:/opt/intel -v $(pwd)/../..:/opt/asterisk-g72x $t /bin/sh -c 'cd /opt/asterisk-g72x && unsupported/g72x-build.sh'
rm -f mkast.sh
ls -l ../../bin/codec_g72*ast200*.so
