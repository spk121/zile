/* Redisplay engine
   Copyright (c) 1997-2004 Sandro Sigala.
   Copyright (c) 2003-2004 Reuben Thomas.
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

/*	$Id: term_redisplay.c,v 1.6 2004/05/20 22:34:50 rrt Exp $	*/

#define ENABLE_FULL_HSCROLL	/* XXX make it configurable */

#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "zile.h"
#include "config.h"
#include "extern.h"
#include "term_ncurses.h"

#ifdef __FreeBSD__
/*
 * A redundant refresh() call fixes a refresh bug under FreeBSD with
 * ncurses 1.8.6 (may be also required under others OSs).
 */
#define NEED_REDUNDANT_REFRESH
#endif

/*
 * The cached variables.
 */
static Font status_line_color;
static int display_time;
static char *display_time_format;
static char *displayable_characters;

static int is_displayable[256];
static int cur_tab_width;
static int point_start_column;
static int point_screen_column;

static Font strtochtype(char *s)
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
			return strtochtype(s + 6)|ZILE_BOLD;
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
static Font get_font(char *font)
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

#if DEBUG
	for (i = 0; i < 256; ++i)
		fprintf(stderr, "0x%x 0%o = %d\n", i, i, is_displayable[i]);
#endif
}

void refresh_cached_variables(void)
{
	/*
	 * Refresh the font cache.
	 */
	status_line_color = get_font("status-line-color");
	display_time = lookup_bool_variable("display-time");
	display_time_format = get_variable("display-time-format");
	displayable_characters = get_variable("displayable-characters");
	parse_displayable_chars(displayable_characters);
}

static int make_char_printable(char **buf, unsigned int c)
{
	if (c == '\0')
                return asprintf(buf, "^@");
	else if (c <= '\32')
		return asprintf(buf, "^%c", 'A' + c - 1);
	else
		return asprintf(buf, "\\%o", c & 255);
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

static void outch(int c, Font font, int *x)
{
	int j, w;
	char *buf;

	if (*x >= ZILE_COLS)
		return;

	if (c == '\t')
		for (w = cur_tab_width - *x % cur_tab_width; w > 0 && *x < ZILE_COLS; w--)
			term_addch(' ' | font), ++(*x);
	else if (is_displayable[(unsigned char)c])
		term_addch(pc_to_ascii[(unsigned char)c] | font), ++(*x);
	else {
		j = make_char_printable(&buf, c);
		for (w = 0; w < j && *x < ZILE_COLS; ++w)
			term_addch(buf[w] | font), ++(*x);
                free(buf);
	}
}

static int in_region(int lineno, int x, Region *r)
{
	if (lineno >= r->start.n && lineno <= r->end.n) {
		if (r->start.n == r->end.n) {
			if (x >= r->start.o && x < r->end.o)
				return TRUE;
		} else if (lineno == r->start.n) {
			if (x >= r->start.o)
				return TRUE;
		} else if (lineno == r->end.n) {
			if (x < r->end.o)
				return TRUE;
		} else {
			return TRUE;
		}
	}

	return FALSE;
}

static void draw_end_of_line(int line, Window *wp, int lineno, Region *r,
                             int highlight, int x, int i)
{
	if (x >= ZILE_COLS) {
		term_move(line, ZILE_COLS-1);
                term_addch('$');
	} else if (highlight) {
		for (; x < wp->ewidth; ++i) {
			if (in_region(lineno, i, r))
				outch(' ', C_FG_WHITE_BG_BLUE, &x);
			else
				x++;
		}
	}
}

static void draw_line(int line, int startcol, Window *wp, Line *lp,
		      int lineno, Region *r, int highlight)
{
	int x, j;

	term_move(line, 0);
	for (x = 0, j = startcol; j < lp->size && x < wp->ewidth; ++j) {
		if (highlight && in_region(lineno, j, r))
			outch(lp->text[j], C_FG_WHITE_BG_BLUE, &x);
		else
			outch(lp->text[j], C_FG_WHITE, &x);
	}

	draw_end_of_line(line, wp, lineno, r, highlight, x, j);
}

static void calculate_highlight_region(Window *wp, Region *r, int *highlight)
{
	if ((wp != cur_wp
	     && !lookup_bool_variable("highlight-nonselected-windows"))
	    || (!wp->bp->mark)
	    || (!transient_mark_mode())
	    || (transient_mark_mode() && !(wp->bp->mark_active))) {
		*highlight = FALSE;
		return;
	}

	*highlight = TRUE;
	r->start = window_pt(wp);
	r->end = wp->bp->mark->pt;
	if (cmp_point(r->end, r->start) < 0)
		swap_point(&r->end, &r->start);
}

static void draw_window(int topline, Window *wp)
{
	int i, startcol, lineno;
	Line *lp;
	Region r;
	int highlight;
	Point pt = window_pt(wp);

	calculate_highlight_region(wp, &r, &highlight);

	/*
	 * Find the first line to display on the first screen line.
	 */
	for (lp = pt.p, lineno = pt.n, i = wp->topdelta;
	     i > 0 && lp->prev != wp->bp->limitp; lp = lp->prev, --i, --lineno)
		;

	cur_tab_width = wp->bp->tab_width;

	/*
	 * Draw the window lines.
	 */
	for (i = topline; i < wp->eheight + topline; ++i, ++lineno) {
		/* Clear the line. */
		term_move(i, 0);
		term_clrtoeol();
		/* If at the end of the buffer, don't write any text. */
		if (lp == wp->bp->limitp)
			continue;
#ifdef ENABLE_FULL_HSCROLL
		startcol = point_start_column;
#else
		if (lp == pt.p)
			startcol = point_start_column;
		else
			startcol = 0;
#endif
                draw_line(i, startcol, wp, lp, lineno, &r, highlight);

#ifdef ENABLE_FULL_HSCROLL
		if (point_start_column > 0) {
			term_move(i, 0);
			term_addch('$');
                }
#endif
		lp = lp->next;
	}
}

static char *make_mode_line_flags(Window *wp)
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
static void calculate_start_column(Window *wp)
{
	int col = 0, lastcol = 0, t = wp->bp->tab_width;
	int rpfact, lpfact;
	char *buf, *rp, *lp, *p;
	Point pt = window_pt(wp);

	rp = pt.p->text + pt.o;
	rpfact = pt.o / (wp->ewidth / 3);

	for (lp = rp; lp >= pt.p->text; --lp) {
		for (col = 0, p = lp; p < rp; ++p)
			if (*p == '\t') {
				col |= t - 1;
				++col;
			} else if (is_displayable[(unsigned char)*p])
				++col;
			else {
				col += make_char_printable(&buf, *p);
                                free(buf);
                        }

		lpfact = (lp - pt.p->text) / (wp->ewidth / 3);

		if (col >= wp->ewidth - 1 || lpfact < (rpfact - 2)) {
			point_start_column = lp + 1 - pt.p->text;
			point_screen_column = lastcol;
			return;
		}

		lastcol = col;
	}

	point_start_column = 0;
	point_screen_column = col;
}

static char *make_screen_pos(Window *wp, char **buf)
{
	Point pt = window_pt(wp);

	if (wp->bp->num_lines <= wp->eheight && wp->topdelta == pt.n)
		asprintf(buf, "All");
	else if (pt.n == wp->topdelta)
		asprintf(buf, "Top");
	else if (pt.n + (wp->eheight - wp->topdelta) > wp->bp->num_lines)
		asprintf(buf, "Bot");
	else
		asprintf(buf, "%2d%%", (int)((float)pt.n / wp->bp->num_lines * 100));

	return *buf;
}

/*
 * Format the time section of status line.
 */
#define TIME_STR_LEN 80
static char *make_time_str(void)
{
	const char *fmt;
        char *buf = zmalloc(TIME_STR_LEN);
	time_t t;
	if ((fmt = display_time_format) == NULL)
		fmt = "%I:%M:%p";
	time(&t);
	strftime(buf, TIME_STR_LEN, fmt, localtime(&t));
	return buf;
}

static void draw_status_line(int line, Window *wp)
{
	int i, someflag = 0;
	char *buf;
	Point pt = window_pt(wp);

	term_attrset(ZILE_REVERSE | status_line_color);

	term_move(line, 0);
	for (i = 0; i < wp->ewidth; ++i)
		term_addch('-');

	term_move(line, 0);
	term_printw("--%2s- %-18s (", make_mode_line_flags(wp), wp->bp->name);

        if (wp->bp->flags & BFLAG_AUTOFILL) {
                term_printw("Fill");
                someflag = 1;
        }
        if (wp->bp->flags & BFLAG_OVERWRITE) {
                term_printw("%sOvwrt", someflag ? " " : "");
                someflag = 1;
        }
        if (thisflag & FLAG_DEFINING_MACRO) {
                term_printw("%sDef", someflag ? " " : "");
                someflag = 1;
        }
        if (wp->bp->flags & BFLAG_ISEARCH)
                term_printw("%sIsearch", someflag ? " " : "");

        term_printw(")--L%d--C%d--%s",
	       pt.n+1, get_goalc_wp(wp),
	       make_screen_pos(wp, &buf));
        free(buf);

	if (display_time) {
		buf = make_time_str();
		term_move(line, wp->ewidth - strlen(buf) - 2);
		term_addnstr(buf, strlen(buf));
                free(buf);
	}

	term_attrset(0);
}

void term_redisplay(void)
{
	int topline, cur_topline = 0;
	Window *wp;

	topline = 0;

	calculate_start_column(cur_wp);

	for (wp = head_wp; wp != NULL; wp = wp->next) {
		if (wp == cur_wp)
			cur_topline = topline;

		draw_window(topline, wp);

#ifdef NEED_REDUNDANT_REFRESH
		term_refresh();
#endif

		/*
		 * Draw the status line only if there is available space
		 * after the buffer text space.
		 */
		if (wp->fheight - wp->eheight > 0)
			draw_status_line(topline + wp->eheight, wp);

		topline += wp->fheight;
	}

#ifndef ENABLE_FULL_HSCROLL
	if (point_start_column > 0)
		term_mvaddch(cur_topline + cur_wp->topdelta, 0, '$');
#endif
	term_move(cur_topline + cur_wp->topdelta, point_screen_column);
}

void term_full_redisplay(void)
{
	term_clear();
	term_redisplay();
}

void resize_windows(void)
{
	Window *wp;
	int hdelta = ZILE_LINES - ncurses_tp->height;

	/* Resize windows horizontally. */
	for (wp = head_wp; wp != NULL; wp = wp->next)
		wp->fwidth = wp->ewidth = ZILE_COLS;

	/* Resize windows vertically. */
	if (hdelta > 0) { /* Increase windows height. */
		for (wp = head_wp; hdelta > 0; wp = wp->next) {
			if (wp == NULL)
				wp = head_wp;
			++wp->fheight;
			++wp->eheight;
			--hdelta;
		}
	} else { /* Decrease windows height. */
		int decreased = TRUE;
		while (decreased) {
			decreased = FALSE;
			for (wp = head_wp; wp != NULL && hdelta < 0; wp = wp->next)
				if (wp->fheight > 2) {
					--wp->fheight;
					--wp->eheight;
					++hdelta;
					decreased = TRUE;
				}
		}
	}

	/*
	 * Sometimes Zile cannot reduce the windows height to a certain
	 * value (too small); take care of this case.
	 */
	ncurses_tp->width = ZILE_COLS;
	ncurses_tp->height = ZILE_LINES - hdelta;

	FUNCALL(recenter);
}

void show_splash_screen(const char *splash)
{
	int i, bold = 0, red = 0;
	const char *p;

	for (i = 0; i < ZILE_LINES - 2; ++i) {
		term_move(i, 0);
		term_clrtoeol();
	}

	term_move(0, 0);
	for (i = 0, p = splash; *p != '\0'; ++p)
		switch (*p) {
		case '%':
			term_attrset(bold ? 0: ZILE_BOLD);
			bold ^= 1;
			break;
		case '$':
			term_attrset(red ? 0: ZILE_BOLD | C_FG_RED);
			red ^= 1;
			break;
		case  '\n':
			term_move(++i, 0);
			break;
		default:
			term_addch(*p);
		}
}
