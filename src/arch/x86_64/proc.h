#ifndef __x86_64_PROC_H__
#define __x86_64_PROC_H__

void PROC_save_context(struct Process *proc, uint64_t *stack);
void PROC_load_context(struct Process *proc, uint64_t *stack);

#endif