#ifndef __X86_64_SYSCALL_H__
#define __X86_64_SYSCALL_H__

static inline void yield(void)
{
   asm volatile ( "mov $10, %%rbx; INT $123;" : : : "rbx" );
}

#endif