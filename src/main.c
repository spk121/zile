/* Program invocation, startup and shutdown
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

/*	$Id: main.c,v 1.74 2005/01/23 18:39:03 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_GETOPT_LONG_ONLY
#include <getopt.h>
#else
#include "getopt.h"
#endif
#include <signal.h>
#if ALLEGRO
#if HAVE_ALLEGRO_H
#include <allegro.h>
#endif
#endif

#include "zile.h"
#include "extern.h"
#include "eval.h"
#include "vars.h"

#define ZILE_VERSION_STRING	"Zile " VERSION

#define ZILE_COPYRIGHT_STRING \
  "Copyright (C) 1997-2004 Sandro Sigala <sandro@sigala.it>\n"\
  "Copyright (C) 2003-2004 David A. Capello <dacap@users.sourceforge.net>\n"\
  "Copyright (C) 2003-2005 Reuben Thomas <rrt@sc3d.org>"

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
" ZILE_COPYRIGHT_STRING "\n\
\n\
Type `C-x C-c' to exit Zile.\n\
Type `C-h h' for help; `C-x u; to undo changes.\n\
Type `C-h C-d' for information on getting the latest version.\n\
Type `C-h t' for a tutorial on using Zile.\n\
Type `C-h s' for a sample configuration file.\n\
Type `C-g' at any time to cancel the current operation.\n\
\n\
`C-x' means hold the CTRL key while typing the character `x'.\n\
`M-x' means hold the META or ALT key down while typing `x'.\n\
If there is no META or ALT key, instead press and release\n\
the ESC key and then type `x'.\n\
Combinations like `C-h h' mean first press `C-h', then `h'.\n\
";

static char about_minibuf_str[] =
"Welcome to Zile!  For help type `C-h h'";

static void about_screen(void)
{
  /* I don't like this hack, but I don't know another way... */
  if (lookup_bool_variable("alternative-bindings")) {
    replace_string(about_splash_str, "C-h", "M-h");
    replace_string(about_minibuf_str, "C-h", "M-h");
  }

  minibuf_write(about_minibuf_str);
  if (!lookup_bool_variable("skip-splash-screen"))
    show_splash_screen(about_splash_str);
  term_refresh();
  waitkey(20 * 10);
}

static void setup_main_screen(int argc, astr as)
{
  Buffer *bp, *last_bp = NULL;
  int c = 0;

  for (bp = head_bp; bp; bp = bp->next) {
    /* Last buffer that isn't *scratch*. */
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

      if (astr_len(as) > 0)
        insert_string(astr_cstr(as));
      else
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

static struct sigaction act; /* For use by signal handlers */

/* What do we do when we catch the suspend signal */
static void suspend_sig_handler(int signal)
{
  term_tidy();
  term_suspend();
        
  /* Trap SIGHUP and SIGTERM so we can properly deal with them while
     suspended */
  act.sa_handler = other_sig_handler;
  sigaction(SIGHUP, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  /* We used to re-enable the default SIG_DFL and raise SIGTSTP, but 
     then we could be (and were) interrupted in the middle of the call.
     So we do it the mutt way instead */
  kill(0, SIGSTOP);
}

static void signal_init(void);

/* Restore the suspend handler when we come back into the prog */
static void cont_sig_handler(int signal)
{
  term_resume();
  term_full_redisplay();

  /* Simplest just to reinitialise everything. */
  signal_init();
}

static void signal_init(void)
{
  /* Set up signal handling */
  signal(SIGSEGV, segv_sig_handler);
  signal(SIGHUP, other_sig_handler);
  signal(SIGINT, other_sig_handler);
  signal(SIGQUIT, other_sig_handler);
  signal(SIGTERM, other_sig_handler);

  /* If we don't do this, it seems other stuff interrupts the
     suspend handler! Without it, suspending zile under e.g.
     pine or mutt freezes the process. */
  sigfillset(&act.sa_mask);

  act.sa_handler = suspend_sig_handler;
  sigaction(SIGTSTP, &act, NULL);
  act.sa_handler = cont_sig_handler;
  sigaction(SIGCONT, &act, NULL);
}

/* Options table */
struct option longopts[] = {
    { "batch",        0, NULL, 'b' },
    { "eval",         1, NULL, 'e' },
    { "help",         0, NULL, 'h' },
    { "load",         1, NULL, 'l' },
    { "no-init-file", 0, NULL, 'q' },
    { "version",      0, NULL, 'v' },
    { 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
  int c;
  int bflag = FALSE, qflag = FALSE, eflag = FALSE;
  astr as = astr_new();

  /* Set up Lisp environment now so it's available to files and
     expressions specified on the command-line. */
  eval_init();
  init_variables();
  
  while ((c = getopt_long_only(argc, argv, "l:q", longopts, NULL)) != -1)
    switch (c) {
    case 'b':
      bflag = TRUE;
      qflag = TRUE;
      break;
    case 'e':
      astr_cat_delete(as, lisp_read_string(optarg));
      eflag = TRUE;
      break;
    case 'l':
      astr_cat_delete(as, lisp_read_file(optarg));
      eflag = TRUE; /* Loading a file counts as reading an expression. */
      break;
    case 'q':
      qflag = TRUE;
      break;
    case 'v':
      fprintf(stderr,
              ZILE_VERSION_STRING "\n"
              ZILE_COPYRIGHT_STRING "\n"
              "Zile comes with ABSOLUTELY NO WARRANTY.\n"
              "You may redistribute copies of Zile\n"
              "under the terms of the GNU General Public License.\n"
              "For more information about these matters, see the file named COPYING.\n"
              );
      return 0;
    case 'h':
      fprintf(stderr,
              "Usage: zile [OPTION-OR-FILENAME]...\n"
              "\n"
              "Run Zile, the lightweight Emacs clone.\n"
              "\n"
              "Initialization options:\n"
              "\n"
              "--batch                do not do interactive display; implies -q\n"
              "--help                 display this help message and exit\n"
              "--no-init-file, -q     do not load ~/.zile\n"
              "--version              display version information and exit\n"
              "\n"
              "Action options:\n"
              "\n"
              "FILE                   visit FILE using find-file\n"
              "+LINE FILE             visit FILE using find-file, then go to line LINE\n"
              "--eval EXPR            evaluate Emacs Lisp expression EXPR\n"
              "--load, -l FILE        load file of Emacs Lisp code using the load function\n"
              );
      return 0;
    }
  argc -= optind;
  argv += optind;

  signal_init();

  setlocale(LC_ALL, "");

  if (bflag)
    printf(astr_cstr(as));
  else {
    if (!qflag) {
      astr as = get_home_dir();
      astr_cat_cstr(as, "/.zile");
      lisp_read_file(astr_cstr(as));
      astr_delete(as);
    }

    term_init();

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
    else if (eflag == FALSE)
      /* Show the splash screen only if no files and no Lisp expression
         or load file is specified on the command line. */
      about_screen();

    setup_main_screen(argc, as);

    /* Run the main Zile loop. */
    loop();

    /* Tidy and close the terminal. */
    term_tidy();
    term_close();

    free_bindings();
  }

  /* Free Lisp state. */
  variableFree(mainVarList);
  variableFree(defunList);
  eval_finalise();

  /* Free all the memory allocated. */
  astr_delete(as);
  free_kill_ring();
  free_registers();
  free_search_history();
  free_macros();
  free_windows();
  free_buffers();
  free_minibuf();
  free_rotation_buffers();

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN();
#endif
