#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"
#include <math.h>

struct kiwi_ctx *kiwi;	/* XXX should it be local variable. */

/* for server */
char *server_addr = NULL;
char *server_port = "18886";

/* for client */
char *server_url = "http://localhost:18880";
char *config_file = "trigon_quick.ini";

int f_debug = 0;

char *prog_name = NULL;

void
usage()
{
	printf(
"Usage: %s [-dh] [-c config] [-p port] [-s name]\n"
"    -c: specify the config file. (default: %s)\n"
"    -p: specify the port number to be listened. (default: %s)\n"
"    -s: specify the server name for publish. (default: %s)\n"
	, prog_name, config_file, server_port, server_url);

	exit(0);
}

int
trigon_output(void *cb_ctx)
{
	struct kiwi_ctx *kiwi = (struct kiwi_ctx *)cb_ctx;
	static double theta = 0;
	char val[10];
	char s_time[KIWI_TIME_MAXLEN];
	struct kiwi_chunk_key *head = NULL;

	kiwi_get_strtime(s_time, sizeof(s_time), 0);

	snprintf(val, sizeof(val), "%8.6f", sin(theta/180*3.14));
	kiwi_chunk_add(&head, kiwi->keymap[0]->key, val, s_time);
	if (kiwi->debug)
		printf("%8s ", val);

	snprintf(val, sizeof(val), "%8.6f", cos(theta/180*3.14));
	kiwi_chunk_add(&head, kiwi->keymap[0]->key, val, s_time);
	if (kiwi->debug)
		printf("%8s ", val);

	snprintf(val, sizeof(val), "%8.6f", tan(theta/180*3.14));
	kiwi_chunk_add(&head, kiwi->keymap[0]->key, val, s_time);
	if (kiwi->debug)
		printf("%8s\n", val);

	theta += 15;

#ifdef USE_KIWI_SERVER
	kiwi_submit_ldb(kiwi, head);
#endif
#ifdef USE_KIWI_CLIENT
	kiwi_submit_peer(kiwi, head);
#endif
	kiwi_chunk_free(head);

	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;

	prog_name = 1 + rindex(argv[0], '/');

	while ((ch = getopt(argc, argv, "c:p:dh")) != -1) {
		switch (ch) {
		case 'c':
			config_file = optarg;
			break;
		case 'p':
			server_url = optarg;
			break;
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

	/* load and check config */
	kiwi_config_load(kiwi, config_file);
	if (!kiwi_config_check_section(kiwi, "keymap"))
		errx(1, "keymap must be defined.");
	kiwi_config_set_keymap(kiwi);

	kiwi_set_debug(kiwi, f_debug);
	kiwi_set_db(kiwi, KIWI_DBTYPE_RINGBUF, NULL, 60);

	kiwi_set_event_timer(kiwi, 60000, trigon_output);

#ifdef USE_KIWI_CLIENT
	kiwi_set_client(kiwi, KIWI_TRANSPORT_IEEE1888_SOAP, server_url, NULL);
#endif

#ifdef USE_KIWI_SERVER
	kiwi_set_server(kiwi, server_addr, server_port, 4096, NULL);
	kiwi_server_loop(kiwi);
#else	/* !USE_KIWI_SERVER */
	kiwi_ev_loop(kiwi);
#endif

	return 0;
}

