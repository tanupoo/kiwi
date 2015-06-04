#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <err.h>

#include "kiwi.h"

char *key[] = {
      "key01",
      "key02",
      "key03",
      "key04",
      "key05",
};

int
test_kiwi_chunk_add_same_key()
{
	struct kiwi_chunk_key *head = NULL;
	int num = 5;
	time_t t0 = time(NULL);
	int i;
	char s_time[KIWI_TIME_MAXLEN];
	char s_value[16];
	long current_time;

	printf("\n## test: %s\n", __FUNCTION__);

	for (i = 0; i < num; i++) {
		current_time = (t0 + i) * 1000;
		kiwi_get_time(s_time, sizeof(s_time), current_time);
		snprintf(s_value, sizeof(s_value), "%.1f",
		    random() / (float)RAND_MAX * 40.0 - 10);
		(void)kiwi_chunk_add(&head, key[0], s_value, s_time);
	}

	kiwi_chunk_dump(head);

	kiwi_chunk_free(head);

	return 0;
}

int
test_kiwi_chunk_add_diff_keys()
{
	struct kiwi_chunk_key *head = NULL;
	int num = sizeof(key)/sizeof(key[0]);
	time_t t0 = time(NULL);
	int i;
	char s_time[KIWI_TIME_MAXLEN];
	char s_value[16];
	long current_time;

	printf("\n## test: %s\n", __FUNCTION__);

	for (i = 0; i < num; i++) {
		current_time = (t0 + i) * 1000;
		kiwi_get_time(s_time, sizeof(s_time), current_time);
		snprintf(s_value, sizeof(s_value), "%.1f",
		    random() / (float)RAND_MAX * 40.0 - 10);
		(void)kiwi_chunk_add(&head, key[i], s_value, s_time);
	}

	kiwi_chunk_dump(head);

	kiwi_chunk_free(head);

	return 0;
}

int
test_kiwi_chunk_add_key_then_value()
{
	struct kiwi_chunk_key *head = NULL, *p_key;
	int num = 5;
	time_t t0 = time(NULL);
	int i;
	char s_time[KIWI_TIME_MAXLEN];
	char s_value[16];
	long current_time;

	printf("\n## test: %s\n", __FUNCTION__);

	p_key = kiwi_chunk_add_key(&head, key[0]);

	for (i = 0; i < num; i++) {
		current_time = (t0 + i) * 1000;
		kiwi_get_time(s_time, sizeof(s_time), current_time);
		snprintf(s_value, sizeof(s_value), "%.1f",
		    random() / (float)RAND_MAX * 40.0 - 10);
		(void)kiwi_chunk_key_add_value(p_key, s_value, s_time);
	}

	kiwi_chunk_dump(head);

	kiwi_chunk_free(head);

	return 0;
}

int
main()
{
	srandom(time(NULL));

	test_kiwi_chunk_add_same_key();
	test_kiwi_chunk_add_diff_keys();
	test_kiwi_chunk_add_key_then_value();

	return 0;
}
