/* $Id: xrealloc.c,v 1.3 2003/05/06 22:28:41 rrt Exp $ */

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

	assert(ptr != NULL);
	assert(size > 0);

	if ((newptr = realloc(ptr, size)) == NULL) {
		fprintf(stderr, "zile: cannot allocate memory\n");
		exit(1);
	}

	return newptr;
}
