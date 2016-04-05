#include <sys/types.h>
#include <stdint.h>
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
		if (strcmp(kiwi->keymap[i]->key, key) == 0)
			return kiwi->keymap[i]->hash;
	}

	return NULL;
}

int
kiwi_set_keymap_tab(struct kiwi_ctx *kiwi, struct kiwi_keymap *keymap_tab, int keymap_len)
{
	struct kiwi_keymap **new;
	int i;

	if ((new = calloc(keymap_len, sizeof(struct kiwi_keymap *))) == NULL)
		err(1, "%s: calloc(keymap)", __FUNCTION__);

	for (i = 0; i < keymap_len; i++)
		new[i] = &keymap_tab[i];

	kiwi->keymap = new;
	kiwi->keymap_len = keymap_len;

	return 0;
}

#ifdef USE_KIWI_SERVER
int
kiwi_set_server(struct kiwi_ctx *kiwi, char *server_addr, char *server_port,
    int buffer_size, char *root_dir)
{
	if (server_addr != NULL)
		kiwi->server_addr = strdup(server_addr);
	if (server_port != NULL)
		kiwi->server_port = strdup(server_port);
	if (root_dir != NULL)
		kiwi->server_root_dir = strdup(root_dir);
	kiwi->encode_buffer_size = buffer_size;

	return 0;
}
#endif /* USE_KIWI_SERVER */

#ifdef USE_KIWI_CLIENT
/*
 * set transport type, server_url, and the parameters..
 * the data pointed by the value of param must be allocated
 * in the non-volatile memory so that the internal functions can refer to it.
 * @param server_url string server url
 * @param param protocol specific parameters
 */
int
kiwi_set_client(struct kiwi_ctx *kiwi, int type, char *server_url, void *param)
{
	kiwi->transport = type;
	kiwi->client_param = param;
	if (server_url != NULL)
		kiwi->server_url = strdup(server_url);

	switch (type) {
	case KIWI_TRANSPORT_IEEE1888_SOAP:
		kiwi->encode = if_ieee1888_soap_encode;
		kiwi->transmit = if_ieee1888_http_transmit;
		if_ieee1888_init(kiwi);
		break;
	case KIWI_TRANSPORT_IEEE1888_JSON:
		kiwi->encode = if_ieee1888_json_encode;
		kiwi->transmit = if_ieee1888_http_transmit;
		if_ieee1888_init(kiwi);
		break;
#ifdef USE_KIWI_TRANSPORT_KII_HTTP
	case KIWI_TRANSPORT_KII_HTTP:
		kiwi->encode = if_kii_encode;
		kiwi->transmit = if_kii_http_transmit;
		if_kii_init(kiwi);
		break;
#endif
	default:
		errx(1, "ERROR: %s: invalid transport type %d",
		    __FUNCTION__, type);
	}

	return 0;
}
#endif /* USE_KIWI_CLIENT */

struct kiwi_ctx *
kiwi_init()
{
	struct kiwi_ctx *new;

	if ((new = calloc(1, sizeof(*new))) == NULL)
		err(1, "ERROR: %s: calloc(kiwi_ctx)", __FUNCTION__);

	new->submit_buffer_size = KIWI_SUBMIT_BUFFER_SIZE;
	kiwi_ev_init(new);

	return new;
}
