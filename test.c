#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <math.h>

#include "wren_collect.h"

int
main()
{
/*
http://fiap.tanu.org/test/temp1
ke15af3b4c5783c56eb41c64a737735a8fc508800
 */

	char *dbname = "wren.db";
	char *tabname = "ke15af3b4c5783c56eb41c64a737735a8fc508800";
	double v, t;
	char s_data[64];
	char *data[1] = { s_data };

	t = 0;
	while (1) {
		v = sin(t/180*3.14);
		snprintf(s_data, sizeof(s_data), "%f", v);
		db_insert(dbname, tabname, data, 1, 0);
		db_clean(dbname, tabname, 60);
		printf("insert %f\n", v);
		sleep(1);
		t += 5;
	}

	return 0;
}
