#include "../syscall.h"
#include "../proc.h"
#include "../printk.h"
#include "procs.h"

void test_func(void *arg)
{
   int *val = (int *)arg, i;
   for (i = *val; i; i--) {
      printk("test: %d\n", i);
      yield();
   }
}

void stress_test_proc(void *arg)
{
   int val1 = 20;
   int val2 = 30;
   int val3 = 40;
   int val4 = 50;
   PROC_create_process("test_process_1", &test_func, &val1);
   PROC_create_process("test_process_2", &test_func, &val2);
   PROC_create_process("test_process_3", &test_func, &val3);
   PROC_create_process("test_process_4", &test_func, &val4);
   while (1)
      yield();
}