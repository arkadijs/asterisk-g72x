#!/bin/bash -x

BUILD_GCC=1
#BUILD_ICC=1
file /bin/ls | grep -q 32-bit && BUILD_32=1
file /bin/ls | grep -q 64-bit && BUILD_64=1

ipproot_prefix53=/opt/intel2/ipp/5.3
ipproot_prefix=/opt/intel2/ipp/6.1

function build()
{
    if [ -n "$ippcore" ]; then
        ippstatic_include="-include $ipproot/tools/staticlib/ipp_$ippcore.h"
        ippstatic_init=""
    else
        ippstatic_include=""
        ippstatic_init=-DIPPCORE_STATIC_INIT
    fi
    if [ -n "$ippcore_no_sse" ]; then
        ippcore_no_sse=-DIPPCORE_NO_SSE
    fi
    if [ "$codec" = "g729" ]; then
       #src="samples/util_e.c samples/util_d.c samples/codec_g729.c api/decg729fp.c api/encg729fp.c api/owng729fp.c api/usc729fp.c"
        src="codec_g72x.c ipp/decg729fp.c ipp/encg729fp.c ipp/owng729fp.c ipp/vadg729fp.c" # floating-point codec
        def="-DG72X_9 -DG72X_9_FP"
    elif [ "$codec" = "g729i" ]; then
        src="codec_g72x.c ipp/decg729.c ipp/encg729.c ipp/owng729.c ipp/vadg729.c ipp/aux_tbls.c" # integer codec
        def="-DG72X_9"
    elif [ "$codec" = "g723" ]; then
       #src="samples/codec_g723.c api/decg723.c api/encg723.c api/owng723.c"
        src="codec_g72x.c ipp/decg723.c ipp/encg723.c ipp/owng723.c ipp/vadg723.c ipp/aux_tbls.c"
        def="-DG72X_3"
    else
        echo "codec is g729, g729i or g723"; exit 1
    fi
    so=bin/codec_$codec-$a-$c-$libc-$cpu.so
    #echo \
    #-D__unix__ -Dlinux -Dlinux32 -DNDEBUG -DLINUX32 -DNO_SCRATCH_MEMORY_USED -Iinclude -Ivm/include \
    "$cc" -shared $ldflags -Xlinker -x -fPIC -o $so \
        -D_GNU_SOURCE $def $def2 -Iipp -I"$ast"/include -I"$ast"/usr/include -I"$ipproot"/include $ippstatic_include \
        $ippcore_no_sse \
        $ippstatic_init \
        $o $opt \
        $src \
        -L"$ipproot"/lib $x86_64libs $ipplibs $icclibs
    unresolved_sym="$(nm $so | grep ' U ' | grep -v @@GLIBC | grep -Ev ' U (_?_?ast_|cw_|STANDARD_USECOUNT|option_verbose|a?cos|log10|pow|sin|sqrt|fabs|calloc|free|malloc|memcpy|memmove|memset|realloc|nanosleep)')"
    if [ -n "$unresolved_sym" ]; then
        echo -e "$so unresolved symbols\n$unresolved_sym"
        exit 1
    fi
    linked_to_intel_libs="$(ldd $so | grep '/intel/')"
    if [ -n "$linked_to_intel_libs" ]; then
        echo -e "$so linked to intel lib\n$linked_to_intel_libs"
        exit 1
    fi
    if [ "$cpu" != "debug" ]; then strip $so; fi
    unset ippcore
    unset ippcore_no_sse
}

function all()
{
pushd "$dir"

if [ -n "$BUILD_GCC" ]; then
# -----------------------
c=gcc4
#o="-O6 -fomit-frame-pointer -flto -fwhole-program"
o="-O6 -fomit-frame-pointer"
cc=gcc
libc=glibc
#cc=i586-pc-linux-uclibc-gcc
#libc=uclibc
ldflags=""
ipproot=$ipproot_prefix/ia32
ipplibs="-lippscmerged -lippsrmerged -lippsmerged -lippcore"

if [ -n "$BUILD_32" ]; then
# -----------------------
cpu=core2-sse4      opt="-march=core2     -mfpmath=sse -msse -msse2 -msse3 -mssse3 -msse4.1" ippcore=p8 build
cpu=core2           opt="-march=core2     -mfpmath=sse -msse -msse2 -msse3 -mssse3"          ippcore=v8 build
cpu=pentium4-sse3   opt="-march=prescott  -mfpmath=sse -msse -msse2 -msse3"                  ippcore=t7 build
cpu=pentium4        opt="-march=pentium4  -mfpmath=sse -msse -msse2"                         ippcore=w7 build
cpu=pentium4-no-sse opt="-march=pentium4  -mno-sse -mno-sse2"                                ippcore=w7 build
cpu=pentium-m       opt="-march=pentium-m -mfpmath=sse -msse -msse2"                         ippcore=w7 build
cpu=pentium2        opt=-march=pentium2    ippcore_no_sse=1                                  ippcore=px build
cpu=pentium         opt=-march=pentium-mmx ippcore_no_sse=1                                  ippcore=px build
cpu=barcelona       opt="-march=barcelona -mfpmath=sse -msse -msse2 -msse3 -msse4a"          ippcore=t7 build
cpu=opteron-sse3    opt="-march=k8-sse3   -mfpmath=sse -msse -msse2 -msse3"                  ippcore=t7 build
cpu=opteron         opt="-march=k8        -mfpmath=sse -msse -msse2"                         ippcore=w7 build
cpu=geode           opt=-march=geode ippcore_no_sse=1                                        ippcore=px build
cpu=debug o=""      opt="-O -g"      ippcore_no_sse=1                                        ippcore=px build
ipproot=$ipproot_prefix/lp32
cpu=atom            opt="-march=core2     -mfpmath=sse -msse3 -mtune=generic"                ippcore=s8 build
# 6.0 removed a6 core
ipproot=$ipproot_prefix53/ia32
cpu=pentium3        opt="-march=pentium3  -mfpmath=sse -msse"                                ippcore=a6 build
cpu=pentium3-no-sse opt="-march=pentium3  -mno-sse"                                          ippcore=a6 build
cpu=athlon-sse      opt="-march=athlon-xp -mfpmath=sse -msse"                                ippcore=a6 build
# -----------------------
fi


if [ -n "$BUILD_64" ]; then
# -----------------------
cc=gcc
ipproot=$ipproot_prefix/em64t
#ipplibs="-lippscemergedem64t -lippsremergedem64t -lippsemergedem64t -lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
ipplibs="-lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
cpu=x86_64-core2-sse4   opt=-march=core2      ippcore=y8 build
cpu=x86_64-core2        opt=-march=core2      ippcore=u8 build
cpu=x86_64-pentium4     opt=-march=nocona     ippcore=m7 build
cpu=x86_64-barcelona    opt=-march=barcelona  ippcore=m7 build
cpu=x86_64-opteron-sse3 opt=-march=k8-sse3    ippcore=m7 build
cpu=x86_64-opteron      opt=-march=k8         ippcore=mx build
# -----------------------
fi

# -----------------------
fi


if [ -n "$BUILD_ICC" ]; then
# -----------------------
#. /opt/intel/cc/bin/iccvars.sh
c=icc
cc=/opt/intel/cc/bin/icc
o="-O3 -fomit-frame-pointer -ipo -idirafter /usr/include/linux"
libc=glibc
ldflags="-static-intel"
ipproot=$ipproot_prefix/ia32
ipplibs="-lippscmerged -lippsrmerged -lippsmerged -lippcore"

if [ -n "$BUILD_32" ]; then
# -----------------------
cpu=pentium2   opt="-march=pentium2 -mtune=pentiumpro" ippcore_no_sse=1 ippcore=px build
cpu=pentium    opt="-mtune=pentium"                    ippcore_no_sse=1 ippcore=px build
cpu=debug o="" opt="-O -g"                             ippcore_no_sse=1 ippcore=px build

icclibs=-lsvml
cpu=core2-sse4    opt=-xS ippcore=p8 build
cpu=core2         opt=-xT ippcore=v8 build
cpu=pentium4-sse3 opt=-xP ippcore=t7 build
cpu=pentium4      opt=-xN ippcore=w7 build
ipproot=$ipproot_prefix53/ia32
cpu=pentium3      opt=-xK ippcore=a6 build
# -----------------------
fi


if [ -n "$BUILD_64" ]; then
# -----------------------
#. /opt/intel/cce/bin/iccvars.sh
cc=/opt/intel/cce/bin/icc
saved_path="$PATH"
#export PATH="/usr/i686-pc-linux-gnu/x86_64-unknown-linux-gnu/binutils-bin/2.21:$PATH"
#export LIBRARY_PATH=/usr/lib/gcc-lib/x86_64-unknown-linux-gnu/4.5.1
export PATH="/opt/crosstool/gcc-3.4.4-glibc-2.3.2/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/bin:$PATH"
ipproot=$ipproot_prefix/em64t
#ipplibs="-lippscemergedem64t -lippsremergedem64t -lippsemergedem64t -lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
ipplibs="-lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
#x86_64libs="-L/usr/lib/gcc/x86_64-unknown-linux-gnu/4.3.2 -L/usr/x86_64-unknown-linux-gnu/lib"
cpu=x86_64-core2-sse4 opt=-xS ippcore=y8 build
cpu=x86_64-core2      opt=-xT ippcore=u8 build
cpu=x86_64-pentium4   opt=-xP ippcore=m7 build
export PATH="$saved_path"
# -----------------------
fi

icclibs=""
# -----------------------
fi

popd
}

srcdir=.
mkdir -p bin/

a=ast170
ast=/opt/asterisk
def2="-DG72X_ASTERISK=170"
codec=g729 dir=$srcdir all &
codec=g723 dir=$srcdir all &

wait
