.global poke
poke:
	# Preload registers: $0x1BADD00D
	mov $0x1BADD00D, %ecx

	# Parameter: I/O port to set bit 2 of
	mov 4(%esp), %edx
	inl %dx, %eax
	or $4, %eax
	outl %eax, %dx

	# Maybe SMM poked something?
	mov %ecx, %eax
	ret
