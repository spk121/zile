/* Program invocation, startup and shutdown

   Copyright (c) 1997-2012 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include <config.h>

#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "progname.h"
#include "dirname.h"
#include "gl_linked_list.h"

#include "main.h"
#include "extern.h"

#define ZILE_COPYRIGHT_STRING \
  "Copyright (C) 2012 Free Software Foundation, Inc."

/* The current window; the first window in list. */
Window *cur_wp = NULL, *head_wp = NULL;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

static const char splash_str[] = "\
Welcome to GNU " PACKAGE_NAME ".\n\
\n\
Undo changes       C-x u        Exit " PACKAGE_NAME "         C-x C-c\n\
(`C-' means use the CTRL key.  `M-' means hold the Meta (or Alt) key.\n\
If you have no Meta key, you may type ESC followed by the character.)\n\
Combinations like `C-x u' mean first press `C-x', then `u'.\n\
\n\
Keys not working properly?  See file://" PATH_DOCDIR "/FAQ\n\
\n\
" ZILE_VERSION_STRING "\n\
" ZILE_COPYRIGHT_STRING "\n\
\n\
GNU " PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n\
" PACKAGE_NAME " is Free Software--Free as in Freedom--so you can redistribute copies\n\
of " PACKAGE_NAME " and modify it; see the file COPYING.  Otherwise, a copy can be\n\
downloaded from http://www.gnu.org/licenses/gpl.html.\n\
";

static _Noreturn void
segv_sig_handler (int signo _GL_UNUSED_PARAMETER)
{
  fprintf (stderr,
           "%s: " PACKAGE_NAME
           " crashed.  Please send a bug report to <"
           PACKAGE_BUGREPORT ">.\r\n",
           program_name);
  zile_exit (true);
}

static _Noreturn void
other_sig_handler (int signo _GL_UNUSED_PARAMETER)
{
  fprintf (stderr, "%s: terminated with signal %d.\r\n",
           program_name, signo);
  zile_exit (false);
}

static void
signal_init (void)
{
  /* Set up signal handling */
  signal (SIGSEGV, segv_sig_handler);
  signal (SIGBUS, segv_sig_handler);
  signal (SIGHUP, other_sig_handler);
  signal (SIGINT, other_sig_handler);
  signal (SIGTERM, other_sig_handler);
}

/* Options table */
struct option longopts[] = {
#define D(text)
#define O(longname, shortname, arg, argstring, docstring) \
  {longname, arg, NULL, shortname},
#define A(argstring, docstring)
#include "tbl_opts.h"
#undef D
#undef O
#undef A
  {0, 0, 0, 0}
};

enum {ARG_FUNCTION = 1, ARG_LOADFILE, ARG_FILE};

int
main (int argc, char **argv)
{
  int qflag = false;
  gl_list_t arg_type = gl_list_create_empty (GL_LINKED_LIST,
                                             NULL, NULL, NULL, false);
  gl_list_t arg_arg = gl_list_create_empty (GL_LINKED_LIST,
                                             NULL, NULL, NULL, false);
  gl_list_t arg_line = gl_list_create_empty (GL_LINKED_LIST,
                                             NULL, NULL, NULL, false);
  size_t line = 1;

  GC_INIT ();
  set_program_name (argv[0]);
  estr_init ();

  /* Set up Lisp environment now so it's available to files and
     expressions specified on the command-line. */
  init_lisp ();
  init_variables ();
  init_eval ();

  opterr = 0; /* Don't display errors for unknown options */
  for (;;)
    {
      int this_optind = optind ? optind : 1, longindex = -1, c;
      char *buf, *shortopt;

      /* Leading `-' means process all arguments in order, treating
         non-options as arguments to an option with code 1 */
      /* Leading `:' so as to return ':' for a missing arg, not '?' */
      c = getopt_long (argc, argv, "-:f:l:q", longopts, &longindex);

      if (c == -1)
        break;
      else if (c == '\001') /* Non-option (assume file name) */
        longindex = 5;
      else if (c == '?') /* Unknown option */
        minibuf_error ("Unknown option `%s'", argv[this_optind]);
      else if (c == ':') /* Missing argument */
        {
          fprintf (stderr, "%s: Option `%s' requires an argument\n",
                   program_name, argv[this_optind]);
          exit (EXIT_FAILURE);
        }
      else if (c == 'q')
        longindex = 0;
      else if (c == 'f')
        longindex = 1;
      else if (c == 'l')
        longindex = 2;

      switch (longindex)
        {
        case 0:
          qflag = true;
          break;
        case 1:
          gl_list_add_last (arg_type, (void *) ARG_FUNCTION);
          gl_list_add_last (arg_arg, (const void *) astr_new_cstr (optarg));
          gl_list_add_last (arg_line, (void *) 0);
          break;
        case 2:
          {
            gl_list_add_last (arg_type, (void *) ARG_LOADFILE);
            astr as = astr_new_cstr (optarg);
            expand_path (as);
            gl_list_add_last (arg_arg, (const void *) astr_cstr (as));
            gl_list_add_last (arg_line, (void *) 0);
            break;
          }
        case 3:
          printf ("Usage: %s [OPTION-OR-FILENAME]...\n"
                  "\n"
                  "Run " PACKAGE_NAME ", the lightweight Emacs clone.\n"
                  "\n",
                  argv[0]);
#define D(text)                                 \
          printf (text "\n");
#define O(longname, shortname, arg, argstring, docstring)               \
          shortopt = xasprintf (", -%c", shortname);                    \
          buf = xasprintf ("--%s%s %s", longname, shortname ? shortopt : "", argstring); \
          printf ("%-24s%s\n", buf, docstring);
#define A(argstring, docstring) \
          printf ("%-24s%s\n", argstring, docstring);
#include "tbl_opts.h"
#undef D
#undef O
#undef A
          printf ("\n"
                  "Report bugs to " PACKAGE_BUGREPORT ".\n");
          exit (EXIT_SUCCESS);
        case 4:
          printf (ZILE_VERSION_STRING "\n"
                  ZILE_COPYRIGHT_STRING "\n"
                  "GNU " PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
                  "You may redistribute copies of " PACKAGE_NAME "\n"
                  "under the terms of the GNU General Public License.\n"
                  "For more information about these matters, see the file named COPYING.\n");
          exit (EXIT_SUCCESS);
        case 5:
          if (*optarg == '+')
            line = strtoul (optarg + 1, NULL, 10);
          else
            {
              gl_list_add_last (arg_type, (void *) ARG_FILE);
              astr as = astr_new_cstr (optarg);
              expand_path (as);
              gl_list_add_last (arg_arg, (const void *) astr_cstr (as));
              gl_list_add_last (arg_line, (void *) line);
              line = 1;
            }
          break;
        default:
          break;
        }
    }

  signal_init ();

  setlocale (LC_ALL, "");

  init_default_bindings ();
  init_minibuf ();

  term_init ();

  /* Create the `*scratch*' buffer, so that initialisation commands
     that act on a buffer have something to act on. */
  create_scratch_window ();
  Buffer *scratch_bp = cur_bp;
  bprintf ("%s", "\
;; This buffer is for notes you don't want to save.\n\
;; If you want to create a file, visit that file with C-x C-f,\n\
;; then enter the text in that file's own buffer.\n\
\n");
  set_buffer_modified (cur_bp, false);

  if (!qflag)
    {
      astr as = get_home_dir ();
      if (as)
        lisp_loadfile (astr_cstr (astr_cat_cstr (as, "/." PACKAGE)));
    }

  /* Create the splash buffer & message only if no files, function or
     load file is specified on the command line, and there has been no
     error. */
  if (gl_list_size (arg_arg) == 0 && minibuf_no_error () &&
      !get_variable_bool ("inhibit-splash-screen"))
    {
      Buffer *bp = create_auto_buffer ("*GNU " PACKAGE_NAME "*");
      switch_to_buffer (bp);
      bprintf ("%s", splash_str);
      set_buffer_readonly (bp, true);
      FUNCALL (beginning_of_buffer);
    }

  /* Load files and load files and run functions given on the command
     line. */
  bool ok = true;
  for (size_t i = 0; ok && i < gl_list_size (arg_arg); i++)
    {
      const char *arg = (const char *) gl_list_get_at (arg_arg, i);

      switch ((ptrdiff_t) gl_list_get_at (arg_type, i))
        {
        case ARG_FUNCTION:
          {
            le *res = execute_function (arg, 1, false);
            if (res == NULL)
              minibuf_error ("Function `%s' not defined", arg);
            ok = res == leT;
            break;
          }
        case ARG_LOADFILE:
          ok = lisp_loadfile (arg);
          if (!ok)
            minibuf_error ("Cannot open load file: %s\n", arg);
          break;
        case ARG_FILE:
          {
            ok = find_file (arg);
            if (ok)
              FUNCALL_ARG (goto_line, (size_t) gl_list_get_at (arg_line, i));
          }
          break;
        default:
          break;
        }
      if (thisflag & FLAG_QUIT)
        break;
    }
  lastflag |= FLAG_NEED_RESYNC;

  /* Set up screen according to number of files loaded. */
  Buffer *last_bp = NULL;
  int c = 0;
  for (Buffer *bp = head_bp; bp; bp = get_buffer_next (bp))
    {
      /* Last buffer that isn't *scratch*. */
      if (get_buffer_next (bp) && !get_buffer_next (get_buffer_next (bp)))
        last_bp = bp;
      c++;
    }
  if (c == 3)
    { /* *scratch* and two files. */
      FUNCALL (split_window);
      switch_to_buffer (last_bp);
      FUNCALL (other_window);
    }
  else if (c > 3) /* More than two files. */
    FUNCALL (list_buffers);

  /* Reinitialise the scratch buffer to catch settings */
  init_buffer (scratch_bp);

  /* Refresh minibuffer in case there was an error that couldn't be
     written during startup */
  minibuf_refresh ();

  /* Run the main loop. */
  while (!(thisflag & FLAG_QUIT))
    {
      if (lastflag & FLAG_NEED_RESYNC)
        window_resync (cur_wp);
      get_and_run_command ();
    }

  /* Tidy and close the terminal. */
  term_finish ();

  return EXIT_SUCCESS;
}
