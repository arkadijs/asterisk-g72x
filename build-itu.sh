# choose Asterisk or Callweaver version
# Asterisk 1.8
#inc=/home/arkadi/opt/asterisk-1.8.0/include
#def=-DG72X_ASTERISK=18

# Asterisk 1.6
inc=/home/arkadi/opt/asterisk-1.6.2/include
def=-DG72X_ASTERISK=16

# Asterisk 1.4
#inc=/home/arkadi/opt/asterisk-1.4/include
#def=-DG72X_ASTERISK=14

# Asterisk TRUNK
#inc=/home/arkadi/opt/asterisk-trunk/include
#def=-DG72X_ASTERISK=19

cc=gcc
#cc=i686-pc-linux-gnu-gcc-4.5.1
#o="-march=i686 -g -fPIC"
o="-march=barcelona -msse -msse2 -msse3 -O3 -fomit-frame-pointer -fPIC"
#o="$o -flto -fwhole-program"

#cc=/opt/intel/cc/bin/icc
#o="-xP -O3 -fomit-frame-pointer -ipo -static-intel -fPIC"

# end of configuration

src3="codec_g72x.c
itu/g.723.1a/basop.c
itu/g.723.1a/cod_cng.c
itu/g.723.1a/coder.c
itu/g.723.1a/dec_cng.c
itu/g.723.1a/decod.c
itu/g.723.1a/exc_lbc.c
itu/g.723.1a/lpc.c
itu/g.723.1a/lsp.c
itu/g.723.1a/tab_lbc.c
itu/g.723.1a/tame.c
itu/g.723.1a/util_cng.c
itu/g.723.1a/util_lbc.c
itu/g.723.1a/vad.c"

src9="codec_g72x.c
itu/g.729a/acelp_ca.c
itu/g.729a/basic_op.c
itu/g.729a/cod_ld8a.c
itu/g.729a/cor_func.c
itu/g.729a/de_acelp.c
itu/g.729a/dec_gain.c
itu/g.729a/dec_lag3.c
itu/g.729a/dec_ld8a.c
itu/g.729a/dspfunc.c
itu/g.729a/filter.c
itu/g.729a/gainpred.c
itu/g.729a/lpc.c
itu/g.729a/lpcfunc.c
itu/g.729a/lspdec.c
itu/g.729a/lspgetq.c
itu/g.729a/oper_32b.c
itu/g.729a/p_parity.c
itu/g.729a/pitch_a.c
itu/g.729a/post_pro.c
itu/g.729a/postfilt.c
itu/g.729a/pre_proc.c
itu/g.729a/pred_lt3.c
itu/g.729a/qua_gain.c
itu/g.729a/qua_lsp.c
itu/g.729a/tab_ld8a.c
itu/g.729a/taming.c
itu/g.729a/util.c"

compile_cmd="$cc -Wall -shared -Xlinker -x -D_GNU_SOURCE $o -fPIC $def -I$inc"
cmd="$compile_cmd -DG72X_3 -DG72X_ITU -Iitu/g.723.1a -o codec_g723.so $src3"; echo $cmd; $cmd
cmd="$compile_cmd -DG72X_9 -DG72X_ITU -Iitu/g.729a -o codec_g729.so $src9"; echo $cmd; $cmd

strip codec_g723.so
strip codec_g729.so
