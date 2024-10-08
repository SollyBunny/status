#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <time.h>

#include <pthread.h>

#include <X11/Xlib.h>

static struct timespec now = { 0, 0 };

#include "config.h"

#define SECOND 1000000000L

#ifndef TIMEINTERVALSECONDPRECISE
	#ifndef TIMEINTERVALSECONDS
		#error Define TIMEINTERVALSECONDS
	#endif
#endif

static char status[BUFFERSIZE];

static Display* dpy;
static Window root;

static inline void setstatus() {
	XStoreName(dpy, root, status);
	XFlush(dpy);
}

static inline char* createstatus(char *ptr, int len) {
	char* end = ptr + len;
	PARTS;
	return ptr;
}

int main() {

	if ((dpy = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}
	root = XDefaultRootWindow(dpy);

	clock_gettime(CLOCK_REALTIME, &now);

	while (1) {

		static char* end;
		end = createstatus(status, BUFFERSIZE - 1);
		*end = '\0';
		#ifdef DEBUG
			printf("%s %lu\n", status, end - status);
		#endif
		setstatus();

		#ifdef TIMEINTERVALSECONDPRECISE
			clock_gettime(CLOCK_REALTIME, &now);
			static struct timespec ts = { 0, 0 };
			ts.tv_nsec = SECOND - now.tv_nsec;
			nanosleep(&ts, NULL);
		#else
			static struct timespec ts = (struct timespec){ TIMEINTERVALSECONDS, 0 };
			nanosleep(&ts, NULL);
		#endif
		clock_gettime(CLOCK_REALTIME, &now);
		#ifdef DEBUG
			printf("%lf\n", (double)now.tv_sec + (double)now.tv_nsec / (double)SECOND);
		#endif

	}

	XCloseDisplay(dpy);

	return 0;
}
