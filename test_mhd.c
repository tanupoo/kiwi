/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>	/* exit() */
#include <unistd.h>	/* getopt() */
#include <err.h>	/* err() */

#include "kiwi.h"

struct kiwi_keymap keymap[] = {
   { "http://fiap.tanu.org/test/alps/ws/e7/temperature",
     "k3eb4b4ebf49164a02a505ce3ceafaa476e32940f" },
   { "http://fiap.tanu.org/test/alps/ws/e7/humidity",
     "k3b05730fac6c7d2a476d97e55c32d739a4b833c6" },
   { "http://fiap.tanu.org/test/alps/ws/e7/pressure",
     "k383a11c439de28596f2a0a5e0f9486aaacf830fb" },
   { "http://fiap.tanu.org/test/alps/ws/e7/battery",
     "k54ea1765b67ed3f98f9a75589c30f62d9c7349d2" },
   { "http://fiap.tanu.org/test/alps/ws/e7/rssi",
     "k2e4c2f851a45c02baeafaf3f8a9312e685841f81" },
   { "http://fiap.tanu.org/test/alps/ws/d6/current",
     "kcc28f6d22d3c963d370c2c85fb8f0947e751a399" },
   { "http://fiap.tanu.org/test/alps/ws/d6/battery",
     "ka483f8552e7179aee1b4c9e8ed318a7ba0bd4e4c" },
   { "http://fiap.tanu.org/test/alps/ws/d6/rssi",
     "kd6d2b5f0b699f8f0bd7c0c3bc7c8ae3084872a7f" },
   { "http://fiap.tanu.org/test/alps/ws/f1/temperature",
     "k8c4c6398d419262e8e67dcb3b2dc47c9ce32680d" },
   { "http://fiap.tanu.org/test/alps/ws/f1/humidity",
     "k814bd991ec5dc7a6d0c69a13f400654ea2e97c17" },
   { "http://fiap.tanu.org/test/alps/ws/f1/light",
     "k6072dc08768e9cf882acd2b6bb2277260b773e55" },
   { "http://fiap.tanu.org/test/alps/ws/f1/rssi",
     "k4b8deb3e95226eac16d759c3ad1a10bda0b24df4" },
};

int
main(int argc, char *argv[])
{
	struct kiwi_ctx *kiwi;

	kiwi = kiwi_init();
	kiwi_set_debug(kiwi, 99);
	kiwi_set_server(kiwi, "0.0.0.0", "8080", 4096, NULL);
#ifdef USE_KIWI_DB_SQLITE3
	kiwi_set_db(kiwi, KIWI_DBTYPE_SQLITE3, "wren.db", 60);
	kiwi_set_keymap_tab(kiwi, keymap, KIWI_KEYMAP_SIZE(keymap));
#endif
	kiwi_server_loop(kiwi);

	return 0;
}

