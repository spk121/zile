/*	$Id: zile.h,v 1.1 2001/01/19 22:03:07 ssigala Exp $	*/

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

#undef TRUE
#define TRUE				1
#undef FALSE
#define FALSE				0

#undef min
#define min(a, b)			((a) < (b) ? (a) : (b))
#undef max
#define max(a, b)			((a) > (b) ? (a) : (b))

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
#define UNDO_START_SEQUENCE		7
#define UNDO_END_SEQUENCE		8

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
/* The buffer is in C mode. */
#define BFLAG_CMODE			(0001000)
/* The buffer is in C++ mode. */
#define BFLAG_CPPMODE			(0002000)
/* The buffer is in autofill mode. */
#define BFLAG_AUTOFILL			(0004000)
/* Do not display the EOB marker in this buffer. */
#define BFLAG_NOEOB			(0010000)

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
	/* The number of completions in the `completions' vector. */
	int	size;
	int	maxsize;

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
	char	*path;

	/* This flag is set when the space character is allowed. */
	int	fl_space;

	/* The completions vector. */
	char	**completions;

	/* The matches vector. */
	char	**matches;
	/* The match buffer. */
	char	*match;
	/* The match buffer size. */
	int	matchsize;

	/* The action functions. */
	int	(*reread)(historyp hp, char *s);
	int	(*try)(historyp hp, char *s);
	void	(*scroll_up)(historyp hp);
	void	(*scroll_down)(historyp hp);
};

#define MINIBUF_SET_FGBG		'\1'
#define MINIBUF_SET_FG			'\2'
#define MINIBUF_SET_BG			'\3'
#define MINIBUF_UNSET			'\4'

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
	void	(*show_about)(char *splash, char *minibuf, char *waitstr);
	void	(*clear)(void);
	void	(*beep)(void);
	void	(*minibuf_write)(const char *fmt);
	char *	(*minibuf_read)(char *prompt, char *value, historyp hp);
	void	(*minibuf_clear)(void);
};

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED			(0000001)
#define GETKEY_NONBLOCKING		(0000002)
#define GETKEY_NONFILTERED		(0000004)

/* Special value returned in non blocking mode, when no key is pressed. */
#define KBD_NOKEY			(-1)

/* Special keys. */
#define KBD_PGUP			(0000001 << 8)
#define KBD_PGDN			(0000002 << 8)
#define KBD_HOME			(0000004 << 8)
#define KBD_END				(0000010 << 8)
#define KBD_DEL				(0000020 << 8)
#define KBD_BS				(0000040 << 8)
#define KBD_INS				(0000100 << 8)
#define KBD_LEFT			(0000200 << 8)
#define KBD_RIGHT			(0000400 << 8)
#define KBD_UP				(0001000 << 8)
#define KBD_DOWN			(0002000 << 8)
#define KBD_F0				(0004000 << 8)
#define KBD_F1				(KBD_F0 + 1)
#define KBD_F2				(KBD_F0 + 2)
#define KBD_F3				(KBD_F0 + 3)
#define KBD_F4				(KBD_F0 + 4)
#define KBD_F5				(KBD_F0 + 5)
#define KBD_F6				(KBD_F0 + 6)
#define KBD_F7				(KBD_F0 + 7)
#define KBD_F8				(KBD_F0 + 8)
#define KBD_F9				(KBD_F0 + 9)
#define KBD_F10				(KBD_F0 + 10)
#define KBD_F11				(KBD_F0 + 11)
#define KBD_F12				(KBD_F0 + 12)

/* Key modifiers. */
#define KBD_CTL				(0010000 << 8)
#define KBD_META			(0020000 << 8)

#define KBD_CANCEL			(KBD_CTL | 'g')
#define KBD_TAB				(KBD_CTL | 'i')
#define KBD_RET				(KBD_CTL | 'm')

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
/* Encountered an error. */
#define FLAG_GOT_ERROR			(0000100)
/* Enable the highlight region. */
#define FLAG_HIGHLIGHT_REGION		(0000200)
/* The highlight region should be left enabled. */
#define FLAG_HIGHLIGHT_REGION_STAYS	(0000400)

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
