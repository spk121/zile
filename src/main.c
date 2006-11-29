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
   Software Foundation, Fifth Floor, 51 Franklin Street, Boston, MA
   02111-1301, USA.  */

/*	$Id: main.c,v 1.103 2006/11/29 21:19:31 rrt Exp $	*/

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
  "Copyright (C) 2003-2006 Reuben Thomas <rrt@sc3d.org>\n"\
  "Copyright (C) 2003-2004 David A. Capello <dacap@users.sourceforge.net>"

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
    size_t key;

    if (lastflag & FLAG_NEED_RESYNC)
      resync_redisplay();
    term_redisplay();
    term_refresh();

    thisflag = 0;
    if (lastflag & FLAG_DEFINING_MACRO)
      thisflag |= FLAG_DEFINING_MACRO;

    key = getkey();
    minibuf_clear();
    process_key(key);

    if (thisflag & FLAG_QUIT_ZILE)
      break;
    if (!(thisflag & FLAG_SET_UNIARG))
      last_uniarg = 1;

    if (last_command() != F_undo)
      cur_bp->next_undop = cur_bp->last_undop;

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
  minibuf_write(about_minibuf_str);
  if (!lookup_bool_variable("skip-splash-screen")) {
    show_splash_screen(about_splash_str);
    term_refresh();
    waitkey(20 * 10);
  }
}

static void execute_functions(list funcs)
{
  list l;
  for (l = list_first(funcs); l != funcs; l = list_next(l)) {
    char *func = (char *)l->item;
    term_redisplay();
    if (!execute_function(func, 1))
      minibuf_error("Function `%s' not defined", func);
    lastflag |= FLAG_NEED_RESYNC;
  }
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

static void signal_init(void)
{
  /* Set up signal handling */
  signal(SIGSEGV, segv_sig_handler);
  signal(SIGHUP, other_sig_handler);
  signal(SIGINT, other_sig_handler);
  signal(SIGQUIT, other_sig_handler);
  signal(SIGTERM, other_sig_handler);
}

/* Options table */
struct option longopts[] = {
    { "batch",        0, NULL, 'b' },
    { "help",         0, NULL, 'h' },
    { "funcall",      1, NULL, 'f' },
    { "no-init-file", 0, NULL, 'q' },
    { "version",      0, NULL, 'v' },
    { 0, 0, 0, 0 }
};

int main(int argc, char **argv)
{
  int c, bflag = FALSE, qflag = FALSE;
  astr as = astr_new();
  list fargs = list_new();

  /* Set up Lisp environment now so it's available to files and
     expressions specified on the command-line. */
  lisp_init();
  init_variables();

  while ((c = getopt_long_only(argc, argv, "bhf:qv", longopts, NULL)) != -1)
    switch (c) {
    case 'b':
      bflag = TRUE;
      qflag = TRUE;
      break;
    case 'f':
      list_append(fargs, optarg);
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
              "--funcall, -f FUNC     call Zile function FUNC with no arguments\n"
              "--no-init-file, -q     do not load ~/.zile\n"
              "--version              display version information and exit\n"
              "\n"
              "Action options:\n"
              "\n"
              "FILE                   visit FILE using find-file\n"
              "+LINE FILE             visit FILE using find-file, then go to line LINE\n"
              );
      return 0;
    }
  argc -= optind;
  argv += optind;

  signal_init();

  setlocale(LC_ALL, "");

  init_bindings();

  if (bflag)
    printf(astr_cstr(as));
  else {
    term_init();

    /* Create the `*scratch*' buffer and initialize key bindings. */
    create_first_window();
    term_redisplay();

    if (argc >= 1)
      while (*argv) {
        size_t line = 1;
        if (**argv == '+')
          line = strtoul(*argv++ + 1, NULL, 10);
        if (*argv)
          open_file(*argv++, line - 1);
      }
    else if (list_length(fargs) == 0)
      /* Show the splash screen only if no files and no Lisp expression
         or load file is specified on the command line. */
      about_screen();

    setup_main_screen(argc, as);

    if (!qflag) {
      le *list;

      astr as = get_home_dir();
      astr_cat_cstr(as, "/.zile");
      list = lisp_read_file(astr_cstr(as));
      astr_delete(lisp_dump(list));
      astr_delete(as);
      leWipe(list);
    }

    execute_functions(fargs);
    list_delete(fargs);

    /* Run the main Zile loop. */
    loop();

    /* Tidy and close the terminal. */
    term_tidy();
    term_close();

    free_bindings();
  }

  /* Free Lisp state. */
  variableFree(mainVarList);
  lisp_finalise();

  /* Free all the memory allocated. */
  astr_delete(as);
  free_kill_ring();
  free_registers();
  free_macros();
  free_windows();
  free_buffers();
  free_minibuf();
  free_rotation_buffers();

  return 0;
}

#ifdef ALLEGRO
END_OF_MAIN()
#endif
