#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "printk.h"

#define KLOG_LEVEL_DEBUG  1   // Debug messages.
#define KLOG_LEVEL_INFO   3   // Informational messages i.e. startup info.
#define KLOG_LEVEL_NOTICE 5   // Nothing serious but could indicate problems. Security problems.
#define KLOG_LEVEL_WARN   7   // Nothing serious but could indicate problems.
#define KLOG_LEVEL_ERR    9   // An error occurred. Often used by drivers.
#define KLOG_LEVEL_CRIT   11  // A serious failure occurred, but the system will remain up.
#define KLOG_LEVEL_EMERG  13  // A failure occurred and a crash is likely.

#ifndef KLOG_MIN_LOG_LEVEL
#define KLOG_MIN_LOG_LEVEL KLOG_LEVEL_DEBUG
#endif

static inline void log_level(int dbg_level)
{
   char *level_str;

   switch (dbg_level) {
      case KLOG_LEVEL_DEBUG:
         level_str = "dbg";
         break;

      case KLOG_LEVEL_INFO:
         level_str = "info";
         break;

      case KLOG_LEVEL_NOTICE:
         level_str = "notc";
         break;

      case KLOG_LEVEL_WARN:
         level_str = "warn";
         break;

      case KLOG_LEVEL_ERR:
         level_str = "err";
         break;

      case KLOG_LEVEL_CRIT:
         level_str = "crit";
         break;

      case KLOG_LEVEL_EMERG:
         level_str = "emrg";
         break;

      default:
         break;
   }
   printk("log[%s]: ", level_str);
}

#define klog(DBG_LEVEL, fmt, ...) \
do { \
   if (DBG_LEVEL > KLOG_MIN_LOG_LEVEL) { \
      log_level(DBG_LEVEL); \
      printk((fmt), ##__VA_ARGS__ ); \
      printk("\n"); \
   } \
} while (0); \

#endif