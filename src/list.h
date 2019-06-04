#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include "types.h"

#define LIST_HEADER_INIT(name) {&name, &name}

struct ListHeader {
   struct ListHeader *next, *prev;
};

struct LinkedList {
   struct ListHeader head;
   offset_t head_offset;
};

void LL_init(struct LinkedList *list, offset_t head_offset);
void LL_enqueue(struct LinkedList *list, void *data);
void *LL_dequeue(struct LinkedList *list);
void LL_del(struct ListHeader *del);
void LL_add_prev(struct ListHeader *new, struct ListHeader *cur);
void LL_add(struct ListHeader *new, struct ListHeader *cur);
int LL_empty(struct LinkedList *list);

#endif