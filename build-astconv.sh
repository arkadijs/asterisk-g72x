#!/bin/sh -xe
gcc -o astconv astconv.c \
  -I${HOME}/asterisk/asterisk-20/include \
  -D_GNU_SOURCE \
  -ldl -lm -O2 -s -rdynamic -Wall
