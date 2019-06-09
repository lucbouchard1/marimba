#include "vga.h"
#include "printk.h"
#include "io.h"
#include "drivers/pci.h"
#include "interrupts.h"
#include "hw_init.h"
#include "mmu.h"
#include "multiboot.h"
#include "kmalloc.h"
#include "klog.h"
#include "syscall.h"
#include "proc.h"
#include "procs/procs.h"
#include "files.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/serial.h"

static struct PhysicalMMap map;

#ifdef STRESS_TEST
extern void stress_test();
#endif

void test_func(void *arg)
{
   int *val = (int *)arg, i;
   for (i = *val; i; i--) {
      printk("test: %d\n", i);
      yield();
   }
}

void kmain(uint32_t mb_magic, uint32_t mb_addr)
{
//   / int val1 = 20;
//   / int val2 = 30;

   VGA_clear();
   HW_init();

   klog(KLOG_LEVEL_INFO, "starting up kernel");

   MB_parse_multiboot(&map, mb_magic, mb_addr);

   MMU_init(&map);

   PCI_enum();

   FILE_temp_dev_init();

   klog(KLOG_LEVEL_INFO, "creating threads");

   #ifdef STRESS_TEST
   stress_test();
   #endif

//   PROC_create_process("test_process_1", &test_func, &val1);
//   PROC_create_process("test_process_2", &test_func, &val2);
   PROC_create_process("keyboard_io", &keyboard_io, NULL);
   PROC_create_process("filesystem", &filesystem_init, NULL);

   while(1) {
      PROC_run();
      asm("hlt;");
   }
}