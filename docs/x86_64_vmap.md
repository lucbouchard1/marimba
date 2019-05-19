# x86_64 Virtual Memory Layout

x86_64 uses a 4-level page table. Each page table spans a single page (4096 bytes), and contains a total of 512 8-byte entries. Each entry in the highest level page table (PML4E -> P4) can address 0x10000000000 bytes, or 512 GB. The virtual memory table below defines this size as B = 0x10000000000.

In this kernel, the kernel space is mapped into every page table to speed up and simplify context shifts.

## Virtual Memory Table
| Allocation                              | Start Address | End Address (Exclusive) |
| :-------------------------------------: | :-----------:  | :---------------------: |
| Physical Page Frames (Identity Mapped)  | 0x0 * B        |    0x1 * B             |
| Kernel Stacks                           | 0x1 * B        |    0x2 * B             |
| Reserved / Kernel Growth                | 0x2 * B        |    0xF * B             |
| Kernel Heap                             | 0xF * B        |    0x10 * B            |
| User Space                              | 0x10 * B       |    Onward              |

Note that the kernel code will be in the physical page frame region. By allocating 1 P4 entry to the physical page frames, our system can only address 512 GB.

Virtual Memory Map is described in src/arch/x86_64/mmap.h