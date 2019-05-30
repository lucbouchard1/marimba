#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "printk.h"

#define DBG_LEVEL_DEBUG  1   // Debug messages.
#define DBG_LEVEL_INFO   3   // Informational messages i.e. startup info.
#define DBG_LEVEL_NOTICE 5   // Nothing serious but could indicate problems. Security problems.
#define DBG_LEVEL_WARN   7   // Nothing serious but could indicate problems.
#define DBG_LEVEL_ERR    9   // An error occurred. Often used by drivers.
#define DBG_LEVEL_CRIT   11  // A serious failure occurred, but the system will remain up.
#define DBG_LEVEL_EMERG  13  // A failure occurred and a crash is likely.

static inline void log_level(int dbg_level)
{
   char *level_str;

   switch (dbg_level) {
      case DBG_LEVEL_DEBUG:
         level_str = "dbg";
         break;

      case DBG_LEVEL_INFO:
         level_str = "info";
         break;

      case DBG_LEVEL_NOTICE:
         level_str = "notc";
         break;

      case DBG_LEVEL_WARN:
         level_str = "warn";
         break;

      case DBG_LEVEL_ERR:
         level_str = "err";
         break;

      case DBG_LEVEL_CRIT:
         level_str = "crit";
         break;

      case DBG_LEVEL_EMERG:
         level_str = "emrg";
         break;

      default:
         break;
   }
   printk("log[%s]: ", level_str);
}

#define klog(DBG_LEVEL, fmt, ...) \
do { \
   log_level(DBG_LEVEL); \
   printk((fmt), ##__VA_ARGS__ ); \
   printk("\n"); \
} while (0); \

#endif