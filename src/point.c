/*	$Id: point.c,v 1.1 2004/02/08 04:39:26 dacap Exp $	*/

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

#include "zile.h"
#include "extern.h"

Point make_point(int lineno, int offset)
{
	Point pt = { cur_bp->limitp->next, lineno, offset };
	while (lineno > 0) {
		pt.p = pt.p->next;
		lineno--;
	}
	return pt;
}

int cmp_point(Point pt1, Point pt2)
{
	if (pt1.n < pt2.n)
		return -1;
	else if (pt1.n > pt2.n)
		return +1;
	else
		return ((pt1.o < pt2.o) ? -1:
			(pt1.o > pt2.o) ? +1: 0);
}

int point_dist(Point pt1, Point pt2)
{
	int size = 0;
	Line *lp;

	if (cmp_point(pt1, pt2) > 0)
		swap_point(&pt1, &pt2);

	for (lp=pt1.p; ; lp=lp->next) {
		size += lp->size;

		if (lp == pt1.p)
			size -= pt1.o;

		if (lp == pt2.p) {
			size -= (lp->size - pt2.o);
			break;
		}
		else
			size++;
	}

	return size;
}

int count_lines(Point pt1, Point pt2)
{
	if (cmp_point(pt1, pt2) > 0)
		swap_point(&pt1, &pt2);

	return pt2.n - pt1.n;
}

void swap_point(Point *pt1, Point *pt2)
{
	Point pt0 = *pt1;
	*pt1 = *pt2;
	*pt2 = pt0;
}

Point point_min(void)
{
	Point pt = { cur_bp->limitp->next, 0, 0 };
	return pt;
}

Point point_max(void)
{
	Point pt = { cur_bp->limitp->prev,
		     cur_bp->num_lines,
		     cur_bp->limitp->prev->size };
	return pt;
}

Point line_beginning_position(int count)
{
	Point pt;

	count--;

	/* Copy current point position without offset (beginning of
	 * line).  */
	pt = cur_bp->pt;
	pt.o = 0;

	if (count < 0) {
		while (count && pt.p->prev != cur_bp->limitp) {
			pt.p = pt.p->prev;
			pt.n--;
			count++;
		}
	}
	else if (count > 0) {
		while (count && pt.p->next != cur_bp->limitp) {
			pt.p = pt.p->next;
			pt.n++;
			count--;
		}
	}

	return pt;
}

Point line_end_position(int count)
{
	Point pt = line_beginning_position(count);
	pt.o = pt.p->size;
	return pt;
}

