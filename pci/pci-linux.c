#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <inttypes.h>

static int _open(int bus, int slot, int fn)
{
	char fname[512];
	
	sprintf(fname, "/proc/bus/pci/%02x/%02x.%01x", bus, slot, fn);
	return open(fname, O_RDWR);
}

void pci_write32(int bus, int slot, int fn, int addr, uint32_t data)
{
	int fd;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
		return;
	lseek(fd, addr, SEEK_SET);
	write(fd, &data, 4);
	close(fd);
}

void pci_write16(int bus, int slot, int fn, int addr, uint16_t data)
{
	int fd;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
		return;
	lseek(fd, addr, SEEK_SET);
	write(fd, &data, 2);
	close(fd);
}

void pci_write8(int bus, int slot, int fn, int addr, uint8_t data)
{
	int fd;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
		return;
	lseek(fd, addr, SEEK_SET);
	write(fd, &data, 1);
	close(fd);
}

uint32_t pci_read32(int bus, int slot, int fn, int addr)
{
	int fd;
	uint32_t data;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
		return;
	lseek(fd, addr, SEEK_SET);
	read(fd, &data, 4);
	close(fd);
	
	return data;
}

uint16_t pci_read16(int bus, int slot, int fn, int addr)
{
	int fd;
	uint16_t data;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
	{
		perror("open");
		return;
	}
	lseek(fd, addr, SEEK_SET);
	read(fd, &data, 2);
	close(fd);
	
	return data;
}


uint8_t pci_read8(int bus, int slot, int fn, int addr)
{
	int fd;
	uint8_t data;
	
	fd = _open(bus, slot, fn);
	if (fd < 0)
	{
		perror("open");
		return;
	}
	lseek(fd, addr, SEEK_SET);
	read(fd, &data, 1);
	close(fd);
	
	return data;
}
