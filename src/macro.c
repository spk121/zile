/*	$Id: macro.c,v 1.1 2001/01/19 22:02:33 ssigala Exp $	*/

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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

typedef struct macro *macrop;

struct macro {
	funcp	func;
	int	uniarg;
	macrop	next;
};

static macrop head_mp, last_mp;

void cancel_kbd_macro(void)
{
	macrop mp, next_mp;

	for (mp = head_mp; mp != NULL; mp = next_mp) {
		next_mp = mp->next;
		free(mp);
	}

	head_mp = last_mp = NULL;

	thisflag &= ~FLAG_DEFINING_MACRO;
}

void add_kbd_macro(funcp func, int uniarg)
{
	macrop mp;

	mp = (macrop)xmalloc(sizeof(*mp));
	mp->func = func;
	mp->uniarg = uniarg;
	mp->next = NULL;

	if (head_mp == NULL)
		head_mp = last_mp = mp;
	else {
		last_mp->next = mp;
		last_mp = mp;
	}
}

DEFUN("start-kbd-macro", start_kbd_macro)
/*+
Record subsequent keyboard input, defining a keyboard macro.
The commands are recorded even as they are executed.
Use C-x ) to finish recording and make the macro available.
+*/
{
	if (thisflag & FLAG_DEFINING_MACRO) {
		minibuf_error("%FCAlready defining a keyboard macro%E");
		return FALSE;
	}

	if (head_mp != NULL)
		cancel_kbd_macro();

	minibuf_write("%FCDefining keyboard macro...%E");

	thisflag |= FLAG_DEFINING_MACRO;

	return TRUE;
}

DEFUN("end-kbd-macro", end_kbd_macro)
/*+
Finish defining a keyboard macro.
The definition was started by C-x (.
The macro is now available for use via C-x e.
+*/
{
	if (!(thisflag & FLAG_DEFINING_MACRO)) {
		minibuf_error("%FCNot defining a keyboard macro%E");
		return FALSE;
	}

	thisflag &= ~FLAG_DEFINING_MACRO;

	return TRUE;
}

static int call_last_kbd_macro(void)
{
	macrop mp;

	if (head_mp == NULL) {
		minibuf_error("%FCKeyboard macro not defined%E");
		return FALSE;
	}

	for (mp = head_mp; mp != NULL; mp = mp->next) {
		thisflag = 0;
		mp->func(mp->uniarg);
		if (lastflag & FLAG_GOT_ERROR)
			return FALSE;
		lastflag = thisflag;
	}

	return TRUE;
}

DEFUN("call-last-kbd-macro", call_last_kbd_macro)
/*+
Call the last keyboard macro that you defined with C-x (.
A prefix argument serves as a repeat count.  Zero means repeat until error.
+*/
{
	int uni;

	if (uniarg == 0) {
		for (;;)
			if (!call_last_kbd_macro())
				break;
	} else
		for (uni = 0; uni < uniarg; ++uni)
			if (!call_last_kbd_macro())
				return FALSE;

	return TRUE;
}

/*
 * Free all the allocated macros (used at Zile exit).
 */
void free_macros(void)
{
	macrop mp, next;

	for (mp = head_mp; mp != NULL; mp = next) {
		next = mp->next;
		free(mp);
	}
}
