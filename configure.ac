# targeting native build, cross-compile is surely broken
AC_INIT(codec_g72x, 1.3, http://groups.google.com/group/asterisk-g729)
saved_cflags="$CFLAGS"
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
AC_PROG_INSTALL
AM_INIT_AUTOMAKE
AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

#echo cflags: "${saved_cflags}"
#echo host_os: "${host_os}"
#echo build_cpu: "${build_cpu}"
#echo prefix: "${prefix}"
#echo default_prefix: "${ac_default_prefix}"
case "${host_os}" in
    freebsd*)
        ac_default_prefix=/usr/local
        ;;
    *)
        ac_default_prefix=/usr
        ;;
esac

# tested on raspberry Pi, may be not valid for other ARM devices
case "${host}" in
  armv6l-*-linux-gnueabihf*)
    cflags="-mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp -O3 -Wall"
    march="armv6zk"
    ;;
esac

if test "${prefix}" = "NONE"; then
    prefix="${ac_default_prefix}"
fi
if test "${prefix}" != "/usr"; then
    CPPFLAGS="$CPPFLAGS -I${prefix}/include"
fi
#echo new prefix: "${prefix}"

ASTERISK_VERSION="140";

AC_ARG_WITH(asterisk-includes,
    AS_HELP_STRING([--with-asterisk-includes=PATH], [Path to Asterisk includes (defaults to PREFIX/include, so try --prefix first)]),
    [ASTERISK_CFLAGS="-I\"$withval\""])
#AC_ARG_WITH(asterisk12,
#    AS_HELP_STRING([--with-asterisk12], [Build for Asterisk 1.2 (not supported yet)]),
#    [ASTERISK_VERSION="12"])
AC_ARG_WITH(asterisk14,
    AS_HELP_STRING([--with-asterisk14], [Build for Asterisk 1.4]),
    [ASTERISK_VERSION="14"])
AC_ARG_WITH(asterisk16,
    AS_HELP_STRING([--with-asterisk16], [Build for Asterisk 1.6]),
    [ASTERISK_VERSION="16"])
AC_ARG_WITH(asterisk18,
    AS_HELP_STRING([--with-asterisk18], [Build for Asterisk 1.8]),
    [ASTERISK_VERSION="18"])
AC_ARG_WITH(asterisk100,
    AS_HELP_STRING([--with-asterisk100], [Build for Asterisk 10, 11, 12]),
    [ASTERISK_VERSION="100"])
AC_ARG_WITH(asterisk130,
    AS_HELP_STRING([--with-asterisk130], [Build for Asterisk 13]),
    [ASTERISK_VERSION="130"])
AC_ARG_WITH(asterisk140,
    AS_HELP_STRING([--with-asterisk140], [Build for Asterisk 14 (the default)]),
    [ASTERISK_VERSION="140"])
#AC_ARG_WITH(callweaver,
#    AS_HELP_STRING([--with-callweaver=PATH], [Build for CallWeaver (not supported yet)]),
#    [CALLWEAVER_CFLAGS="-DG72X_CALLWEAVER -I\"$withval\""])

AC_ARG_WITH(ipp,
    AS_HELP_STRING([--with-ipp=PATH], [Path to Intel IPP (/path/to/ipp)]),
    [ipp_root="$withval"])

AC_ARG_WITH(g729nofp,
    AS_HELP_STRING([--with-g729nofp], [Use integer (no floating point) version of G.729 IPP codec (marginally faster on AMD Geode CPU)]),
    [G72X_9_NOFP="-DG72X_9_NOFP"])

AC_ARG_WITH(bcg729,
    AS_HELP_STRING([--with-bcg729], [Build against Bcg729 library; without G.723.1]),
    [G72X_BCG729="-DG72X_BCG729"; CPPFLAGS="$CPPFLAGS -I/usr/local/include"; LDFLAGS="$LDFLAGS -L/usr/local/lib"])

# http://software.intel.com/en-us/articles/intel-integrated-performance-primitives-intel-ipp-understanding-cpu-optimized-code-used-in-intel-ipp/
# http://software.intel.com/sites/products/documentation/doclib/ipp_sa/71/ipp_userguide_lnx/GUID-BABCDBB5-E674-4ECF-AE3A-4945274FDA80.htm
# IPP cores are:
# 32-bit
# px - pentium mmx
# a6 - pentium3 sse (removed in IPP 6.0)
# w7 - pentium4 sse2
# t7 - pentium4 prescott sse3 (removed in IPP 7.0)
# v8 - core2 ssse3
# p8 - core2 penryn sse4.1, core i7 nehalem sse4.2 & westmere aes-ni mini-dispatcher
# s8 - atom ssse3
# g9 - sandy bridge avx
# h9 - haswell avx2
# 64-bit
# mx - older amd64 w/o sse3?
# m7 - pentium4 prescott sse3 em64t
# u8 - core2 ssse3
# y8 - core2 penryn sse4.1, core i7 nehalem sse4.2 & westmere aes-ni mini-dispatcher
# n8 - atom ssse3
# e9 - sandy bridge avx
# l9 - haswell avx2
# i7 - itanium

#              sse3   ssse3  sse4.1
# k8 w/o sse3
# k8            +
# k10           +
# prescott      +
# core2         +      +
# penryn        +      +      +
# atom          +      +      +

AC_CHECK_HEADERS([asterisk/asterisk.h], [ASTERISK_ASTERISK_H=-DASTERISK_ASTERISK_H])

AX_CHECK_COMPILER_FLAGS(-march=native,    [march=native])
AX_CHECK_COMPILER_FLAGS(-mavx,            [mavx=-mavx],            [mavx=""])
AX_CHECK_COMPILER_FLAGS(-march=core2,     [march_core2=core2],     [march_core2=nocona])
AX_CHECK_COMPILER_FLAGS(-march=atom,      [march_atom=atom],       [march_core2=$march_core2; mtune_generic=-mtune=generic])
AX_CHECK_COMPILER_FLAGS(-march=k8-sse3,   [march_k8_sse3=k8-sse3], [march_k8_sse3=k8])
AX_CHECK_COMPILER_FLAGS(-march=barcelona, [march_k10=barcelona],   [march_k10=$march_k8_sse3])
AX_CHECK_COMPILER_FLAGS(-march=geode,     [march_geode=geode],     [march_geode=k6-3; mtune_generic=-mtune=generic])

if test "$CC" = "icc"; then
    if test -z "$CFLAGS"; then
        AC_MSG_ERROR([For ICC please set CFLAGS])
    fi
    flto=-ipo
else
    AX_CHECK_COMPILER_FLAGS(-flto, [flto="-flto"; fwholeprg="-fwhole-program"])
fi

AC_ARG_ENABLE([barcelona],
    AS_HELP_STRING([--enable-barcelona],
    [Optimize for AMD K10/Opteron/Phenom (enables SSE3)]),
    [march=$march_k10; ipp_core=t7; ipp_core64=m7; cflags="-msse3 -mfpmath=sse"])
AC_ARG_ENABLE([opteron-sse3],
    AS_HELP_STRING([--enable-opteron-sse3],
    [Optimize for AMD K8/Opteron/Athlon64 (enables SSE3)]),
    [march=$march_k8_sse3; ipp_core=t7; ipp_core64=m7; cflags="-msse3 -mfpmath=sse"])
AC_ARG_ENABLE([opteron],
    AS_HELP_STRING([--enable-opteron],
    [Optimize for AMD K8/Opteron/Athlon64 (enables SSE2)]),
    [march=k8; ipp_core=w7; ipp_core64=mx; cflags="-msse2 -mfpmath=sse"])
AC_ARG_ENABLE([athlon],
    AS_HELP_STRING([--enable-athlon], [Optimize for AMD K7/Athlon-XP (enables SSE)]),
    [march=athlon-xp; ipp_core=a6; cflags="-msse -mfpmath=sse"])
AC_ARG_ENABLE([geode],
    AS_HELP_STRING([--enable-geode], [Optimize for AMD Geode LX]),
    [march=$march_geode; ipp_core=px; ipp_cdefs=-DIPPCORE_NO_SSE; cflags="$mtune_generic"])

AC_ARG_ENABLE([avx],
    AS_HELP_STRING([--enable-avx], [Optimize for Intel Sandy Bridge (enables AVX)]),
    [march=$march_core2; ipp_core=g9; ipp_core64=e9; cflags="$mavx"])
AC_ARG_ENABLE([penryn],
    AS_HELP_STRING([--enable-penryn], [Optimize for Intel Core2 Penryn and Core i7 Nehalem/Westmere (enables SSE4.1)]),
    [march=$march_core2; ipp_core=p8; ipp_core64=y8; cflags="-msse3 -mfpmath=sse"])
AC_ARG_ENABLE([core2],
    AS_HELP_STRING([--enable-core2], [Optimize for Intel Core2 (enables SSE3 and SSSE3)]),
    [march=$march_core2; ipp_core=v8; ipp_core64=u8; cflags="-msse3 -mfpmath=sse"])
AC_ARG_ENABLE([atom],
    AS_HELP_STRING([--enable-atom], [Optimize for Intel Atom (enables SSE4.1)]),
    [march=$march_core2; ipp_core=s8; ipp_core64=n8; cflags="-mtune=generic -msse3 -mfpmath=sse"])
AC_ARG_ENABLE([nocona],
    AS_HELP_STRING([--enable-nocona], [Optimize for Intel Pentium4 Prescott and Nocona (enables SSE3)]),
    [march=nocona; ipp_core=t7; ipp_core64=m7; cflags="-msse3 -mfpmath=sse"])
AC_ARG_ENABLE([pentium4],
    AS_HELP_STRING([--enable-pentium4], [Optimize for Intel Pentium4 (enables SSE2)]),
    [march=pentium4; ipp_core=w7; cflags="-msse2 -mfpmath=sse"])
AC_ARG_ENABLE([pentiumm],
    AS_HELP_STRING([--enable-pentiumm], [Optimize for Intel PentiumM (enables SSE2)]),
    [march=pentium-m; ipp_core=w7; cflags="-msse2 -mfpmath=sse"])
AC_ARG_ENABLE([pentium3],
    AS_HELP_STRING([--enable-pentium3], [Optimize for Intel Pentium3 (enables SSE)]),
    [march=pentium3; ipp_core=a6; cflags="-msse -mfpmath=sse"])
AC_ARG_ENABLE([pentium2],
    AS_HELP_STRING([--enable-pentium2], [Optimize for Intel Pentium2]),
    [march=pentium2; ipp_core=px; ipp_cdefs=-DIPPCORE_NO_SSE; cflags=""])
AC_ARG_ENABLE([pentium],
    AS_HELP_STRING([--enable-pentium], [Optimize for Intel Pentium]),
    [march=pentium-mmx; ipp_core=px; ipp_cdefs=-DIPPCORE_NO_SSE; cflags=""])

if test -z "$G72X_BCG729"; then
    if test -z "$ipp_core"; then
        AC_CHECK_FILE([/proc/cpuinfo],
            [cpu_features="`grep -iE ^flags /proc/cpuinfo|cut -f2 -d:|head -1`"],
            [AC_MSG_ERROR([unable to determine CPU features, use --enable-pentium/etc. to choose the core])])
        for f in $cpu_features; do
            eval "cpu_feature_$f=1"
        done
        AC_MSG_CHECKING([host CPU features])
        if grep -iE '^model name.* Atom' /proc/cpuinfo >/dev/null; then
            ipp_core=s8; ipp_core64=n8;
        elif test -n "$cpu_feature_avx"; then
            ipp_core=g9; ipp_core64=e9;
        elif test -n "$cpu_feature_sse4_1"; then
            ipp_core=p8; ipp_core64=y8;
        elif test -n "$cpu_feature_ssse3"; then
            ipp_core=v8; ipp_core64=u8;
        elif test -n "$cpu_feature_pni"; then
            ipp_core=t7; ipp_core64=m7;
        elif test -n "$cpu_feature_sse2"; then
            ipp_core=w7; ipp_core64=mx;
        elif test -n "$cpu_feature_sse"; then
            ipp_core=a6;
        elif test -n "$cpu_feature_mmx"; then
            ipp_core=px;
        else
            AC_MSG_ERROR([unable to choose IPP core, use --enable-pentium/etc. to select one])
        fi

        if test "$ipp_core" = "px"; then
            ipp_cdefs=-DIPPCORE_NO_SSE
        fi
        AC_MSG_RESULT([$cpu_features])
    fi

    if test -z "$saved_cflags" -a -z "$march"; then
        AC_MSG_ERROR([compiler does not support -march=native, use --enable-pentium/etc. to choose the core, or set CFLAGS])
    fi

    case "$build_cpu" in
        i?86)
            ipp_arch=ia32;
            ;;
        x86_64|amd64)
            ipp_arch=em64t;
            ;;
    esac

    if test -z "$ipp_root"; then
        AC_MSG_CHECKING([location of IPP libraries])
        with_ipp_to_override=" (use --with-ipp=/path/to/ipp to override)"

        ipp_dir="/opt/intel/ipp"
        if test -d "$ipp_dir/"; then
            ipp_root="$ipp_dir"
        fi

        if test -z "$ipp_root"; then
            # IPP 7.0, 8.0
            ipp_dir="/opt/intel/composerxe/ipp"
            if test -d "$ipp_dir/"; then
                ipp_root="$ipp_dir"
            fi
        fi

        if test -z "$ipp_root"; then
            # 5.3-6.1
            case $ipp_core in
                # Pentium-Pentium3 prefer IPP 5.3
                px|a6)
                    ipp_versions="5.3 6.1 6.0"
                    ;;
                *)
                    ipp_versions="6.1 6.0 5.3"
                    ;;
            esac
            for ipp_ver in $ipp_versions; do
                ipp_dir="`echo /opt/intel/ipp/${ipp_ver}*/$ipp_arch`"
                if test -d "$ipp_dir"; then
                    ipp_root="$ipp_dir"
                    break
                fi
            done
        fi

        AC_MSG_RESULT([$ipp_root])

        if test -z "$ipp_root"; then
            AC_MSG_ERROR([IPP not found - use --with-ipp=/path/to/ipp to specify])
        fi
    fi

    ipp_version_h="$ipp_root"/include/ippversion.h
    if test ! -f "$ipp_version_h"; then
        AC_MSG_ERROR([IPP $ipp_version_h not found])
    fi

    AC_MSG_CHECKING([version of IPP libraries])
    ipp_major=$(grep IPP_VERSION_MAJOR "$ipp_version_h" | awk '{print $3}')
    ipp_minor=$(grep IPP_VERSION_MINOR "$ipp_version_h" | awk '{print $3}')
    ipp_ver=$ipp_major.$ipp_minor
    AC_MSG_RESULT([$ipp_ver])

    if test $ipp_major -ge 7 -a "$ipp_arch" = "em64t"; then
        ipp_arch=intel64
    fi

    case $ipp_core in
        # Pentium3 a6 removed in IPP 6.0
        a6)
            if test "$ipp_ver" != "5.3"; then
                if test "$ipp_ver" != "6.0" -a "$ipp_ver" != "6.1"; then
                    AC_MSG_ERROR([Use IPP 5.3 (preferred) or 6.1 for Pentium3])
                fi
                ipp_core=px
                use_ipp53=" (slower MMX core used, use IPP 5.3 for SSE instead)"
            fi
            ;;
        s8)
            if test "$ipp_ver" = "5.3"; then
                ipp_core=v8; ipp_core64=u8
                use_ipp60=" (Core2 scheduling used, IPP $ipp_ver does not support Atom, use IPP 6.0 or later instead)"
            fi
            ;;
        # 32-bit SSE3 removed in 7.0
        t7)
            if test $ipp_major -ge 7; then
                ipp_core=w7;
                case "$build_cpu" in
                    i?86)
                        use_ipp60=" (SSE2 core, use IPP 6.1 for SSE3)"
                        ;;
                esac;
            fi
            ;;
        # check missing staticlib/ include
        g9)
            if test $ipp_major -lt 7; then
                AC_MSG_ERROR([Use IPP 7.0+ for Sandy Bridge AVX])
            fi
            case "$build_cpu" in
                i?86)
                    staticlib_inc="$ipp_root/tools/$ipp_arch/staticlib/ipp_$ipp_core.h"
                    ;;
                x86_64|amd64)
                    staticlib_inc="$ipp_root/tools/$ipp_arch/staticlib/ipp_$ipp_core64.h"
                    ;;
            esac;
            if ! test -f "$staticlib_inc"; then
                AC_MSG_ERROR([Create "$staticlib_inc" manually as described in http://software.intel.com/en-us/articles/How-to-call-AVX-based-code-in-IPP-v-6-1/])
            fi
            ;;
    esac

    IPP_CFLAGS="-I\"$ipp_root/include\" $ipp_cdefs";
    if test $ipp_major -ge 9; then
        # need IPP Legacy to build speech codecs
        AC_MSG_CHECKING([for legacy IPP 9.0 libraries])
        ipp_legacy_h="$ipp_root"/legacy/include/ipps90legacy.h
        if test ! -f "$ipp_legacy_h"; then
            AC_MSG_ERROR([IPP $ipp_legacy_h not found])
        fi
        ipp_legacy_libsvml="$ipp_root"/legacy/lib/$ipp_arch/libsvml.a
        if test ! -f "$ipp_legacy_libsvml"; then
            AC_MSG_ERROR([IPP $ipp_legacy_libsvml not found])
        fi
        AC_MSG_RESULT([$ipp_legacy_h $ipp_legacy_libsvml])
        IPP_CFLAGS="$IPP_CFLAGS -DG72X_IPP_LEGACY -I\"$ipp_root/legacy/include\" -Wno-deprecated-declarations";
        IPP_LIBS="$ipp_root/legacy/lib/$ipp_arch/libippsc90lgc.a $ipp_root/legacy/lib/$ipp_arch/libipps90lgc.a $ipp_root/lib/$ipp_arch/libipps.a $ipp_root/lib/$ipp_arch/libippvm.a $ipp_root/lib/$ipp_arch/libippcore.a $ipp_root/legacy/lib/$ipp_arch/libirc.a $ipp_root/legacy/lib/$ipp_arch/libsvml.a"
    elif test $ipp_major -eq 8; then
        IPP_LIBS="$ipp_root/lib/$ipp_arch/libippsc.a $ipp_root/lib/$ipp_arch/libipps.a $ipp_root/lib/$ipp_arch/libippvm.a $ipp_root/lib/$ipp_arch/libippcore.a $ipp_root/../compiler/lib/$ipp_arch/libirc.a $ipp_root/../compiler/lib/$ipp_arch/libsvml.a"
    elif test $ipp_major -eq 7; then
        IPP_LIBS="-lippsc_l -lipps_l -lippcore_l"
        IPP_LDFLAGS="-L\"$ipp_root/lib/$ipp_arch\"";
    else
        IPP_LDFLAGS="-L\"$ipp_root/lib\"";
    fi

    case "$build_cpu" in
        i?86)
            if test $ipp_major -ge 7; then
                IPP_CFLAGS="$IPP_CFLAGS -include \"$ipp_root/tools/$ipp_arch/staticlib/ipp_$ipp_core.h\""
            else
                IPP_CFLAGS="$IPP_CFLAGS -include \"$ipp_root/tools/staticlib/ipp_$ipp_core.h\""
                IPP_LIBS="-lippscmerged -lippsrmerged -lippsmerged -lippcore"
            fi
            ;;
        x86_64|amd64)
            # IPP 5.3 does not have tools/staticlib helpers
            if test "$ipp_ver" = "5.3"; then
                IPP_CFLAGS="$IPP_CFLAGS -DIPPCORE_STATIC_INIT"
                IPP_LIBS="-lippscemergedem64t -lippsremergedem64t -lippsemergedem64t -lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
            elif test $ipp_major -ge 7; then
                IPP_CFLAGS="$IPP_CFLAGS -include \"$ipp_root/tools/$ipp_arch/staticlib/ipp_$ipp_core64.h\""
            else # 6.x
                IPP_CFLAGS="$IPP_CFLAGS -include \"$ipp_root/tools/staticlib/ipp_$ipp_core64.h\""
                IPP_LIBS="-lippscmergedem64t -lippsrmergedem64t -lippsmergedem64t -lippcoreem64t"
            fi
            ipp_core=$ipp_core64 # for output
            ;;
    esac

else # G72X_BCG729 is defined
    AC_CHECK_LIB([bcg729], [initBcg729EncoderChannel], [], [AC_MSG_ERROR([unable to find Bcg729 library])])
fi

if test -z "$saved_cflags"; then
    CFLAGS="-O3 -fomit-frame-pointer -march=$march $flto $cflags"
    FLTO_LDFLAGS="$fwholeprg"
fi

AM_CONDITIONAL([BCG729], [test -n "$G72X_BCG729"])
AM_CONDITIONAL([G729_NOFP], [test -n "$G72X_9_NOFP" -a -z "$G72X_BCG729"])

AC_SUBST([ASTERISK_ASTERISK_H])
AC_SUBST([ASTERISK_CFLAGS])
AC_SUBST([ASTERISK_VERSION])
AC_SUBST([CALLWEAVER_CFLAGS])
AC_SUBST([IPP_CFLAGS])
AC_SUBST([IPP_LDFLAGS])
AC_SUBST([IPP_LIBS])
AC_SUBST([FLTO_LDFLAGS])
AC_SUBST([G72X_BCG729])
AC_SUBST([G72X_9_NOFP])

AC_OUTPUT([Makefile])

echo
echo "Architecture: $build_cpu"
echo "  CPU -march: $march"
echo "      CFLAGS: $CFLAGS $IPP_CFLAGS $ASTERISK_CFLAGS"
echo "     LDFLAGS: $FLTO_LDFLAGS" $IPP_LDFLAGS $IPP_LIBS
echo " Codecs impl:" $(test -z "$G72X_BCG729" && echo IPP || echo Bcg729) $(test -z "$G729_NOFP_TRUE" && echo "/ G.729 integer codec")
if test -z "$G72X_BCG729"; then
echo "    IPP path: $ipp_root$with_ipp_to_override"
echo "    IPP ver.: $ipp_ver"
echo "    IPP core: $ipp_core$use_ipp53$use_ipp60"
fi
