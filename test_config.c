#include <sys/types.h>

#include <stdio.h>

#include "kiwi.h"

int
main(int argc, char* argv[])
{
	struct kiwi_ctx *kiwi;

	if (argc == 1) {
		printf("Usage: this (config)\n");
		return -1;
	}

	kiwi = kiwi_init();
	kiwi_config_load(kiwi, argv[1]);
	kiwi_config_dump(kiwi);

	return 0;
}
