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

#ifndef PATHBUFFER_H
#define PATHBUFFER_H

#include <sys/types.h>

typedef struct __pathbuffer_t pathbuffer_t;

/* Create a path buffer whose initial size will be default_size.
 * If default_size==0, use a reasonable size (!= 0).
 * Return the created buffer if successful or NULL on error (buffer larger
 * than system limit for example).
 */
pathbuffer_t*
pathbuffer_create(size_t default_size);

/* Ask the reallocation of this buffer with a larger size.
 * Returns the buffer on success, NULL on error.
 * The content of the buffer is conserved but pointer pointing in the buffer
 * may be invalid !
 */
pathbuffer_t*
pathbuffer_realloc_larger(pathbuffer_t* buffer);

/* Append str to the buffer.
 * Assumes the buffer contains a valid string (i.e. ends with a  \0 )
 * If the buffer is not enough large, realloc a larger buffer.
 * Return NULL if the buffer is larger than the system limit or if there is
 * no \0 character in the buffer.
 */
pathbuffer_t*
pathbuffer_append(pathbuffer_t* buffer, const char* str);

/* Copy the content of the given string to the beginning of the buffer.
 * Return the buffer if successful, NULL on error.
 */
pathbuffer_t*
pathbuffer_put(pathbuffer_t* buffer, const char* str);

/* Copy the formatted string obtained from the format and all the optional
 * arguments to the beginning of the buffer.
 * Return the buffer if successful, NULL on error.
 */
pathbuffer_t*
pathbuffer_fput(pathbuffer_t* buffer, const char* format, ...);

/* Free the given buffer.
 */
void
pathbuffer_free(pathbuffer_t* buffer);

/* Free the pathbuffer only and returns a pointer to the actual char* buffer.
 * This returned buffer is to be freed by the caller.
 */
char*
pathbuffer_free_struct_only(pathbuffer_t* buffer);

/* Returns the actual string of the buffer.
 * Don't free the returned char*, it belongs to the pathbuffer !
 * Be carreful, the returned pointer may be invalide after a pathbuffer_append
 * pathbuffer_put or pathbuffer_realloc_larger !!!
 */
char*
pathbuffer_str(pathbuffer_t* buffer);

/* Returns the size of the buffer. Yes it does ! ;-)
 */
size_t
pathbuffer_size(pathbuffer_t* buffer);

#endif /* PATHBUFFER_H */
