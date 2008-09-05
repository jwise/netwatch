/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 * The author can be reached at:
 *
 *  Jan-Derk Bakker
 *  Information and Communication Theory Group
 *  Faculty of Information Technology and Systems
 *  Delft University of Technology
 *  P.O. Box 5031
 *  2600 GA Delft
 *  The Netherlands
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>


#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main(int argc, char **argv) {

	int fd, input_fd;

	void *map_base, *virt_addr; 
	unsigned long read_result, writeval;
	off_t target;
	int access_type = 'w';

	if(argc < 3) {
		fprintf(stderr, "\nUsage:\t%s { address } { file }\n"
			"\taddress : memory address to act upon\n"
			"\tfile    : file to load\n\n", argv[0]);
		exit(1);
	}

	input_fd = open(argv[2], O_RDONLY);
	if (input_fd < 0) FATAL;

	struct stat st;
	if (fstat(input_fd, &st) < 0) FATAL;

	char * buf = malloc(st.st_size);
	if (!buf) FATAL;
	if (read(input_fd, buf, st.st_size) != st.st_size) FATAL;

	target = strtoul(argv[1], 0, 0);

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) FATAL;

	printf("/dev/mem opened.\n"); 
	fflush(stdout);
    
	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE,
	                MAP_SHARED, fd, target & ~MAP_MASK);

	if (map_base == (void *) -1) FATAL;

	printf("Memory mapped at address %p.\n", map_base); 
	fflush(stdout);
    
	virt_addr = map_base + (target & MAP_MASK);
	memcpy(virt_addr, buf, st.st_size);

	printf("Copied %d bytes.\n", st.st_size);
	
	return 0;
}
