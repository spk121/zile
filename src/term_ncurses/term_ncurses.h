/* Exported terminal
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

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

/*	$Id: term_ncurses.h,v 1.7 2004/05/09 18:00:34 rrt Exp $	*/

#define ZILE_COLOR_BLACK	0
#define ZILE_COLOR_RED		1
#define ZILE_COLOR_GREEN	2
#define ZILE_COLOR_YELLOW	3
#define ZILE_COLOR_BLUE		4
#define ZILE_COLOR_MAGENTA	5
#define ZILE_COLOR_CYAN		6
#define ZILE_COLOR_WHITE	7
#define ZILE_COLOR_BLUEBG	8

#define C_FG_BLACK		COLOR_PAIR(ZILE_COLOR_BLACK)
#define C_FG_RED		COLOR_PAIR(ZILE_COLOR_RED)
#define C_FG_GREEN		COLOR_PAIR(ZILE_COLOR_GREEN)
#define C_FG_YELLOW		COLOR_PAIR(ZILE_COLOR_YELLOW)
#define C_FG_BLUE		COLOR_PAIR(ZILE_COLOR_BLUE)
#define C_FG_MAGENTA		COLOR_PAIR(ZILE_COLOR_MAGENTA)
#define C_FG_CYAN		COLOR_PAIR(ZILE_COLOR_CYAN)
#define C_FG_WHITE		COLOR_PAIR(ZILE_COLOR_WHITE)
#define C_FG_WHITE_BG_BLUE	COLOR_PAIR(ZILE_COLOR_BLUEBG)

extern int LINES, COLS;

#define ZILE_LINES              LINES
#define ZILE_COLS               COLS

extern Terminal *ncurses_tp;

void term_getyx(int *y, int *x);
void term_move(int y, int x);
void term_clrtoeol(void);
void term_refresh(void);
void term_addch(char c);
