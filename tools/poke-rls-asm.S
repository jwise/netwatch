.global poke
poke:
	mov 8(%esp), %ecx

	# Parameter: I/O port to set bit 2 of
	mov 4(%esp), %edx
	inl %dx, %eax
	or $4, %eax
	outl %eax, %dx

	# Maybe SMM poked something?
	mov %ecx, %eax
	ret
