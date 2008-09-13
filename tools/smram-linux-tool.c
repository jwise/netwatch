#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <smram.h>

static struct option longopts[] = {
	{ "open",	no_argument,	NULL,	'o'	},
	{ "close",	no_argument,	NULL,	'c'	},
	{ "dump",	no_argument,	NULL,	'd'	},
	{ NULL,		0,		NULL,	0	}
};

#define OP_DUMP 1
#define OP_SET 2

void usage(int argc, char **argv)
{
	printf("Usage: %s [ --dump ] [ --open | --close ]\n",
		argv[0]);
	exit(1);
}

int main(int argc, char **argv)
{
	if (geteuid() != 0)
	{
		printf("%s: This program must be run as root.\n", argv[0]);
		return 1;
	}
	
	int ch;
	int op = 0;
	int do_open = 0;
	while ((ch = getopt_long(argc, argv, "ocsd:", longopts, NULL)) != -1)
	{
		switch (ch)
		{
		case 'o':
			if (op & OP_SET) usage(argc, argv);
			op |= OP_SET;
			do_open = 1;
			break;
		case 'c':
			if (op & OP_SET) usage(argc, argv);
			op |= OP_SET;
			break;
		case 'd':
			op |= OP_DUMP;
			break;
		default:
			usage(argc, argv);
		}
	}

	if (!op) usage(argc, argv);

	if (op & OP_DUMP)
	{
		smram_aseg_dump();
	}

	if (op & OP_SET)
	{
		smram_aseg_set_state(do_open ? SMRAM_ASEG_OPEN : SMRAM_ASEG_SMMONLY);
	}
	
	return 0;
}
