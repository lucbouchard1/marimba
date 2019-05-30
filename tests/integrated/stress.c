#include "../../src/kmalloc.h"
#include "../../src/mmu.h"
#include "../../src/printk.h"
#include "../../src/string.h"

void kmalloc_stress_test()
{
   int i, *temp[100];
   int count = 100;

   printk("Allocating %d %ld byte blocks and testing allocation...\n", count, sizeof(int));
   for (i = 0; i < count; i++) {
      temp[i] = kmalloc(sizeof(int));
      *temp[i] = i;
   }

   for (i = count-1; i >= 0; i--) {
      if (*temp[i] != i)
         printk("error: found invalid write code\n");
      kfree(temp[i]);
   }
   printk("done\n");

   printk("Allocating %d %ld byte blocks and testing allocation again...\n", count, sizeof(int));
   for (i = 0; i < count; i++) {
      temp[i] = kmalloc(sizeof(int));
      *temp[i] = i;
   }

   for (i = count-1; i >= 0; i--) {
      if (*temp[i] != i)
         printk("error: found invalid write code\n");
      kfree(temp[i]);
   }
   printk("done\n");

   printk("Allocating %d %ld byte blocks and testing allocation...\n", count, count*sizeof(int));
   for (i = 0; i < count; i++) {
      temp[i] = kmalloc(count*sizeof(int));
      temp[i][i] = i;
   }

   for (i = count-1; i >= 0; i--) {
      if (temp[i][i] != i)
         printk("error: found invalid write code\n");
      kfree(temp[i]);
   }
   printk("done\n");

   count = 10;
   printk("Allocating %d %ld byte blocks and testing allocation...\n", count, 10000*sizeof(int));
   for (i = 0; i < count; i++) {
      temp[i] = kmalloc(10000*sizeof(int));
      temp[i][10*i] = i;
   }

   for (i = count-1; i >= 0; i--) {
      if (temp[i][10*i] != i)
         printk("error: found invalid write code\n");
      kfree(temp[i]);
   }
   printk("done\n");
}


void mmu_stress_test()
{
   void *page_buff[100], *res = NULL;
   int i, cycle, total_cycles = 10000;
   const char *bit_patt_base = "bit pattern woohoo!";
   int bit_patt_len = strlen(bit_patt_base);
   int bit_patt_off = 20;

   printk("Large Allocation Stress Test:\n");
   for (cycle = 0; cycle <= total_cycles; cycle++) {
      printk("Allocating and freeing 100 pages [%d/%d]", cycle, total_cycles);
      for (i = 0; i < 100; i++)
         page_buff[i] = MMU_alloc_frame();

      for (i = 0; i < 100; i++)
         MMU_free_frame(page_buff[i]);
      printk("\r");
   }
   printk("\nDone.\n");

   printk("Bit Pattern Stress Test:\n");
   printk("   applying bit patterns on all pages...\n");
   for (cycle = 0; 1; cycle++) {
      res = MMU_alloc_frame();
      /* Check if we've gotten through all the pages by looking for the bit pattern */
      if (!memcmp(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len))
         break;
      /* Write bit pattern to page */
      memcpy(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len);
      ((int *)res)[bit_patt_off + bit_patt_len] = cycle;
      MMU_free_frame(res);
   }
   printk("   applied bit pattern to %d pages...\n", cycle);
   printk("   checking bit patterns on all pages...\n");
   for (i = 0; i < cycle; i++) {
      MMU_free_frame(res);
      if (memcmp(&((char *)res)[bit_patt_off], bit_patt_base, bit_patt_len) ||
               ((int *)res)[bit_patt_off + bit_patt_len] != i)
         printk("Bit Pattern Failure!\n");
      res = MMU_alloc_frame();
   }
   MMU_free_frame(res);
   printk("   checked bit pattern on %d pages...\n", i);
   printk("Done.\n");
}

void stress_test()
{
   printk("MMU Stress Test\n");
   mmu_stress_test();
   printk("kmalloc Stress Test\n");
   kmalloc_stress_test();
}