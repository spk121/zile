/* Key sequences functions
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

/*	$Id: keys.c,v 1.8 2004/03/09 16:30:00 rrt Exp $	*/

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"

/*
 * Convert a key into its ASCII representation
 */
astr keytostr(int key)
{
	astr as = astr_new();

	if (key & KBD_CTL)
		astr_append_cstr(as, "C-");
	if (key & KBD_META)
		astr_append_cstr(as, "M-");
	key &= ~(KBD_CTL | KBD_META);

	switch (key) {
	case KBD_PGUP:
		astr_append_cstr(as, "PGUP");
		break;
	case KBD_PGDN:
		astr_append_cstr(as, "PGDN");
		break;
	case KBD_HOME:
		astr_append_cstr(as, "HOME");
		break;
	case KBD_END:
		astr_append_cstr(as, "END");
		break;
	case KBD_DEL:
		astr_append_cstr(as, "DEL");
		break;
	case KBD_BS:
		astr_append_cstr(as, "BS");
		break;
	case KBD_INS:
		astr_append_cstr(as, "INS");
		break;
	case KBD_LEFT:
		astr_append_cstr(as, "LEFT");
		break;
	case KBD_RIGHT:
		astr_append_cstr(as, "RIGHT");
		break;
	case KBD_UP:
		astr_append_cstr(as, "UP");
		break;
	case KBD_DOWN:
		astr_append_cstr(as, "DOWN");
		break;
	case KBD_RET:
		astr_append_cstr(as, "RET");
		break;
	case KBD_TAB:
		astr_append_cstr(as, "TAB");
		break;
	case KBD_F1:
		astr_append_cstr(as, "F1");
		break;
	case KBD_F2:
		astr_append_cstr(as, "F2");
		break;
	case KBD_F3:
		astr_append_cstr(as, "F3");
		break;
	case KBD_F4:
		astr_append_cstr(as, "F4");
		break;
	case KBD_F5:
		astr_append_cstr(as, "F5");
		break;
	case KBD_F6:
		astr_append_cstr(as, "F6");
		break;
	case KBD_F7:
		astr_append_cstr(as, "F7");
		break;
	case KBD_F8:
		astr_append_cstr(as, "F8");
		break;
	case KBD_F9:
		astr_append_cstr(as, "F9");
		break;
	case KBD_F10:
		astr_append_cstr(as, "F10");
		break;
	case KBD_F11:
		astr_append_cstr(as, "F11");
		break;
	case KBD_F12:
		astr_append_cstr(as, "F12");
		break;
	default:
                astr_append_char(as, key & 255);
	}

	return as;
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
			key = '\\';
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
int keytovec(char *key, int **keys)
{
        vector *v = vec_new(sizeof(int));
	int size;

	for (size = 0; *key != '\0'; size++) {
                int len;
                int keycode = strtokey(key, &len);
                vec_item(v, size, int) = keycode;
		if ((vec_item(v, size, int) = keycode) == -1) {
                        vec_delete(v);
			return -1;
                }
		key += len;
	}

        *keys = vec_toarray(v);
	return size;
}

/*
 * Convert a key like "\\C-xrs" to "C-x r s"
 */
char *simplify_key(char *dest, char *key)
{
	int i, j, *keys;

	dest[0] = '\0';
	if (key == NULL)
		return dest;
	i = keytovec(key, &keys);
	for (j = 0; j < i; j++) {
                astr as;
		if (j > 0)
			strcat(dest, " ");
		as = keytostr(keys[j]);
		strcat(dest, astr_cstr(as));
                astr_delete(as);
	}
        if (i > 0)
                free(keys);

	return dest;
}
