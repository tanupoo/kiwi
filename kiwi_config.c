#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "kiwi.h"
#include "tini/tini.h"

void
kiwi_config_print(struct kiwi_ctx *kiwi)
{
	tini_print(kiwi->config);
}

int
kiwi_config_check_section(struct kiwi_ctx *kiwi, const char *s)
{
	return tini_get_sect(kiwi->config, s) == NULL ? 0 : 1;
}

const char *
kiwi_config_get_v(struct kiwi_ctx *kiwi, const char *s, const char *k)
{
	return tini_get_v(kiwi->config, s, k);
}

int
kiwi_config_set_keymap(struct kiwi_ctx *kiwi)
{
	struct tini_sect *sect;

	if ((sect = tini_get_sect(kiwi->config, "keymap")) == NULL)
		return -1;

	kiwi->keymap = (struct kiwi_keymap **)sect->kv;
	kiwi->keymap_len = sect->n_kv;

	return 0;
}

void
kiwi_config_load(struct kiwi_ctx *kiwi, const char *config_file)
{
	if (kiwi->config != NULL) {
		warnx("WARNING: %s: config %s is already loaded.  ignored.",
		    __FUNCTION__, config_file);
		return;
	}
	if (tini_parse(config_file, &kiwi->config) < 0)
		errx(1, "ERROR: %s: can't load %s", __FUNCTION__, config_file);
}

void
kiwi_config_reload(struct kiwi_ctx *kiwi, const char *config_file)
{
	tini_free(kiwi->config);
	kiwi->config = NULL;
	return kiwi_config_load(kiwi, config_file);
}
