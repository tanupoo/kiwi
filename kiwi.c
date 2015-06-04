#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

void
kiwi_version(struct kiwi_ctx *kiwi)
{
	printf("kiwi version %s", KIWI_VERSION);
	printf("  functions:");
#ifdef USE_KIWI_SERVER
	printf(" server");
#endif
#ifdef USE_KIWI_CLIENT
	printf(" client");
#endif
	printf("\n");
}

int
kiwi_set_debug(struct kiwi_ctx *kiwi, int debug)
{
	kiwi->debug = debug;

	return 0;
}

void
kiwi_dump(char *data, int datalen)
{
	int i;

	for (i = 0; i < datalen; i++) {
		if (i != 0) {
			if (i % 16 == 0)
				printf("\n");
			else
			if (i % 4 == 0)
				printf(" ");
		}
		printf("%02x", data[i]&0xff);
	}
	printf("\n");
}

char *
kiwi_find_keymap_hash(struct kiwi_ctx *kiwi, const char *key)
{
	int i;

	if (kiwi->keymap_len == 0)
		return NULL;

	for (i = 0; i < kiwi->keymap_len; i++) {
		if (strcmp(kiwi->keymap[i].key, key) == 0)
			return kiwi->keymap[i].hash;
	}

	return NULL;
}

int
kiwi_set_keymap(struct kiwi_ctx *kiwi, struct kiwi_keymap *keymap, int keymap_len)
{
	kiwi->keymap = keymap;
	kiwi->keymap_len = keymap_len;

	return 0;
}

int
kiwi_set_server_param(struct kiwi_ctx *kiwi, char *server_addr, char *server_port)
{
	kiwi->server_addr = server_addr;
	kiwi->server_port = server_port;

	return 0;
}

int
kiwi_set_codec(struct kiwi_ctx *kiwi, int type)
{
	switch (type) {
	case KIWI_CODEC_IEEE1888:
		kiwi->encode = ieee1888_encode;
		//kiwi->decode = ieee1888_decode;
		break;
#ifdef USE_KIWI_CODEC_JSON
	case KIWI_CODEC_JSON:
		kiwi->encode = json_encode;
		//kiwi->decode = json_decode;
		break;
#endif
	default:
		errx(1, "%s: invalid codec type %d", __FUNCTION__, type);
	}
	kiwi->codec = type;

	return 0;
}

int
kiwi_set_client_param(struct kiwi_ctx *kiwi, int type, char *peer_name)
{
	switch (type) {
	case KIWI_TRANSPORT_HTTP:
		kiwi->transmit = ieee1888_transmit;
		kiwi->peer_name = peer_name;
		break;
	default:
		errx(1, "%s: invalid transport type %d", __FUNCTION__, type);
	}
	kiwi->transport = type;

	return 0;
}

struct kiwi_ctx *
kiwi_init()
{
	struct kiwi_ctx *new;

	if ((new = calloc(1, sizeof(*new))) == NULL)
		err(1, "calloc(kiwi_ctx)");

	return new;
}
