; realmode.asm
; Routines to return the system to real mode
; NetWatch multiboot loader
;
; Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
; This program is free software; you can redistribute and/or modify it under
; the terms found in the file LICENSE in the root of this source tree. 

	[bits 32]	; Starts in 32 bit mode, then will drop back later.
	org 0x4000
entry:
	cli
	; clean up 32 bit regs
	mov eax, 0x0
	mov ebx, 0x0
	mov ecx, 0x0
	mov edx, 0x0
	mov esi, 0x0
	mov edi, 0x0
	mov ebp, 0x0
	mov esp, 0x0
	
	lgdt [gdtp]
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	jmp 0x8:nextp	; Clear CS bits now too

nextp:
	
	mov eax, cr0
	xor eax, 0x1	; Clear PE
	mov cr0, eax
	
	jmp 0x0:nowreal	; and jmp away


	[bits 16]
nowreal:
	mov long eax, 0x0000
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov sp, 0xFFFF	; set up the stack

	mov al, 0xAB
	out 0x80, al

	cli
	lidt [idtp]

	mov si, RETMSG
	call disp

	int 0x19		; warm boot without clearing RAM.
	call handload_drive	; hopefully shouldn't happen
	; ok, we give up
	cli
halt:	hlt
	jmp halt

disp:
	lodsb
	or al, al
	jz .done
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	jmp disp
.done:
	ret

; handload_drive should be unnecessary; int 19 should take care of it for
; us...
handload_drive:
	mov ax, 0x07C0
	mov es, ax
	mov ax, 0x0201	; read one sector
	mov bx, 0x0000	; to the normal location
	mov cx, 0x0001	; disk sector 1
	mov dx, 0x0080	; drive 0
	int 0x13
	jc readerr
	
	mov si, READSUCC
	call disp
	
	jmp 0x7C0:0x0	; and return control to grub!
readerr:
	mov si, READFAIL
	call disp
	ret

RETMSG	db "SMM installer finished, booting your system of choice.",10,13,0
READSUCC db "Read successful; rebooting into boot sector.",10,13,0
READFAIL db "Boot disk read failure; system halted.",10,13,0


idtp	db 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00

gdtp	dw 0x1F
	dd gdt

gdt	dd 0, 0
	dd 0x0000FFFF, 0x00009e00	; CS
	dd 0x0000FFFF, 0x00009300	; DS
