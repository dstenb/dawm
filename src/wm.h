
#ifndef _WM_H_
#define _WM_H_

#include <stdlib.h>
#include <stdio.h>

struct wm {

	int restart;
};

struct wm *wm_init(void);

int wm_eventloop(struct wm *);

int wm_destroy(struct wm *);


#endif
