/* Exported terminal
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
   All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*	$Id: term_ncurses.c,v 1.10 2004/02/17 23:20:31 rrt Exp $	*/

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
extern char *ncurses_minibuf_read(const char *prompt, const char *value, Completion *cp, History *hp);
extern void ncurses_minibuf_clear(void);

static Terminal thisterm = {
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

Terminal *ncurses_tp = &thisterm;
