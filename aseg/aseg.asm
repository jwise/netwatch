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

;	mov al, [cstat]
;	add al, 1
;	out 0x80, al
;	mov [cstat], al
	
	mov dx, 0xCF8			; save off the old config value
	in dword eax, dx
	mov [esp-4], eax
	
	mov eax, 0x80000070		; load in smramc
	out dx, eax
	mov dx, 0xCFC
	in byte al, dx
	mov [esp-5], al
	and al, 0xF3			; Allow graphics access
	or al, 0x08
	out dx, al
	
	xor eax, eax
	mov dx, 0x3D4
	in byte al, dx
	mov [esp-6], al			; save off the old VGA command
	mov al, 0xC
	out dx, al
	inc dx
	in al, dx
	mov ah, al
	dec dx
	mov al, 0xD
	out dx, al
	inc dx
	in al, dx
	shl eax, 1
	add eax, 0xB8000		; yay
	mov byte [eax+0], '1'
	mov byte [eax+1], 0x1F
	mov byte [eax+2], '5'
	mov byte [eax+3], 0x1F
	mov byte [eax+4], '-'
	mov byte [eax+5], 0x1F
	mov byte [eax+6], '4'
	mov byte [eax+7], 0x1F
	mov byte [eax+8], '1'
	mov byte [eax+9], 0x1F
	mov byte [eax+10], '2'
	mov byte [eax+11], 0x1F
	
	mov dx, 0x3D4			; restore the old stuff
	mov al, [esp-6]
	out dx, al

	mov dx, 0xCFC			; restore smramc
	mov al, [esp-5]
	out dx, al

	mov dx, 0xCF8			; restore the old PCI config value
	mov eax, [esp-4]
	out dx, eax

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

needclear:
	db 0x01

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

cstat:
	db 0x00

TIMES   512-($-$$) DB 0
dataptr:
	; 4 bytes of stack top
	; 4 bytes of BSS start
	; 4 bytes of BSS length
	; 4 bytes of C entry point