#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "kiwi.h"

int
test_ieee1888()
{
	char s_time[30];
	struct kiwi_ctx *kiwi;
	struct kiwi_chunk_key *head = NULL;
	char s_val[16];
	char *server_url = "http://localhost:18880";

	srandom(time(NULL));

	kiwi = kiwi_init();
	kiwi_set_debug(kiwi, 99);
	kiwi_set_db(kiwi, KIWI_DBTYPE_RINGBUF, NULL, 60);
	kiwi_set_client(kiwi, KIWI_TRANSPORT_IEEE1888_SOAP, server_url, NULL);

	kiwi_get_strtime(s_time, sizeof(s_time), 0);

	snprintf(s_val, sizeof(s_val), "%.1f",
	    random() / (float)RAND_MAX * 40.0 - 10);

	kiwi_chunk_add(&head, "http://fiap.tanu.org/test/temperature",
	    s_val, s_time);

	kiwi_submit_ldb(kiwi, head);
	kiwi_chunk_free(head);

	return 0;
}

int
main(int argc, char *argv[])
{
	test_ieee1888();

	return 0;
}
