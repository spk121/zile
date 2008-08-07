/* Lisp lists

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005 Reuben Thomas.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "zile.h"
#include "extern.h"
#include "lists.h"
#include "eval.h"


le *
leNew (const char *text)
{
  le *new = (le *) xzalloc (sizeof (le));

  new->branch = NULL;
  new->data = text ? xstrdup (text) : NULL;
  new->quoted = 0;
  new->tag = -1;
  new->list_prev = NULL;
  new->list_next = NULL;

  return new;
}

void
leReallyWipe (le * list)
{
  if (list)
    {
      /* free descendants */
      leWipe (list->branch);
      leWipe (list->list_next);

      /* free ourself */
      if (list->data)
	free (list->data);
      free (list);
    }
}

void
leWipe (le * list)
{
  if (list != leNIL && list != leT)
    leReallyWipe (list);
}

le *
leAddTail (le * list, le * element)
{
  le *temp = list;

  /* if either element or list doesn't exist, return the `new' list */
  if (!element)
    return list;
  if (!list)
    return element;

  /* find the end element of the list */
  while (temp->list_next)
    temp = temp->list_next;

  /* tack ourselves on */
  temp->list_next = element;
  element->list_prev = temp;

  /* return the list */
  return list;
}


le *
leAddBranchElement (le * list, le * branch, int quoted)
{
  le *temp = leNew (NULL);
  temp->branch = branch;
  temp->quoted = quoted;
  return leAddTail (list, temp);
}

le *
leAddDataElement (le * list, const char *data, int quoted)
{
  le *newdata = leNew (data);
  assert (newdata);
  newdata->quoted = quoted;
  return leAddTail (list, newdata);
}

le *
leDup (le * list)
{
  le *temp;
  if (!list)
    return NULL;

  temp = leNew (list->data);
  temp->branch = leDup (list->branch);
  temp->list_next = leDup (list->list_next);

  if (temp->list_next)
    temp->list_next->list_prev = temp;

  return temp;
}

static astr
leDumpReformat (le * tree)
{
  int notfirst = false;
  astr as = astr_new ();

  if (tree)
    {
      astr_cat_char (as, '(');

      for (; tree; tree = tree->list_next)
	{
	  if (tree->data)
	    {
	      astr_afmt (as, "%s%s", notfirst ? " " : "", tree->data);
	      notfirst = true;
	    }

	  if (tree->branch)
	    {
	      astr_afmt (as, " %s", tree->quoted ? "\'" : "");
	      astr_cat_delete (as, leDumpReformat (tree->branch));
	    }
	}

      astr_cat_char (as, ')');
    }

  return as;
}

astr
leDumpEval (le * list, int indent GCC_UNUSED)
{
  le *start = list;
  astr as = astr_new ();

  for (; list; list = list->list_next)
    {
      if (list->branch)
	{
	  le *le_value = evaluateBranch (list->branch);
	  if (list != start)
	    astr_cat_char (as, '\n');
	  astr_cat_delete (as, leDumpReformat (le_value));
	  if (le_value && le_value != leNIL)
	    leWipe (le_value);
	}
    }

  return as;
}
