/* $Id: xmalloc.c,v 1.2 2003/04/24 15:11:59 rrt Exp $ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/*
 * Return an allocated memory area.
 */
void *(xmalloc)(size_t size)
{
	void *ptr;

	assert(size > 0);

	if ((ptr = malloc(size)) == NULL) {
		fprintf(stderr, "zile: cannot allocate memory\n");
		exit(1);
	}

	return ptr;
}
