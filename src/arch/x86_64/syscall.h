#ifndef __X86_64_SYSCALL_H__
#define __X86_64_SYSCALL_H__

static inline void yield(void)
{
   asm volatile ( "mov $0, %%rbx; INT $123;" : : : "rbx" );
}

static inline void exit(void)
{
   asm volatile ( "mov $1, %%rbx; INT $123;" : : : "rbx" );
}


#endif