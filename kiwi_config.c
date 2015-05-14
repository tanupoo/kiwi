#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "inih/ini.h"

#include "kiwi.h"

#define CFS_DEFAULT   "default"
#define CFS_MODE      "mode"
#define CFS_QUICK     "quick"
#define CFS_BATCH     "batch"
#define CFS_SPAN      "span"
#define CFS_COMPRESS  "compress"
#define CFS_TRUE      "true"
#define CFS_FALSE     "false"
#define CFS_ENCODING  "encoding"
#define CFS_RAW       "raw"
#define CFS_XML       "xml"
#define CFS_JSON      "json"
#define CFS_MPACK     "mpack"
#define CFS_TRANSPORT "transport"
#define CFS_FILE      "file"
#define CFS_HTTP      "http"
#define CFS_COAP      "coap"
#define CFS_DEST      "dest"

#define CFV_MODE_QUICK     1
#define CFV_MODE_BATCH     2
#define CFV_TRUE           1
#define CFV_FALSE          0
#define CFV_ENCODING_RAW   1
#define CFV_CODEC_IEEE1888 2
#define CFV_CODEC_JSON     3
#define CFV_CODEC_CABO     4
#define CFV_TRANSPORT_FILE 1
#define CFV_TRANSPORT_HTTP 2
#define CFV_TRANSPORT_COAP 3

struct kiwi_cf_base {
	char *id;
	int mode;
	int compress;
	int encoding;
	int transport;
	int span;
	char *dest;
};

static void
cf_free_cf_base(struct kiwi_ctx *kiwi)
{
	int i;

	if (kiwi->cf == NULL)
		return;

	for (i = 0; i < kiwi->cf_num; i++) {
		free(kiwi->cf[i]->id);
		free(kiwi->cf[i]->dest);
		free(kiwi->cf[i]);
	}
	free(kiwi->cf);

	kiwi->cf = NULL;
	kiwi->cf_num = 0;
}

static struct kiwi_cf_base *
cf_new_cf_base(struct kiwi_ctx *kiwi, const char *section)
{
	struct kiwi_cf_base **new;
	int i = kiwi->cf_num;

	kiwi->cf_num++;

	if ((new = realloc(kiwi->cf, kiwi->cf_num * sizeof(struct kiwi_cf_base *))) == NULL)
		err(1, "realloc");

	kiwi->cf = new;

	if ((new[i] = malloc(sizeof(struct kiwi_cf_base))) == NULL)
		err(1, "malloc");

	memset(new[i], 0, sizeof(*new[i]));

	if ((new[i]->id = strdup(section)) == NULL)
		err(1, "strdup()");

	return new[i];
}

static struct kiwi_cf_base *
cf_get_cf_base(struct kiwi_ctx *kiwi, const char *section)
{
	int i;

	if (kiwi->cf_num == 0) {
		return cf_new_cf_base(kiwi, section);
	}

	for (i = 0; i < kiwi->cf_num; i++) {
		if (strcmp(kiwi->cf[i]->id, section) == 0)
			return kiwi->cf[i];
	}

	return cf_new_cf_base(kiwi, section);
}

static int
cf_handler(void *item, const char *section, const char *key,
                   const char *value)
{
	struct kiwi_ctx *kiwi = item;
	struct kiwi_cf_base *b;

	b = cf_get_cf_base(kiwi, section);

	if (strcmp(key, CFS_MODE) == 0) {
		if (strcmp(CFS_QUICK, value) == 0)
			b->mode = CFV_MODE_QUICK;
		else if (strcmp(CFS_BATCH, value) == 0)
			b->mode = CFV_MODE_BATCH;
		else {
			errx(1, "invalid mode, [%s] %s = %s",
			    section, key, value);
		}
	}
	else if (strcmp(key, CFS_SPAN) == 0) {
		b->span = atoi(value);
		if (b->span == 0) {
			errx(1, "invalid span, [%s] %s = %s",
			    section, key, value);
		}
	}
	else if (strcmp(key, CFS_COMPRESS) == 0) {
		if (strcmp(CFS_TRUE, value) == 0)
			b->compress = CFV_TRUE;
		else if (strcmp(CFS_FALSE, value) == 0)
			b->compress = CFV_FALSE;
		else {
			errx(1, "invalid compress type, [%s] %s = %s",
			    section, key, value);
		}
	}
	else if (strcmp(key, CFS_ENCODING) == 0) {
		if (strcmp(CFS_RAW, value) == 0)
			b->encoding = CFV_ENCODING_RAW;
		else if (strcmp(CFS_XML, value) == 0)
			b->encoding = CFV_ENCODING_XML;
		else if (strcmp(CFS_JSON, value) == 0)
			b->encoding = CFV_ENCODING_JSON;
		else if (strcmp(CFS_MPACK, value) == 0)
			b->encoding = CFV_ENCODING_MPACK;
		else {
			errx(1, "invalid encoding type, [%s] %s = %s",
			    section, key, value);
		}
	}
	else if (strcmp(key, CFS_TRANSPORT) == 0) {
		if (strcmp(CFS_FILE, value) == 0)
			b->transport = CFV_TRANSPORT_FILE;
		else if (strcmp(CFS_HTTP, value) == 0)
			b->transport = CFV_TRANSPORT_HTTP;
		else if (strcmp(CFS_COAP, value) == 0)
			b->transport = CFV_TRANSPORT_COAP;
		else {
			errx(1, "invalid transport type, [%s] %s = %s",
			    section, key, value);
		}
	}
	else if (strcmp(key, CFS_DEST) == 0) {
		if ((b->dest = strdup(value)) == NULL)
			err(1, "strdup(CFS_DEST)");
	}
	else {
		errx(1, "invalid key, [%s] %s = %s",
		    section, key, value);
	}

	return 1;
}

void
kiwi_config_dump(struct kiwi_ctx *kiwi)
{
	int i;

	for (i = 0; i < kiwi->cf_num; i++) {
		printf("[%s]\n", kiwi->cf[i]->id);
		printf("  mode = %d\n", kiwi->cf[i]->mode);
		printf("  span = %d\n", kiwi->cf[i]->span);
		printf("  compress = %d\n", kiwi->cf[i]->compress);
		printf("  encoding = %d\n", kiwi->cf[i]->encoding);
		printf("  transport = %d\n", kiwi->cf[i]->transport);
		printf("  dest = %s\n", kiwi->cf[i]->dest);
	}
}

void
kiwi_config_reload(struct kiwi_ctx *kiwi, char *config)
{
	cf_free_cf_base(kiwi);

	if (ini_parse(config, cf_handler, kiwi) < 0)
		errx(1, "can't load %s", config);
}

void
kiwi_config_load(struct kiwi_ctx *kiwi, char *config)
{
	if (kiwi->cf != NULL) {
		warnx("%s: config is already loaded.", __FUNCTION__);
		return;
	}

	kiwi_config_reload(kiwi, config);
}

