#!/bin/sh
gcc -o astconv astconv.c -I/home/arkadi/opt/asterisk-1.8.2/include -D_GNU_SOURCE -ldl -lm -O2 -s -rdynamic -Wall
