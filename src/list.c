/* Circular doubly-linked lists/queues */

#include <stdlib.h>
#include <assert.h>

#include "list.h"
#include "zile.h"
#include "extern.h"


/* Create an empty list, returning a pointer to the list */
list list_new(void)
{
  list l = zmalloc(sizeof(list));

  l->next = l->prev = l;
  l->item = NULL;

  return l;
}

/* Delete a list, freeing its nodes */
void list_delete(list l)
{
  list p = l, q;

  do {
    q = p;
    p = p->next;
    free(q);
  } while (p != l);
}

/* Return the length of a list */
unsigned long list_length(list l)
{
  list p;
  unsigned long length = 0;

  for (p = l->next; p != l; p = p->next)
    ++length;

  return length;
}

/* Add an item to the head of a list, returning the new list head */
list list_prepend(list l, void *i)
{
  list n = zmalloc(sizeof(list));

  n->next = l->next;
  n->prev = l;
  n->item = i;
  l->next = l->next->prev = n;

  return n;
}

/* Add an item to the tail of a list, returning the new list tail */
list list_append(list l, void *i)
{
  list n = zmalloc(sizeof(list));

  n->next = l;
  n->prev = l->prev;
  n->item = i;
  l->prev = l->prev->next = n;

  return n;
}

/* Return the first item of a list, or NULL if the list is empty */
void *list_head(list l)
{
  list p = l->next;

  if (p == l)
    return NULL;

  return p->item;
}

/* Remove the first item of a list, returning the item, or NULL if the
   list is empty */
void *list_behead(list l)
{
  void *i;
  list p = l->next;

  if (p == l)
    return NULL;
  i = p->item;
  l->next = l->next->next;
  l->next->prev = l;
  free(p);

  return i;
}

/* Remove the last item of a list, returning the item, or NULL if the
   list is empty */
void *list_betail(list l)
{
  void *i;
  list p = l->prev;

  if (p == l)
    return NULL;
  i = p->item;
  l->prev = l->prev->prev;
  l->prev->next = l;
  free(p);

  return i;
}

/* Return the nth item of l, or l->item (usually NULL) if that is out
   of range */
void *list_at(list l, unsigned int n)
{
  int i;
  list p;
        
  assert(l != NULL);

  for (p = list_first(l), i = 0; p != l && i < n; p = list_next(p), i++);
  
  return p->item;
}

/* Sort list l with qsort using comparison function cmp */
void list_sort(list l, int (*cmp)(const void *p1, const void *p2))
{
  list p;
  void **vec;
  unsigned long i, len = list_length(l);

  assert(l != NULL && cmp != NULL);

  vec = (void **)zmalloc(sizeof(void *) * len);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    vec[i] = (void *)p->item;

  qsort(vec, len, sizeof(void *), cmp);

  for (p = list_first(l), i = 0; i < len; p = list_next(p), ++i)
    p->item = vec[i];

  free(vec);
}
