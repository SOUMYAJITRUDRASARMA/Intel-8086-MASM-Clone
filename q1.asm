.model small
.stack 100h
.data
	x db "Name: Kanko Ghosh$"
	y db "Title: Program 1$"
.code
    push ax
	mov ax,@data	;load data addr to ax accumulator
	mov ds,ax		
	lea dx,x
	mov ah,09h
	int 21h

;newline
	mov dl,0ah
	mov ah,02h
	int 21h
;carriage return
	mov dl,0dh
	mov ah,02h
	int 21h

;prints the 2nd line
	lea dx,y
	mov ah,09h
	int 21h

;something related to end of the program..
	mov ah,4ch
	int 21h

end