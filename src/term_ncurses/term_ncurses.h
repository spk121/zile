/*	$Id: term_ncurses.h,v 1.1 2001/01/19 22:03:37 ssigala Exp $	*/

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

#define ZILE_COLOR_BLACK	0
#define ZILE_COLOR_RED		1
#define ZILE_COLOR_GREEN	2
#define ZILE_COLOR_YELLOW	3
#define ZILE_COLOR_BLUE		4
#define ZILE_COLOR_MAGENTA	5
#define ZILE_COLOR_CYAN		6
#define ZILE_COLOR_WHITE	7

#define C_FG_BLACK		COLOR_PAIR(ZILE_COLOR_BLACK)
#define C_FG_RED		COLOR_PAIR(ZILE_COLOR_RED)
#define C_FG_GREEN		COLOR_PAIR(ZILE_COLOR_GREEN)	
#define C_FG_YELLOW		COLOR_PAIR(ZILE_COLOR_YELLOW)
#define C_FG_BLUE		COLOR_PAIR(ZILE_COLOR_BLUE)
#define C_FG_MAGENTA		COLOR_PAIR(ZILE_COLOR_MAGENTA)
#define C_FG_CYAN		COLOR_PAIR(ZILE_COLOR_CYAN)
#define C_FG_WHITE		COLOR_PAIR(ZILE_COLOR_WHITE)

extern terminalp ncurses_tp;
