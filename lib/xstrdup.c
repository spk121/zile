/* $Id: xstrdup.c,v 1.1 2001/01/19 22:01:45 ssigala Exp $ */

#include <string.h>

#include "config.h"

/*
 * Duplicate a string.
 */
char *(xstrdup)(const char *s)
{
	return strcpy(xmalloc(strlen(s) + 1), s);
}
