#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

/* MITM two USB serial ports and snoop on the data going back and forth */
/* hardcoded to /dev/ttyUSB[01] and 300 baud - modify as you see fit */

/* (C)2023 Cameron Kaiser. All rights reserved. BSD license. */

int porta, portb, who;
struct termios ttya, ttya_saved, ttyb, ttyb_saved;
struct sigaction hothotsig;

void
cleanup()
{
	(void)tcsetattr(porta, TCSAFLUSH, &ttya_saved);
	(void)tcsetattr(portb, TCSAFLUSH, &ttyb_saved);
	(void)close(porta);
	(void)close(portb);
}

void
hotsigaction(int hotsig)
{
	fprintf(stdout, "\n\nexiting on signal\n");
	cleanup();
	exit(0);
}

void
out(unsigned char c, int port)
{
	if (port != who) {
		who = port;
		fprintf(stdout, "\n"
"-- port %d ---------------------------------------------------------------------\n",
			who);
	}
	fprintf(stdout, " %02x ", c);
}

int
main(int argc, char **argv)
{
	fd_set master, rfd, wfd;
	struct timeval tv;
	unsigned char c;

	porta = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (tcgetattr(porta, &ttya_saved)) {
		perror("tcgetattr (A)");
		close(porta);
		return 1;
	}

	portb = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY);
	if (tcgetattr(portb, &ttyb_saved)) {
		perror("tcgetattr (B)");
		close(portb);
		close(porta);
		return 1;
	}

	atexit(cleanup);
	(void)memset(&hothotsig, 0, sizeof(hothotsig));
	hothotsig.sa_handler = hotsigaction;
	sigaction(SIGINT, &hothotsig, NULL);
	sigaction(SIGTERM, &hothotsig, NULL);
	sigaction(SIGHUP, &hothotsig, NULL);

	memset(&ttya, 0, sizeof(ttya));
	ttya.c_cflag = (CS8 | CREAD | CLOCAL); /* 300bps 8N1 no flow control */
	ttya.c_cc[VTIME] = 5;
	ttya.c_cc[VMIN] = 1;
	cfsetospeed(&ttya, B300);
	cfsetispeed(&ttya, B300);
	tcsetattr(porta, TCSANOW, &ttya);

	memset(&ttyb, 0, sizeof(ttyb));
	ttyb.c_cflag = (CS8 | CREAD | CLOCAL); /* 300bps 8N1 no flow control */
	ttyb.c_cc[VTIME] = 5;
	ttyb.c_cc[VMIN] = 1;
	cfsetospeed(&ttyb, B300);
	cfsetispeed(&ttyb, B300);
	tcsetattr(portb, TCSANOW, &ttyb);

	FD_ZERO(&master);
	FD_SET(porta, &master);
	FD_SET(portb, &master);
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	setvbuf(stdout, NULL, _IONBF, 0); /* select(STDOUT); $|++; */

	who = -1;
	for(;;) {
		rfd = master;
		wfd = master;

		/* wait for something to read */
		if (select(portb+1, &rfd, NULL, NULL, NULL) < 0)
			perror("select");

		/* immediately check if we have writeable ports */
		(void)select(portb+1, NULL, &wfd, NULL, &tv);

		if (FD_ISSET(porta, &rfd)) {
			if (read(porta, &c, 1) &&
					FD_ISSET(portb, &wfd)) {
				write(portb, &c, 1);
				out(c, 1);
			} else {
				perror("couldn't read/write A->B");
				return 1;
			}
		}
		if (FD_ISSET(portb, &rfd)) {
			if (read(portb, &c, 1) &&
					FD_ISSET(porta, &wfd)) {
				write(porta, &c, 1);
				out(c, 2);
			} else {
				perror("couldn't read/write B->A");
				return 1;
			}
		}
	}

	return 255; /* not reached */
}
