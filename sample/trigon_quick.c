#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

struct kiwi_keymap map[] = {
    { "http://fiap.tanu.org/test/sin0001",
	"k4814162ce50b1a0bc25e7c6a5a5631ad40d65c8e" },
    { "http://fiap.tanu.org/test/cos0001",
	"k77838fb52e904cf55cea4e4e03678c681336a7b6" },
    { "http://fiap.tanu.org/test/tan0001",
	"k85ae23c4a7955ba1572f605fe971b706de4505b3" }
};

#ifdef USE_KIWI_SERVER
char *server_addr = NULL;
char *server_port = "18880";
#endif

#ifdef USE_KIWI_CLIENT
char *submit_point = "http://localhost:18880";
#endif

char *slite3_db_name = "wren.db";

int f_debug = 0;

char *prog_name = NULL;

void
usage()
{
	printf(
"Usage: %s [-dh]\n"
	, prog_name);

	exit(0);
}

int
trigon_output(struct kiwi_ctx *kiwi)
{
	static double theta = 0;
	char d1[10], d2[10], d3[10];
	char s_time[KIWI_TIME_MAXLEN];
	struct kiwi_chunk_key *head = NULL;

	kiwi_get_time(s_time, sizeof(s_time), 0);
	kiwi_chunk_add(&head, s_key, asp->s_data, s_time);

	snprintf(d1, sizeof(d1), "%8.6f", sin(theta/180*3.14));
	snprintf(d2, sizeof(d2), "%8.6f", cos(theta/180*3.14));
	snprintf(d3, sizeof(d3), "%8.6f", tan(theta/180*3.14));
	kiwi_get_time(s_time, sizeof(s_time), 0);

	if (kiwi->debug)
		printf("%8s %8s %8s\n", d1, d2, d3);

	kiwi_chunk_add(&head, map[0].key, d1, s_time);
	kiwi_chunk_add(&head, map[1].key, d2, s_time);
	kiwi_chunk_add(&head, map[2].key, d3, s_time);

	kiwi_submit(kiwi, head);
	head = NULL;

	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;
	struct kiwi_ctx *kiwi;

	prog_name = 1 + rindex(argv[0], '/');

	while ((ch = getopt(argc, argv, "dh")) != -1) {
		switch (ch) {
		case 'd':
			f_debug++;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	kiwi = kiwi_init();
	kiwi_set_debug(kiwi, debug);

	kiwi_set_db(kiwi, KIWI_DBTYPE_RINGBUF, db_name, 60);
	kiwi_set_keymap(kiwi, map, KIWI_KEYMAP_SIZE(map));

	kiwi_set_codec(kiwi, KIWI_CODEC_IEEE1888);
#ifdef USE_KIWI_CLIENT
	kiwi_set_client_param(kiwi, KIWI_TRANSPORT_HTTP, peer_name);
#endif

	kiwi_set_io_event(kiwi, trigon_output);

#ifdef USE_KIWI_SERVER
	kiwi_set_server_param(kiwi, server_addr, server_port);
	kiwi_server_loop(kiwi);
#else	/* !USE_KIWI_SERVER */
	kiwi_io_loop(kiwi);
#endif

	return 0;
}

