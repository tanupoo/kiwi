/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

void
kiwi_ev_init(struct kiwi_ctx *kiwi)
{
	simple_ev_init(&(kiwi->ev_ctx));
}

/*
 * @param timer timer in miliseconds.
 */
int
kiwi_set_event_timer(struct kiwi_ctx *kiwi, int timer, int (*cb)(void *))
{
	struct timeval tv_timer;

	memset(&tv_timer, 0, sizeof(tv_timer));
	tv_timer.tv_sec = timer / 1000;
	tv_timer.tv_usec = timer % 1000;
	simple_ev_set_timer(kiwi->ev_ctx, cb, kiwi, &tv_timer, 1);

	return 0;
}

int
kiwi_set_event_sio(struct kiwi_ctx *kiwi, char *devname, int speed,
    int blocking, int buflen, int (*cb)(struct sio_ctx *))
{
	struct sio_ctx *sio_ctx;

	sio_ctx = sio_init(devname, speed, blocking, buflen, 0,
	    cb, kiwi, kiwi->debug);
	simple_ev_set_sio(kiwi->ev_ctx, sio_readx, sio_ctx, sio_ctx->fd);

	return 0;
}

void
kiwi_ev_loop(struct kiwi_ctx *kiwi)
{
	fd_set rfd, wfd, efd;
	int fd_max;
	int nfd;
	struct timeval *timeout = NULL;

	simple_ev_init_fdset(kiwi->ev_ctx, &rfd, &wfd, &efd, &fd_max);

	while (1) {
		fd_max = 0;
		simple_ev_set_fdset(kiwi->ev_ctx, &fd_max);
		simple_ev_set_timeout(kiwi->ev_ctx, &timeout);

		nfd = select(fd_max + 1, &rfd, &wfd, &efd, timeout);
		if (nfd < 0) {
			warn("ERROR: %s: select()", __FUNCTION__);
			break;
		}

		/* check it anyway regardless of timeout */
		simple_ev_check(kiwi->ev_ctx);
	}
}

