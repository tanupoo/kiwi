/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

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
	kiwi_config_print(kiwi);
	kiwi_config_set_keymap(kiwi);
	kiwi_find_keymap_hash(kiwi, "key3");
	kiwi_find_keymap_hash(kiwi, "key2");
	kiwi_find_keymap_hash(kiwi, "key1");

	return 0;
}
