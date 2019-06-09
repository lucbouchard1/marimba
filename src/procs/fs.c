#include "../files.h"
#include "../syscall.h"
#include "procs.h"

void filesystem_init(void *arg)
{
   BLK_open("ata");

   while (1)
      yield();
}