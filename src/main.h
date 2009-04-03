/* Main types and definitions

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

#ifndef ZILE_H
#define ZILE_H

#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <lua.h>
#include <lauxlib.h>
#include "config.h"
#include "xalloc.h"
#include "size_max.h"
#include "minmax.h"
#include "gl_list.h"

#include "clue.h"
#include "astr.h"
#include "lists.h"

#define ZILE_VERSION_STRING	"GNU " PACKAGE_NAME " " VERSION

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/* Opaque types. */
typedef struct Line Line;
typedef struct Region Region;
typedef struct Marker Marker;
typedef struct History History;
typedef struct Undo Undo;
typedef struct Macro Macro;
typedef struct Binding *Binding;
typedef struct Buffer Buffer;
typedef struct Window Window;
typedef struct Completion Completion;

/* FIXME: Types which should really be opaque. */
typedef struct Point Point;

struct Point
{
  Line *p;			/* Line pointer. */
  size_t n;			/* Line number. */
  size_t o;			/* Offset. */
};

/* Undo delta types. */
enum
{
  UNDO_REPLACE_BLOCK,		/* Replace a block of characters. */
  UNDO_START_SEQUENCE,		/* Start a multi operation sequence. */
  UNDO_END_SEQUENCE		/* End a multi operation sequence. */
};

enum
{
  COMPLETION_NOTMATCHED,
  COMPLETION_MATCHED,
  COMPLETION_MATCHEDNONUNIQUE,
  COMPLETION_NONUNIQUE
};

/* Completion flags */
#define CFLAG_POPPEDUP	0000001	/* Completion window has been popped up. */
#define CFLAG_CLOSE	0000002	/* The completion window should be closed. */
#define CFLAG_FILENAME	0000004	/* This is a filename completion. */

/*--------------------------------------------------------------------------
 * Object field getter and setter generators.
 *--------------------------------------------------------------------------*/

#define GETTER(Obj, name, ty, field)            \
  ty                                            \
  get_ ## name ## _ ## field (const Obj *p)     \
  {                                             \
    return p->field;                            \
  }                                             \

#define SETTER(Obj, name, ty, field)            \
  void                                          \
  set_ ## name ## _ ## field (Obj *p, ty field) \
  {                                             \
    p->field = field;                           \
  }

#define STR_SETTER(Obj, name, field)                            \
  void                                                          \
  set_ ## name ## _ ## field (Obj *p, const char *field)        \
  {                                                             \
    free ((char *) p->field);                                   \
    p->field = field ? xstrdup (field) : NULL;                  \
  }

/*--------------------------------------------------------------------------
 * Zile commands to C bindings.
 *--------------------------------------------------------------------------*/

/*
 * The type of a Zile exported function.
 * `uniarg' is the number of times to repeat the function.
 */
typedef le * (*Function) (long uniarg, le * list);

/* Turn a bool into a Lisp boolean */
#define bool_to_lisp(b) ((b) ? leT : leNIL)

/* Define an interactive function. */
#define DEFUN(zile_func, c_func) \
        DEFUN_ARGS(zile_func, c_func, )
#define DEFUN_ARGS(zile_func, c_func, args) \
        le * F_ ## c_func (long uniarg GCC_UNUSED, le *arglist GCC_UNUSED) \
        { \
          le * ok = leT; \
          args
#define END_DEFUN \
          return ok; \
        }

/* Define a non-user-visible function. */
#define DEFUN_NONINTERACTIVE(zile_func, c_func) \
        DEFUN(zile_func, c_func)
#define DEFUN_NONINTERACTIVE_ARGS(zile_func, c_func, args) \
        DEFUN_ARGS(zile_func, c_func, args)

/* String argument. */
#define STR_ARG(name) \
        const char *name = NULL; \
        bool free_ ## name = true;
#define STR_INIT(name) \
        if (arglist && arglist->next) \
          { \
            name = arglist->next->data; \
            arglist = arglist->next; \
            free_ ## name = false; \
          }
#define STR_FREE(name) \
        if (free_ ## name) \
          free ((char *) name);

/* Integer argument. */
#define INT_ARG(name) \
        long name = 1;
#define INT_INIT(name) \
        if (arglist && arglist->next) \
          { \
            const char *s = arglist->next->data; \
            arglist = arglist->next; \
            name = strtol (s, NULL, 10); \
            if (name == LONG_MAX) \
              ok = leNIL; \
          }

/* Boolean argument. */
#define BOOL_ARG(name) \
        bool name = true;
#define BOOL_INIT(name) \
        if (arglist && arglist->next) \
          { \
            const char *s = arglist->next->data; \
            arglist = arglist->next; \
            if (strcmp (s, "nil") == 0) \
              name = false; \
          }

/* Call an interactive function. */
#define FUNCALL(c_func)                         \
        F_ ## c_func (1, NULL)

/* Call an interactive function with a universal argument. */
#define FUNCALL_ARG(c_func, uniarg)             \
        F_ ## c_func (uniarg, NULL)

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED                  0001
#define GETKEY_UNFILTERED               0002

/* Special value returned for invalid key codes, or when no key is pressed. */
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
 * Miscellaneous stuff.
 *--------------------------------------------------------------------------*/

/* Global flags, stored in thisflag and lastflag. */
#define FLAG_DONE_CPCN		0001	/* Last command was C-p or C-n. */
#define FLAG_DONE_KILL		0002	/* The last command was a kill. */
#define FLAG_NEED_RESYNC	0004	/* A resync is required. */
#define FLAG_QUIT		0010	/* The user has asked to quit. */
#define FLAG_SET_UNIARG		0020	/* The last command modified the
                                           universal arg variable `uniarg'. */
#define FLAG_UNIARG_EMPTY	0040	/* Current universal arg is just C-u's
                                           with no number. */
#define FLAG_DEFINING_MACRO	0100	/* We are defining a macro. */

/*
 * Zile font codes
 * Designed to fit in an int, leaving room for a char underneath.
 */
#define FONT_NORMAL		0x000
#define FONT_REVERSE		0x100

/* Default waitkey pause in ds */
#define WAITKEY_DEFAULT 20

/* Avoid warnings about unused parameters. */
#undef GCC_UNUSED
#ifdef __GNUC__
#define GCC_UNUSED __attribute__ ((unused))
#else
#define GCC_UNUSED
#endif

#endif /* !ZILE_H */
