/* Lisp lists

   Copyright (c) 2008, 2011 Free Software Foundation, Inc.

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

#include <config.h>

#include "main.h"
#include "extern.h"

le *
leNew (const char *text)
{
  le *new = (le *) XZALLOC (le);

  if (text)
    new->data = xstrdup (text);

  return new;
}

static le *
leAddTail (le * list, le * element)
{
  le *temp = list;

  /* if either element or list doesn't exist, return the `new' list */
  if (!element)
    return list;
  if (!list)
    return element;

  /* find the end element of the list */
  while (temp->next)
    temp = temp->next;

  /* tack ourselves on */
  temp->next = element;

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
  temp->next = leDup (list->next);

  return temp;
}
