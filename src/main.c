/* Program invocation, startup and shutdown

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

#include "config.h"

#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "dirname.h"
#include "gl_linked_list.h"

#include "main.h"
#include "extern.h"

#define ZILE_COPYRIGHT_STRING \
  "Copyright (C) 2009 Free Software Foundation, Inc."

/* The executable name */
char *prog_name = PACKAGE;

/* The current window; the first window in list. */
Window *cur_wp = NULL, *head_wp = NULL;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

static char about_splash_str[] = "\
" ZILE_VERSION_STRING "\n\
\n\
" ZILE_COPYRIGHT_STRING "\n\
\n\
Type `C-x C-c' to exit " PACKAGE_NAME ".\n\
Type `C-x u' to undo changes.\n\
Type `C-g' at any time to quit the current operation.\n\
\n\
`C-x' means hold the CTRL key while typing the character `x'.\n\
`M-x' means hold the META or ALT key down while typing `x'.\n\
If there is no META or ALT key, instead press and release\n\
the ESC key and then type `x'.\n\
Combinations like `C-x u' mean first press `C-x', then `u'.\n\
";

static char about_minibuf_str[] = "Welcome to " PACKAGE_NAME "!";

static void
about_screen (void)
{
  minibuf_write (about_minibuf_str);
  if (!get_variable_bool ("inhibit-splash-screen"))
    {
      show_splash_screen (about_splash_str);
      term_refresh ();
      waitkey (20 * 10);
    }
}

static void
setup_main_screen (void)
{
  Buffer *bp, *last_bp = NULL;
  int c = 0;

  for (bp = head_bp; bp; bp = get_buffer_next (bp))
    {
      /* Last buffer that isn't *scratch*. */
      if (get_buffer_next (bp) && !get_buffer_next (get_buffer_next (bp)))
        last_bp = bp;
      c++;
    }

  /* *scratch* and two files. */
  if (c == 3)
    {
      FUNCALL (split_window);
      switch_to_buffer (last_bp);
      FUNCALL (other_window);
    }
  /* More than two files. */
  else if (c > 3)
    FUNCALL (list_buffers);
}

static void
segv_sig_handler (int signo GCC_UNUSED)
{
  fprintf (stderr,
           "%s: " PACKAGE_NAME
           " crashed.  Please send a bug report to <"
           PACKAGE_BUGREPORT ">.\r\n",
           prog_name);
  zile_exit (true);
}

static void
other_sig_handler (int signo GCC_UNUSED)
{
  fprintf (stderr, "%s: terminated with signal %d.\r\n",
           prog_name, signo);
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
  signal (SIGQUIT, other_sig_handler);
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
  size_t i, line;
  Buffer *scratch_bp;
  astr as;
  bool ok = true;

  /* Set prog_name to executable name, if available */
  if (argv[0])
    prog_name = base_name (argv[0]);

  /* Set up Lisp environment now so it's available to files and
     expressions specified on the command-line. */
  init_search ();
  init_lisp ();
  init_variables ();
  init_eval ();

  opterr = 0; /* Don't display errors for unknown options */
  for (;;)
    {
      int this_optind = optind ? optind : 1, longindex, c;
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
                   prog_name, argv[this_optind]);
          exit (1);
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
          gl_list_add_last (arg_arg, (void *) optarg);
          gl_list_add_last (arg_line, (void *) 0);
          break;
        case 2:
          gl_list_add_last (arg_type, (void *) ARG_LOADFILE);
          gl_list_add_last (arg_arg, (void *) optarg);
          gl_list_add_last (arg_line, (void *) 0);
          break;
        case 3:
          printf ("Usage: %s [OPTION-OR-FILENAME]...\n"
                  "\n"
                  "Run " PACKAGE_NAME ", the lightweight Emacs clone.\n"
                  "\n",
                  argv[0]);
#define D(text)                                 \
          printf (text "\n");
#define O(longname, shortname, arg, argstring, docstring)               \
          xasprintf (&shortopt, ", -%c", shortname);                    \
          xasprintf (&buf, "--%s%s %s", longname, shortname ? shortopt : "", argstring); \
          printf ("%-24s%s\n", buf, docstring);                         \
          free (buf);                                                   \
          free (shortopt);
#define A(argstring, docstring) \
          printf ("%-24s%s\n", argstring, docstring);
#include "tbl_opts.h"
#undef D
#undef O
#undef A
          printf ("\n"
                  "Report bugs to " PACKAGE_BUGREPORT ".\n");
          exit (0);
        case 4:
          printf (ZILE_VERSION_STRING "\n"
                  ZILE_COPYRIGHT_STRING "\n"
                  "GNU " PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
                  "You may redistribute copies of " PACKAGE_NAME "\n"
                  "under the terms of the GNU General Public License.\n"
                  "For more information about these matters, see the file named COPYING.\n");
          exit (0);
        case 5:
          if (*optarg == '+')
            line = strtoul (optarg + 1, NULL, 10);
          else
            {
              gl_list_add_last (arg_type, (void *) ARG_FILE);
              gl_list_add_last (arg_arg, (void *) optarg);
              gl_list_add_last (arg_line, (void *) line);
            }
          line = 1;
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
  create_first_window ();
  scratch_bp = cur_bp;
  as = astr_new_cstr ("\
;; This buffer is for notes you don't want to save.\n\
;; If you want to create a file, visit that file with C-x C-f,\n\
;; then enter the text in that file's own buffer.\n\
\n");
  insert_astr (as);
  astr_delete (as);
  set_buffer_modified (cur_bp, false);

  if (!qflag)
    {
      astr as = get_home_dir ();
      if (as)
        {
          astr_cat_cstr (as, "/." PACKAGE);
          lisp_loadfile (astr_cstr (as));
          astr_delete (as);
        }
    }

  /* Show the splash screen only if no files, function or load file is
     specified on the command line, and there has been no error. */
  if (minibuf_contents == NULL && gl_list_size (arg_arg) == 0)
    about_screen ();
  setup_main_screen ();

  /* Load files and load files and run functions given on the command
     line. */
  for (i = 0; ok && i < gl_list_size (arg_arg); i++)
    {
      char *arg = (char *) gl_list_get_at (arg_arg, i);

      switch ((int) gl_list_get_at (arg_type, i))
        {
        case ARG_FUNCTION:
          ok = get_function (arg);
          if (ok)
            ok = execute_function (arg, 1);
          else
            minibuf_error ("Function `%s' not defined", arg);
          break;
        case ARG_LOADFILE:
          ok = lisp_loadfile (arg);
          if (!ok)
            minibuf_error ("Cannot open load file: %s\n", arg);
          break;
        case ARG_FILE:
          {
            astr as = astr_afmt (astr_new (), "%d", line);
            le *branch = leAddDataElement (leAddDataElement (NULL, "", 0), astr_cstr (as), 0);
            ok = find_file (arg);
            if (ok)
              F_goto_line (0, branch);
            leWipe (branch);
            astr_delete (as);
            lastflag |= FLAG_NEED_RESYNC;
          }
          break;
        }
      if (thisflag & FLAG_QUIT)
        break;
    }
  gl_list_free (arg_type);
  gl_list_free (arg_arg);
  gl_list_free (arg_line);
  lastflag |= FLAG_NEED_RESYNC;

  /* Reinitialise the scratch buffer to catch settings */
  init_buffer (scratch_bp);

  /* Refresh minibuffer in case there's an error that couldn't be
     written during startup */
  if (minibuf_contents != NULL)
    {
      /* Copy minibuf_contents because minibuf_write frees it */
      char *buf = xstrdup (minibuf_contents);

      minibuf_write (buf);
      free (buf);
    }

  /* Run the main loop. */
  while (!(thisflag & FLAG_QUIT))
    {
      if (lastflag & FLAG_NEED_RESYNC)
        resync_redisplay ();
      term_redisplay ();
      term_refresh ();
      process_key (getkey ());
    }

  /* Tidy and close the terminal. */
  term_finish ();

  free_default_bindings ();
  free_eval ();

  /* Free Lisp state. */
  free_variables ();
  free_lisp ();

  /* Free all the memory allocated. */
  free_search ();
  free_kill_ring ();
  free_registers ();
  free_macros ();
  free_windows ();
  free_buffers ();
  free_minibuf ();
  free (prog_name);

  return 0;
}
