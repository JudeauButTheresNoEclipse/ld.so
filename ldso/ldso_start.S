	.global _start
	.hidden _start
	.type _start, @function
_start:
	xor %ebp, %ebp
	mov %rsp, %rdi
    leaq _DYNAMIC(%rip),%rsi
	call ldso_main

	.size _start, . - _start
