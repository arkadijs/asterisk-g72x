# choose Asterisk or Callweaver version
# Asterisk 10.0
#inc=/home/arkadi/opt/asterisk-10.0/include
#def=-DG72X_ASTERISK=100

# Asterisk 1.8
inc=/home/arkadi/opt/asterisk-1.8.3/include
def=-DG72X_ASTERISK=18

# Asterisk 1.6
#inc=/home/arkadi/opt/asterisk-1.6.2/include
#def=-DG72X_ASTERISK=16

# Asterisk 1.4
#inc=/home/arkadi/opt/asterisk-1.4/include
#def=-DG72X_ASTERISK=14

# Asterisk TRUNK
#inc=/home/arkadi/opt/asterisk-trunk/include
#def=-DG72X_ASTERISK=17

# Callweaver - not supported yet
#inc=/home/arkadi/opt/callweaver-rc5/include
#incw=-I/home/arkadi/tmp/src/callweaver-1.2.1/include # source dir for confdefs.h
#def=-DG72X_CALLWEAVER

# specify IPP location
ipproot=/opt/intel/ipp/6.1/ia32 # 32-bit
#ipproot=/opt/intel/ipp/6.1/lp32 # 32-bit Atom
#ipproot=/opt/intel/ipp/6.1/em64t # 64-bit

o="-O3 -fomit-frame-pointer" # generic optimization
#o="-O -g"
o="$o -flto -fwhole-program"

# the defaults below are pretty reasonable choice for pentium4 class cpu
# running in 32-bit mode with static link

# choose the compiler
# GNU C Compiler
cc=gcc
#cc=i686-pc-linux-gnu-gcc-4.5.1
#o="$o -flto -fwhole-program"

# Intel C compiler
# to achieve fully static link move libsvml.so (and libirc.so for em64t)
# out of /opt/intel/cc/lib and the linker will use static library
#cc=/opt/intel/cc/bin/icc # 32-bit
#cc=/opt/intel/cce/bin/icc # 64-bit
#o="$o -ipo" # enable multi-file IP optimizations
#icclibs=-lsvml
#export PATH="/opt/crosstool/gcc-3.4.4-glibc-2.3.2/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/bin:$PATH"

# choose 32-bit or 64-bit
# 32-bit
  # gcc
    # core2 penryn with sse4
   #opt=-march=core2
   #ippcore=p8

    # core2
   #opt=-march=core2
   #ippcore=v8

    # pentium4 prescott with sse3 - check for PNI flag in /proc/cpuinfo
   #opt=-march=prescott
   #ippcore=t7

    # pentium4
    opt=-march=pentium4
    ippcore=w7

    # pentium4 but disable compiler generated sse
   #opt="-march=pentium4 -mno-sse -mno-sse2"
   #ippcore=w7

    # pentium-m
   #opt=-march=pentium-m
   #ippcore=w7

    # pentium3
   #opt=-march=pentium3
   #ippcore=a6

    # pentium3 but disable compiler generated sse
   #opt="-march=pentium3 -mno-sse"
   #ippcore=a6

    # pentium2
   #opt=-march=pentium2
   #ippcore=px
   #def2=-DIPPCORE_NO_SSE

    # pentium
   #opt=-march=pentium
   #ippcore=px
   #def2=-DIPPCORE_NO_SSE

    # opteron athlon64
   #opt=-march=k8
   #ippcore=w7

    # opteron athlon64 with sse3
   #opt=-march=k8
   #ippcore=t7

    # athlon with sse
   #opt=-march=athlon-xp
   #ippcore=a6

  # icc
  # cflags from http://www.intel.com/support/performancetools/sb/CS-009787.htm
    # core2 penryn with sse4
   #opt=-xS
   #ippcore=p8

    # core2
   #opt=-xT
   #ippcore=v8

    # pentium4 pentium-m with sse3
   #opt=-xP
   #ippcore=t7

    # pentium4 pentium-m
   #opt=-xN
   #ippcore=w7

    # pentium3
   #opt=-xK
   #ippcore=a6

# 32-bit static link
ippstatic_include="-include $ipproot/tools/staticlib/ipp_$ippcore.h"
ipplibs="-L$ipproot/lib -lippscmerged -lippsrmerged -lippsmerged -lippcore"
#o="$o -static-intel" # if ICC is used

# 32-bit dynamic link
#ipplibs="-L$ipproot/sharedlib -lippsc -lippsr -lipps -lippcore"


# 64-bit
  # gcc
    # x86_64 core2 with sse4.1
   #opt=-march=core2
   #ippcore=y8

    # x86_64 core2
   #opt=-march=core2
   #ippcore=u8

    # x86_64 pentium4
   #opt=-march=nocona
   #ippcore=m7

  # icc
    # x86_64 core2 with sse4.1
   #opt=-xS
   #ippcore=y8

    # x86_64 core2
   #opt=-xT
   #ippcore=u8

    # x86_64 pentium4
   #opt=-xP
   #ippcore=m7

# 64-bit static link for IPP 5.3
#def2=-DIPPCORE_STATIC_INIT
#ipplibs="-L$ipproot/lib -lippscemergedem64t -lippsremergedem64t -lippsemergedem64t -lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"

# 64-bit static link for IPP 6.0+
#ippstatic_include="-include $ipproot/tools/staticlib/ipp_$ippcore.h"
#ipplibs="-L$ipproot/lib -lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"

#o="$o -static-intel" # if ICC is used

# 64-bit dynamic link
#ipplibs="-L$ipproot/sharedlib -lippscem64t -lippsrem64t -lippsem64t -lippscem64t -lippsrem64t -lippsem64t -lippcoreem64t"


# end of configuration


src3="codec_g72x.c ipp/decg723.c ipp/encg723.c ipp/owng723.c ipp/vadg723.c ipp/aux_tbls.c"
src9="codec_g72x.c ipp/decg729fp.c ipp/encg729fp.c ipp/owng729fp.c ipp/vadg729fp.c" # floating-point codec
src9nofp="codec_g72x.c ipp/decg729.c ipp/encg729.c ipp/owng729.c ipp/vadg729.c ipp/aux_tbls.c" # integer codec

compile_cmd="$cc -Wall -shared -Xlinker -x \
    -D_GNU_SOURCE $def $def2 -Iipp -I"$inc" $incw -I"$ipproot"/include $ippstatic_include \
    $opt $o \
    -fPIC"
libs="$ipplibs $icclibs"

cmd="$compile_cmd -DG72X_3 -o codec_g723.so $src3 $libs"; echo $cmd; $cmd
cmd="$compile_cmd -DG72X_9 -o codec_g729.so $src9 $libs"; echo $cmd; $cmd
cmd="$compile_cmd -DG72X_9 -DG72X_9_NOFP -o codec_g729nofp.so $src9nofp $libs"; echo $cmd; $cmd

strip codec_g723.so
strip codec_g729.so
strip codec_g729nofp.so
