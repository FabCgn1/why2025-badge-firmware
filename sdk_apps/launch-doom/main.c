#include "badgevms/process.h"
#include <stdio.h>


int main (int argc, char *argv[])
{
	char *args[] = {
		"doomgeneric",
		"-iwad",
		"APPS:[doomgeneric]doom1.wad",
		NULL
	};
	pid_t pid;

	puts ("launching doom...");

	process_create (
		"APPS:[doomgeneric]doomgeneric.elf",
		65536,
		3,
		args
	);

	puts ("launched doom.");

	while (1) {
		puts ("waiting for doom to exit...");
		pid = wait (true, 1000);
		if (pid != -1)
			break;
	}

	printf ("doom exited: %d\n", pid);
	return 0;
}
