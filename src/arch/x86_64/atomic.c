#include "../../atomic.h"

void atomic_add(atomic_t *mem, int add)
{
   asm( "lock add %1, %0"
            : "=m"(*mem)
            : "r"(add)
            : "memory");
}

void atomic_sub(atomic_t *mem, int sub)
{
   asm( "lock sub %1, %0"
            : "=m"(*mem)
            : "r"(sub)
            : "memory");
}

int atomic_get(atomic_t *mem)
{
   return *mem;
}