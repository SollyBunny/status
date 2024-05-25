#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#define LENGTH 512
char status[LENGTH];

Display *dpy;
Window root;

inline void setstatus() {
	XStoreName(dpy, root, status);
    XFlush(dpy);
    
}

inline char* createstatus(char *ptr, int len) {
	char *end = ptr + len;
	PARTS;
	return ptr;
}

int main() {

    if ((dpy = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }
    root = XDefaultRootWindow(dpy);

    while (1) {
    	static char *end;
    	end = createstatus(status, LENGTH - 1);
		*end = '\0';
		#ifdef DEBUG
			printf("%s %lu\n", status, end - status);
		#endif
    	setstatus();
    }

    XCloseDisplay(dpy);

    return 0;
}
