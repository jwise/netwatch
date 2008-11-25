#include <pci.h>

extern struct pci_driver a3c90x_driver;
extern struct pci_driver tnt2_driver;

struct pci_driver *drivers[] =
{
	&a3c90x_driver,
	&tnt2_driver,
	0
};

