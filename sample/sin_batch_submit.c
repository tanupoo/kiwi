#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

struct key_keymap map[] = {
    { "http://fiap.tanu.org/test/sin0001",
	"k4814162ce50b1a0bc25e7c6a5a5631ad40d65c8e" },
    { "http://fiap.tanu.org/test/cos0001",
	"k77838fb52e904cf55cea4e4e03678c681336a7b6" }
};

#ifdef USE_KIWI_SERVER
char *server_addr = NULL;
char *server_port = "18880";
#endif

#ifdef USE_KIWI_CLIENT
char *submit_point = "http://localhost:18880";
#endif

int debug = 0;

char *prog = NULL;
char *db_name = "wren.db";

int
main()
{
	double i = 0;
	char d1[10], d2[10];
	char s_time[KIWI_TIME_STRLEN];
	struct chunk_data *head = NULL, *p_data;
	struct chunk_value *p_value;
	char buf[5120];
	int chunk = 10, num;

	num = 0;
	for (i = 0; ; i += 5) {
		snprintf(d1, sizeof(d1), "%8.6f", sin(i/180*3.14));
		snprintf(d2, sizeof(d2), "%8.6f", cos(i/180*3.14));
		iso8601_get_time(s_time, sizeof(s_time), 0);

		printf("%s %s\n", d1, d2);

		p_data = chunk_data_add(&head, map[0].key);
		p_value = chunk_data_value_add(p_data, d1, s_time);
		db_insert_chunk_value(dbname, map[0].hash, p_value);
		db_clean(dbname, map[0].hash, 60);

		p_data = chunk_data_add(&head, map[1].key);
		p_value = chunk_data_value_add(p_data, d2, s_time);
		db_insert_chunk_value(dbname, map[1].hash, p_value);
		db_clean(dbname, map[1].hash, 60);

		num++;

		if (num == chunk) {
			ieee1888_encode(buf, sizeof(buf), head);
			printf("%s\n", buf);
			ieee1888_submit(buf, strlen(buf), s_peer);
			chunk_data_free(head);
			head = NULL;
			num = 0;
		}

		usleep(500000);
	}


	return 0;
}
