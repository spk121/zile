/* Main types and definitions
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2007 Reuben Thomas.
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

#ifndef ZILE_H
#define ZILE_H

#include <stdlib.h>
#include <limits.h>

#include "list.h"
#include "astr.h"
#include "vector.h"
#include "parser.h"
#include "eval.h"

#undef TRUE
#define TRUE                            1
#undef FALSE
#define FALSE                           0

#undef min
#define min(a, b)                       ((a) < (b) ? (a) : (b))
#undef max
#define max(a, b)                       ((a) > (b) ? (a) : (b))

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/*
 * The forward opaque types used in the editor.
 */
typedef struct Point Point;
typedef struct Marker Marker;
typedef struct list_s Line;   /* This is evil! */
typedef struct Undo Undo;
typedef struct Region Region;
typedef struct Buffer Buffer;
typedef struct Window Window;
typedef struct Completion Completion;
typedef struct History History;
typedef struct Terminal Terminal;

/*
 * The type of a Zile exported function.  `uniarg' is the number of
 * times to repeat the function.
 */
typedef int (*Function)(int uniarg, le *list);

/* Line.
 * A line is a list whose items are astrs. The newline at the end of
 * each line is implicit.
 */

/* Point and Marker. */
struct Point {
  Line *p;                      /* Line pointer. */
  size_t n;                     /* Line number. */
  size_t o;                     /* Offset. */
};

struct Marker {
  Buffer *bp;             /* Buffer that points into. */
  Point pt;               /* Point position. */
  Marker *next;           /* Used to chain all markers in the buffer. */
};

/* Undo delta types. */
enum {
  UNDO_INSERT_BLOCK,      /* Insert a block of characters. */
  UNDO_REMOVE_CHAR,       /* Remove a character. */
  UNDO_REMOVE_BLOCK,      /* Remove a block of characters. */
  UNDO_REPLACE_CHAR,      /* Replace a character. */
  UNDO_REPLACE_BLOCK,     /* Replace a block of characters. */
  UNDO_START_SEQUENCE,    /* Start a multi operation sequence. */
  UNDO_END_SEQUENCE,      /* End a multi operation sequence. */
  UNDO_INTERCALATE_CHAR   /* Insert a char without moving the
                             current pointer. */
};

struct Undo {
  /* Next undo delta in list. */
  Undo *next;

  /* The type of undo delta. */
  int type;

  /* Where the undo delta need to be applied.
     Warning!: Do not use the "pt.p" field. */
  Point pt;

  /* Flag indicating that reverting this undo leaves the buffer
     in an unchanged state. */
  int unchanged;

  /* The undo delta. */
  union {
    /* The character to insert or replace. */
    int c;

    /* The block to insert. */
    struct {
      char *text;
      size_t osize;      /* Original size; only for replace. */
      size_t size;       /* New block size. */
    } block;
  } delta;
};

struct Region {
  Point start;            /* The region start. */
  Point end;              /* The region end. */
  size_t size;                /* The region size. */

  /* The total number of lines ('\n' newlines) in region. */
  int num_lines;
};

/* Buffer flags or minor modes. */

#define BFLAG_MODIFIED  (0000001) /* The buffer has been modified. */
#define BFLAG_NOSAVE    (0000002) /* The buffer need not to be saved. */
#define BFLAG_NEEDNAME  (0000004) /* On save, ask for a file name. */
#define BFLAG_TEMPORARY (0000010) /* The buffer is a temporary buffer. */
#define BFLAG_READONLY  (0000020) /* The buffer cannot be modified. */
#define BFLAG_OVERWRITE (0000040) /* The buffer is in overwrite mode. */
#define BFLAG_BACKUP    (0000100) /* The old file has already been
                                     backed up. */
#define BFLAG_NOUNDO    (0000200) /* Do not record undo informations. */
#define BFLAG_AUTOFILL  (0000400) /* The buffer is in Auto Fill mode. */
#define BFLAG_ISEARCH   (0001000) /* The buffer is in Isearch loop. */

/* Formats of end-of-line. */
extern char coding_eol_lf[3];
extern char coding_eol_crlf[3];
extern char coding_eol_cr[3];
/* This value is used to signal that the type is not yet decided. */
extern char coding_eol_undecided[3];

struct Buffer {
  /* The next buffer in buffer list. */
  Buffer *next;

  /* The lines of text. */
  Line *lines;

  /* The point. */
  Point pt;

  /* The mark. */
  Marker *mark;

  /* Markers (points that are updated when text is modified).  */
  Marker *markers;

  /* The undo deltas recorded for this buffer. */
  Undo *next_undop;
  Undo *last_undop;

  /* Buffer flags. */
  int flags;
  int mark_active;

  /* Buffer-local variables. */
  le *vars;

  /* The total number of lines in the buffer. */
  size_t num_lines;

  /* The name of the buffer and the file name. */
  char *name;
  char *filename;

  /* EOL string (up to 2 chars) for this buffer. */
  char *eol;
};

struct Window {
  /* The next window in window list. */
  Window *next;

  /* The buffer displayed in window. */
  Buffer *bp;

  /* The top line delta and last point line number. */
  size_t topdelta;
  int lastpointn;

  /* The start column of the window (>0 if scrolled sideways) */
  size_t start_column;

  /* The point line pointer, line number and offset (used to
     hold the point in non-current windows). */
  Marker *saved_pt;

  /* The formal and effective width and height of window. */
  size_t fwidth, fheight;
  size_t ewidth, eheight;
};

enum {
  COMPLETION_NOTMATCHED,
  COMPLETION_MATCHED,
  COMPLETION_MATCHEDNONUNIQUE,
  COMPLETION_NONUNIQUE
};

struct Completion {
  /* This flag is set when the vector is sorted. */
  int fl_sorted;
  /* This flag is set when a completion window has been popped up. */
  int fl_poppedup;

  /* This flag is set when the completion window should be closed. */
  int fl_close;
  /* The old buffer. */
  Buffer *old_bp;

  /* This flag is set when this is a filename completion. */
  int fl_dir;
  astr path;

  /* This flag is set when the space character is allowed. */
  int fl_space;

  list completions;             /* The completions list. */

  list matches;                 /* The matches list. */
  char *match;                  /* The match buffer. */
  size_t matchsize;             /* The match buffer size. */
};

struct History {
  list elements;                /* Elements (strings).  */
  list sel;
};

typedef struct Macro {
  size_t nkeys;                 /* The number of keystrokes. */
  size_t *keys;                 /* Vector of keystrokes. */
  char *name;                   /* Name of the macro. */
  struct Macro *next;           /* Next macro in the list. */
} Macro;

/* Type of font attributes */
typedef size_t Font;

/* Zile font codes
 * Designed to fit in an int, leaving room for a char underneath. */
#define FONT_NORMAL		0x000
#define FONT_REVERSE		0x100

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED                  0001
#define GETKEY_UNFILTERED               0002

/* Special value returned in non blocking mode, when no key is pressed. */
#define KBD_NOKEY                       UINT_MAX

/* Key modifiers. */
#define KBD_CTRL                        01000
#define KBD_META                        02000

/* Common non-alphanumeric keys. */
#define KBD_CANCEL                      (KBD_CTRL | 'g')
#define KBD_TAB                         00402
#define KBD_RET                         00403
#define KBD_PGUP                        00404
#define KBD_PGDN                        00405
#define KBD_HOME                        00406
#define KBD_END                         00407
#define KBD_DEL                         00410
#define KBD_BS                          00411
#define KBD_INS                         00412
#define KBD_LEFT                        00413
#define KBD_RIGHT                       00414
#define KBD_UP                          00415
#define KBD_DOWN                        00416
#define KBD_F1                          00420
#define KBD_F2                          00421
#define KBD_F3                          00422
#define KBD_F4                          00423
#define KBD_F5                          00424
#define KBD_F6                          00425
#define KBD_F7                          00426
#define KBD_F8                          00427
#define KBD_F9                          00430
#define KBD_F10                         00431
#define KBD_F11                         00432
#define KBD_F12                         00433

/*--------------------------------------------------------------------------
 * Global flags.
 *--------------------------------------------------------------------------*/

/* The last command was a C-p or a C-n. */
#define FLAG_DONE_CPCN                  0000001
/* The last command was a kill. */
#define FLAG_DONE_KILL                  0000002
/* Hint for the redisplay engine: a resync is required. */
#define FLAG_NEED_RESYNC                0000004
/* Quit the editor as soon as possible. */
#define FLAG_QUIT_ZILE                  0000010
/* The last command modified the universal argument variable `uniarg'. */
#define FLAG_SET_UNIARG                 0000020
/* We are defining a macro. */
#define FLAG_DEFINING_MACRO             0000040

/*--------------------------------------------------------------------------
 * Miscellaneous stuff.
 *--------------------------------------------------------------------------*/

/* Ensure PATH_MAX is defined */
#ifndef PATH_MAX
#ifdef _POSIX_PATH_MAX
#define PATH_MAX	_POSIX_PATH_MAX
#else
/* Guess if all else fails */
#define PATH_MAX	254
#endif
#endif

/* Avoid warnings about unused `uniarg' parameters. */
#undef GCC_UNUSED
#ifdef __GNUC__
#define GCC_UNUSED __attribute__ ((unused))
#else
#define GCC_UNUSED
#endif

/* Define an interactive function. */
#define DEFUN(zile_func, c_func) \
        int F_ ## c_func(int uniarg GCC_UNUSED, le *arglist GCC_UNUSED) \
        { \
          if (arglist && arglist->data) \
            uniarg = atoi(arglist->data);
#define END_DEFUN \
        }

/* Call an interactive function. */
#define FUNCALL(c_func)                         \
        F_ ## c_func(1, NULL)

/* Call an interactive function with a universal argument. */
#define FUNCALL_ARG(c_func, uniarg)             \
        F_ ## c_func(uniarg, NULL)

/* Default waitkey pause in ds */
#define WAITKEY_DEFAULT 20

#endif /* !ZILE_H */
