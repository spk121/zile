/* $Id: xstrdup.c,v 1.3 2003/05/06 22:28:41 rrt Exp $ */

#include <string.h>

#include "config.h"

/*
 * Duplicate a string.
 */
char *(xstrdup)(const char *s)
{
	return strcpy(xmalloc(strlen(s) + 1), s);
}
