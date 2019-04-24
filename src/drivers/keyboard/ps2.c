#include "../../io.h"
#include "../../printk.h"
#include "../../string.h"
#include "keyboard.h"

#define PS2_DATA_PORT 0x60
#define PS2_CMD_PORT 0x64
#define PS2_STATUS_PORT 0x64

#define PS2_CMD_DISABLE_P1 0xAD
#define PS2_CMD_ENABLE_P1 0xAE
#define PS2_CMD_DISABLE_P2 0xA7
#define PS2_CMD_READ_CNTL_CFG 0x20
#define PS2_CMD_WRITE_CNTL_CFG 0x60
#define PS2_CMD_P1_SELF_TEST 0xAA
#define PS2_CMD_P1_INTERFACE_TEST 0xAB
#define PS2_CMD_WRITE_OUTPUT 0xD1

#define PS2_STATUS_OUTPUT 0x1 << 0
#define PS2_STATUS_INPUT 0x1 << 1

#define PS2_CNTL_CFG_P1_INT 0x1 << 0
#define PS2_CNTL_CFG_P2_INT 0x1 << 1
#define PS2_CNTL_CFG_2ND_CLK 0x1 << 5
#define PS2_CNTL_CFG_TRANS 0x1 << 6

struct PS2Device {
   struct KeyboardDevice kb;
};

static struct PS2Device gdev;

static inline void ps2_wait_writable()
{
   uint8_t status;

   status = inb(PS2_STATUS_PORT);
   while(status & PS2_STATUS_INPUT)
      status = inb(PS2_STATUS_PORT);
}

static inline void ps2_wait_readable()
{
   uint8_t status;

   status = inb(PS2_STATUS_PORT);
   while(!(status & PS2_STATUS_OUTPUT))
      status = inb(PS2_STATUS_PORT);
}

static uint8_t ps2_read_cmd(uint8_t cmd)
{
   /* Write command to command port */
   outb(PS2_CMD_PORT, cmd);

   /* Wait for data to be available */
   ps2_wait_readable();
   
   /* Read data */
   return inb(PS2_DATA_PORT);
}

static void ps2_write_cmd(uint8_t cmd, uint8_t val)
{
   /* Write command to command port */
   outb(PS2_CMD_PORT, cmd);

   /* Wait for PS2 data port to be ready */
   ps2_wait_writable();

   /* Write data */
   outb(PS2_DATA_PORT, val);
}

static void ps2_write_data_p1(uint8_t val)
{
   /* Wait for PS2 data port to be ready */
   ps2_wait_writable();

   /* Write data */
   outb(PS2_DATA_PORT, val);
}

static uint8_t ps2_read_data_p1()
{
   /* Wait for PS2 data port to be ready */
   ps2_wait_readable();

   /* Read data */
   return inb(PS2_DATA_PORT);
}

char ps2_read_char(struct KeyboardDevice *dev)
{
   uint8_t scode;

   while (1) {
      scode = ps2_read_data_p1();
      if (scode == LEFT_SHIFT_SCAN_CODE || scode == RIGHT_SHIFT_SCAN_CODE)
         dev->shift_pressed = 1;
      else if (scode != RELEASED_SCAN_CODE)
         return translate_scan_code(dev, scode);

      if (scode == RELEASED_SCAN_CODE) { 
         scode = ps2_read_data_p1();
         if (scode == LEFT_SHIFT_SCAN_CODE ||
               scode == RIGHT_SHIFT_SCAN_CODE)
            dev->shift_pressed = 0;
      }
   }

   return 0;
}

struct KeyboardDevice *init_ps2()
{
   uint8_t cntl_cfg, resp;

   memset(&gdev, 0, sizeof(struct PS2Device));
   gdev.kb.read_char = &ps2_read_char;

   /* Disable both ports */
   outb(PS2_CMD_PORT, PS2_CMD_DISABLE_P1);
   outb(PS2_CMD_PORT, PS2_CMD_DISABLE_P2);

   /* Flush PS2 output buffer */
   inb(PS2_DATA_PORT);

   /* Read the current controller configuration */
   cntl_cfg = ps2_read_cmd(PS2_CMD_READ_CNTL_CFG);

   /* Set controller configuration appropriately */
   cntl_cfg &= ~(PS2_CNTL_CFG_P1_INT
         | PS2_CNTL_CFG_P2_INT | PS2_CNTL_CFG_TRANS);
   /* enable interrupts for p1 */
   cntl_cfg |= PS2_CNTL_CFG_P1_INT;

   /* Write controller configuration back out */
   ps2_write_cmd(PS2_CMD_WRITE_CNTL_CFG, cntl_cfg);

   /* Perform self-test */
   resp = ps2_read_cmd(PS2_CMD_P1_SELF_TEST);
   if (resp != 0x55)
      printk("error: PS2 port 1 failed self check\n");

   /* Re-write controller configuration back out */
   ps2_write_cmd(PS2_CMD_WRITE_CNTL_CFG, cntl_cfg);

   /* Perform interface test */
   resp = ps2_read_cmd(PS2_CMD_P1_INTERFACE_TEST);
   if (resp != 0x00)
      printk("error: PS2 port 1 interface test failed\n");

   outb(PS2_CMD_PORT, PS2_CMD_ENABLE_P1);

   /* Reset device on port 2 by writing it a 0xFF */
   ps2_write_data_p1(0xFF);
   resp = ps2_read_data_p1();
   if (resp != 0xFA)
      printk("error: PS2 device did not reset\n");
   resp = ps2_read_data_p1();
   if (resp != 0xAA)
      printk("error: PS2 device did not reset\n");

   /* setup poll loop just for now */
   //printk("Starting to loop\n");
   //while(1) {
   //   printk("%c", ps2_read_char((struct KeyboardDevice *)&gdev));
   //}

   return 0;
}