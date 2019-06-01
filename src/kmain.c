#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "hw_init.h"
#include "mmu.h"
#include "multiboot.h"
#include "kmalloc.h"
#include "klog.h"
#include "syscall.h"
#include "proc.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/serial.h"

static struct PhysicalMMap map;

#ifdef STRESS_TEST
extern void stress_test();
#endif

void keyboard_isr(int irq, int err, void *arg)
{
   struct KeyboardDevice *dev = (struct KeyboardDevice *)arg;
   char c;

   if (!dev->char_avail(dev))
      return;

   c = dev->read_char(dev);
   printk("%c", c);
}

void test_func(void *arg)
{
   int *val = (int *)arg;
   while (1) {
      printk("test: %d\n", *val);
      yield();
   }
}

void kmain(uint32_t mb_magic, uint32_t mb_addr)
{
   struct KeyboardDevice *kdev;
   // /int val1 = 20;
   // /int val2 = 30;

   VGA_clear();
   HW_init();

   klog(KLOG_LEVEL_INFO, "starting up kernel");

   MB_parse_multiboot(&map, mb_magic, mb_addr);

   MMU_init(&map);

   klog(KLOG_LEVEL_INFO, "creating threads\n");

   #ifdef STRESS_TEST
   stress_test();
   #endif

   kdev = init_ps2(1);
   IRQ_set_handler(0x21, keyboard_isr, kdev);
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!

   //PROC_create_kthread(&test_func, &val1);
   //PROC_create_kthread(&test_func, &val2);

   printk("entering hlt loop\n");
   while(1)
      asm("hlt;");
      //PROC_run();
}