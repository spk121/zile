/*	$Id: keys.c,v 1.1 2001/01/19 22:02:26 ssigala Exp $	*/

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "zile.h"
#include "extern.h"

#define ADDNSTR(p, s, n)			\
	do {					\
		strncpy(p, s, n);		\
		p += n;				\
	} while (0)

/*
 * Convert a key into his ASCII representation.
 */
char *keytostr(char *buf, int key, int *len)
{
	char *p = buf;

	if (key & KBD_CTL && key != KBD_TAB && key != KBD_RET)
		ADDNSTR(p, "\\C-", 3);
	if (key & KBD_META)
		ADDNSTR(p, "\\M-", 3);

	switch (key) {
	case KBD_PGUP:
		ADDNSTR(p, "\\PGUP", 5);
		break;
	case KBD_PGDN:
		ADDNSTR(p, "\\PGDN", 5);
		break;
	case KBD_HOME:
		ADDNSTR(p, "\\HOME", 5);
		break;
	case KBD_END:
		ADDNSTR(p, "\\END", 4);
		break;
	case KBD_DEL:
		ADDNSTR(p, "\\DEL", 4);
		break;
	case KBD_BS:
		ADDNSTR(p, "\\BS", 3);
		break;
	case KBD_INS:
		ADDNSTR(p, "\\INS", 4);
		break;
	case KBD_LEFT:
		ADDNSTR(p, "\\LEFT", 5);
		break;
	case KBD_RIGHT:
		ADDNSTR(p, "\\RIGHT", 6);
		break;
	case KBD_UP:
		ADDNSTR(p, "\\UP", 3);
		break;
	case KBD_DOWN:
		ADDNSTR(p, "\\DOWN", 5);
		break;
	case KBD_RET:
		ADDNSTR(p, "\\RET", 4);
		break;
	case KBD_TAB:
		ADDNSTR(p, "\\TAB", 4);
		break;
	case KBD_F1:
		ADDNSTR(p, "\\F1", 3);
		break;
	case KBD_F2:
		ADDNSTR(p, "\\F2", 3);
		break;
	case KBD_F3:
		ADDNSTR(p, "\\F3", 3);
		break;
	case KBD_F4:
		ADDNSTR(p, "\\F4", 3);
		break;
	case KBD_F5:
		ADDNSTR(p, "\\F5", 3);
		break;
	case KBD_F6:
		ADDNSTR(p, "\\F6", 3);
		break;
	case KBD_F7:
		ADDNSTR(p, "\\F7", 3);
		break;
	case KBD_F8:
		ADDNSTR(p, "\\F8", 3);
		break;
	case KBD_F9:
		ADDNSTR(p, "\\F9", 3);
		break;
	case KBD_F10:
		ADDNSTR(p, "\\F10", 4);
		break;
	case KBD_F11:
		ADDNSTR(p, "\\F11", 4);
		break;
	case KBD_F12:
		ADDNSTR(p, "\\F12", 4);
		break;
	default:
		key &= 255;
		if (key == '\\')
			ADDNSTR(p, "\\\\", 2);
		else
			*p++ = key;
	}

	*p++ = '\0';

	*len = p - buf;

	return buf;
}

/*
 * Convert a key into his ASCII representation.  Do not put
 * backslashes in the output.
 */
char *keytostr_nobs(char *buf, int key, int *len)
{
	char *p = buf;

	if (key & KBD_CTL && key != KBD_TAB && key != KBD_RET)
		ADDNSTR(p, "C-", 2);
	if (key & KBD_META)
		ADDNSTR(p, "M-", 2);

	switch (key) {
	case KBD_PGUP:
		ADDNSTR(p, "PGUP", 4);
		break;
	case KBD_PGDN:
		ADDNSTR(p, "PGDN", 4);
		break;
	case KBD_HOME:
		ADDNSTR(p, "HOME", 4);
		break;
	case KBD_END:
		ADDNSTR(p, "END", 3);
		break;
	case KBD_DEL:
		ADDNSTR(p, "DEL", 3);
		break;
	case KBD_BS:
		ADDNSTR(p, "BS", 2);
		break;
	case KBD_INS:
		ADDNSTR(p, "INS", 3);
		break;
	case KBD_LEFT:
		ADDNSTR(p, "LEFT", 4);
		break;
	case KBD_RIGHT:
		ADDNSTR(p, "RIGHT", 5);
		break;
	case KBD_UP:
		ADDNSTR(p, "UP", 2);
		break;
	case KBD_DOWN:
		ADDNSTR(p, "DOWN", 4);
		break;
	case KBD_RET:
		ADDNSTR(p, "RET", 3);
		break;
	case KBD_TAB:
		ADDNSTR(p, "TAB", 3);
		break;
	case KBD_F1:
		ADDNSTR(p, "F1", 2);
		break;
	case KBD_F2:
		ADDNSTR(p, "F2", 2);
		break;
	case KBD_F3:
		ADDNSTR(p, "F3", 2);
		break;
	case KBD_F4:
		ADDNSTR(p, "F4", 2);
		break;
	case KBD_F5:
		ADDNSTR(p, "F5", 2);
		break;
	case KBD_F6:
		ADDNSTR(p, "F6", 2);
		break;
	case KBD_F7:
		ADDNSTR(p, "F7", 2);
		break;
	case KBD_F8:
		ADDNSTR(p, "F8", 2);
		break;
	case KBD_F9:
		ADDNSTR(p, "F9", 2);
		break;
	case KBD_F10:
		ADDNSTR(p, "F10", 3);
		break;
	case KBD_F11:
		ADDNSTR(p, "F11", 3);
		break;
	case KBD_F12:
		ADDNSTR(p, "F12", 3);
		break;
	default:
		*p++ = key & 255;
	}

	*p++ = '\0';

	*len = p - buf;

	return buf;
}

/*
 * Convert a single key to the right key code.
 * Used internally by `strtokey()'.
 */
static int strtokey0(char *buf, int *len)
{
	char *p = buf;
	int key;

	key = -1;

	if (*p == '\\') {
		switch (*++p) {
		case '\\':
			key = '\\', ++p;
			break;
		case 'C':
			if (p[1] == '-')
				key = KBD_CTL, ++p;
			break;
		case 'M':
			if (p[1] == '-')
				key = KBD_META, ++p;
			break;
		case 'P':
			if (p[1] == 'G') {
				if (p[2] == 'U' && p[3] == 'P')
					key = KBD_PGUP, p += 3;
				else if (p[2] == 'D' && p[3] == 'N')
					key = KBD_PGDN, p += 3;
			}
			break;
		case 'H':
			if (p[1] == 'O' && p[2] == 'M' && p[3] == 'E')
				key = KBD_HOME, p += 3;
			break;
		case 'E':
			if (p[1] == 'N' && p[2] == 'D')
				key = KBD_END, p += 2;
			break;
		case 'D':
			if (p[1] == 'E' && p[2] == 'L')
				key = KBD_DEL, p += 2;
			else if (p[1] == 'O' && p[2] == 'W' && p[3] == 'N')
				key = KBD_DOWN, p += 3;
			break;
		case 'B':
			if (p[1] == 'S')
				key = KBD_BS, ++p;
			break;
		case 'I':
			if (p[1] == 'N' && p[2] == 'S')
				key = KBD_INS, p += 2;
			break;
		case 'L':
			if (p[1] == 'E' && p[2] == 'F' && p[3] == 'T')
				key = KBD_LEFT, p += 3;
			break;
		case 'R':
			if (p[1] == 'I' && p[2] == 'G' && p[3] == 'H'
			    && p[4] == 'T')
				key = KBD_RIGHT, p += 4;
			else if (p[1] == 'E' && p[2] == 'T')
				key = KBD_RET, p += 2;
			break;
		case 'T':
			if (p[1] == 'A' && p[2] == 'B')
				key = KBD_TAB, p += 2;
			break;
		case 'U':
			if (p[1] == 'P')
				key = KBD_UP, ++p;
			break;
		case 'F':
			switch (p[1]) {
			case '1':
				switch (p[2]) {
				case '0':
					key = KBD_F10, p += 2;
					break;
				case '1':
					key = KBD_F11, p += 2;
					break;
				case '2':
					key = KBD_F12, p += 2;
					break;
				default:
					key = KBD_F1, ++p;
				}
				break;
			case '2':
				key = KBD_F2, ++p;
				break;
			case '3':
				key = KBD_F3, ++p;
				break;
			case '4':
				key = KBD_F4, ++p;
				break;
			case '5':
				key = KBD_F5, ++p;
				break;
			case '6':
				key = KBD_F6, ++p;
				break;
			case '7':
				key = KBD_F7, ++p;
				break;
			case '8':
				key = KBD_F8, ++p;
				break;
			case '9':
				key = KBD_F9, ++p;
				break;
			}
			break;
		}

		if (key == -1)
			*len = 0;
		else
			*len = p - buf + 1;

		return key;
	}

	*len = 1;

	return *buf;
}

/*
 * Convert a key to the right key code.
 */
int strtokey(char *buf, int *len)
{
	int key, l;

	key = strtokey0(buf, &l);
	if (key == -1) {
		*len = 0;
		return -1;
	}

	*len = l;

	if (key == KBD_CTL || key == KBD_META) {
		int k;
		k = strtokey(buf + l, &l);
		if (k == -1) {
			*len = 0;
			return -1;
		}
		*len += l;
		key |= k;
	}

	return key;
}

/*
 * Convert a key sequence into the right key code sequence.
 */
int keytovec(char *key, int *keyvec)
{
	char *p = key;
	int len, size = 0;

	while (*p != '\0') {
		if ((keyvec[size++] = strtokey(p, &len)) == -1)
			return -1;
		p += len;
	}

	return size;
}

/*
 * Convert a key like "\\C-xrs" to "C-x r s"
 */
char *simplify_key(char *dest, char *key)
{
	char buf[128];
	int keys[64];
	int i, j, l;

	dest[0] = '\0';
	if (key == NULL)
		return dest;
	i = keytovec(key, keys);
	for (j = 0; j < i; ++j) {
		if (j > 0)
			strcat(dest, " ");
		keytostr_nobs(buf, keys[j], &l);
		strcat(dest, buf);
	}

	return dest;
}
