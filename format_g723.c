/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! 
 * \file
 *
 * \brief Old-style G.723.1 frame/timestamp format.
 * 
 * \arg Extensions: g723, g723sf
 * \ingroup formats
 */
 
#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 233694 $")

#include "asterisk/mod_format.h"
#include "asterisk/module.h"

#define BUF_SIZE        24      /* one G723.1 6.3kbps frame */
#define G723_SAMPLES   240

static struct ast_frame *g723_read(struct ast_filestream *s, int *whennext)
{
	int res;
	/* Read the data into the buffer */
	s->fr.frametype = AST_FRAME_VOICE;
	s->fr.subclass = AST_FORMAT_G723_1;
	s->fr.mallocd = 0;
	s->fr.samples = G723_SAMPLES;
	AST_FRAME_SET_BUFFER(&s->fr, s->buf, AST_FRIENDLY_OFFSET, BUF_SIZE);
	if ((res = fread(s->fr.data.ptr, 1, s->fr.datalen, s->f)) != s->fr.datalen) {
		if (res != 0 || !feof(s->f))
			ast_log(LOG_WARNING, "Short read (%d of %d bytes) (%s)!\n", res, s->fr.datalen, strerror(errno));
		return NULL;
	}
	*whennext = s->fr.samples;
	return &s->fr;
}

static int g723_write(struct ast_filestream *s, struct ast_frame *f)
{
	int res;
	if (f->frametype != AST_FRAME_VOICE) {
		ast_log(LOG_WARNING, "Asked to write non-voice frame!\n");
		return -1;
	}
	if (f->subclass != AST_FORMAT_G723_1) {
		ast_log(LOG_WARNING, "Asked to write non-g723 frame!\n");
		return -1;
	}
	if (f->datalen <= 0) {
		ast_log(LOG_WARNING, "Short frame ignored (%d bytes long?)\n", f->datalen);
		return 0;
	}
	if ((res = fwrite(f->data.ptr, 1, f->datalen, s->f)) != f->datalen) {
		ast_log(LOG_WARNING, "Unable to write frame: res=%d (%s)\n", res, strerror(errno));
		return -1;
	}	
	return 0;
}

static int g723_seek(struct ast_filestream *fs, off_t sample_offset, int whence)
{
        long bytes;
        off_t min,cur,max,offset=0;
        min = 0;
        cur = ftello(fs->f);
        fseeko(fs->f, 0, SEEK_END);
        max = ftello(fs->f);
        
        bytes = BUF_SIZE * (sample_offset / G723_SAMPLES);
        if (whence == SEEK_SET)
                offset = bytes;
        else if (whence == SEEK_CUR || whence == SEEK_FORCECUR)
                offset = cur + bytes;
        else if (whence == SEEK_END)
                offset = max - bytes;
        if (whence != SEEK_FORCECUR) {
                offset = (offset > max)?max:offset;
        }
        /* protect against seeking beyond begining. */
        offset = (offset < min)?min:offset;
        if (fseeko(fs->f, offset, SEEK_SET) < 0)
                return -1;
        return 0;
}

static int g723_trunc(struct ast_filestream *fs)
{
	/* Truncate file to current length */
	if (ftruncate(fileno(fs->f), ftello(fs->f)) < 0)
		return -1;
	return 0;
}

static off_t g723_tell(struct ast_filestream *fs)
{
        off_t offset = ftello(fs->f);
        return (offset/BUF_SIZE)*G723_SAMPLES;
}

static const struct ast_format g723_1_f = {
	.name = "g723sf",
	.exts = "g723|g723sf",
	.format = AST_FORMAT_G723_1,
	.write = g723_write,
	.seek =	g723_seek,
	.trunc = g723_trunc,
	.tell =	g723_tell,
	.read =	g723_read,
	.buf_size = BUF_SIZE + AST_FRIENDLY_OFFSET,
};

static int load_module(void)
{
	if (ast_format_register(&g723_1_f))
		return AST_MODULE_LOAD_FAILURE;
	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void)
{
	return ast_format_unregister(g723_1_f.name);
}	

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_LOAD_ORDER, "Raw G.723.1 data",
	.load = load_module,
	.unload = unload_module,
	.load_pri = 10,
);
