#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

/* 300bps dummy receiver for Newsroom Wire Service */
/* pass path to USB device if not using /dev/ttyUSB0 */
/* modify baud rate if not using 300bps - modify as you see fit */

/* (C)2023 Cameron Kaiser. All rights reserved. BSD license. */

/*
 * known gaps:
 *
 * does not handle multifile transfers (needs to keep sending ACKs) - restart
   with the connection still in place to send another ACK
 * doesn't write to disk yet
 */

int porta;
struct termios ttya, ttya_saved;
struct sigaction hothotsig;

void
cleanup()
{
	(void)tcsetattr(porta, TCSAFLUSH, &ttya_saved);
	(void)close(porta);
}

void
hotsigaction(int hotsig)
{
	fprintf(stdout, "\n\nexiting on signal\n");
	cleanup();
	exit(0);
}

int
main(int argc, char **argv)
{
	fd_set master, rfd, wfd;
	struct timeval tv;
	unsigned char c;
	int i;

	if (argc == 2)
		porta = open(argv[1], O_RDWR | O_NOCTTY);
	else
		porta = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (tcgetattr(porta, &ttya_saved)) {
		perror("tcgetattr (A)");
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

	FD_ZERO(&master);
	FD_SET(porta, &master);
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	setvbuf(stdout, NULL, _IONBF, 0); /* select(STDOUT); $|++; */

	for(;;) {
		rfd = master;
		wfd = master;

		/* immediately check if we have writeable ports */
		(void)select(porta+1, NULL, &wfd, NULL, &tv);

		if (!FD_ISSET(porta, &wfd)) {
			perror("unable to send ACK");
			return 1;
		}
		c = 6;
		write(porta, &c, 1);
		fprintf(stdout, "--- 06 ---\n");

		/* read 134 byte packet */
		for(i=0;i<134;i++) {
			read(porta, &c, 1); /* blocks */
			fprintf(stdout, " %02x ", c);
		}
		c = 6;
		write(porta, &c, 1);
		fprintf(stdout, "\n--- 06 ---\n");
	}

	return 255; /* not reached */
}
