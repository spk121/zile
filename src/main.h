/* Main types and definitions

   Copyright (c) 1997-2011 Free Software Foundation, Inc.

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

#include <config.h>

#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include "xalloc.h"
#include "xvasprintf.h"
#include "size_max.h"
#include "minmax.h"
#include "hash.h"
#include "gl_xlist.h"
#include "unused-parameter.h"

#ifdef HAVE_GC_H
#include <gc.h>
#else
#include <gc/gc.h>
#endif

#include "astr.h"
#include "estr.h"
#include "lists.h"

#define ZILE_VERSION_STRING	"GNU " PACKAGE_NAME " " VERSION

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/* Opaque types. */
typedef struct Marker Marker;
typedef struct History History;
typedef struct Undo Undo;
typedef struct Macro Macro;
typedef struct Binding *Binding;
typedef struct Buffer Buffer;
typedef struct Window Window;
typedef struct Completion Completion;

/* FIXME: Types which should really be opaque. */
typedef struct Region Region;

struct Region
{
  size_t start;		/* The region start. */
  size_t end;		/* The region end. */
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
  _GL_ATTRIBUTE_PURE ty                         \
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
    p->field = field ? xstrdup (field) : NULL;                  \
  }

/*--------------------------------------------------------------------------
 * Zile commands to C bindings.
 *--------------------------------------------------------------------------*/

/*
 * The type of a Zile exported function.
 * `uniarg' is the universal argument, if any, whose presence is
 * indicated by `is_uniarg'.
 */
typedef le * (*Function) (long uniarg, bool is_uniarg, le * list);

/* Turn a bool into a Lisp boolean */
#define bool_to_lisp(b) ((b) ? leT : leNIL)

/* Define an interactive function. */
#define DEFUN(zile_func, c_func) \
  DEFUN_ARGS(zile_func, c_func, )
#define DEFUN_ARGS(zile_func, c_func, args) \
  le * F_ ## c_func (long uniarg _GL_UNUSED_PARAMETER, bool is_uniarg _GL_UNUSED_PARAMETER, le *arglist _GL_UNUSED_PARAMETER) \
  {                                                                     \
    le * ok = leT;                                                      \
    args
#define END_DEFUN    \
    return ok;       \
  }

/* Define a non-user-visible function. */
#define DEFUN_NONINTERACTIVE(zile_func, c_func) \
  DEFUN(zile_func, c_func)
#define DEFUN_NONINTERACTIVE_ARGS(zile_func, c_func, args) \
  DEFUN_ARGS(zile_func, c_func, args)

/* String argument. */
#define STR_ARG(name)                           \
  castr name = NULL;
#define STR_INIT(name)                                  \
  if (arglist && arglist->next)                         \
    {                                                   \
      if (arglist->next->data)                          \
        name = astr_new_cstr (arglist->next->data);     \
      arglist = arglist->next;                          \
    }

/* Integer argument. */
#define INT_ARG(name) \
  long name = 1;
#define INT_INIT(name) \
  if (arglist && arglist->next)                 \
    {                                           \
      const char *s = arglist->next->data;      \
      arglist = arglist->next;                  \
      if (s != NULL)                            \
        name = strtol (s, NULL, 10);            \
      if (name == LONG_MAX)                     \
        ok = leNIL;                             \
    }

/* Integer argument which can either be argument or uniarg. */
#define INT_OR_UNIARG(name) \
  long name = 1;            \
  bool noarg = false;
#define INT_OR_UNIARG_INIT(name)                             \
  INT_INIT (name)                                            \
  else                                                       \
    {                                                        \
      if (!(lastflag & FLAG_SET_UNIARG) && !is_uniarg &&     \
          (arglist == NULL || arglist->next == NULL))        \
        noarg = true;                                        \
      name = uniarg;                                         \
    }

/* Boolean argument. */
#define BOOL_ARG(name)                          \
  bool name = true;
#define BOOL_INIT(name)                         \
  if (arglist && arglist->next)                 \
    {                                           \
      const char *s = arglist->next->data;      \
      arglist = arglist->next;                  \
      if (s != NULL && STREQ (s, "nil"))        \
        name = false;                           \
    }

/* Call an interactive function. */
#define FUNCALL(c_func)                         \
  F_ ## c_func (1, false, leNIL)

/* Call an interactive function with a universal argument. */
#define FUNCALL_ARG(c_func, uniarg)             \
  F_ ## c_func (uniarg, true, leNIL)

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

/* Standard pauses in ds */
#define GETKEY_DEFAULT                  -1
#define GETKEY_DELAYED                  2000

/* Special value returned for invalid key codes, or when no key is pressed. */
#define KBD_NOKEY                       SIZE_MAX

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
#define FLAG_NEED_RESYNC	0001	/* A resync is required. */
#define FLAG_QUIT		0002	/* The user has asked to quit. */
#define FLAG_SET_UNIARG		0004	/* The last command modified the
                                           universal arg variable `uniarg'. */
#define FLAG_UNIARG_EMPTY	0010	/* Current universal arg is just C-u's
                                           with no number. */
#define FLAG_DEFINING_MACRO	0020	/* We are defining a macro. */

/* Zile font codes */
#define FONT_NORMAL		0000
#define FONT_REVERSE		0001
#define FONT_UNDERLINE		0002

/* Custom exit code */
#define EXIT_CRASH	2


/*--------------------------------------------------------------------------
 * Things that should really be in gnulib.
 *--------------------------------------------------------------------------*/

/* String comparison */
#define STREQ(a, b) (strcmp (a, b) == 0)

#endif /* !ZILE_H */
