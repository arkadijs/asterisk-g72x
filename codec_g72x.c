#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* single file implements both G.729 and G.723.1, both IPP and Bcg729 based codecs,
   Asterisk 1.4 to 12 */
/* quite a lot of preprocessor abuse, but still better than maintaining multiple
   similar files */

#if G72X_ASTERISK

    #ifdef ASTERISK_ASTERISK_H
        /* Ubuntu */
        #include <asterisk/asterisk.h>
    #else
        #include <asterisk.h>
    #endif
    #include <asterisk/lock.h>
    #include <asterisk/translate.h>
    #include <asterisk/module.h>
    #include <asterisk/logger.h>
    #include <asterisk/channel.h>
    #include <asterisk/utils.h>
    #include <asterisk/options.h>
    #include <asterisk/cli.h>

    #if G72X_ASTERISK < 14
        #error not supported
    #endif

#elif G72X_CALLWEAVER
    #error not supported yet

    #include "confdefs.h"
    #include <callweaver/lock.h>
    #include <callweaver/translate.h>
    #include <callweaver/module.h>
    #include <callweaver/logger.h>
    #include <callweaver/channel.h>
    #include <callweaver/utils.h>
    #include <callweaver/options.h>
    #include <callweaver/cli.h>

    #define ast_frame          opbx_frame
    #define ast_log            opbx_log
    #define ast_translator     opbx_translator
    #define ast_mutex_lock     opbx_mutex_lock
    #define ast_verbose        opbx_verbose
    #define ast_trans_frameout        opbx_trans_frameout
    #define ast_register_translator   opbx_register_translator
    #define ast_unregister_translator opbx_unregister_translator
    #define AST_FRAME_VOICE     OPBX_FRAME_VOICE
    #define AST_FORMAT_SLINEAR  OPBX_FORMAT_SLINEAR
    #define AST_FORMAT_G729A    OPBX_FORMAT_G729A
    #define AST_FORMAT_G723_1   OPBX_FORMAT_G723_1
    #define AST_FRIENDLY_OFFSET OPBX_FRIENDLY_OFFSET

#else
    #error either G72X_ASTERISK or G72X_CALLWEAVER must be defined
#endif

#if !G72X_BCG729
    #include <ippcore.h>
    #include <ipps.h>
#endif

#if G72X_9
    #define G72X_CODEC "g729"

    #if G72X_BCG729
        #include "bcg729/decoder.h"
        #include "bcg729/encoder.h"

    #else
        #if !G72X_9_NOFP
            #include "g729fpapi.h"

            #define apiG729Encoder_InitBuff apiG729FPEncoder_InitBuff
            #define apiG729Decoder_InitBuff apiG729FPDecoder_InitBuff
            #define apiG729Encoder_Init     apiG729FPEncoder_Init
            #define apiG729Decoder_Init     apiG729FPDecoder_Init
            #define apiG729Encode           apiG729FPEncode
            #define apiG729Decode           apiG729FPDecode
            #define apiG729Encoder_Alloc    apiG729FPEncoder_Alloc
            #define apiG729Decoder_Alloc    apiG729FPDecoder_Alloc
            #define apiG729Codec_ScratchMemoryAlloc apiG729FPCodec_ScratchMemoryAlloc

        #else
            #include "g729api.h"
        #endif
    #endif

    #include "slin_g72x_ex.h"
    #include "g729_slin_ex.h"

    #define SLIN_FRAME_LEN  160
    #define G729_FRAME_LEN  10
    #define G729_SAMPLES    80 /* 10ms at 8000 hz, 160 bytes signed linear */
    #define BUFFER_SAMPLES  8000

    #define G72X_FRAME_LEN  G729_FRAME_LEN
    #define G72X_SAMPLES    G729_SAMPLES
    #define G72X_AST_FORMAT AST_FORMAT_G729A

#elif G72X_3
    #define G72X_CODEC "g723"

    #if G72X_BCG729
        #error no portable code for G.723.1, ITU-T ref impl is too slow
    #else
        #include "g723api.h"
    #endif

    #include "slin_g72x_ex.h"
    #include "g723_slin_ex.h"

    #define SLIN_FRAME_LEN  480
    #define G723_FRAME_LEN  24 /* maximum frame length */
    #define G723_SAMPLES    240 /* 30ms at 8000 hz, 480 bytes signed linear */
    #define BUFFER_SAMPLES  8000

    #define G72X_FRAME_LEN  G723_FRAME_LEN
    #define G72X_SAMPLES    G723_SAMPLES
    #define G72X_AST_FORMAT AST_FORMAT_G723_1

    #define G723_RATE_63 0 /* G723_Rate63 in owng723.h */
    #define G723_RATE_53 1 /* G723_Rate53 */
    /* XXX also see "enum Crate WrkRate" below */
    #define G723_DEFAULT_SEND_RATE G723_RATE_63

    static int g723_sendrate = G723_DEFAULT_SEND_RATE;

#else
    #error either G72X_9 or G72X_3 must be defined
#endif

#if G72X_ASTERISK > 16
    #define FRAME_SUBCLASS subclass.codec
#else
    #define FRAME_SUBCLASS subclass
#endif

#if G72X_ASTERISK >= 16
    #define FRAME_DATA data.ptr
    #define OUTBUF_SLIN outbuf.i16
    #define OUTBUF_G72X outbuf.uc
#else
    #define FRAME_DATA data
    #define OUTBUF_SLIN outbuf
    #define OUTBUF_G72X outbuf
#endif

#define AST_MODULE "codec_" G72X_CODEC
#if !G72X_BCG729
    #define G72X_DESC G72X_CODEC " Coder/Decoder, based on Intel IPP"
#else
    #define G72X_DESC G72X_CODEC " Coder/Decoder, based on Bcg729"
#endif

struct g72x_coder_pvt {
    void *coder;
#if !G72X_BCG729
    void *scratch_mem;
#endif
    int16_t buf[BUFFER_SAMPLES]; /* 1 second */
};

#if !G72X_BCG729
    static int encoder_size;
    static int decoder_size;
    static int coder_size_scratch;
#endif

/* debug array to collect information about incoming frame sizes */
/* the code is not aiming at correctness so there are no locking and no atomic operations */
static int *frame_sizes = NULL;
#define DEBUG_MAX_FRAME_SIZE 2000
#define DEBUG_FRAME_SIZE_INC \
    do { \
    if (frame_sizes != NULL) { \
        if (f->datalen >= DEBUG_MAX_FRAME_SIZE) \
            ++frame_sizes[DEBUG_MAX_FRAME_SIZE]; \
        else \
            ++frame_sizes[f->datalen]; \
        } \
    } while (0)

static int lintog72x_new(struct ast_trans_pvt *pvt)
{
    struct g72x_coder_pvt *state = pvt->pvt;

#if !G72X_BCG729
    #ifndef IPPCORE_NO_SSE
        ippSetFlushToZero(1, NULL); /* is FZM flag per-thread or not? does it matter at all? */
    #endif
    state->coder = ippsMalloc_8u(encoder_size);
    state->scratch_mem = ippsMalloc_8u(coder_size_scratch);
    #if G72X_9
        apiG729Encoder_InitBuff(state->coder, state->scratch_mem);
        apiG729Encoder_Init(state->coder, G729A_CODEC, G729Encode_VAD_Disabled);
    #else
        apiG723Encoder_InitBuff(state->coder, state->scratch_mem);
        apiG723Encoder_Init(state->coder, G723Encode_DefaultMode);
    #endif
#else
    state->coder = initBcg729EncoderChannel();
#endif
    return 0;
}

static int g72xtolin_new(struct ast_trans_pvt *pvt)
{
    struct g72x_coder_pvt *state = pvt->pvt;

#if !G72X_BCG729
    #ifndef IPPCORE_NO_SSE
        ippSetFlushToZero(1, NULL);
    #endif
    state->coder = ippsMalloc_8u(decoder_size);
    state->scratch_mem = ippsMalloc_8u(coder_size_scratch);
    #if G72X_9
        apiG729Decoder_InitBuff(state->coder, state->scratch_mem);
        apiG729Decoder_Init(state->coder, G729A_CODEC);
    #else
        apiG723Decoder_InitBuff(state->coder, state->scratch_mem);
        apiG723Decoder_Init(state->coder, G723Decode_DefaultMode);
    #endif
#else
    state->coder = initBcg729DecoderChannel();
#endif
    return 0;
}

static struct ast_frame *lintog72x_sample(void)
{
    static struct ast_frame f;
    f.frametype = AST_FRAME_VOICE;
#if G72X_ASTERISK < 100
    f.FRAME_SUBCLASS = AST_FORMAT_SLINEAR;
#else
    ast_format_set(&f.subclass.format, AST_FORMAT_SLINEAR, 0);
#endif
    f.datalen = sizeof(slin_g72x_ex);
    f.samples = sizeof(slin_g72x_ex)/2;
    f.mallocd = 0;
    f.offset = 0;
    f.src = __PRETTY_FUNCTION__;
    f.FRAME_DATA = slin_g72x_ex;
    return &f;
}

static struct ast_frame *g72xtolin_sample(void)
{
    static struct ast_frame f;
    f.frametype = AST_FRAME_VOICE;
#if G72X_ASTERISK < 100
    f.FRAME_SUBCLASS = G72X_AST_FORMAT;
#else
    ast_format_set(&f.subclass.format, G72X_AST_FORMAT, 0);
#endif
    f.datalen = sizeof(g72x_slin_ex);
    f.samples = G72X_SAMPLES;
    f.mallocd = 0;
    f.offset = 0;
    f.src = __PRETTY_FUNCTION__;
    f.FRAME_DATA = g72x_slin_ex;
    return &f;
}

static unsigned char lost_frame[G72X_FRAME_LEN] = { 0 };

#if G72X_9
    #if !G72X_BCG729
        static int g729_frame_type(int datalen)
        {
            switch (datalen) {
                case 0: return -1;  /* erased */
            /* case 0: return 0; maybe it should be 0 - untransmitted silence? */
                case 2: return 1;  /* SID */
                case 8: return 2;  /* 729d */
                case 10: return 3; /* 729, 729a */
                case 15: return 4; /* 729e */
            }
            return 0;
        }
    #endif

static int g72xtolin_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
    struct g72x_coder_pvt *state = pvt->pvt;
    int16_t *dst = (int16_t *)pvt->OUTBUF_SLIN;
    int framesize;
    int x;

    DEBUG_FRAME_SIZE_INC;

    if (f->datalen == 0) {  /* Native PLC interpolation */
        if (option_verbose > 2)
            ast_verbose(VERBOSE_PREFIX_3 "G.729 PLC\n");
        if (pvt->samples + G729_SAMPLES > BUFFER_SAMPLES) {
            ast_log(LOG_WARNING, "Out of buffer space\n");
            return -1;
        }
#if !G72X_BCG729
        apiG729Decode(state->coder, (unsigned char *)lost_frame, g729_frame_type(0), dst + pvt->samples);
#else
        bcg729Decoder(state->coder, (unsigned char *)lost_frame, 1, dst + pvt->samples);
#endif
        pvt->samples += G729_SAMPLES;
        pvt->datalen += 2 * G729_SAMPLES; /* 2 bytes/sample */
        return 0;
    }

    for(x = 0; x < f->datalen; x += framesize) {
        if (pvt->samples + G729_SAMPLES > BUFFER_SAMPLES) { /* XXX how the hell this BUFFER_SAMPLES check is supposed to catch memory overruns? use buf_size */
            ast_log(LOG_WARNING, "Out of buffer space\n");
            return -1;
        }
        if(f->datalen - x < 8)
            framesize = 2;  /* SID */
        else
            framesize = 10; /* regular 729a frame */
#if !G72X_BCG729
        apiG729Decode(state->coder, (unsigned char *)f->FRAME_DATA + x, g729_frame_type(framesize), dst + pvt->samples);
#elif G72X_9
        bcg729Decoder(state->coder, (unsigned char *)f->FRAME_DATA + x, 0, dst + pvt->samples);
#endif
        pvt->samples += G729_SAMPLES;
        pvt->datalen += 2*G729_SAMPLES;
    }
    return 0;
}

#else /* G72X_3 */
static int g723_frame_length(int frametype)
{
    switch(frametype) {
        case 0: return 24; /* 6.3kbps */
        case 1: return 20; /* 5.3kbps */
        case 2: return 4;  /* SID */
    }
    return 1; /* XXX untransmitted */
}

static int g72xtolin_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
    struct g72x_coder_pvt *state = pvt->pvt;
    int16_t *dst = (int16_t *)pvt->OUTBUF_SLIN;
    int badframe;
    int frametype;
    int framesize;
    int x;

    DEBUG_FRAME_SIZE_INC;

    if (f->datalen == 0) {  /* Native PLC interpolation */
        if (option_verbose > 2)
            ast_verbose(VERBOSE_PREFIX_3 "G.723.1 PLC\n");
        if (pvt->samples + G723_SAMPLES > BUFFER_SAMPLES) {
            ast_log(LOG_WARNING, "Out of buffer space\n");
            return -1;
        }
        badframe = 1; /* the frame is lost */
        apiG723Decode(state->coder, (void *)lost_frame, badframe, dst + pvt->samples);
        pvt->samples += G723_SAMPLES;
        pvt->datalen += 2 * G723_SAMPLES; /* 2 bytes/sample */
        return 0;
    }

    badframe = 0;
    for(x = 0; x < f->datalen; x += framesize) {
        if (pvt->samples + G723_SAMPLES > BUFFER_SAMPLES) {
            ast_log(LOG_WARNING, "Out of buffer space\n");
            return -1;
        }
        frametype = *((unsigned char *)f->FRAME_DATA + x) & (short)0x0003;
        framesize = g723_frame_length(frametype);
        apiG723Decode(state->coder, (void *)f->FRAME_DATA + x, badframe, dst + pvt->samples);
        pvt->samples += G723_SAMPLES;
        pvt->datalen += 2*G723_SAMPLES;
    }
    return 0;
}

#endif

static int lintog72x_framein(struct ast_trans_pvt *pvt, struct ast_frame *f)
{
    struct g72x_coder_pvt *state = pvt->pvt;

    memcpy(state->buf + pvt->samples, f->FRAME_DATA, f->datalen);
    pvt->samples += f->samples;
    return 0;
}

#if G72X_9 && !G72X_BCG729
    /* length != 10 can't happen but let it be here for reference */
    static int g729_frame_length(int frametype)
    {
        switch (frametype) {
            case 0: return 0;  /* not transmited  */
            case 1: return 2;  /* SID */
            case 2: return 8;  /* 729d */
            case 3: return G729_FRAME_LEN; /* 729, 729a */
            case 4: return 15; /* 729e */
        }
        return 0;
    }
#endif

static struct ast_frame *lintog72x_frameout(struct ast_trans_pvt *pvt)
{
    struct g72x_coder_pvt *state = pvt->pvt;
    int datalen = 0;
    int samples = 0;
#if G72X_9 && !G72X_BCG729
    int frametype;
#endif

    /* We can't work on anything less than a frame in size */
    if (pvt->samples < G72X_SAMPLES)
        return NULL;
    while (pvt->samples >= G72X_SAMPLES) {
#if !G72X_BCG729
    #if G72X_9
        apiG729Encode(state->coder, state->buf + samples, (unsigned char *)(pvt->OUTBUF_G72X) + datalen, G729A_CODEC, &frametype);
        datalen += g729_frame_length(frametype);
        /* if (frametype == 1) break; if encoding with VAD enabled then terminate the frame */
    #else
        apiG723Encode(state->coder, state->buf + samples, g723_sendrate, (void *)(pvt->OUTBUF_G72X + datalen));
        datalen += (g723_sendrate == G723_RATE_63) ? 24 : 20;
    #endif
#else
        bcg729Encoder(state->coder, state->buf + samples, (unsigned char *)(pvt->OUTBUF_G72X) + datalen);
        datalen += G729_FRAME_LEN;
#endif
        samples += G72X_SAMPLES;
        pvt->samples -= G72X_SAMPLES;
    }

    /* Move the data at the end of the buffer to the front */
    if (pvt->samples)
        memmove(state->buf, state->buf + samples, pvt->samples * 2);

    return ast_trans_frameout(pvt, datalen, samples);
}

static void g72x_destroy(struct ast_trans_pvt *pvt)
{
    int i;
    struct g72x_coder_pvt *state = pvt->pvt;
#if !G72X_BCG729
    ippsFree(state->coder);
    ippsFree(state->scratch_mem);
#else
    free(state->coder);
#endif
    /* output the sizes of frames passed to decoder */
    if (option_verbose > 2 && frame_sizes != NULL) {
        ast_verbose(VERBOSE_PREFIX_3 G72X_CODEC " frames\n");
        ast_verbose(VERBOSE_PREFIX_3 "length: count\n");
        for (i = 0; i <= DEBUG_MAX_FRAME_SIZE; ++i) {
            if (frame_sizes[i] > 0)
                ast_verbose(VERBOSE_PREFIX_3 "%6d: %d\n", i, frame_sizes[i]);
        }
    }
}

static struct ast_translator g72xtolin = {
    .name = G72X_CODEC "tolin",
#if G72X_CALLWEAVER
    .src_format = G72X_AST_FORMAT,
    .dst_format = AST_FORMAT_SLINEAR,
#elif G72X_ASTERISK < 100
    .srcfmt = G72X_AST_FORMAT,
    .dstfmt = AST_FORMAT_SLINEAR,
#endif
    .newpvt = g72xtolin_new,
    .framein = g72xtolin_framein,
    .destroy = g72x_destroy,
    .sample = g72xtolin_sample,
#if G72X_CALLWEAVER
    .src_rate = 8000,
    .dst_rate = 8000
#elif G72X_ASTERISK >= 14
    .desc_size = sizeof(struct g72x_coder_pvt) - BUFFER_SAMPLES*2, /* buffer is not needed for g723/9 -> slin */
    .buf_size = SLIN_FRAME_LEN*100, /* 1 second */
    .native_plc = 1
#endif
};

static struct ast_translator lintog72x = {
    .name = "linto" G72X_CODEC,
#if G72X_CALLWEAVER
    .src_format = AST_FORMAT_SLINEAR,
    .dst_format = G72X_AST_FORMAT,
#elif G72X_ASTERISK < 100
    .srcfmt = AST_FORMAT_SLINEAR,
    .dstfmt = G72X_AST_FORMAT,
#endif
    .newpvt = lintog72x_new,
    .framein = lintog72x_framein,
    .frameout = lintog72x_frameout,
    .destroy = g72x_destroy,
    .sample = lintog72x_sample,
#if G72X_CALLWEAVER
    .src_rate = 8000,
    .dst_rate = 8000
#elif G72X_ASTERISK >= 14
    .desc_size = sizeof(struct g72x_coder_pvt), /* buffer up-to 1 second of speech */
    #if G72X_9
        .buf_size = G729_FRAME_LEN*100 /* 1 sec of g729 */
    #else
        .buf_size = G723_FRAME_LEN*33 /* almost 1 sec of g723 at 6.3kbps */
    #endif
#endif
};

#if G72X_3
    static void parse_config(void)
    {
    #if G72X_ASTERISK >= 15
        /* XXX struct ast_flags config_flags = { reload ? CONFIG_FLAG_FILEUNCHANGED : 0 }; */
        struct ast_flags config_flags = { 0 };
        struct ast_config *cfg = ast_config_load("codecs.conf", config_flags);
    #else
        struct ast_config *cfg = ast_config_load("codecs.conf");
    #endif
        struct ast_variable *var;
        int rate;

        if (cfg == NULL)
            return;
        for (var = ast_variable_browse(cfg, "g723"); var; var = var->next) {
            if (!strcasecmp(var->name, "sendrate")) {
                rate = atoi(var->value);
                if (rate == 53 || rate == 63) {
                    if (option_verbose > 2)
                        ast_verbose(VERBOSE_PREFIX_3 "G.723.1 setting sendrate to %d\n", rate);
                    g723_sendrate = (rate == 63) ? G723_RATE_63 : G723_RATE_53;
                } else {
                    ast_log(LOG_ERROR, "G.723.1 sendrate must be 53 or 63\n");
                }
            } else {
                ast_log(LOG_ERROR, "G.723.1 has only one option \"sendrate=<53|63>\" for 5.3 and 6.3Kbps respectivelly\n");
            }
        }
        ast_config_destroy(cfg);
    }
#endif

#if G72X_ASTERISK >= 16
    static char* g72x_toggle_debug(int fd)
#else
    static int g72x_toggle_debug(int fd)
    #define CLI_SUCCESS RESULT_SUCCESS
    #define CLI_FAILURE RESULT_FAILURE
#endif
{
    struct timespec delay = { 0, 100000000 }; /* 100ms */
    void *tmp;

    /* no locking intentionally */
    if (frame_sizes != NULL) {
        tmp = frame_sizes;
        frame_sizes = NULL;
        nanosleep(&delay, NULL); /* hope all users are gone */
        ast_free(tmp);
        ast_cli(fd, G72X_CODEC " debug disabled\n");
    } else {
        frame_sizes = (int*)ast_malloc((DEBUG_MAX_FRAME_SIZE+1)*sizeof(int));
        if (frame_sizes == NULL)
            return CLI_FAILURE;
        memset(frame_sizes, 0, (DEBUG_MAX_FRAME_SIZE+1)*sizeof(int));
        ast_cli(fd, G72X_CODEC " debug enabled\n");
    }
    return CLI_SUCCESS;
}

static char g72x_toggle_debug_desc[] = "Toggle " G72X_CODEC " codec frame size statistics";
static char g72x_usage[] =
    "Usage: " G72X_CODEC " debug\n"
    "       Toggle " G72X_CODEC " codec frame size statistics\n";

#if G72X_ASTERISK >= 16
    static char *handle_cli_g72x_toggle_debug(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
    {
        switch (cmd) {
        case CLI_INIT:
            e->command = G72X_CODEC " debug";
            e->usage = g72x_usage;
            return NULL;
        case CLI_GENERATE:
            return NULL;
        }
        if (a->argc != 2)
            return CLI_SHOWUSAGE;
        g72x_toggle_debug(a->fd);
        return CLI_SUCCESS;
    }

    static struct ast_cli_entry cli_g72x = AST_CLI_DEFINE(handle_cli_g72x_toggle_debug, g72x_toggle_debug_desc);

#else /* 1.4 or Callweaver */
    static int handle_cli_g72x_toggle_debug(int fd, int argc, char **argv)
    {
        if (argc != 2)
            return RESULT_SHOWUSAGE;
        return g72x_toggle_debug(fd);
    }

    static struct ast_cli_entry cli_g72x = {
        { G72X_CODEC, "debug", NULL }, handle_cli_g72x_toggle_debug,
        g72x_toggle_debug_desc, g72x_usage, NULL
    };

#endif

static int load_module(void)
{
    int res;

#if G72X_ASTERISK >= 100
    ast_format_set(&lintog72x.src_format, AST_FORMAT_SLINEAR, 0);
    ast_format_set(&lintog72x.dst_format, G72X_AST_FORMAT, 0);

    ast_format_set(&g72xtolin.src_format, G72X_AST_FORMAT, 0);
    ast_format_set(&g72xtolin.dst_format, AST_FORMAT_SLINEAR, 0);
#endif

#if !G72X_BCG729 && IPPCORE_STATIC_INIT
    ippStaticInit();
#endif

#if G72X_3
    parse_config();
#endif

#if !G72X_BCG729
    #if G72X_9
        apiG729Decoder_Alloc(G729A_CODEC, &decoder_size);
        apiG729Encoder_Alloc(G729A_CODEC, &encoder_size);
        apiG729Codec_ScratchMemoryAlloc(&coder_size_scratch);
    #else
        apiG723Decoder_Alloc(&decoder_size);
        apiG723Encoder_Alloc(&encoder_size);
        apiG723Codec_ScratchMemoryAlloc(&coder_size_scratch);
    #endif
#endif

    res = ast_register_translator(&g72xtolin);
    if (!res)
        res = ast_register_translator(&lintog72x);
    else
        ast_unregister_translator(&g72xtolin);

    ast_cli_register(&cli_g72x);

    return res;
}

static int unload_module(void)
{
    int res;

    res = ast_unregister_translator(&lintog72x);
    res |= ast_unregister_translator(&g72xtolin);

    ast_cli_unregister(&cli_g72x);

    return res;
}

ASTERISK_FILE_VERSION(__FILE__, "1.0")

#if G72X_ASTERISK >= 14
    /* AST_MODULE_INFO_STANDARD(ASTERISK_GPL_KEY, G72X_CODEC " Coder/Decoder"); */
    AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, G72X_DESC, .load = load_module, .unload = unload_module, .buildopt_sum = "");

#else /* Asterisk 1.2 or Callweaver */

    static int localusecnt = 0;
    static char *tdesc = G72X_DESC;

    char *description(void) {
            return tdesc;
    }
    int usecount(void) {
            int res;
            STANDARD_USECOUNT(res);
            return res;
    }
    #if G72X_ASTERISK
        char *key() {
                return ASTERISK_GPL_KEY;
        }
    #endif
#endif
