/* Circular doubly-linked lists/queues */

#ifndef LIST_H
#define LIST_H

typedef struct list_s *list;
struct list_s {
  list prev;
  list next;
  void *item;
};

list list_new(void);
void list_delete(list l);
unsigned long list_length(list l);
list list_prepend(list l, void *i);
list list_append(list l, void *i);
void *list_head(list l);
void *list_behead(list l);
void *list_betail(list l);
void *list_at(list l, unsigned int n);
void list_sort(list l, int (*cmp)(const void *p1, const void *p2));

#define list_first(l) ((l)->next)
#define list_last(l)  ((l)->prev)
#define list_next(l)  ((l)->next)
#define list_prev(l)  ((l)->prev)

#endif
