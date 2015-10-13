#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

struct kiwi_keymap keymap[] = {
  { "key1", "kf86ae4231bef0081881043bea14ae149e7e439e0" },
  { "key2", "k096983f4a1e88b89fa0edd94bb694e33156608ef" },
  { "key3", "ke5e4c89f6706edab154ed987e11ee002bdddf99d" },
  { "key4", "kd10e56429f3d72f6547b33ad46ce83e79bec802b" },
};

int
test_kiwi_db_insert(struct kiwi_ctx *kiwi)
{
	struct kiwi_chunk_key *head = NULL;
	int keymap_len = sizeof(keymap)/sizeof(keymap[0]);
	time_t t0 = time(NULL);
	char s_time[KIWI_TIME_MAXLEN];
	char s_value[16];
	int i;
	long current_time;

	printf("\n## test: %s\n", __FUNCTION__);

	for (i = 0; i < keymap_len; i++) {
		current_time = (t0 + i) * 1000;
		kiwi_get_time(s_time, sizeof(s_time), current_time);
		snprintf(s_value, sizeof(s_value), "%.1f",
		    random() / (float)RAND_MAX * 40.0 - 10);
		(void)kiwi_chunk_add(&head, keymap[i].key, s_value, s_time);
	}

	kiwi_db_insert(kiwi, head);

	kiwi_chunk_dump(head);

	kiwi_chunk_free(head);

	return 0;
}

int
test_kiwi_db_get_latest(struct kiwi_ctx *kiwi)
{
	struct kiwi_xbuf *x_time, *x_value;
	int keymap_len = sizeof(keymap)/sizeof(keymap[0]);
	int i;

	printf("\n## test: %s\n", __FUNCTION__);

	x_time = kiwi_xbuf_new(KIWI_TIME_MAXLEN);
	x_value = kiwi_xbuf_new(KIWI_VALUE_MAXLEN);

	for (i = 0; i < keymap_len; i++) {
		kiwi_db_get_latest(kiwi, keymap[i].key, x_time, x_value);
		printf("%s %s %s\n", keymap[i].key, x_time->buf, x_value->buf);
	}

	kiwi_xbuf_free(x_time);
	kiwi_xbuf_free(x_value);

	return 0;
}

int
main(int argc, char *argv[])
{
	struct kiwi_ctx *kiwi;

	srandom(time(NULL));

	kiwi = kiwi_init();
	kiwi_set_db(kiwi, KIWI_DBTYPE_SQLITE3, "wren.db", 60);
	kiwi_set_keymap_tab(kiwi, keymap, sizeof(keymap)/sizeof(keymap[0]));

	test_kiwi_db_insert(kiwi);
	test_kiwi_db_get_latest(kiwi);

	kiwi_db_close(kiwi);

	return 0;
}
