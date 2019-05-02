#include "../../io.h"
#include "../../interrupts.h"

#define OUTPUT_BUFF_SIZE 1024
#define BUFF_POS(pos) (pos % OUTPUT_BUFF_SIZE)

#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8
#define COM3_PORT 0x3E8
#define COM4_PORT 0x2E8

#define COM1_COM3_IRQ 0x24
#define COM2_COM4_IRQ 0x23

#define DATA_REG_OFFSET 0
#define DIVISOR_LSB_OFFSET 0
#define IE_REG_OFFSET 1
#define DIVISOR_MSB_OFFSET 1
#define FIFO_CNTL_REG_OFFSET 2
#define LINE_CNTL_REG_OFFSET 3
#define MODEM_CNTL_REG_OFFSET 4
#define LINE_STATUS_REG_OFFSET 5
#define MODEM_STATUS_REG_OFFSET 6

static struct x86SerialDevice{
   struct SerialDevice dev;
   char output_buff[OUTPUT_BUFF_SIZE];
   int out_producer_pos;
   int out_consumer_pos;
} gdev;

static int is_transmit_empty() {
   return inb(COM1_PORT + LINE_STATUS_REG_OFFSET) & 0x20;
}

void consumer_write_next(struct x86SerialDevice *dev)
{
   char next;

   /* Check if there is data in buffer */
   if (dev->out_consumer_pos == dev->out_producer_pos)
      return;

   next = dev->output_buff[dev->out_consumer_pos];
   dev->out_consumer_pos = BUFF_POS(dev->out_consumer_pos+1);
   outb(COM1_PORT, next);
}

void producer_add_char(struct x86SerialDevice *dev, char c)
{
   /* Check if buffer is full */
   if (dev->out_producer_pos == BUFF_POS(dev->out_consumer_pos - 1))
      return;

   dev->output_buff[dev->out_producer_pos] = c;
   dev->out_producer_pos = BUFF_POS(dev->out_producer_pos+1);
}

static void x86_serial_isr(int irq, int err, void *arg)
{
   struct x86SerialDevice *dev = (struct x86SerialDevice *)arg;

   if (is_transmit_empty())
      consumer_write_next(dev);
}

static int x86_write(struct SerialDevice *sdev, char c)
{
   struct x86SerialDevice *dev = (struct x86SerialDevice *)sdev;

   IRQ_disable(dev);

   producer_add_char(dev, c);

   if (is_transmit_empty())
      consumer_write_next(dev);

   IRQ_enable(dev);

   return 0;
}

struct SerialDevice *init_x86_serial()
{
   /* Disable all interrupts */
   outb(COM1_PORT + IE_REG_OFFSET, 0x00);
   /* Enable DLAB bit to configure baud rate */
   outb(COM1_PORT + LINE_CNTL_REG_OFFSET, 0x80);
   /* Set LSB of divisor to 3 and MSB of divisor to 0 */
   outb(COM1_PORT + DIVISOR_LSB_OFFSET, 0x03);
   outb(COM1_PORT + DIVISOR_MSB_OFFSET, 0x00);
   /* Disable DLAB, 8 bits, no parity, one stop (8n1) */
   outb(COM1_PORT + LINE_CNTL_REG_OFFSET, 0x03);
   /* Enable FIFO and clear. 14 byte threshold */
   outb(COM1_PORT + FIFO_CNTL_REG_OFFSET, 0xC7);
   /* IRQs enabled, RTS/DSR set */
   outb(COM1_PORT + MODEM_CNTL_REG_OFFSET, 0x0B);

   gdev.dev.write = &x86_write;

   /* Enable interrupts from COM1 and COM3 */
   IRQ_set_handler(COM1_COM3_IRQ, x86_serial_isr, &gdev);
   IRQ_clear_mask(COM1_COM3_IRQ);

   /* Enable transmit register empty interrupt */
   outb(COM1_PORT + IE_REG_OFFSET, 0x02);

   return (struct SerialDevice *)&gdev;
}