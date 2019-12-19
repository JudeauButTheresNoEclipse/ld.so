.section .text
.global __reloc
.type __reloc, @function
__reloc:
    .cfi_startproc
	.cfi_adjust_cfa_offset	16
	pushfq				# save registers
	.cfi_adjust_cfa_offset	8
	/*.cfi_offset	%rflags, -16 */
	pushq	%rax
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%rax, -24
	pushq	%rcx
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%rcx, -32
	pushq	%rdx
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%rdx, -40
	pushq	%rsi
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%rsi, -48
	pushq	%rdi
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%rdi, -56
	pushq	%r8
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%r8, -64
	pushq	%r9
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%r9, -72
	pushq	%r10
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%r10, -80
	pushq	%r11
	.cfi_adjust_cfa_offset	8
	.cfi_offset	%r11, -88

	movq	80(%rsp), %rdi		# Copy of reloff
	movq	88(%rsp), %rsi		# Copy of obj
	call	foo		# Call the binder
    movq	%rax,88(%rsp)		# Store function to be called in obj

	popq	%r11			# restore registers
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%r11
	popq	%r10
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%r10
	popq	%r9
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%r9
	popq	%r8
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%r8
	popq	%rdi
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%rdi
	popq	%rsi
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%rsi
	popq	%rdx
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%rdx
	popq	%rcx
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%rcx
	popq	%rax
	.cfi_adjust_cfa_offset	-8
	.cfi_restore	%rax
	popfq
	.cfi_adjust_cfa_offset	-8
	/*.cfi_restore	%rflags */

	leaq	8(%rsp),%rsp		# Discard reloff, do not change eflags
	.cfi_adjust_cfa_offset	-8
	ret
	.cfi_endproc
