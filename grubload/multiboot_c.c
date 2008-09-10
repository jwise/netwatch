void c_start(unsigned int magic, void *wee)
{
	*(unsigned char *)0xB8000 = 'A';
	while (1)
		;
}
