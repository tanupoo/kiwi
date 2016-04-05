#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "kiwi.h"

int f_debug = 0;

struct kiwi_keymap keymap[] = {
  { "key1", "kf86ae4231bef0081881043bea14ae149e7e439e0" },
  { "key2", "k096983f4a1e88b89fa0edd94bb694e33156608ef" },
  { "key3", "ke5e4c89f6706edab154ed987e11ee002bdddf99d" },
  { "key4", "kd10e56429f3d72f6547b33ad46ce83e79bec802b" },
};

void
test_submit()
{
	char s_time[30];
	struct kiwi_ctx *kiwi;
	struct kiwi_chunk_key *head = NULL;
	char s_val[3][16];

	srandom(time(NULL));

	kiwi = kiwi_init();

	kiwi_set_debug(kiwi, f_debug);
#ifdef USE_KIWI_DB_SQLITE3
	kiwi_set_db(kiwi, KIWI_DBTYPE_SQLITE3, "wren.db", 60);
	kiwi_set_keymap_tab(kiwi, keymap, sizeof(keymap)/sizeof(keymap[0]));
#else
	kiwi_set_db(kiwi, KIWI_DBTYPE_RINGBUF, NULL, 6);
	kiwi_set_keymap_tab(kiwi, keymap, sizeof(keymap)/sizeof(keymap[0]));
#endif

	kiwi_get_strtime(s_time, sizeof(s_time), 0);

	snprintf(s_val[0], sizeof(s_val[0]), "%.1f",
	    random() / (float)RAND_MAX * 40.0 - 10);
	snprintf(s_val[1], sizeof(s_val[1]), "%.1f",
	    random() / (float)RAND_MAX * 99.0);
	snprintf(s_val[2], sizeof(s_val[2]), "%.1f",
	    random() / (float)RAND_MAX * 1000);

	kiwi_chunk_add(&head, "key1", s_val[0], s_time);
	kiwi_chunk_add(&head, "key2", s_val[1], s_time);
	kiwi_chunk_add(&head, "key3", s_val[2], s_time);

	kiwi_submit_ldb(kiwi, head);
	kiwi_chunk_free(head);
}

int
main(int argc, char *argv[])
{
	test_submit();

	return 0;
}
