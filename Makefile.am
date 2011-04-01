AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CFLAGS  = @ASTERISK_ASTERISK_H@ -DG72X_ASTERISK=@ASTERISK_VERSION@ @ASTERISK_CFLAGS@ \
             @G72X_ITU@ @IPP_CFLAGS@ @CALLWEAVER_CFLAGS@ \
             -Wall -D_GNU_SOURCE
AM_LDFLAGS = @IPP_LDFLAGS@ @FLTO_LDFLAGS@ -avoid-version -module
ACLOCAL_AMFLAGS = -I m4

moddir = ${libdir}/asterisk/modules

if G729_NOFP
mod_LTLIBRARIES = codec_g723.la codec_g729nofp.la
else
mod_LTLIBRARIES = codec_g723.la codec_g729.la
endif

if ITU_CODE
codec_g723_la_CFLAGS  = ${AM_CFLAGS} -Iitu/g.723.1a -DG72X_3
codec_g723_la_SOURCES = codec_g72x.c \
	itu/g.723.1a/basop.c \
	itu/g.723.1a/cod_cng.c \
	itu/g.723.1a/coder.c \
	itu/g.723.1a/dec_cng.c \
	itu/g.723.1a/decod.c \
	itu/g.723.1a/exc_lbc.c \
	itu/g.723.1a/lpc.c \
	itu/g.723.1a/lsp.c \
	itu/g.723.1a/tab_lbc.c \
	itu/g.723.1a/tame.c \
	itu/g.723.1a/util_cng.c \
	itu/g.723.1a/util_lbc.c \
	itu/g.723.1a/vad.c


codec_g729_la_CFLAGS  = ${AM_CFLAGS} -Iitu/g.729a -DG72X_9
codec_g729_la_SOURCES = codec_g72x.c \
	itu/g.729a/acelp_ca.c \
	itu/g.729a/basic_op.c \
	itu/g.729a/cod_ld8a.c \
	itu/g.729a/cor_func.c \
	itu/g.729a/de_acelp.c \
	itu/g.729a/dec_gain.c \
	itu/g.729a/dec_lag3.c \
	itu/g.729a/dec_ld8a.c \
	itu/g.729a/dspfunc.c \
	itu/g.729a/filter.c \
	itu/g.729a/gainpred.c \
	itu/g.729a/lpc.c \
	itu/g.729a/lpcfunc.c \
	itu/g.729a/lspdec.c \
	itu/g.729a/lspgetq.c \
	itu/g.729a/oper_32b.c \
	itu/g.729a/p_parity.c \
	itu/g.729a/pitch_a.c \
	itu/g.729a/post_pro.c \
	itu/g.729a/postfilt.c \
	itu/g.729a/pre_proc.c \
	itu/g.729a/pred_lt3.c \
	itu/g.729a/qua_gain.c \
	itu/g.729a/qua_lsp.c \
	itu/g.729a/tab_ld8a.c \
	itu/g.729a/taming.c \
	itu/g.729a/util.c

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
