/*	$Id: term_ncurses.c,v 1.6 2004/01/07 00:45:20 rrt Exp $	*/

/*
 * Copyright (c) 1997-2003 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This module exports only the `ncurses_tp' pointer.
 */

#include "config.h"

#include <stddef.h>

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "zile.h"
#include "extern.h"

#include "term_ncurses.h"

extern int ncurses_init(void);
extern int ncurses_open(void);
extern int ncurses_close(void);
extern int ncurses_getkey(void);
extern int ncurses_xgetkey(int mode, int arg);
extern int ncurses_ungetkey(int c);
extern void ncurses_refresh_cached_variables(void);
extern void ncurses_refresh(void);
extern void ncurses_redisplay(void);
extern void ncurses_full_redisplay(void);
extern void ncurses_show_about(const char *splash, const char *minibuf);
extern void ncurses_clear(void);
extern void ncurses_beep(void);
extern void ncurses_minibuf_write(const char *fmt);
extern char *ncurses_minibuf_read(const char *prompt, const char *value, historyp hp);
extern void ncurses_minibuf_clear(void);

static struct terminal thisterm = {
	/* Unitialised screen pointer */
	NULL,

	/* Uninitialized width and height. */
	-1, -1,

	/* Pointers to ncurses terminal functions. */
	ncurses_init,
	ncurses_open,
        ncurses_close,
	ncurses_getkey,
	ncurses_xgetkey,
	ncurses_ungetkey,
	ncurses_refresh_cached_variables,
	ncurses_refresh,
	ncurses_redisplay,
	ncurses_full_redisplay,
	ncurses_show_about,
	ncurses_clear,
	ncurses_beep,
	ncurses_minibuf_write,
	ncurses_minibuf_read,
	ncurses_minibuf_clear,
};

terminalp ncurses_tp = &thisterm;
