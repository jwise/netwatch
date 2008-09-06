	org 0xA8000
[bits 16]
entry:
	mov ax, 0xA800
	mov ds, ax		; Take us out of flat unreal mode, and
	mov es, ax		; put us in true real mode.
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0xA800:(entry2-0xA8000)	; Long jump to a correct cs.
entry2:
	lgdt [(gdtr-0xA8000)]		; Set up a new GDT.
	mov eax, 0x1
	mov cr0, eax			; ... and enter pmode!
	jmp long 0x10:continue		; Now longjmp into the new code.
[bits 32]
continue:
	mov ax, 0x08			; Set up segment selectors.
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, [dataptr]

	mov al, [needclear]
	cmp al, 0
	jz noclear
	mov al, 0			; clear BSS
	mov edi, [dataptr+4]
	mov ecx, [dataptr+8]
	rep stosb
	mov [needclear], al
	
noclear:
	mov eax, [dataptr+12]		; jump into C
	call eax

	mov al, 0x40			; ack the periodic IRQ
	mov dx, 0x834
	out dx, al
	
	mov dx, 0x830			; now ack the SMI itself
	in al, dx
	or al, 0x02
	and al, 0xBF
	out dx, al
	or al, 0x40
	out dx, al

	rsm				; and leave SMM

	align 0x4
gdtr:
	db 0x27, 0x00
	dd gdt
	align 0x4
gdt:
	db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x93, 0xCF, 0x00	; data segment
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9B, 0xCF, 0x00	; code segment
	db 0xFF, 0xFF, 0x00, 0x80, 0x0A, 0x9B, 0xCF, 0x00	; code segment for trampoline

needclear:
	db 0x01

dataptr:
	; 4 bytes of stack top
	; 4 bytes of BSS start
	; 4 bytes of BSS length
	; 4 bytes of C entry point
	; These show up 
