/* $Id: xstrdup.c,v 1.2 2003/04/24 15:11:59 rrt Exp $ */

#include <string.h>

#include "config.h"

/*
 * Duplicate a string.
 */
char *(xstrdup)(const char *s)
{
	return strcpy(xmalloc(strlen(s) + 1), s);
}
