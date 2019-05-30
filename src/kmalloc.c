#include "mmu.h"
#include "printk.h"
#include "types.h"

#define POOL_PAGE_INCREASE 4
#define NUM_POOLS 6

struct KmallocBlockHeader {
   struct KmallocPool *pool;
   struct KmallocBlockHeader *next;
};

struct KmallocPool {
   size_t block_size;
   struct KmallocBlockHeader *free_head;
};

static struct KmallocPool pools[] = {
   {
      .block_size = 32,
      .free_head = NULL
   },
   {
      .block_size = 64,
      .free_head = NULL
   },
   {
      .block_size = 128,
      .free_head = NULL
   },
   {
      .block_size = 512,
      .free_head = NULL
   },
   {
      .block_size = 1024,
      .free_head = NULL
   },
   {
      .block_size = 2048,
      .free_head = NULL
   }
};

static int kmalloc_increase_pool(struct KmallocPool *pool, int num_pages)
{
   int i, num_blocks;
   struct KmallocBlockHeader **prev, *cur;

   if (pool->free_head)
      return 0;

   cur = MMU_alloc_pages(num_pages);
   if (!cur)
      return -1;
   num_blocks = (num_pages * PAGE_SIZE) / pool->block_size;

   prev = &pool->free_head;
   for (i = 0; i < num_blocks; i++,
         cur = (struct KmallocBlockHeader *)((uint8_t *)cur + pool->block_size)) {
      *prev = cur;
      cur->pool = pool;
      cur->next = NULL;
      prev = &cur->next;
   }

   return 0;
}

struct KmallocBlockHeader *kmalloc_alloc(struct KmallocPool *pool)
{
   struct KmallocBlockHeader *ret;

   if (!pool->free_head && kmalloc_increase_pool(pool, POOL_PAGE_INCREASE) < 0)
         return NULL;

   ret = pool->free_head;
   pool->free_head = pool->free_head->next;
   ret->next = NULL;
   return ret;
}

void *kmalloc(size_t size)
{
   int num_pages, i;
   struct KmallocBlockHeader *ret = NULL;

   if (!size)
      return NULL;

   for (i = 0; i < NUM_POOLS; i++) {
      if (size + sizeof(struct KmallocBlockHeader) <= pools[i].block_size) {
         if (!(ret = kmalloc_alloc(&pools[i])))
            return NULL;
         break;
      }
   }

   if (!ret) {
      num_pages = size / PAGE_SIZE;
      if (size % PAGE_SIZE)
         num_pages++;
      ret = MMU_alloc_pages(num_pages);
      if (!ret)
         return NULL;
      /* Repurpose the block header to store number of allocated pages */
      ret->pool = NULL;
      ret->next = int_to_ptr(num_pages);
   }

   return ret + 1;
}

void kfree(void *addr)
{
   struct KmallocBlockHeader *block = ((struct KmallocBlockHeader *)addr) - 1;
   struct KmallocPool *pool;

   if (!block->pool) {
      MMU_free_pages(block, ptr_to_int(block->next));
      return;
   }

   pool = block->pool;
   block->next = pool->free_head;
   pool->free_head = block;
}