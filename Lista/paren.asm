org 0x7c00 
jmp 0x0000:start

digits db 2 dup(0) ; guardar os dois dígitos de n
n db 0 ; guardar o valor n
f db 0 ; flag
ans db 100 dup(0) ; onde as respostas vão ficar guardadas p printar depois

start:
	xor ax, ax
	xor bx, bx
	mov ds, ax

	mov di, digits
	
read_line:
	call getc
	call putc

	cmp al, 13
	je .skip

		sub al, '0'
		inc bl 
		stosb ; *(di++) = al

	.skip:

	cmp al, 13
	jne read_line

	call newline

	; al = dig[0];
	mov al, [digits]

	; if n != 1 then a *= 10
	cmp bl, 1
	je .skip2
		imul eax, 10
	.skip2:


	add al, [digits+1]
	mov bl, al
	mov [n], al

	mov di, ans

ntimes: ; vai chamar main_routine bl vezes

	dec bl

	call main_routine

	cmp bl, 0
	jne ntimes

	; printa @ pra marcar o fim do primeiro loop
	;mov al, '@'
	;call putc
	;call newline

	mov bl, [n]
	mov si, ans

printloop:
	dec bl

	;mov al, [si]
	;add al, 48

	mov al, [si]

	mov cl, al
	imul ecx, 6

	add al, 'S'
	sub al, cl

	inc si

	call putc
	call newline

	cmp bl, 0
	jne printloop

	jmp $


main_routine: ; main routine vai processar uma linha
	mov bp, sp
	xor dx, dx

	.read:
		call getc

		cmp al, 13
		jne .notend ; if al = RET then ...
			call newline
			jmp .clear

		.notend:

		call putc

		mov cl, 0

		cmp al, ')'
		jne .closeparen ; if al = ) then ...

			mov cl, 1
			cmp bp, sp 
			jbe .notempty ; if !stack.empty()

				pop eax
				mov ch, al
				xor eax, eax

				cmp ch, '('
				jne .notempty

				mov cl, 0

			.notempty:
			or dl, cl
		.closeparen:

		cmp al, ']'
		jne .closecolc ; if al = ] then ...

			mov cl, 1
			cmp bp, sp 
			jbe .notempty1 ; if !stack.empty()

				pop eax
				mov ch, al
				xor eax, eax

				cmp ch, '['
				jne .notempty1

				mov cl, 0

			.notempty1:
			or dl, cl
		.closecolc:

		cmp al, '}'
		jne .closechav ; if al = } then ...

			mov cl, 1
			cmp bp, sp 
			jbe .notempty2 ; if !stack.empty()

				pop eax
				mov ch, al
				xor eax, eax

				cmp ch, '{'
				jne .notempty2

				mov cl, 0

			.notempty2:
			or dl, cl
		.closechav:

		cmp al, '('
		jne .paren ; if al = ( then ...
			push eax
		.paren:

		cmp al, '['
		jne .colc ; if al = [ then ...
			push eax
		.colc:

		cmp al, '{'
		jne .chav ; if al = { then ...
			push eax
		.chav:

	jmp .read

	;push eax
	;push eax

	.clear:
	cmp sp, bp ; while(!s.empty) pop() // limpar a pilha
	je .skip
		mov dl, 1 ; se a pilha não está vazia, então a string está desbalanceada
		pop eax

		;call putc ; debug

		jmp .clear
	.skip:

	mov al, dl
	stosb ; ans[di++] := al

	ret

newline:
	mov al, 13
	call putc

	mov al, 10
	call putc

	ret

putc:
	mov ah, 0eh
	int 10h

	ret

getc:
	mov ah, 00h
	int 16h 

	ret

jmp $
times 510-($-$$) db 0
dw 0xaa55