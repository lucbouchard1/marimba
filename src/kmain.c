#include "vga.h"
#include "printk.h"
#include "io.h"
#include "panic.h"
#include "drivers/pci.h"
#include "interrupts.h"
#include "hw_init.h"
#include "mmu.h"
#include "boot.h"
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

void kmain(uint32_t mb_magic, uint32_t mb_addr)
{
   VGA_clear();
   HW_init();

   klog(KLOG_LEVEL_INFO, "starting up kernel");

   if (BOOT_parse_multiboot(&map, mb_magic, mb_addr) < 0)
      panic();

   if (MMU_init(&map) < 0)
      panic();

   PCI_enum();
   FILE_temp_dev_init();

   klog(KLOG_LEVEL_INFO, "creating threads");

   #ifdef STRESS_TEST
   stress_test();
   PROC_create_process("stress_test", &stress_test_proc, NULL);
   #endif

   PROC_create_process("keyboard_io", &keyboard_io, NULL);
   PROC_create_process("filesystem", &filesystem_init, NULL);

   while (1) {
      PROC_run();
      asm("hlt;");
   }
}