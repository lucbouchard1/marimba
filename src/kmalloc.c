#include "mmu.h"

#define HEADER_SIZE (sizeof(struct KmallocBlockHeader))
#define PAGE_INCREASE_STEP 4

struct KmallocBlockHeader {
   struct KmallocPool *pool;
   struct KmallocBlockHeader *next;
};

struct KmallocPool {
   size_t block_size;
   struct KmallocBlockHeader *free_head;
};

static struct KmallocPool pool_64 = {
   .block_size = 64,
   .free_head = NULL
};

static struct KmallocPool pool_128 = {
   .block_size = 128,
   .free_head = NULL
};

static struct KmallocPool pool_512 = {
   .block_size = 512,
   .free_head = NULL
};

static struct KmallocPool pool_1024 = {
   .block_size = 1024,
   .free_head = NULL
};

static struct KmallocPool pool_2048 = {
   .block_size = 2048,
   .free_head = NULL
};

static int malloc_increase_pool(struct KmallocPool *pool, int num_pages)
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
   for (i = 0; i < num_blocks; i++) {
      *prev = &cur[i];
      cur->pool = pool;
      cur->next = NULL;
      prev = &cur->next;
   }

   return 0;
}

struct KmallocBlockHeader *kmalloc_alloc(struct KmallocPool *pool)
{
   void *ret;

   if (!pool->free_head && malloc_increase_pool(pool, PAGE_INCREASE_STEP) < 0)
         return NULL;

   ret = pool->free_head;
   pool->free_head = pool->free_head->next;
   return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

void *kmalloc(size_t size)
{
   int num_pages;
   struct KmallocBlockHeader *ret;

   if (size < 64)
      ret = kmalloc_alloc(&pool_64);
   else if (size < 128)
      ret = kmalloc_alloc(&pool_128);
   else if (size < 512)
      ret = kmalloc_alloc(&pool_512);
   else if (size < 1024)
      ret = kmalloc_alloc(&pool_1024);
   else if (size < 2048)
      ret = kmalloc_alloc(&pool_2048);
   else {
      num_pages = size / PAGE_SIZE;
      if (size % PAGE_SIZE)
         num_pages++;
      ret = MMU_alloc_pages(num_pages);
      if (!ret)
         return NULL;
      ret->pool = NULL;
      ret->next = (struct KmallocBlockHeader *)num_pages;
   }

   if (!ret)
      return NULL;
   return ret + 1;
}

void kfree(void *addr)
{
   struct KmallocBlockHeader *block = ((struct KmallocBlockHeader *)addr) - 1;
   struct KmallocPool *pool;

   if (!block->pool) {
      MMU_free_pages(block, (int)block->next);
      return;
   }

   pool = block->pool;
   block->next = pool->free_head;
   pool->free_head = block;
}

#pragma GCC diagnostic pop