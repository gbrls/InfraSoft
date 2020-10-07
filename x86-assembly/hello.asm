org 0x7c00 
jmp 0x0000:start

; mesma coisa que 'Hello'+(CR)+'\n'+'\0';
hello db 'Hello There',13,10,0

start:
	; nunca se esqueca de zerar o ds,
	; pois apartir dele que o processador busca os 
	; dados utilizados no programa.

	xor ax, ax
	mov ds, ax

	mov ah, 0eh ; modo de imprimir
	mov si, hello
	call print_string

	jmp $

print_string:
	lodsb ; lodsb incrementa si
	cmp al, 0
	je .end

	int 10h ; interrupt da bios de video

	jmp print_string

	.end:
		ret

times 510-($-$$) db 0		; preenche o resto do setor com zeros 
dw 0xaa55			; coloca a assinatura de boot no final
				; do setor (x86 : little endian)
