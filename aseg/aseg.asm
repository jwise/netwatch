	org 0xA8000
[bits 16]
entry:
	mov ax, 0xA800			; Take us out of flat unreal mode,
	mov ds, ax			; and put us in true real mode.
	mov es, ax
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
	mov esp, [dataptr]		; Load stack pointer.

	mov al, [needclear]		; Has the aseg been run before?
	cmp al, 0			; If so,
	jz noclear			; don't clear BSS.
	mov al, 0			; Otherwise, clear BSS.
	mov edi, [dataptr+4]
	mov ecx, [dataptr+8]
	rep stosb
	mov [needclear], al
	
noclear:
	mov eax, [dataptr+12]		; Load target jump address
	call eax			; then jump into C.

	rsm				; and leave SMM

	align 0x4
gdtr:
	db 0x1F, 0x00
	dd gdt
	align 0x4
gdt:
	db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	; initial null entry
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x93, 0xCF, 0x00	; data segment
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x9B, 0xCF, 0x00	; code segment

needclear:
	db 0x01

dataptr:
	; 4 bytes of stack top
	; 4 bytes of BSS start
	; 4 bytes of BSS length
	; 4 bytes of C entry point
	; These show up 
