/* $Id: xrealloc.c,v 1.4 2004/03/08 14:32:18 rrt Exp $ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/*
 * Resize an allocated memory area.
 */
void *(xrealloc)(void *ptr, size_t size)
{
	void *newptr;

	if ((newptr = realloc(ptr, size)) == NULL) {
		fprintf(stderr, "zile: cannot reallocate memory\n");
		exit(1);
	}

	return newptr;
}
