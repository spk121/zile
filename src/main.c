/* Startup functions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2004-2005 Reuben Thomas.
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

/*	$Id: main.c,v 1.58 2005/01/12 10:55:34 rrt Exp $	*/

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
#include <signal.h>
#if ALLEGRO
#if HAVE_ALLEGRO_H
#include <allegro.h>
#endif
#endif

#include "zile.h"
#include "extern.h"

#define ZILE_VERSION_STRING	"Zile " VERSION

/* The current window; the first window in list. */
Window *cur_wp = NULL, *head_wp = NULL;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

static void loop(void)
{
  for (;;) {
    if (lastflag & FLAG_NEED_RESYNC)
      resync_redisplay();
    term_redisplay();
    term_refresh();

    minibuf_clear();

    thisflag = 0;
    if (lastflag & FLAG_DEFINING_MACRO)
      thisflag |= FLAG_DEFINING_MACRO;

    process_key(term_getkey());

    if (thisflag & FLAG_QUIT_ZILE)
      break;
    if (!(thisflag & FLAG_SET_UNIARG))
      last_uniarg = 1;

    lastflag = thisflag;
  }
}

static char about_splash_str[] = "\
" ZILE_VERSION_STRING "\n\
\n\
%Copyright (C) 1997-2004 Sandro Sigala <sandro@sigala.it>%\n\
%Copyright (C) 2003-2004 David A. Capello <dacap@users.sourceforge.net>%\n\
%Copyright (C) 2003-2005 Reuben Thomas <rrt@sc3d.org>%\n\
\n\
Type %C-x C-c% to exit Zile.\n\
Type %C-h h% or %F1% for help; %C-x u% to undo changes.\n\
Type %C-h C-d% for information on getting the latest version.\n\
Type %C-h t% for a tutorial on using Zile.\n\
Type %C-h s% for a sample configuration file.\n\
Type %C-g% at any time to cancel the current operation.\n\
\n\
%C-x% means hold the CTRL key while typing the character %x%.\n\
%M-x% means hold the META or EDIT or ALT key down while typing %x%.\n\
If there is no META, EDIT or ALT key, instead press and release\n\
the ESC key and then type %x%.\n\
Combinations like %C-h t% mean first do %C-h%, then press %t%.\n\
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

  if (!lookup_bool_variable("skip-splash-screen"))
    show_splash_screen(about_splash_str);
  minibuf_write(about_minibuf_str);
}

/*
 * Do some sanity checks on the environment.
 */
static void sanity_checks(void)
{
  /* The functions `read_rc_file' and `help_with_tutorial' rely on a
     usable `HOME' environment variable. */
  if (getenv("HOME") == NULL) {
    fprintf(stderr, "fatal error: please set `HOME' to point to your home directory\n");
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
  fprintf(stderr, "Usage: zile [-hqV] [-e EXPR] [-u RCFILE] [+NUMBER] [FILE...]\n");
  exit(1);
}

static void setup_main_screen(int argc)
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
    if (argc < 1) {
      undo_nosave = TRUE;
      insert_string("\
This buffer is for notes you don't want to save.\n\
If you want to create a file, visit that file with C-x C-f,\n\
then enter the text in that file's own buffer.\n\
\n");
      undo_nosave = FALSE;
      cur_bp->flags &= ~BFLAG_MODIFIED;
      resync_redisplay();
    }
  }
}

static void segv_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, "Zile crashed.  Please send a bug report to <" PACKAGE_BUGREPORT ">.\r\n");
  zile_exit(2);
}

static void other_sig_handler(int signo)
{
  (void)signo;
  fprintf(stderr, "Zile terminated with signal %d.\r\n", signo);
  zile_exit(2);
}

int main(int argc, char **argv)
{
  int c;
  int qflag = 0;
  char *uarg = NULL, *earg = NULL;

  while ((c = getopt(argc, argv, "e:hqu:V")) != -1)
    switch (c) {
    case 'e':
      earg = optarg;
    case 'q':
      qflag = 1;
      break;
    case 'u':
      uarg = optarg;
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

  sanity_checks();

  /* Set up signal handling */
  signal(SIGFPE, segv_sig_handler);
  signal(SIGSEGV, segv_sig_handler);
  signal(SIGHUP, other_sig_handler);
  signal(SIGINT, other_sig_handler);
  signal(SIGQUIT, other_sig_handler);
  signal(SIGTERM, other_sig_handler);

  setlocale(LC_ALL, "");

  term_init();

  init_variables();
  if (!qflag)
    read_rc_file(uarg);

  /* Create the `*scratch*' buffer and initialize key bindings. */
  create_first_window();
  term_redisplay();
  init_bindings();

  if (argc >= 1)
    while (*argv) {
      int line = 0;
      if (**argv == '+')
        line = atoi(*argv++ + 1);
      if (*argv)
        open_file(*argv++, line - 1);
    }
  else
    /* Show the splash screen only if there isn't any file specified
       on command line. */
    about_screen();

  setup_main_screen(argc);

  /* Run the main Zile loop. */
  loop();

  /* Clear last line of display, and leave cursor there. */
  term_move(ZILE_LINES - 1, 0);
  term_clrtoeol();
  term_attrset(1, ZILE_NORMAL); /* Make sure we end in normal font */
  term_refresh();

  /* Free all the memory allocated. */
  free_kill_ring();
  free_registers();
  free_search_history();
  free_macros();
  free_windows();
  free_buffers();
  free_bindings();
  free_variables();
  free_minibuf();
  free_rotation_buffers();

  term_close();

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN();
#endif
