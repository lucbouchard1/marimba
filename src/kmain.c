#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "hw_init.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/serial.h"

void keyboard_isr(int irq, int err, void *arg)
{
   struct KeyboardDevice *dev = (struct KeyboardDevice *)arg;
   char c;

   c = dev->read_char(dev);
   printk("%c", c);
}

void kmain(void)
{
   struct KeyboardDevice *kdev;   
   struct SerialDevice *sdev;   
   //int counter, i;
   //long delay;

   HW_init();
   VGA_clear();
   IRQ_init();

   sdev = init_x86_serial();
   SER_write_str(sdev, "Hello World\n");
   SER_write_str(sdev, "My name is luc :)\n");

   kdev = init_ps2(1);
   IRQ_set_handler(0x21, keyboard_isr, kdev);
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!
   while(1)
      asm("hlt;");
}