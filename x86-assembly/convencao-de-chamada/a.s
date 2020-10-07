; este código é uma versão modificada do resultado de compilar 
; o arquivo C com este comando: 
; gcc -masm=intel -fno-asynchronous-unwind-tables -fno-dwarf2-cfi-asm -S a.c -o a.s 

d:
	push	ebp
	mov	ebp, esp
	mov	[ebp-4], edi
	mov	eax, [ebp-4]
	add	eax, eax
	pop	ebp
	ret
main:
	push	ebp
	mov	ebp, esp
	sub	esp, 16
	mov ebx, 42h
	mov	[ebp-8], ebx
	mov	eax, [ebp-8]
	mov	edi, eax
	call	d
	mov	[ebp-4], eax
	mov	eax, 0
	leave
	ret
