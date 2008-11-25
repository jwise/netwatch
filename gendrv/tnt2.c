#include <io.h>
#include <pci.h>

static void *fbaddr;

static int tnt2_probe(struct pci_dev *pci, void *data)
{
	
}

static struct pci_id tnt2_pci[] = {
{0x10DE, 0x0028, "TNT2", "RIVA TNT2"}
};

struct pci_driver tnt2_driver = {
	.name     = "tnt2",
	.probe    = tnt2_probe,
	.ids      = tnt2_pci,
	.id_count = sizeof(tnt2_pci)/sizeof(tnt2_pci[0]),
};
