org 0x7c00 
jmp 0x0000:start

string db 100 dup(0)

start:
	xor ax, ax
	mov ds, ax

    mov di, string

read_key:
    mov ah, 00h ; input do teclado
    int 16h ; al vai guardar qual tecla foi pressionada

    stosb

    mov ah, 0eh ; printando a letra lida
    int 10h

    cmp al, 0dh ; testar se Enter foi pressionado
    jne read_key

    mov al, 10
    int 10h

print_sting:
    dec di

    mov ah, 0eh 
    mov al, [di] ; printar o valor para que di está apontando
    int 10h

    cmp di, string
    jne print_sting

jmp $
times 510-($-$$) db 0
dw 0xaa55			
