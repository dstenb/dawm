#include "wm.h"

struct wm *wm_init(void)
{
	struct wm *wm;

	if (!(wm = malloc(sizeof(wm))))
		return NULL;

	wm->restart = 0;

	return wm;
}

int wm_eventloop(struct wm *wm)
{
	(void)wm;
	return 0;
}

int wm_destroy(struct wm *wm)
{
	(void)wm;
	return 0;
}
