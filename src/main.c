/*	$Id: main.c,v 1.1 2001/01/19 22:02:36 ssigala Exp $	*/

/*
 * Copyright (c) 1997-2001 Sandro Sigala.  All rights reserved.
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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "zile.h"
#include "extern.h"
#include "version.h"
#ifdef USE_NCURSES
#include "term_ncurses/term_ncurses.h"
#else
#error "One terminal should be defined."
#endif

#define ZILE_VERSION_STRING \
	"Zile " ZILE_VERSION " of " CONFIGURE_DATE " on " CONFIGURE_HOST

/* The current window; the first window in list. */
windowp cur_wp, head_wp;
/* The current buffer; the previous buffer; the first buffer in list. */
bufferp cur_bp, prev_bp, head_bp;
terminalp cur_tp;

/* The global editor flags. */
int thisflag, lastflag;
/* The universal argument repeat count. */
int last_uniarg = 1;

#ifdef DEBUG
/*
 * This function checks the line list of the specified buffer and
 * appends list informations to the `zile.abort' file if
 * some unexpected corruption is found.
 */
static void check_list(windowp wp)
{
	linep lp, prevlp;

	prevlp = wp->bp->limitp;
	for (lp = wp->bp->limitp->next;; lp = lp->next) {
		if (lp->prev != prevlp || prevlp->next != lp) {
			FILE *f = fopen("zile.abort", "a");
			fprintf(f, "---------- buffer `%s' corruption\n", wp->bp->name);
			fprintf(f, "limitp = %p, limitp->prev = %p, limitp->next = %p\n", wp->bp->limitp, wp->bp->limitp->prev, wp->bp->limitp->next);
			fprintf(f, "prevlp = %p, prevlp->prev = %p, prevlp->next = %p\n", prevlp, prevlp->prev, prevlp->next);
			fprintf(f, "pointp = %p, pointp->prev = %p, pointp->next = %p\n", wp->bp->save_pointp, wp->bp->save_pointp->prev, wp->bp->save_pointp->next);
			fprintf(f, "lp = %p, lp->prev = %p, lp->next = %p\n", lp, lp->prev, lp->next);
			fclose(f);
			cur_tp->close();
			fprintf(stderr, "\aAborting due to internal buffer structure corruption\n");
			fprintf(stderr, "\aFor more informations read the `zile.abort' file\n");
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
#ifdef DEBUG
		check_list(cur_wp);
#endif
		cur_tp->redisplay();

		c = cur_tp->getkey();
		minibuf_clear();

		thisflag = 0;
		if (lastflag & FLAG_DEFINING_MACRO)
			thisflag |= FLAG_DEFINING_MACRO;
		if (lastflag & FLAG_HIGHLIGHT_REGION)
			thisflag |= FLAG_HIGHLIGHT_REGION;

		process_key(c);

		if (thisflag & FLAG_HIGHLIGHT_REGION) {
			if (!(thisflag & FLAG_HIGHLIGHT_REGION_STAYS))
				thisflag &= ~FLAG_HIGHLIGHT_REGION;
		}

		if (thisflag & FLAG_QUIT_ZILE)
			break;
		if (!(thisflag & FLAG_SET_UNIARG))
			last_uniarg = 1;

		lastflag = thisflag;
	}
}

static void select_terminal(void)
{
#ifdef USE_NCURSES
	/* Only the ncurses terminal is available for now. */
	cur_tp = ncurses_tp;
#endif
}

static char about_splash_str[] = "\
" ZILE_VERSION_STRING "\n\
\n\
%Copyright (C) 1997-2001 Sandro Sigala <ssigala@tiscalinet.it>%\n\
\n\
Type %C-x C-c% to exit Zile.\n\
Type %C-h h% or %F1% for help; %C-x u% to undo changes.\n\
Type %C-h C-h% to show a mini help window.\n\
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
"For help type %FWC-h h%E or %FWF1%E; to show a mini help window type %FWC-h C-h%E";
static char *about_wait_str = "[Press any key to leave this screen]";

static void about_screen(void)
{
	if (lookup_bool_variable("alternative-bindings")) {
		replace_string(about_splash_str, "C-h", "M-h");
		replace_string(about_minibuf_str, "C-h", "M-h");
	}

	if (lookup_bool_variable("novice-level")) {
		cur_tp->show_about(about_splash_str, about_minibuf_str,
				   about_wait_str);
		waitkey_discard(200 * 1000);
	} else {
		cur_tp->show_about(about_splash_str, about_minibuf_str, NULL);
		waitkey(20 * 1000);
	}
	minibuf_clear();
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
	if (strlen(getenv("HOME")) + 12 > PATH_MAX) {
		fprintf(stderr, "fatal error: `HOME' is longer than the longest pathname your system supports\n");
		exit(1);
	}
}

/*
 * Output the program syntax then exit.
 */
static void usage(void)
{
	fprintf(stderr, "usage: zile [-hqV] [-f function] [+number] [file ...]\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int c;
	int hflag = 0, qflag = 0;
	char *farg = NULL;

	while ((c = getopt(argc, argv, "f:hqV")) != -1)
		switch (c) {
		case 'f':
			farg = optarg;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'V':
			fprintf(stderr, ZILE_VERSION_STRING "\n");
			exit(0);
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	setlocale(LC_ALL, "");

	sanity_checks();

	select_terminal();
	cur_tp->init();

	init_variables();
	if (!qflag)
		read_rc_file();
	cur_tp->open();

	create_first_window();
	cur_tp->redisplay();
	init_bindings();

	if (argc < 1) {
		if (!hflag && farg == NULL)
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

	/*
	 * Show the Mini Help window if the `-h' flag was specified
	 * on command line or the novice level is enabled.
	 */
	if (hflag || lookup_bool_variable("novice-level"))
		FUNCALL(toggle_minihelp_window);

	if (argc < 1 && lookup_bool_variable("novice-level")) {
		/* Cutted & pasted from Emacs 20.2. */
		insert_string("\
This buffer is for notes you don't want to save.\n\
If you want to create a file, visit that file with C-x C-f,\n\
then enter the text in that file's own buffer.\n\
\n");
		cur_bp->flags &= ~BFLAG_MODIFIED;
		resync_redisplay();
	}

	if (farg != NULL) {
		cur_tp->redisplay();
		if (!execute_function(farg, 1))
			minibuf_error("%FCFunction %FY`%s'%FC not defined%E", farg);
		lastflag |= FLAG_NEED_RESYNC;
	}

	/* Run the main Zile loop. */
	loop();

	/* Free all the memory allocated. */
	free_kill_ring();
	free_registers();
	free_search_history();
	free_macros();
	free_windows();
	free_buffers();
	free_bindings();
	free_variables();

	cur_tp->close();

	return 0;
}
