#include <minilib.h>
#include <output.h>

unsigned long memory_v2p(void *virt)
{
	unsigned long _virt = (unsigned long)virt;
	
	if (_virt >= 0xA0000 && _virt < 0xC0000)
		return _virt;
	if (_virt >= 0x200000 && _virt < 0x300000)
		return _virt - 0x200000 + /* XXX */ 0x1FF80000;
	outputf("WARNING: v2p(%08x)", _virt);
	return 0xFFFFFFFF;
}

void *memory_p2v(unsigned long phys)
{
	if (phys >= 0xA0000 && phys < 0xC0000)
		return (void*)phys;
	if (phys >= 0x1FF80000 && phys < 0x20000000)
		return (void*)(phys - 0x1FF80000 + 0x200000);
	outputf("WARNING: p2v(%08x)", phys);
	return (void *)0xFFFFFFFF;
}
