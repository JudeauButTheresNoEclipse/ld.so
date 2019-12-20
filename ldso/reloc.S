.section .text
.global __reloc
.type __reloc, @function
__reloc:
	pushfq                                      // 8
	pushq	%rax                                // 8
	pushq	%rcx                                // 8
	pushq	%rdx                                // 8
	pushq	%rsi                                // 8
	pushq	%rdi    //saving registers          // 8
	pushq	%r8                                 // 8
	pushq	%r9                                 // 8
	pushq	%r10                                // 8
	pushq	%r11                                // 8
                                                // = 80

	movq	80(%rsp), %rdi //link map addr
	movq	88(%rsp), %rsi // relocation index
	call	runtime_relocations // calling resolution
    movq	%rax,88(%rsp)   // newly resolved address

	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	popq	%rax
	popfq

	leaq	8(%rsp),%rsp
	ret
