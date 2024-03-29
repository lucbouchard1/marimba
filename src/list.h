#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include "types.h"

#define LL_nhead(list, data) ((struct ListHeader *)&((uint8_t *)data)[(list)->head_offset])
#define LL_ndata(list, data) ((void *)&((uint8_t *)data)[-1*(list)->head_offset])

#define LINKED_LIST_INIT(list, type, field) { \
   .head = {&list.head, &list.head}, \
   .head_offset=offsetof(type, field)}

#define LL_for_each(list, curr) \
   for (curr = LL_ndata(list, (list)->head.next); \
         LL_nhead(list, curr) != &(list)->head; \
         curr = LL_ndata(list, LL_nhead(list, curr)->next))

struct ListHeader {
   struct ListHeader *next, *prev;
};

struct LinkedList {
   struct ListHeader head;
   offset_t head_offset;
};

void LL_init(struct LinkedList *list, offset_t head_offset);
void LL_append(struct LinkedList *list, void *data);
void LL_enqueue(struct LinkedList *list, void *data);
void *LL_dequeue(struct LinkedList *list);
void LL_del(struct ListHeader *del);
void LL_add_prev(struct ListHeader *new, struct ListHeader *cur);
void LL_add(struct ListHeader *new, struct ListHeader *cur);
void *LL_next(struct LinkedList *list, void *curr);
int LL_empty(struct LinkedList *list);
void *LL_head(struct LinkedList *list);

#endif