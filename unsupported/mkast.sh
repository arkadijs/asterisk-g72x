#!/bin/sh -xe

b="$1"
shift
./configure --prefix="$b" --localstatedir="$b/var" --sysconfdir="$b/etc" \
    --without-h323 --without-pwlib --disable-xmldoc --without-sdl "$@" \
  && nice make -j4 \
  && make install \
  && make samples

#--without-curl
#--host=i586-pc-linux-uclibc
