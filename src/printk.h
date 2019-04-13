#ifndef __PRINTK_H__
#define __PRINTK_H__

int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif