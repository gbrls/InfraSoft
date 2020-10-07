org 0x7c00 
jmp 0x0000:start

start:
	; nunca se esqueca de zerar o ds,
	; pois apartir dele que o processador busca os 
	; dados utilizados no programa.
	xor ax, ax
	mov ds, ax

	;Início do seu código
	
	
	
	

jmp $
times 510-($-$$) db 0
dw 0xaa55