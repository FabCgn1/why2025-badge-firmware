#include <stdlib.h>
#include <stdio.h>

extern void rust_main();

int main (void)
{
	puts ("=== Entering Rustland...");
	rust_main ();
	puts ("=== Exited Rustland.");
	exit (0);
}

