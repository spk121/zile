/*	$Id: marker.c,v 1.1 2004/02/08 04:39:26 dacap Exp $	*/

/*
 * Copyright (c) 2004 David A. Capello.  All rights reserved.
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

#include <stdlib.h>
#include <ctype.h>

#include "zile.h"
#include "extern.h"
#include "editfns.h"

Marker *make_marker(void)
{
	Marker *marker;
	marker = (Marker *)zmalloc(sizeof(Marker));
	memset(marker, 0, sizeof(Marker));
	return marker;
}

void free_marker(Marker *marker)
{
	unchain_marker(marker);
	free(marker);
}

void unchain_marker(Marker *marker)
{
	Marker *m, *next, *prev = NULL;

	if (!marker->bp)
		return;

	for (m=marker->bp->markers; m; m=next) {
		next = m->next;
		if (m == marker) {
			if (prev)
				prev->next = next;
			else
				m->bp->markers = next;

			m->bp = NULL;
			break;
		}
		prev = m;
	}
}

void move_marker(Marker *marker, Buffer *bp, Point pt)
{
	if (bp != marker->bp) {
		/* Unchain with the previous pointed buffer.  */
		unchain_marker(marker);

		/* Change the buffer.  */
		marker->bp = bp;

		/* Chain with the new buffer.  */
		marker->next = bp->markers;
		bp->markers = marker;
	}

	/* Change the point.  */
	marker->pt = pt;
}

Marker *copy_marker(Marker *m)
{
	Marker *marker = make_marker();
	move_marker(marker, m->bp, m->pt);
	return marker;
}

Marker *point_marker(void)
{
	Marker *marker = make_marker();
	move_marker(marker, cur_bp, cur_bp->pt);
	return marker;
}

Marker *point_min_marker(void)
{
	Marker *marker = make_marker();
	move_marker(marker, cur_bp, point_min());
	return marker;
}

Marker *point_max_marker(void)
{
	Marker *marker = make_marker();
	move_marker(marker, cur_bp, point_max());
	return marker;
}

void set_marker_insertion_type(Marker *marker, int type)
{
	marker->type = type;
}

int marker_insertion_type(Marker *marker)
{
	return marker->type;
}
