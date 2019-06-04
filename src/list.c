#include "list.h"

static void ll_add(struct ListHeader *new, struct ListHeader *prev,
      struct ListHeader *next)
{
   new->next = next;
   new->prev = prev;
   prev->next = new;
   next->prev = new;
}

static void ll_del(struct ListHeader *del, struct ListHeader *prev,
      struct ListHeader *next)
{
   prev->next = del->next;
   next->prev = del->prev;
   del->next = NULL;
   del->prev = NULL;
}

int LL_empty(struct LinkedList *list)
{
   return list->head.next == &list->head;
}

void LL_add(struct ListHeader *new, struct ListHeader *cur)
{
   ll_add(new, cur, cur->next);
}

void LL_del(struct ListHeader *del)
{
   ll_del(del, del->prev, del->next);
}

void LL_add_prev(struct ListHeader *new, struct ListHeader *cur)
{
   ll_add(new, cur->prev, cur);
}

void LL_enqueue(struct LinkedList *list, void *data)
{
   LL_append(list, data);
}

void LL_append(struct LinkedList *list, void *data)
{
   LL_add_prev(LL_HEAD(list, data), &list->head);
}

void *LL_dequeue(struct LinkedList *list)
{
   void *ret;

   if (LL_empty(list))
      return NULL;

   ret = list->head.next;
   LL_del(ret);
   return LL_DATA(list, ret);
}

void LL_init(struct LinkedList *list, offset_t head_offset)
{
   list->head.next = &list->head;
   list->head.prev = &list->head;
   list->head_offset = head_offset;
}