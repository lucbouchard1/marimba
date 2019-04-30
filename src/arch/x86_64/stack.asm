global stack0_bottom
global stack0_top
global stack1_top
global stack1_bottom
global stack2_top
global stack2_bottom
global stack3_top
global stack3_bottom
global stack4_top
global stack4_bottom

section .bss
; Reserve bytes for stack 0 (kernel stack)
stack0_bottom:
   resb 4096
stack0_top:

; Reserve bytes for stack 1 (kernel stack)
stack1_bottom:
   resb 4096
stack1_top:

; Reserve bytes for stack 2 (kernel stack)
stack2_bottom:
   resb 4096
stack2_top:

; Reserve bytes for stack 3 (kernel stack)
stack3_bottom:
   resb 4096
stack3_top:

; Reserve bytes for stack 4 (kernel stack)
stack4_bottom:
   resb 4096
stack4_top: