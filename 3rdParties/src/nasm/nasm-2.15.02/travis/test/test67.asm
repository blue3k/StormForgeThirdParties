	bits 16

	mov ax,[bx]
	mov ax,[foo]
	mov ax,[word foo]
	mov ax,[dword foo]
	mov ax,[ebx]
	rep movsb
	a16 rep movsb
	a32 rep movsb
	a32 mov ax,bx

	bits 32

	mov ax,[bx]
	mov ax,[foo]
	mov ax,[word foo]
	mov ax,[dword foo]
	mov ax,[ebx]
	rep movsb
	a16 rep movsb
	a32 rep movsb

	bits 64

	mov ax,[rbx]
	mov ax,[foo]
	mov ax,[qword foo]
	mov ax,[dword foo]
	mov ax,[ebx]
	rep movsb
	a32 rep movsb
	a64 rep movsb

foo:
