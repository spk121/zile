/*
 * Copyright (c) 2003 Nicolas Duboc.  All rights reserved.
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

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <string.h>

#include "zile.h"
#include "pathbuffer.h"
#include "extern.h"

#ifdef PATH_MAX
#define DEFAULT_SIZE PATH_MAX
#else
#define DEFAULT_SIZE 4096
#endif

#define ENLARGE_FACTOR 2

struct __pathbuffer_t {
  char* buffer;
  size_t size;
};


pathbuffer_t*
pathbuffer_create(size_t default_size)
{
	pathbuffer_t *buf;
	size_t size;

	assert(default_size >= 0);

#ifdef PATH_MAX
	if (default_size > PATH_MAX) {
		fprintf(stderr, "zile: trying to allocate a path buffer larger"
			" than system limit\n");
		zile_exit(1);
	}
#endif

	size = default_size ? default_size : DEFAULT_SIZE;
	
	buf = (pathbuffer_t*)zmalloc(sizeof(pathbuffer_t));
	
	buf->buffer = (char*)zmalloc(size);
	buf->buffer[0] = '\0';

	buf->size = size;

	return buf;
}

pathbuffer_t*
pathbuffer_realloc_larger(pathbuffer_t* buffer)
{
	size_t new_size;
	char* new_buf;

	assert(buffer != NULL);
#ifdef PATH_MAX
	if (buffer->size >= PATH_MAX) {
		fprintf(stderr, "zile: trying to allocate a path buffer larger"
			" than system limit\n");
		zile_exit(1);
	}
#endif

	new_size = buffer->size * ENLARGE_FACTOR;

#ifdef PATH_MAX
	new_size = new_size > PATH_MAX ? PATH_MAX : new_size;
#endif

	new_buf = (char*)zrealloc(buffer->buffer, new_size);
	buffer->buffer = new_buf;
	buffer->size = new_size;

	return buffer;
}

char*
pathbuffer_str(pathbuffer_t* buffer)
{
	assert(buffer != NULL);
	return buffer->buffer;
}

size_t
pathbuffer_size(pathbuffer_t* buffer)
{
	assert(buffer != NULL);
	return buffer->size;
}

void
pathbuffer_free(pathbuffer_t* buffer)
{
	free(pathbuffer_str(buffer));
	free(buffer);
}

char*
pathbuffer_free_struct_only(pathbuffer_t* buffer)
{
	char* actual_buffer;

	actual_buffer = pathbuffer_str(buffer);
	free(buffer);

	return actual_buffer;
}
pathbuffer_t*
pathbuffer_append(pathbuffer_t* buffer, const char* str)
{
	size_t current_buf_size, current_string_size;

	current_buf_size = pathbuffer_size(buffer);
	current_string_size = strlen(pathbuffer_str(buffer));

	if (current_string_size + strlen(str) + 1 > current_buf_size) {
		pathbuffer_realloc_larger(buffer);
	}

	strcat(pathbuffer_str(buffer), str);

	return buffer;
}

pathbuffer_t*
pathbuffer_put(pathbuffer_t* buffer, const char* str)
{
	if (pathbuffer_size(buffer) <= strlen(str)) {
		pathbuffer_realloc_larger(buffer);
	}

	strcpy(pathbuffer_str(buffer), str);

	return buffer;
}

pathbuffer_t*
pathbuffer_fput(pathbuffer_t* buffer, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	while (vsnprintf(pathbuffer_str(buffer), pathbuffer_size(buffer),
			 format, ap) == -1) {
		pathbuffer_realloc_larger(buffer);
	}

	va_end(ap);

	return buffer;
}
