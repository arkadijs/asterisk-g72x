AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CFLAGS  = @ASTERISK_ASTERISK_H@ -DG72X_ASTERISK=@ASTERISK_VERSION@ @ASTERISK_CFLAGS@ \
             @G72X_BCG729@ @IPP_CFLAGS@ @CALLWEAVER_CFLAGS@ \
             -Wall -D_GNU_SOURCE
AM_LDFLAGS = @IPP_LDFLAGS@ @FLTO_LDFLAGS@ -avoid-version -module
ACLOCAL_AMFLAGS = -I m4

moddir = ${libdir}/asterisk/modules

if BCG729
mod_LTLIBRARIES = codec_g729.la
else
if G729_NOFP
mod_LTLIBRARIES = codec_g723.la codec_g729nofp.la
else
mod_LTLIBRARIES = codec_g723.la codec_g729.la
endif
endif

if BCG729
codec_g729_la_CFLAGS  = ${AM_CFLAGS} -DG72X_9
codec_g729_la_SOURCES = codec_g72x.c
codec_g729_la_LIBADD  = -lbcg729

else
codec_g723_la_CFLAGS  = ${AM_CFLAGS} -Iipp -DG72X_3
codec_g723_la_SOURCES = codec_g72x.c ipp/decg723.c ipp/encg723.c ipp/owng723.c ipp/vadg723.c ipp/aux_tbls.c
codec_g723_la_LIBADD  = @IPP_LIBS@

codec_g729_la_CFLAGS  = ${AM_CFLAGS} -Iipp -DG72X_9
codec_g729_la_SOURCES = codec_g72x.c ipp/decg729fp.c ipp/encg729fp.c ipp/owng729fp.c ipp/vadg729fp.c
codec_g729_la_LIBADD  = @IPP_LIBS@

codec_g729nofp_la_SOURCES = codec_g72x.c ipp/decg729.c ipp/encg729.c ipp/owng729.c ipp/vadg729.c ipp/aux_tbls.c
codec_g729nofp_la_CFLAGS  = ${AM_CFLAGS} -Iipp -DG72X_9 @G72X_9_NOFP@
codec_g729nofp_la_LIBADD  = @IPP_LIBS@
endif

install-data-hook:
	rm -f "${DESTDIR}${moddir}"/codec_g72[39]*.la;
