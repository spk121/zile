/* $Id: xstrdup.c,v 1.4 2004/03/10 12:59:32 rrt Exp $ */

#include "config.h"

#ifndef HAVE_XSTRDUP

#include <string.h>

/*
 * Duplicate a string.
 */
char *(xstrdup)(const char *s)
{
	return strcpy(xmalloc(strlen(s) + 1), s);
}

#endif
