/*	$Id: ncurses_redisplay.c,v 1.8 2004/01/21 02:12:32 dacap Exp $	*/

/*
 * Copyright (c) 1997-2003 Sandro Sigala.  All rights reserved.
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

/*
 * ncurses redisplay engine.
 *
 * This redisplay engine is simple since ncurses does all the work of
 * finding what is changed on the screen and updating when is needed.
 */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "zile.h"
#include "extern.h"

#include "term_ncurses.h"

#ifdef __FreeBSD__
/*
 * XXX a redundant refresh() call fixes a refresh bug under
 * FreeBSD with ncurses 1.8.6 (may be also required under others OSs).
 */
#define NEED_REDUNDANT_REFRESH
#endif

/*
 * The cached variables.
 */
#if ENABLE_NONTEXT_MODES
static chtype font_character;
static chtype font_character_delimiters;
static chtype font_comment;
static chtype font_directive;
static chtype font_here_document;
static chtype font_identifier;
static chtype font_keyword;
static chtype font_number;
static chtype font_other;
static chtype font_string;
static chtype font_string_delimiters;
#if ENABLE_MAIL_MODE
static chtype font_mail[5];
static int quoting_char;
#endif
#endif /* ENABLE_NONTEXT_MODES */
static int highlight_region;
static chtype status_line_color;
static int display_time;
static char *display_time_format;
static char *displayable_characters;
static int show_eob_marker;

static int is_displayable[256];
static int cur_tab_width;
static int point_start_column;
static int point_screen_column;

static chtype strtochtype(char *s)
{
	switch (*s) {
	case 'b':
		if (!strcmp(s, "black"))
			return C_FG_BLACK;
		else if (!strcmp(s, "blue"))
			return C_FG_BLUE;
		return C_FG_WHITE;
	case 'c':
		if (!strcmp(s, "cyan"))
			return C_FG_CYAN;
		return C_FG_WHITE;
	case 'g':
		if (!strcmp(s, "green"))
			return C_FG_GREEN;
		return C_FG_WHITE;
	case 'l':
		if (!strncmp(s, "light-", 6))
			return strtochtype(s + 6)|A_BOLD;
		return C_FG_WHITE;
	case 'm':
		if (!strcmp(s, "magenta"))
			return C_FG_MAGENTA;
		return C_FG_WHITE;
	case 'r':
		if (!strcmp(s, "red"))
			return C_FG_RED;
		return C_FG_WHITE;
	case 'y':
		if (!strcmp(s, "yellow"))
			return C_FG_YELLOW;
		return C_FG_WHITE;
	}

	return C_FG_WHITE;
}

/*
 * Get the font color value from the user specified variables.
 */
static chtype get_font(char *font)
{
	char *s;
	if ((s = get_variable(font)) == NULL)
		return C_FG_WHITE;
	return strtochtype(s);
}

/*
 * Parse the `displayable-characters' variable and set accordingly the
 * `is_displayable[]' vector.
 *
 * The valid syntax is:
 *     set       ::= range | value ( ',' range | value )*
 *     range     ::= value '-' value
 *     value     ::= hex_value | oct_value | dec_value
 *     hex_value ::= '0x'[0-9a-fA-F]+
 *     oct_value ::= '0'[0-7]*
 *     dec_value ::= [1-9][0-9]*
 *
 * Examples of valid ranges:
 *     0-15,17,0xef-0xff
 *     012,015-0x50
 *     0x20-0x7e		(standard English-only character set)
 *     0x20-0x7e,0xa1-0xff	(typical European character set)
 */
static void parse_displayable_chars(const char *s)
{
	int i, num, in_range, range_low, range_high;

	if (s == NULL || *s == '\0') { /* Invalid set */
		for (i = 0; i < 256; ++i)
			is_displayable[i] = isprint(i) != 0;
		return;
	}

	for (i = 0; i < 256; ++i)
		is_displayable[i] = 0;
	num = in_range = range_low = range_high = 0;
	while (*s != '\0') {
		if (*s == '0') { /* Hex or Oct */
			if (*(s+1) == 'x' || *(s+1) == 'X') { /* Hex */
				s += 2;
				if (!isxdigit(*s)) /* Bogus hex number. */
					continue;
				num = 0;
				for (; isxdigit(*s); ++s) {
					num *= 16;
					if (isalpha(*s)) /* [a-fA-F] */
						num += tolower(*s) - 'a' + 10;
					else
						num += *s - '0';
				}
			} else { /* Oct */
				++s;
				num = 0;
				for (; isdigit(*s); ++s) {
					num *= 8;
					num += *s - '0';
				}
			}
		} else if (isdigit(*s)) { /* Dec */
			num = 0;
			for (; isdigit(*s); ++s) {
				num *= 10;
				num += *s - '0';
			}
		} else { /* Ignore character. */
			++s;
			continue;
		}

		/* Save the character. */
		if (in_range) {
			/* We are in a range; the `range_low' was already
			   specified before. */
			in_range = 0;
			range_high = num;
			if (range_low > range_high) {
				/* Swap bogus ranges. */
				int tmp = range_low;
				range_low = range_high;
				range_high = tmp;
			}
			/* Make displayable the characters in range. */
			for (i = range_low; i <= range_high; ++i)
				is_displayable[(unsigned char)i] = 1;
		} else if (*s == '-') {
			/* The "range_low" of a range. */
			in_range = 1;
			range_low = num;
			++s;
		} else /* A sole character not in a range. */
			is_displayable[(unsigned char)num] = 1;
	}

#if 0
	for (i = 0; i < 256; ++i)
		fprintf(stderr, "0x%x 0%o = %d\n", i, i, is_displayable[i]);
#endif
}

void ncurses_refresh_cached_variables(void)
{
#if ENABLE_NONTEXT_MODES
#if ENABLE_MAIL_MODE
	char *p;
#endif
	/*
	 * Refresh the font cache.
	 */
	font_character = get_font("font-character");
	font_character_delimiters = get_font("font-character-delimiters");
	font_comment = get_font("font-comment");
	font_directive = get_font("font-directive");
	font_here_document = get_font("font-here-document");
	font_identifier = get_font("font-identifier");
	font_keyword = get_font("font-keyword");
#if ENABLE_MAIL_MODE
	font_mail[0] = get_font("font-mail1");
	font_mail[1] = get_font("font-mail2");
	font_mail[2] = get_font("font-mail3");
	font_mail[3] = get_font("font-mail4");
	font_mail[4] = get_font("font-mail5");
#endif
	font_number = get_font("font-number");
	font_other = get_font("font-other");
	font_string = get_font("font-string");
	font_string_delimiters = get_font("font-string-delimiters");
#if ENABLE_MAIL_MODE
	p = get_variable("mail-quoting-char");
	if (p != NULL && strlen(p) > 0)
		quoting_char = *p;
	else
		quoting_char = '>';
#endif
#endif /* ENABLE_NONTEXT_MODES */
	highlight_region = lookup_bool_variable("highlight-region");
	status_line_color = get_font("status-line-color");
	display_time = lookup_bool_variable("display-time");
	display_time_format = get_variable("display-time-format");
	displayable_characters = get_variable("displayable-characters");
	parse_displayable_chars(displayable_characters);
	show_eob_marker = lookup_bool_variable("show-eob-marker");

	refresh_clock_timeout ();
}

static int make_char_printable(char *buf, unsigned int c)
{
	if (c == '\0') {
		strcpy(buf, "^@");
		return 2;
	}

	if (c <= '\32')
		sprintf(buf, "^%c", 'A' + c - 1);
	else
		sprintf(buf, "\\%o", c & 255);

	/*
	 * We could use the returned value of sprintf to get the length
	 * of the string, but on some platforms the sprintf function is
	 * broken and the returned value is not correct.
	 */
	return strlen(buf);
}

static int pc_to_ascii[256] =
{
	0x20, 0x4f, 0x4f, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f,
	0x6f, 0x6f, 0x6f, 0x6f, 0x21, 0x21, 0x21, 0x2a,
	0x3e, 0x3c, 0x7c, 0x21, 0x21, 0x4f, 0x5f, 0x7c,
	0x5e, 0x56, 0x3e, 0x3c, 0x2d, 0x2d, 0x5e, 0x56,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x20,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static void outch(int c, chtype font, int *x)
{
	int j, w;
	char buf[16];

	if (*x >= COLS)
		return;

	if (c == '\t')
		for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < COLS; w--)
			addch(' ' | font), ++(*x);
	else if (is_displayable[(unsigned char)c])
		addch(pc_to_ascii[(unsigned char)c] | font), ++(*x);
	else {
		j = make_char_printable(buf, c);
		for (w = 0; w < j && *x < COLS; ++w)
			addch(buf[w] | font), ++(*x);
	}
}

/*
 * Structure used for calculating the highlighted region.
 *
 * We could use the `region' structure and the `calculate_region' function,
 * but this way is faster (no need to calculate the region size, for example).
 */
struct highlight_region {
	/* TRUE if highlight is enabled. */
	int	highlight;

	/* The beginning of the region. */
	linep	startp;
	int	startn;
	int	starto;

	/* The end of the region. */
	linep	endp;
	int	endn;
	int	endo;
};

static int in_region(windowp wp, linep lp, int lineno, int x,
		     struct highlight_region *d)
{
	if (lineno >= d->startn && lineno <= d->endn) {
		if (d->startn == d->endn) {
			if (x >= d->starto && x < d->endo)
				return TRUE;
		} else if (lineno == d->startn) {
			if (x >= d->starto)
				return TRUE;
		} else if (lineno == d->endn) {
			if (x < d->endo)
				return TRUE;
		} else {
			return TRUE;
		}
	}

	return FALSE;
}

static void draw_end_of_line(int line, windowp wp, linep lp,
			     int lineno, struct highlight_region *d,
			     int x, int i)
{
	if (x >= COLS) {
		mvaddch(line, COLS-1, '>' | C_FG_GREEN | A_BOLD);
	} else if (d->highlight) {
		for (; x < wp->ewidth; ++i) {
			if (in_region(wp, lp, lineno, i, d))
				outch(' ', C_FG_WHITE_BG_BLUE, &x);
			else
				x++;
		}
	}
}

static void draw_line(int line, int startcol, windowp wp, linep lp,
		      int lineno, struct highlight_region *d)
{
	int x, j;

	move(line, 0);
	for (x = 0, j = startcol; j < lp->size && x < wp->ewidth; ++j) {
		if (d->highlight && in_region(wp, lp, lineno, j, d))
			outch(lp->text[j], C_FG_WHITE_BG_BLUE, &x);
		else
			outch(lp->text[j], C_FG_WHITE, &x);
	}

	draw_end_of_line(line, wp, lp, lineno, d, x, j);
}

#if ENABLE_NONTEXT_MODES
/*
 * This hack is required for Font Lock because the full line must
 * be parsed always also when is displayed only one truncated fraction.
 */

#define OUTCH(c, font)							    \
do {									    \
	if (i >= startcol) {						    \
		if (d->highlight && in_region(wp, lp, lineno, i, d))	    \
			outch(c, (font & A_BOLD) | C_FG_WHITE_BG_BLUE, &x); \
		else							    \
			outch(c, font, &x);				    \
	}								    \
} while (0)
#endif

#if ENABLE_CLIKE_MODES
/*
 * Draw a line on the screen with font lock color.
 * C/C++/C#/Java Mode.
 */
static void draw_line_cpp(int line, int startcol, windowp wp, linep lp,
			  int lineno, struct highlight_region *d,
			  int *lastanchor)
{
	int i, x, c, nonspace = FALSE;

	move(line, 0);
	for (x = i = 0; i < lp->size; ++i) {
		c = lp->text[i];

		/* Comment handling. */
		if (lp->anchors[i] == ANCHOR_END_COMMENT) {
			if (*lastanchor == ANCHOR_BEGIN_COMMENT) {
				OUTCH(c, font_comment);
				*lastanchor = ANCHOR_NULL;
			} else
				OUTCH(c, font_other);
		} else if (lp->anchors[i] == ANCHOR_BEGIN_COMMENT) {
			OUTCH(c, font_comment);
			*lastanchor = ANCHOR_BEGIN_COMMENT;
		} else if (*lastanchor == ANCHOR_BEGIN_COMMENT)
			OUTCH(c, font_comment);

		/* String handling. */
		else if (lp->anchors[i] == ANCHOR_END_STRING) {
			OUTCH(c, font_string_delimiters);
			*lastanchor = ANCHOR_NULL;
		} else if (lp->anchors[i] == ANCHOR_BEGIN_STRING) {
			OUTCH(c, font_string_delimiters);
			*lastanchor = ANCHOR_BEGIN_STRING;
		} else if (*lastanchor == ANCHOR_BEGIN_STRING)
			OUTCH(c, font_string);

		/* Other tokens handling. */
		else if (isalpha(c) || c == '_') {
			int j;
			chtype attr;

			j = i;
			while (j < lp->size && (isalnum(lp->text[j]) || lp->text[j] == '_'))
				++j;
			if (0) {} /* Hack ;) */
#if ENABLE_C_MODE
			else if (wp->bp->mode == BMODE_C &&
				 is_c_keyword(lp->text + i, j-i) != NULL)
				attr = font_keyword;
#endif
#if ENABLE_CPP_MODE
			else if (wp->bp->mode == BMODE_CPP &&
				 is_cpp_keyword(lp->text + i, j-i) != NULL)
				attr = font_keyword;
#endif
#if ENABLE_CSHARP_MODE
			else if (wp->bp->mode == BMODE_CSHARP &&
				 is_csharp_keyword(lp->text + i, j-i) != NULL)
				attr = font_keyword;
#endif
#if ENABLE_JAVA_MODE
			else if (wp->bp->mode == BMODE_JAVA &&
				 is_java_keyword(lp->text + i, j-i) != NULL)
				attr = font_keyword;
#endif
			else
				attr = font_identifier;
			for (; i < j; ++i)
				OUTCH(lp->text[i], attr);
			i--;
		} else if (isdigit(c) || c == '.') { /* Integer or float. */
			/* XXX Fix this to support floats correctly. */
			if (c == '.' && (i+1 >= lp->size || !isdigit(lp->text[i+1]))) {
			        OUTCH(c, font_other);
				continue;
			}
			do {
				OUTCH(lp->text[i], font_number);
				++i;
			} while (i < lp->size && (isxdigit(lp->text[i]) || strchr("xeE.+-lLfFuU", lp->text[i]) != NULL));
			i--;
		} else if (c == '#' && !nonspace) { /* Preprocessor directive. */
			OUTCH(lp->text[i], font_directive);
			++i;
			while (i < lp->size && isspace(lp->text[i])) {
			        OUTCH(lp->text[i], font_directive);
				++i;
			}
			while (i < lp->size && isalpha(lp->text[i])) {
				OUTCH(lp->text[i], font_directive);
				++i;
			}
			i--;
		} else if (c == '\'') { /* Character constant. */
			OUTCH(lp->text[i], font_character_delimiters);
			while (++i < lp->size && lp->text[i] != '\'')
				if (lp->text[i] == '\\') {
					OUTCH('\\', font_character);
					if (++i < lp->size)
						OUTCH(lp->text[i], font_character);
				} else
					OUTCH(lp->text[i], font_character);
			if (i < lp->size)
				OUTCH(lp->text[i], font_character_delimiters);
		} else if (c == '/' && i+1 < lp->size && lp->text[i+1] == '/') {
			/* C++ style // comment. */
			for (; i < lp->size; ++i)
				OUTCH(lp->text[i], font_comment);
		} else
			OUTCH(c, font_other);

		/* We got a non-space character; the next `#' characters will
		   not begin a directive. */
		if (!isspace(c))
			nonspace = TRUE;
	}

	draw_end_of_line(line, wp, lp, lineno, d, x, i);
}
#endif /* ENABLE_CLIKE_MODES */

#if ENABLE_SHELL_MODE
/* Contributed by David A. Capello <dacap@users.sourceforge.net> */
static char *my_strnchr(const char *text, int size, int chr)
{
	int c;
	for (c = 0; c < size; ++c, ++text)
		if (*text == chr)
			return (char *)text;
	return NULL;
}

/*
 * Draw a line on the screen with font lock color.
 * Shell Mode.
 *
 * Identifiers hightlighting contributed by
 * David A. Capello <dacap@users.sourceforge.net>
 */
static void draw_line_shell(int line, int startcol, windowp wp, linep lp,
			    int lineno, struct highlight_region *d,
			    int *lastanchor)
{
	int i, x, c;

	move(line, 0);
	for (x = i = 0; i < lp->size; ++i) {
		c = lp->text[i];

		/* ``Here documents'' "<<EOF" handling. */
		if (lp->anchors[i] == ANCHOR_END_HEREDOC) {
			OUTCH(c, font_here_document);
			*lastanchor = ANCHOR_NULL;
		} else if (lp->anchors[i] == ANCHOR_BEGIN_HEREDOC) {
			OUTCH(c, font_here_document);
			*lastanchor = ANCHOR_BEGIN_HEREDOC;
		} else if (*lastanchor == ANCHOR_BEGIN_HEREDOC) {
			OUTCH(c, font_here_document);
		}
		/* String handling. */
		else if (lp->anchors[i] == ANCHOR_END_STRING) {
			OUTCH(c, font_string_delimiters);
			*lastanchor = ANCHOR_NULL;
		} else if (lp->anchors[i] == ANCHOR_BEGIN_STRING) {
			OUTCH(c, font_string_delimiters);
			*lastanchor = ANCHOR_BEGIN_STRING;
		} else if (*lastanchor == ANCHOR_BEGIN_STRING) {
			OUTCH(c, font_string);
		} else if (c == '#') {
			/* Comment. */
			if ((i == 0) || (lp->text[i-1] != '$')) {
				for (; i < lp->size; ++i)
					OUTCH(lp->text[i], font_comment);
			} else
				OUTCH(c, font_identifier);
		} else if ((c == '$') ||
			 ((x == 0) && (my_strnchr(lp->text, lp->size, '=')))) {
			/* Identifier name. */
			int first, gotid = TRUE;

			/* $identifier */
			if (c == '$') {
				OUTCH(c, font_other);
				if (++i < lp->size) {
					c = lp->text[i];
					if (c == '$') {
						/* $$ special variable */
						OUTCH(c, font_identifier);
						gotid = FALSE;
					}
				}
			} else {
				/* Assignment: identifier=... */
				int j, oldc = c;
				for (first = TRUE, j = i; j < lp->size; ++j) {
					c = lp->text[j];
					if (c == '_' || isalpha(c) ||
					    (!first && isdigit(c))) {
						first = FALSE;
					} else if (c != '=') {
						OUTCH(oldc, font_other);
						gotid = FALSE;
						break;
					} else
						break;
				}
			}

			if (gotid) {
				for (first = TRUE; i < lp->size; ++i) {
					c = lp->text[i];
					if (c == '_' || isalpha(c) ||
					    (!first && isdigit(c))) {
						OUTCH(c, font_identifier);
						first = FALSE;
					} else {
						--i;
						break;
					}
				}
			}
		} else {
			OUTCH(c, font_other);
		}
	}

	draw_end_of_line(line, wp, lp, lineno, d, x, i);
}
#endif /* ENABLE_SHELL_MODE */

#if ENABLE_MAIL_MODE
/*
 * Draw a line on the screen with font lock color.
 * Mail Mode.
 */
static void draw_line_mail(int line, int startcol, windowp wp, linep lp,
			    int lineno, struct highlight_region *d,
			    int *lastanchor)
{
	int i, x, level;

	move(line, 0);
	for (i = level = 0; i < lp->size; ++i) {
		int c = lp->text[i];
		if (c == quoting_char) {
			++level;
			if (level > 5)
				level = 1;
		} else if (!isspace(c))
			break;
	}
	for (x = i = 0; i < lp->size; ++i) {
		if (level > 0)
			OUTCH(lp->text[i], font_mail[level - 1]);
		else
			OUTCH(lp->text[i], font_other);
	}
	if (x >= COLS)
		mvaddch(line, COLS-1, '>' | C_FG_GREEN | A_BOLD);
}
#endif /* ENABLE_MAIL_MODE */

static void calculate_highlight_region(windowp wp, struct highlight_region *tdata)
{
	if (!highlight_region || !(thisflag & FLAG_HIGHLIGHT_REGION) ||
	    wp != cur_wp || wp->bp->markp == NULL) {
		tdata->highlight = FALSE;
		return;
	}

	tdata->highlight = TRUE;
	tdata->startp = wp->pointp;
	tdata->startn = wp->pointn;
	tdata->starto = wp->pointo;
	tdata->endp = wp->bp->markp;
	tdata->endn = calculate_mark_lineno(wp);
	tdata->endo = wp->bp->marko;
	if (tdata->endn < tdata->startn ||
	    (tdata->endn == tdata->startn && tdata->endo <= tdata->starto)) {
		linep lp;
		int n, o;
		lp = tdata->startp;
		n = tdata->startn;
		o = tdata->starto;
		tdata->startp = tdata->endp;
		tdata->startn = tdata->endn;
		tdata->starto = tdata->endo;
		tdata->endp = lp;
		tdata->endn = n;
		tdata->endo = o;
	}
}

static void draw_window(int topline, windowp wp)
{
	int i, startcol, lineno;
#if ENABLE_NONTEXT_MODES
        int lastanchor;
#endif
	linep lp;
	struct highlight_region tdata;

	calculate_highlight_region(wp, &tdata);

	/*
	 * Find the first line to display on the first screen line.
	 */
	for (lp = wp->pointp, lineno = wp->pointn, i = wp->topdelta;
	     i > 0 && lp->prev != wp->bp->limitp; lp = lp->prev, --i, --lineno)
		;

#if ENABLE_NONTEXT_MODES
	if (wp->bp->flags & BFLAG_FONTLOCK)
		lastanchor = find_last_anchor(wp->bp, lp->prev);
#endif

	cur_tab_width = wp->bp->tab_width;

	/*
	 * Draw the window lines.
	 */
	for (i = topline; i < wp->eheight + topline; ++i, ++lineno) {
		/* Clear the line. */
		move(i, 0);
		clrtoeol();
		/* If at the end of the buffer, don't write any text. */
		if (lp == wp->bp->limitp)
			continue;
		if (lp == cur_wp->pointp)
			startcol = point_start_column;
		else
			startcol = 0;
#if ENABLE_NONTEXT_MODES
		if (wp->bp->flags & BFLAG_FONTLOCK && lp->anchors != NULL) {
			switch (wp->bp->mode) {
			case BMODE_C:
			case BMODE_CPP:
			case BMODE_CSHARP:
			case BMODE_JAVA:
#if ENABLE_CLIKE_MODES
				draw_line_cpp(i,  startcol, wp, lp,
					      lineno, &tdata, &lastanchor);
#endif
				break;
			case BMODE_SHELL:
#if ENABLE_SHELL_MODE
				draw_line_shell(i, startcol, wp, lp,
						lineno, &tdata, &lastanchor);
#endif
				break;
			default:
				draw_line(i, startcol, wp, lp, lineno, &tdata);
			}
		}
#if ENABLE_MAIL_MODE
		else if (wp->bp->mode == BMODE_MAIL &&
			   wp->bp->flags & BFLAG_FONTLOCK)
			draw_line_mail(i, startcol, wp, lp,
				       lineno, &tdata, &lastanchor);
#endif
		else
#endif /* ENABLE_NONTEXT_MODES */
			draw_line(i, startcol, wp, lp, lineno, &tdata);

		/*
		 * Draw the `[EOB]' end of buffer marker if
		 * the `show-eob-marker' variable is enabled
		 * and the marker is on the current page.
		 */
		if (lp->next == wp->bp->limitp
		    && !(wp->bp->flags & BFLAG_NOEOB) && show_eob_marker) {
			static chtype eob_str[] = {
				'['|C_FG_CYAN|A_BOLD,
				'E'|C_FG_CYAN,
				'O'|C_FG_CYAN,
				'B'|C_FG_CYAN,
				']'|C_FG_CYAN|A_BOLD,
				'\0'
			};
			int y, x;
			getyx(stdscr, y, x);
			addchnstr(eob_str, COLS-x > 5 ? 5 : COLS-x);
		}

		lp = lp->next;
	}
}

static char *make_mode_line_flags(windowp wp)
{
	static char buf[3];

	if ((wp->bp->flags & (BFLAG_MODIFIED | BFLAG_READONLY)) == (BFLAG_MODIFIED | BFLAG_READONLY))
		buf[0] = '%', buf[1] = '*';
	else if (wp->bp->flags & BFLAG_MODIFIED)
		buf[0] = buf[1] = '*';
	else if (wp->bp->flags & BFLAG_READONLY)
		buf[0] = buf[1] = '%';
	else
		buf[0] = buf[1] = '-';

	return buf;
}

/*
 * This function calculates the best start column to draw if the line
 * needs to get truncated.
 * Called only for the line where is the point.
 */
static void calculate_start_column(windowp wp)
{
	int col = 0, lastcol = 0, t = wp->bp->tab_width;
	int rpfact, lpfact;
	char buf[16], *rp, *lp, *p;

	rp = wp->pointp->text + wp->pointo;
	rpfact = wp->pointo / (wp->ewidth / 3);

	for (lp = rp; lp >= cur_wp->pointp->text; --lp) {
		for (col = 0, p = lp; p < rp; ++p)
			if (*p == '\t') {
				col |= t - 1;
				++col;
			} else if (is_displayable[(unsigned char)*p])
				++col;
			else
				col += make_char_printable(buf, *p);

		lpfact = (lp - cur_wp->pointp->text) / (wp->ewidth / 3);

		if (col >= wp->ewidth - 1 || lpfact < (rpfact - 2)) {
			point_start_column = lp + 1 - cur_wp->pointp->text;
			point_screen_column = lastcol;
			return;
		}

		lastcol = col;
	}

	point_start_column = 0;
	point_screen_column = col;
}

static char *make_screen_pos(windowp wp)
{
	static char buf[16];
	if (wp->bp->num_lines <= wp->eheight && wp->topdelta == wp->pointn)
		strcpy(buf, "All");
	else if (wp->pointn == wp->topdelta)
		strcpy(buf, "Top");
	else if (wp->pointn + (wp->eheight - wp->topdelta) > wp->bp->num_lines)
		strcpy(buf, "Bot");
	else
		sprintf(buf, "%2d%%", (int)((float)wp->pointn / wp->bp->num_lines * 100));
	return buf;
}

/*
 * Format the time section of status line.
 */
static char *make_time_str(char *buf)
{
	const char *fmt;
	time_t t;
	if ((fmt = display_time_format) == NULL)
		fmt = "%I:%M:%p";
	time(&t);
	strftime(buf, 80, fmt, localtime(&t));
	return buf;
}

static void draw_status_line(int line, windowp wp)
{
	int i;
	char *mode;

	attrset(A_REVERSE | status_line_color);

	move(line, 0);
	for (i = 0; i < wp->ewidth; ++i)
		addch('-');

	switch (wp->bp->mode) {
#if ENABLE_C_MODE
	case BMODE_C:
		mode = "C";
		break;
#endif
#if ENABLE_CPP_MODE
	case BMODE_CPP:
		mode = "C++";
		break;
#endif
#if ENABLE_CSHARP_MODE
	case BMODE_CSHARP:
		mode = "C#";
		break;
#endif
#if ENABLE_JAVA_MODE
	case BMODE_JAVA:
		mode = "Java";
		break;
#endif
#if ENABLE_SHELL_MODE
	case BMODE_SHELL:
		mode = "Shell-script";
		break;
#endif
#if ENABLE_MAIL_MODE
	case BMODE_MAIL:
		mode = "Mail";
		break;
#endif
	default:
		mode = "Text";
	}

	move(line, 0);
	printw("--%2s-Zile: %-18s (%s%s%s%s%s)--L%d--C%d--%s",
	       make_mode_line_flags(wp),
	       wp->bp->name, mode,
	       /* (wp->bp->flags & BFLAG_FONTLOCK) ? " Font" : "", */
	       (wp->bp->flags & BFLAG_AUTOFILL) ? " Fill" : "",
	       (wp->bp->flags & BFLAG_OVERWRITE) ? " Ovwrt" : "",
	       (thisflag & FLAG_DEFINING_MACRO) ? " Def" : "",
	       (wp->bp->flags & BFLAG_ISEARCH) ? " Isearch" : "",
	       wp->pointn+1, get_text_goalc(wp),
	       make_screen_pos(wp));

	if (display_time) {
		char buf[64];
		make_time_str(buf);
		mvaddstr(line, wp->ewidth - strlen(buf) - 2, buf);
	}

	attrset(0);
}

static void do_redisplay(void)
{
	int topline, cur_topline = 0;
	windowp wp;

	topline = 0;

	calculate_start_column(cur_wp);

	for (wp = head_wp; wp != NULL; wp = wp->next) {
		if (wp == cur_wp)
			cur_topline = topline;

		draw_window(topline, wp);

#ifdef NEED_REDUNDANT_REFRESH
		refresh();
#endif

		/*
		 * Draw the status line only if there is available space
		 * after the buffer text space.
		 */
		if (wp->fheight - wp->eheight > 0)
			draw_status_line(topline + wp->eheight, wp);

		topline += wp->fheight;
	}

	if (point_start_column > 0)
		mvaddch(cur_topline + cur_wp->topdelta, 0, '<' | C_FG_GREEN | A_BOLD);
	move(cur_topline + cur_wp->topdelta, point_screen_column);
}

void ncurses_redisplay(void)
{
	do_redisplay();
}

void ncurses_full_redisplay(void)
{
	clear();
	do_redisplay();
}
