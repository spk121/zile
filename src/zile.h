/*	$Id: zile.h,v 1.8 2003/05/25 21:20:12 rrt Exp $	*/

/*
 * Copyright (c) 1997-2002 Sandro Sigala.  All rights reserved.
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

#ifndef ZILE_H
#define ZILE_H

#include <alist.h>
#include "astr.h"

#undef TRUE
#define TRUE				1
#undef FALSE
#define FALSE				0

#undef min
#define min(a, b)			((a) < (b) ? (a) : (b))
#undef max
#define max(a, b)			((a) > (b) ? (a) : (b))

#ifndef PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#endif

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/*
 * The forward opaque types used in the editor.
 */
typedef struct line *linep;
typedef struct undo *undop;
typedef struct region *regionp;
typedef struct buffer *bufferp;
typedef struct window *windowp;
typedef struct history *historyp;
typedef struct terminal *terminalp;

/*
 * The type of a Zile exported function.  `uniarg' is the number of
 * times to repeat the function.
 */
typedef int (*funcp)(int uniarg);

/* Font lock anchors. */
#define ANCHOR_NULL			0
#define ANCHOR_BEGIN_COMMENT		1
#define ANCHOR_END_COMMENT		2
#define ANCHOR_BEGIN_STRING		3
#define ANCHOR_END_STRING		4
#define ANCHOR_BEGIN_SPECIAL		5
#define ANCHOR_END_SPECIAL		6

struct line {
	/* The previous line and next line pointers. */
	linep	prev, next;

	/* The used size and the allocated space size. */
	int	size;
	int	maxsize;

	/* Pointer to anchors for font lock. */
	char	*anchors;

	/*
	 * The text space label; must be the last entry of the structure!
	 *
	 * Using this trick we can avoid allocating two memory areas for each
	 * line (one for the structure and one for the contained text).
	 */
	char	text[1];
};

/* Insert a character. */
#define UNDO_INSERT_CHAR		1
/* Insert a block of characters. */
#define UNDO_INSERT_BLOCK		2
/* Remove a character. */
#define UNDO_REMOVE_CHAR		3
/* Remove a block of characters. */
#define UNDO_REMOVE_BLOCK		4
/* Replace a character. */
#define UNDO_REPLACE_CHAR		5
/* Replace a block of characters. */
#define UNDO_REPLACE_BLOCK		6
/* Start a multi operation sequence. */
#define UNDO_START_SEQUENCE		7
/* End a multi operation sequence. */
#define UNDO_END_SEQUENCE		8
/* Insert a char without moving the current pointer */
#define UNDO_INTERCALATE_CHAR		9

struct undo {
	/* Next undo delta in list. */
	undop	next;

	/* The type of undo delta. */
	int	type;

	/* Where the undo delta need to be applied. */
	int	pointn;
	int	pointo;

	/* The undo delta. */
	union {
		/* The character to insert or replace. */
		int	c;

		/* The block to insert. */
		struct {
			char	*text;
			int	osize;	/* Original size; only for replace. */
			int	size;	/* New block size. */
		}	block;
	}	delta;
};

struct region {
	/* The region start line pointer, line number and offset. */
	linep	startp;
	int	startn;
	int	starto;

	/* The region end line pointer, line number and offset. */
	linep	endp;
	int	endn;
	int	endo;

	/* The region size. */
	int	size;

	/* The total number of lines ('\n' newlines) in region. */
	int	num_lines;
};

/* Buffer flags or minor modes. */

/* The buffer has been modified. */
#define BFLAG_MODIFIED			(0000001)
/* The buffer need not to be saved. */
#define BFLAG_NOSAVE			(0000002)
/* On save, ask for a file name. */
#define BFLAG_NEEDNAME			(0000004)
/* The buffer is a temporary buffer. */
#define BFLAG_TEMPORARY			(0000010)
/* The buffer cannot be modified. */
#define BFLAG_READONLY			(0000020)
/* The buffer is in overwrite mode. */
#define BFLAG_OVERWRITE			(0000040)
/* The old file has already been backed up. */
#define BFLAG_BACKUP			(0000100)
/* The buffer has font lock mode turned on. */
#define BFLAG_FONTLOCK			(0000200)
/* Do not record undo informations. */
#define BFLAG_NOUNDO			(0000400)
/* The buffer is in Auto Fill mode. */
#define BFLAG_AUTOFILL			(0001000)
/* Do not display the EOB marker in this buffer. */
#define BFLAG_NOEOB			(0002000)

/* Mutually exclusive buffer major modes. */

/* The buffer is in Text mode. */
#define BMODE_TEXT			0
/* The buffer is in C mode. */
#define BMODE_C				1
/* The buffer is in C++ mode. */
#define BMODE_CPP			2
/* The buffer is in C# (C sharp) mode. */
#define BMODE_CSHARP			3
/* The buffer is in Java mode */
#define BMODE_JAVA			4
/* The buffer is in Shell-script mode. */
#define BMODE_SHELL			5
/* The buffer is in Mail mode */
#define BMODE_MAIL			6

struct buffer {
	/* The next buffer in buffer list. */
	bufferp	next;

	/* limitp->next == first line; limitp->prev == last line. */
	linep	limitp;

	/* The point line pointer, line number and offset. */
	linep	save_pointp;
	int	save_pointn;
	int	save_pointo;

	/* The mark line and offset. */
	linep	markp;
	int	marko;

	/* The undo deltas recorded for this buffer. */
	undop	next_undop;
	undop	last_undop;

	/* Buffer flags. */
	int	flags;
	int	mode;
	int	tab_width;
	int	fill_column;

	/* The total number of lines ('\n' newlines) in buffer. */
	int	num_lines;

	/* The number of windows that display this buffer. */
	int	num_windows;

	/* The name of the buffer and the file name. */
	char	*name;
	char	*filename;
};

struct window {
	/* The next window in window list. */
	windowp	next;

	/* The buffer displayed in window. */
	bufferp bp;

	/* The top line delta and last point line number. */
	int	topdelta;
	int	lastpointn;

	/* The point line pointer, line number and offset. */
	linep	pointp;
	int	pointn;
	int	pointo;

	/* The formal and effective width and height of window. */
	int	fwidth, fheight;
	int	ewidth, eheight;
};

#define HISTORY_NOTMATCHED		0
#define HISTORY_MATCHED			1
#define HISTORY_MATCHEDNONUNIQUE	2
#define HISTORY_NONUNIQUE		3

struct history {
	/* This flag is set when the vector is sorted. */
	int	fl_sorted;
	/* This flag is set when a completion window has been popped up. */
	int	fl_poppedup;

	/* This flag is set when the completion window should be closed. */
	int	fl_close;
	/* The old buffer. */
	bufferp	old_bp;

	/* This flag is set when this is a filename history. */
	int	fl_dir;
	astr    path;

	/* This flag is set when the space character is allowed. */
	int	fl_space;

	/* The completions list. */
	alist	completions;

	/* The matches list. */
	alist	matches;
	/* The match buffer. */
	char	*match;
	/* The match buffer size. */
	int	matchsize;

	/* The action functions. */
	int	(*reread)(historyp hp, astr as);
	int	(*try)(historyp hp, const char *s);
	void	(*scroll_up)(historyp hp);
	void	(*scroll_down)(historyp hp);
};

#define MINIBUF_SET_COLOR	'\1'
#define MINIBUF_UNSET_COLOR	'\2'

struct terminal {
	int	width, height;

	int	(*init)(void);
	int	(*open)(void);
	int	(*close)(void);
	int	(*getkey)(void);
	int	(*xgetkey)(int mode, int arg);
	int	(*ungetkey)(int c);
	void	(*refresh_cached_variables)(void);
	void	(*refresh)(void);
	void	(*redisplay)(void);
	void	(*full_redisplay)(void);
	void	(*show_about)(const char *splash, const char *minibuf);
	void	(*clear)(void);
	void	(*beep)(void);
	void	(*minibuf_write)(const char *fmt);
	char *	(*minibuf_read)(const char *prompt, const char *value, historyp hp);
	void	(*minibuf_clear)(void);
};

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED			(0001)
#define GETKEY_NONBLOCKING		(0002)
#define GETKEY_NONFILTERED		(0004)

/* Special value returned in non blocking mode, when no key is pressed. */
#define KBD_NOKEY			(-1)

/* Key modifiers. */
#define KBD_CTL				(01000)
#define KBD_META			(02000)

/* Common non-alphanumeric keys. */
#define KBD_CANCEL			(KBD_CTL | 'g')
#define KBD_TAB				(00402)
#define KBD_RET				(00403)
#define KBD_PGUP			(00404)
#define KBD_PGDN			(00405)
#define KBD_HOME			(00406)
#define KBD_END				(00407)
#define KBD_DEL				(00410)
#define KBD_BS				(00411)
#define KBD_INS				(00412)
#define KBD_LEFT			(00413)
#define KBD_RIGHT			(00414)
#define KBD_UP				(00415)
#define KBD_DOWN			(00416)
#define KBD_F1				(00420)
#define KBD_F2				(00421)
#define KBD_F3				(00422)
#define KBD_F4				(00423)
#define KBD_F5				(00424)
#define KBD_F6				(00425)
#define KBD_F7				(00426)
#define KBD_F8				(00427)
#define KBD_F9				(00430)
#define KBD_F10				(00431)
#define KBD_F11				(00432)
#define KBD_F12				(00433)

/*--------------------------------------------------------------------------
 * Global flags.
 *--------------------------------------------------------------------------*/

/* The last command was a C-p or a C-n. */
#define FLAG_DONE_CPCN			(0000001)
/* The last command was a kill. */
#define FLAG_DONE_KILL			(0000002)
/* Hint for the redisplay engine: a resync is required. */
#define FLAG_NEED_RESYNC		(0000004)
/* Quit the editor as soon as possible. */
#define FLAG_QUIT_ZILE			(0000010)
/* The last command modified the universal argument variable `uniarg'. */
#define FLAG_SET_UNIARG			(0000020)
/* We are defining a macro. */
#define FLAG_DEFINING_MACRO		(0000040)
/* We are executing a macro. */
#define FLAG_EXECUTING_MACRO		(0000100)
/* Encountered an error. */
#define FLAG_GOT_ERROR			(0000200)
/* Enable the highlight region. */
#define FLAG_HIGHLIGHT_REGION		(0000400)
/* The highlight region should be left enabled. */
#define FLAG_HIGHLIGHT_REGION_STAYS	(0001000)

/*--------------------------------------------------------------------------
 * Miscellaneous stuff.
 *--------------------------------------------------------------------------*/

/* Define an interactive function (callable with `M-x'). */
#define DEFUN(zile_func, c_func)		\
	int F_ ## c_func(int uniarg)

/* Call an interactive function. */
#define FUNCALL(c_func)				\
	F_ ## c_func(1)

/* Call an interactive function with an universal argument. */
#define FUNCALL_ARG(c_func, uniarg)		\
	F_ ## c_func(uniarg)

/*--------------------------------------------------------------------------
 * Missing functions.
 *--------------------------------------------------------------------------*/

#ifndef HAS_VASPRINTF
int vasprintf(char ** ptr, const char * format_string, va_list vargs);
#endif

#endif /* !ZILE_H */
