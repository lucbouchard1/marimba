#include "../../io.h"
#include "../../string.h"
#include "../../klog.h"
#include "../../kmalloc.h"
#include "../../proc.h"
#include "../../files.h"
#include "../../interrupts.h"
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

#define PS2_BUFF_SIZE 128
#define BUFF_POS(pos) ((pos == -1 ? PS2_BUFF_SIZE-1 : pos) % PS2_BUFF_SIZE)

struct OpenFile *ps2_open(struct File *file, uint32_t flags);
int ps2_read(struct OpenFile *fd, char *dest, size_t len);
void ps2_close(struct OpenFile *fd);

struct OpenPS2 {
   struct OpenFile fd;
   int pos;
};

static struct PS2Device {
   struct CharDevice cdev;
   struct ProcessQueue blocked_procs;
   uint8_t shift_pressed;
   char data[PS2_BUFF_SIZE];
   int pos;
} ps2_dev = {
   .cdev.num_open = 0,
   .shift_pressed = 0,
   .pos = 0,
   .blocked_procs = PROC_QUEUE_INIT(ps2_dev.blocked_procs)
};

struct FileOps ps2_fops = {
   .open = &ps2_open,
   .close = &ps2_close,
   .read = &ps2_read,
};

static inline char consumer_read_next(struct OpenPS2 *user)
{
   char ret = ps2_dev.data[user->pos];
   user->pos = BUFF_POS(user->pos+1);
   return ret;
}

static inline void producer_add(char c)
{
   ps2_dev.data[ps2_dev.pos] = c;
   ps2_dev.pos = BUFF_POS(ps2_dev.pos+1);
}

static inline int consumer_has_bytes(struct OpenPS2 *user)
{
   return !(user->pos == ps2_dev.pos);
}

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

static int ps2_char_avail()
{
   return inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT;
}

char ps2_read_char()
{
   struct PS2Device *dev = &ps2_dev;
   uint8_t scode;

   if (!ps2_char_avail())
      return 0;

   scode = ps2_read_data_p1();
   if (scode == LEFT_SHIFT_SCAN_CODE || scode == RIGHT_SHIFT_SCAN_CODE)
      dev->shift_pressed = 1;
   else if (scode != RELEASED_SCAN_CODE)
      return translate_scan_code(dev->shift_pressed, scode);

   if (scode == RELEASED_SCAN_CODE) {
      scode = ps2_read_data_p1();
      if (scode == LEFT_SHIFT_SCAN_CODE ||
            scode == RIGHT_SHIFT_SCAN_CODE)
         dev->shift_pressed = 0;
   }

   return 0;
}

static void ps2_isr(int irq, int err, void *arg)
{
   char c;

   if (!(c = ps2_read_char()))
      return;

   producer_add(c);
   PROC_unblock_all(&ps2_dev.blocked_procs);
}

/* TODO: Dont initialize interrupts and stuff on init, but on first open */
int init_ps2()
{
   uint8_t cntl_cfg, resp;

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
      klog(KLOG_LEVEL_ERR, "PS2 port 1 failed self check");

   /* Re-write controller configuration back out */
   ps2_write_cmd(PS2_CMD_WRITE_CNTL_CFG, cntl_cfg);

   /* Perform interface test */
   resp = ps2_read_cmd(PS2_CMD_P1_INTERFACE_TEST);
   if (resp != 0x00)
      klog(KLOG_LEVEL_ERR, "PS2 port 1 interface test failed");

   outb(PS2_CMD_PORT, PS2_CMD_ENABLE_P1);

   /* Reset device on port 2 by writing it a 0xFF */
   ps2_write_data_p1(0xFF);
   resp = ps2_read_data_p1();
   if (resp != 0xFA)
      klog(KLOG_LEVEL_ERR, "PS2 device did not reset");
   resp = ps2_read_data_p1();
   if (resp != 0xAA)
      klog(KLOG_LEVEL_ERR, "PS2 device did not reset");

   IRQ_set_handler(0x21, ps2_isr, NULL);
   IRQ_clear_mask(0x21);

   FILE_cdev_init(&ps2_dev.cdev);
   ps2_dev.cdev.fops = &ps2_fops;
   FILE_register_chrdev(&ps2_dev.cdev, "ps2");
   return 0;
}

int cleanup_ps2()
{
   // TODO: Implement IRQ_release_handler!!
   IRQ_set_mask(0x21);
   return 0;
}

struct OpenFile *ps2_open(struct File *file, uint32_t flags)
{
   struct OpenPS2 *ret;

   ret = kmalloc(sizeof(struct OpenPS2));
   if (!ret)
      return NULL;
   ret->fd.file = file;

   IRQ_disable();
   ps2_dev.cdev.num_open++;
   ret->pos = ps2_dev.pos;
   IRQ_enable();

   return (struct OpenFile *)ret;
}

int ps2_read(struct OpenFile *fd, char *dest, size_t len)
{
   struct OpenPS2 *user = (struct OpenPS2 *)fd;
   char *curr = dest;

   IRQ_disable();
   while (!consumer_has_bytes(user)) {
      PROC_block_on(&ps2_dev.blocked_procs, 1);
      IRQ_disable();
   }

   for (; len && consumer_has_bytes(user); len--, curr++)
      *curr = consumer_read_next(user);

   IRQ_enable();
   return curr - dest;
}

void ps2_close(struct OpenFile *fd)
{
   struct OpenPS2 *user = (struct OpenPS2 *)fd;

   kfree(user);

   IRQ_disable();
   ps2_dev.cdev.num_open--;
   if (!ps2_dev.cdev.num_open)
      cleanup_ps2();
   IRQ_enable();
}