/* Startup functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: main.c,v 1.19 2004/03/13 17:27:50 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <ctype.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "alist.h"

#include "zile.h"
#include "extern.h"
#include "term_ncurses/term_ncurses.h"

#ifndef PATH_MAX
#define PATH_MAX	_POSIX_PATH_MAX
#endif

#define ZILE_VERSION_STRING \
	"Zile " VERSION

/* The current window; the first window in list. */
Window *cur_wp = NULL, *head_wp = NULL;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;
/* The current output terminal. */
Terminal *cur_tp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

#if DEBUG
/*
 * This function checks the line list of the specified buffer and
 * appends list informations to the `zile.abort' file if
 * some unexpected corruption is found.
 */
static void check_list(Window *wp)
{
	Line *lp, *prevlp;

	prevlp = wp->bp->limitp;
	for (lp = wp->bp->limitp->next;; lp = lp->next) {
		if (lp->prev != prevlp || prevlp->next != lp) {
			FILE *f = fopen("zile.abort", "a");
			fprintf(f, "---------- buffer `%s' corruption\n", wp->bp->name);
			fprintf(f, "limitp = %p, limitp->prev = %p, limitp->next = %p\n", wp->bp->limitp, wp->bp->limitp->prev, wp->bp->limitp->next);
			fprintf(f, "prevlp = %p, prevlp->prev = %p, prevlp->next = %p\n", prevlp, prevlp->prev, prevlp->next);
			fprintf(f, "pointp = %p, pointp->prev = %p, pointp->next = %p\n", wp->bp->pt.p, wp->bp->pt.p->prev, wp->bp->pt.p->next);
			fprintf(f, "lp = %p, lp->prev = %p, lp->next = %p\n", lp, lp->prev, lp->next);
			fclose(f);
			cur_tp->close();
			fprintf(stderr, "\aAborting due to internal buffer structure corruption\n");
			fprintf(stderr, "\aFor more information read the `zile.abort' file\n");
			abort();
		}
		if (lp == wp->bp->limitp)
			break;
		prevlp = lp;
	}
}
#endif /* DEBUG */

static void loop(void)
{
	int c;

	for (;;) {
		if (lastflag & FLAG_NEED_RESYNC)
			resync_redisplay();
#if DEBUG
		check_list(cur_wp);
#endif
		/* Redisplay every second to update clock */
		do {
			cur_tp->redisplay();
			c = waitkey_discard(1000);
		} while (c == KBD_NOKEY);

		minibuf_clear();

		thisflag = 0;
		if (lastflag & FLAG_DEFINING_MACRO)
			thisflag |= FLAG_DEFINING_MACRO;

		process_key(c);

		if (thisflag & FLAG_QUIT_ZILE)
			break;
		if (!(thisflag & FLAG_SET_UNIARG))
			last_uniarg = 1;

		lastflag = thisflag;
	}
}

/*
 * Select the output interface; this function is dummy for now (only ncurses
 * is supported), but other terminals can be easily added.
 */
static void select_terminal(void)
{
	/* Only the ncurses terminal is available for now. */
	cur_tp = ncurses_tp;
}

static char about_splash_str[] = "\
" ZILE_VERSION_STRING "\n\
\n\
%Copyright (C) 1997-2004 Sandro Sigala <sandro@sigala.it>%\n\
%Copyright (C) 2003-2004 David A. Capello <dacap@users.sourceforge.net>%\n\
%Copyright (C) 2003-2004 Reuben Thomas <rrt@sc3d.org>%\n\
\n\
Type %C-x C-c% to exit Zile.\n\
Type %C-h h% or %F1% for help; %C-x u% to undo changes.\n\
Type %C-h C-h% or %F10% to show a mini help window.\n\
Type %C-h C-d% for information on getting the latest version.\n\
Type %C-h t% for a tutorial on using Zile.\n\
Type %C-h s% for a sample configuration file.\n\
Type %C-g% anytime to cancel the current operation.\n\
\n\
%C-x% means hold the CTRL key while typing the character %x%.\n\
%M-x% means hold the META or EDIT or ALT key down while typing %x%.\n\
If there is no META, EDIT or ALT key, instead press and release\n\
the ESC key and then type %x%.\n\
Combinations like %C-h t% mean first do %C-h%, then press %t%.\n\
$For tips and answers to frequently asked questions, see the Zile FAQ.$\n\
$(Type C-h F [a capital F!].)$\
";

static char about_minibuf_str[] =
"Welcome to Zile!  For help type `C-h h' or `F1'";

static void about_screen(void)
{
	/* I don't like this hack, but I don't know another way... */
	if (lookup_bool_variable("alternative-bindings")) {
		replace_string(about_splash_str, "C-h", "M-h");
		replace_string(about_minibuf_str, "C-h", "M-h");
	}

	cur_tp->show_about(about_splash_str, about_minibuf_str);
}

/*
 * Do some sanity checks on the environment.
 */
static void sanity_checks(void)
{
	/*
	 * The functions `read_rc_file' and `help_tutorial' rely
	 * on a usable `HOME' environment variable.
	 */
	if (getenv("HOME") == NULL) {
		fprintf(stderr, "fatal error: please set `HOME' to point to your home-directory\n");
		exit(1);
	}
#ifdef PATH_MAX
	if (strlen(getenv("HOME")) + 12 > PATH_MAX) {
		fprintf(stderr, "fatal error: `HOME' is longer than the longest pathname your system supports\n");
		exit(1);
	}
#endif
}

static void execute_functions(alist al)
{
	char *func;
	for (func = alist_first(al); func != NULL; func = alist_next(al)) {
		cur_tp->redisplay();
		if (!execute_function(func, 1))
			minibuf_error("Function `%s' not defined", func);
		lastflag |= FLAG_NEED_RESYNC;
	}
}

/*
 * Return the number of occurrences of c into s.
 */
static int countchr(const char *s, int c)
{
	int count = 0;
	for (; *s != '\0'; ++s)
		if (*s == c)
			++count;
	return count;
}

/*
 * Check the `variable=expression' syntax correctness.
 */
static void check_var_syntax(const char *expr)
{
	if (strlen(expr) < 3 || countchr(expr, '=') != 1 ||
	    strchr(expr, '=') == expr ||
	    strchr(expr, '=') == expr + strlen(expr) - 1) {
		fprintf(stderr, "zile: invalid `variable=expression' syntax\n");
		exit(1);
	}
}

static void set_variables(alist al)
{
	char *expr;
	for (expr = alist_first(al); expr != NULL; expr = alist_next(al)) {
		char *var = expr;
		char *eq = strchr(expr, '=');
		char *val = eq + 1;
		*eq = '\0';
		fprintf(stderr, "'%s' = '%s'\n", var, val);
		set_variable(var, val);
	}
}

/*
 * Output the program syntax then exit.
 */
static void usage(void)
{
	fprintf(stderr, "usage: zile [-hqV] [-f function] [-v variable=value] [-u rcfile]\n"
			"	     [+number] [file ...]\n");
	exit(1);
}

static void setup_main_screen(int argc, int hflag)
{
	Buffer *bp, *last_bp = NULL;
	int c = 0;

	for (bp = head_bp; bp; bp = bp->next) {
		/* Last buffer that isn't *scratch*.  */
		if (bp->next && !bp->next->next)
			last_bp = bp;

		c++;
	}

	/* *scratch* and two files.  */
	if (c == 3) {
		FUNCALL(split_window);
		switch_to_buffer(last_bp);
		FUNCALL(other_window);
	}
	/* More than two files.  */
	else if (c > 3) {
		FUNCALL(list_buffers);
	}
	else {
		/*
		 * Show the Mini Help window if the `-h' flag was specified
		 * on command line or the novice level is enabled.
		 */
		if (hflag || lookup_bool_variable("novice-level"))
			FUNCALL(minihelp_toggle_window);

		if (argc < 1 && lookup_bool_variable("novice-level")) {
			/* Cut & pasted from Emacs 20.2. */
			insert_string("\
This buffer is for notes you don't want to save.\n\
If you want to create a file, visit that file with C-x C-f,\n\
then enter the text in that file's own buffer.\n\
\n");
			cur_bp->flags &= ~BFLAG_MODIFIED;
			resync_redisplay();
		}
	}
}

int main(int argc, char **argv)
{
	int c;
	int hflag = 0, qflag = 0;
	char *uarg = NULL;
	alist fargs = alist_new();
	alist vargs = alist_new();

	while ((c = getopt(argc, argv, "f:hqu:v:V")) != -1)
		switch (c) {
		case 'f':
			alist_append(fargs, optarg);
			break;
		case 'h':
			hflag = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'u':
			uarg = optarg;
			break;
		case 'v':
			check_var_syntax(optarg);
			alist_append(vargs, optarg);
			break;
		case 'V':
			fprintf(stderr, ZILE_VERSION_STRING "\n");
			return 0;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	/*
	 * With this call `display-time-format' variable will be formatted
	 * by strftime() according to the current locale.
	 */
	setlocale(LC_ALL, "");

	sanity_checks();

	select_terminal();
	cur_tp->init();

#if ENABLE_LUA
        /* Initialise Lua */
        zlua_open();
#endif

	init_variables();
	if (!qflag)
		read_rc_file(uarg);
	set_variables(vargs);

	/* Force refresh of cached variables. */
	cur_tp->refresh_cached_variables();
	cur_tp->open();

	/* Create the `*scratch*' buffer and initialize key bindings. */
	create_first_window();
	cur_tp->redisplay();
	init_bindings();

	if (argc < 1) {
		/*
		 * Show the splash screen only if there isn't any file
		 * specified on command line and no function was specified
		 * with the `-f' flag.
		 */
		if (!hflag && alist_count(fargs) == 0)
			about_screen();
	} else {
		while (*argv) {
			int line = 0;
			if (**argv == '+')
				line = atoi(*argv++ + 1);
			if (*argv)
				open_file(*argv++, line - 1);
		}
	}

	setup_main_screen(argc, hflag);

	execute_functions(fargs);

	/* Run the main Zile loop (read key, process key, read key, ...). */
	loop();

#if ENABLE_LUA
        /* Finalise Lua */
        zlua_close();
#endif

	/* Free all the memory allocated. */
	alist_delete(fargs);
	alist_delete(vargs);
	free_kill_ring();
	free_registers();
	free_search_history();
	free_macros();
	free_windows();
	free_buffers();
	free_bindings();
	free_variables();
	free_minibuf();

	cur_tp->close();

	return 0;
}
