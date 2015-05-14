#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

void
kiwi_ev_init(struct kiwi_ctx *kiwi)
{
	if ((kiwi->ev_ctx = calloc(1, sizeof(*kiwi->ev_ctx))) == NULL)
		err(1, "ERROR: %s: calloc(ev_ctx)", __FUNCTION__);
	simple_ev_init(kiwi->ev_ctx);
}

int
kiwi_set_event_sio(struct kiwi_ctx *kiwi, char *devname, int speed,
    int blocking, int buflen, int (*parse_func)(struct sio_ctx *), int debug)
{
	struct sio_ctx *sio_ctx;

	sio_ctx = sio_init(devname, speed, blocking, buflen, parse_func, kiwi, debug);
	simple_ev_set_sio(kiwi->ev_ctx, sio_readx, sio_ctx, sio_ctx->fd);

	return 0;
}

void
kiwi_ev_loop(struct kiwi_ctx *kiwi)
{
	fd_set rfd, wfd, efd;
	int fd_max;
	int nfd;
	struct timeval *timeout;

	simple_ev_init_fdset(kiwi->ev_ctx, &rfd, &wfd, &efd, &fd_max);

	while (1) {
		fd_max = 0;
		simple_ev_set_fdset(kiwi->ev_ctx, &fd_max);
		simple_ev_set_timeout(kiwi->ev_ctx, &timeout);

		nfd = select(fd_max, &rfd, &wfd, &efd, timeout);
		if (nfd < 0) {
			warn("select()");
			break;
		}

		/* check it anyway regardless of timeout */
		simple_ev_check(kiwi->ev_ctx);
	}
}

