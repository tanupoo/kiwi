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

int
kiwi_mp_get_mean(struct kiwi_chunk_value *vhead, char *buf, int buflen)
{
	struct kiwi_chunk_value *q;
	double v_sum, v;
	char *ep;
	int n;

	/* XXX it must be atomic */
	v_sum = 0;
	n = 0;
	for (q = vhead; q != NULL; q = q->next) {
		v = strtod(q->value, &ep);
		if (*ep != '\0') {
			/* skip it */
			continue;
		}
		v_sum += v;
		n++;
	}
	if (n == 0) {
		snprintf(buf, buflen, "0");
		return 0;
	}

	snprintf(buf, buflen, "%f", v_sum / n);

	return 0;
}

int
kiwi_mp_get_max(struct kiwi_chunk_value *vhead, char *buf, int buflen)
{
	struct kiwi_chunk_value *q;
	int first = 1;
	double v_max, v;
	char *s_max = NULL;
	char *ep;

	/* XXX it must be atomic */
	for (q = vhead; q != NULL; q = q->next) {
		v = strtod(q->value, &ep);
		if (*ep != '\0') {
			/* skip it */
			continue;
		}
		if (first) {
			first = 0;
			v_max = v;
			s_max = q->value;
			continue;
		}
		if (v > v_max) {
			v_max = v;
			s_max = q->value;
			continue;
		}
	}

	snprintf(buf, buflen, "%s", s_max);

	return 0;
}

int
kiwi_mp_get_min(struct kiwi_chunk_value *vhead, char *buf, int buflen)
{
	struct kiwi_chunk_value *q;
	int first = 1;
	double v_min, v;
	char *s_min = NULL;
	char *ep;

	/* XXX it must be atomic */
	for (q = vhead; q != NULL; q = q->next) {
		v = strtod(q->value, &ep);
		if (*ep != '\0') {
			/* skip it */
			continue;
		}
		if (first) {
			first = 0;
			v_min = v;
			s_min = q->value;
			continue;
		}
		if (v < v_min) {
			v_min = v;
			s_min = q->value;
			continue;
		}
	}

	snprintf(buf, buflen, "%s", s_min);

	return 0;
}
