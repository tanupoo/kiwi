#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

int debug = 0;

void
usage()
{
	printf("Usage: this [-d] [-s speed] (dev)\n");
	exit(-1);
}

speed_t
get_brate(int speed)
{
	switch (speed) {
	case 0: return B0;
	case 50: return B50;
	case 75: return B75;
	case 110: return B110;
	case 134: return B134;
	case 150: return B150;
	case 200: return B200;
	case 300: return B300;
	case 600: return B600;
	case 1200: return B1200;
	case 1800: return B1800;
	case 2400: return B2400;
	case 4800: return B4800;
	case 9600: return B9600;
	case 19200: return B19200;
	case 38400: return B38400;
#ifndef _POSIX_SOURCE
	case 7200: return B7200;
	case 14400: return B14400;
	case 28800: return B28800;
	case 57600: return B57600;
	case 76800: return B76800;
	case 115200: return B115200;
	case 230400: return B230400;
#endif
	default: return speed;
	}
}

int
sio_init(char *device, int speed, int blocking)
{
	int fd;
	int mode;
	struct termios tty;
	speed_t brate;

	mode = O_RDWR;
	mode |= O_NOCTTY;
	if (!blocking)
		mode |= O_NONBLOCK;

	fd = open(device, mode);
	if (fd == -1)
		err(1, "open(%s)", device);

	memset(&tty, 0, sizeof(tty));
	if (tcgetattr (fd, &tty) == -1)
		err(1, "tcgetattr()");

	tty.c_iflag |= ICRNL;
	tty.c_iflag |= IGNPAR;
	tty.c_iflag &= ~IXON;
	tty.c_iflag &= ~IXOFF;
	tty.c_iflag &= ~ISTRIP;
#if 0
	tty.c_iflag = 0;
	tty.c_iflag |= BRKINT;
#endif

	tty.c_oflag = 0;

#if 0
	tty.c_cflag = 0;
#endif
	tty.c_cflag |= CREAD;
	tty.c_cflag |= CS8;
	tty.c_cflag |= CLOCAL;
	tty.c_cflag &= ~PARENB; 
	tty.c_cflag &= ~PARODD; 
	tty.c_cflag &= ~CRTSCTS; 

#if 0
	tty.c_lflag = 0;
#endif
	if (blocking)
		tty.c_lflag |= ICANON;
	else
		tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ISIG;
	tty.c_lflag &= ~ECHO;

#ifdef __linux__
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 5;
#else
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;
#endif

	brate = get_brate(speed);

	cfsetospeed(&tty, brate);
	cfsetispeed(&tty, brate);

	if (tcsetattr(fd, TCSANOW, &tty) == -1)
		err(1, "tcsetattr()");

	return fd;
}

void
debug_dump(char *buf, int datalen)
{
	int i;

	for (i = 0; i < datalen; i++) {
		printf("%c", buf[i]&0xff);
	}
	printf("\n");
}

/* for little endian */
#define SL08(a)	(*(a) & 0xff)
#define SL16(a)	(((*(a+1) << 8) & 0xff00) | (*(a) & 0xff))
#define SL24(a)	(((*(a+2) << 16) & 0xff0000) | ((*(a+1) << 8) & 0xff00) | (*(a) & 0xff))

/* for big endian */
#define SB08(a)	(*(a) & 0xff)
#define SB16(a)	(((*(a) << 8) & 0xff00) | (*(a+1) & 0xff))
#define SB24(a)	(((*(a) << 16) & 0xff0000) | ((*(a+1) << 8) & 0xff00) | (*(a+2) & 0xff))

/*
 * !0106039EBF3FA0
 */
int
parse(char *data, int datalen)
{
	char *p = data;
	char buf[10];

	p += 5;
	snprintf(buf, sizeof(buf), "%c%c", *p&0xff, *(p+1)&0xff);
	int data_size = atoi(buf);
	p += 2;
	snprintf(buf, sizeof(buf), "%c%c", *p&0xff, *(p+1)&0xff);
	int check_sum = atoi(buf);
	p += 2;
	snprintf(buf, sizeof(buf), "%c%c", *p&0xff, *(p+1)&0xff);
	int sensor_id = atoi(buf);
	p += 2;
	snprintf(buf, sizeof(buf), "%c%c%c%c",
	    *(p+2)&0xff, *(p+3)&0xff, *p&0xff, *(p+1)&0xff);
	float value = atof(buf) / 0xff;

	printf("%f\n", value);
	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;
	char buf[512];
	int fd;
	int recvlen, pos;
	int datalen = 16;
	int speed = 19200;
	int f_blocking = 1;

	while ((ch = getopt(argc, argv, "dh")) != -1) {
		switch (ch) {
		case 'd':
			debug++;
			break;
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	fd = sio_init(argv[0], speed, 1);

	pos = 0;
	while (1) {
		recvlen = read(fd, buf + pos, sizeof(buf));
		if (debug)
			printf("    recvlen=%d\n", recvlen);
		if (recvlen == 0)
			continue;
		if (recvlen == -1) {
			switch(errno) {
			case EAGAIN:
				if (debug)
					printf("    EAGAIN\n");
				continue;
			}
			err(1, "read()");
		}
		pos += recvlen;
		if (pos < datalen)
			continue;

		if (debug)
			debug_dump(buf, datalen);
		parse(buf, datalen);

		memcpy(buf, buf + pos - datalen, pos - datalen);
		pos = pos > datalen ? pos - datalen : 0;
	}

	exit(0);
}
