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

/*	$Id: term_allegro.h,v 1.1 2004/09/03 02:09:08 dacap Exp $	*/

#define ZILE_COLOR_BLACK	0
#define ZILE_COLOR_RED		1
#define ZILE_COLOR_GREEN	2
#define ZILE_COLOR_YELLOW	3
#define ZILE_COLOR_BLUE		4
#define ZILE_COLOR_MAGENTA	5
#define ZILE_COLOR_CYAN		6
#define ZILE_COLOR_WHITE	7
#define ZILE_COLOR_BLUEBG	8

extern int al_LINES, al_COLS;

#define ZILE_LINES              al_LINES
#define ZILE_COLS               al_COLS

extern Terminal *termp;

#define ZILE_REVERSE		0x1000
#define ZILE_BOLD		0x2000

#define C_FG_BLACK		0x0100
#define C_FG_RED		0x0200
#define C_FG_GREEN		0x0300
#define C_FG_YELLOW		0x0400
#define C_FG_BLUE		0x0500
#define C_FG_MAGENTA		0x0600
#define C_FG_CYAN		0x0700
#define C_FG_WHITE		0x0800
#define C_FG_WHITE_BG_BLUE	0x0900

extern void show_splash_screen(const char *splash);
extern void resize_windows(void);
extern void free_rotation_buffers(void);
