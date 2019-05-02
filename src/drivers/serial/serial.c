#include "serial.h"

int SER_write_char(struct SerialDevice *dev, char c)
{
   return dev->write(dev, c);
}

int SER_write_str(struct SerialDevice *dev, const char *str)
{
   int ret;
   const char *curr;

   for (curr = str; *curr; curr++) {
      if ((ret = dev->write(dev, *curr)) < 0)
         return ret;
   }
   return 0;
}

int SER_write(struct SerialDevice *dev, const char *buff, size_t len)
{
   int curr, ret;

   for (curr = 0; curr < len; curr++) {
      if ((ret = dev->write(dev, buff[curr])) < 0)
         return ret;
   }
   return 0;
}