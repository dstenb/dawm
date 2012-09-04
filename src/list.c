#include "dawm.h"

static struct list *merge(struct list *, struct list *, int (void *, void *));
static struct list *merge_sort(struct list *, int (void *, void *));

struct list *
merge(struct list *a, struct list *b, int (*func) (void *, void *))
{
	struct list *r;

	if (!a)
		return b;
	if (!b)
		return a;

	if (func(a->data, b->data) < 0) {
		r = a;
		r->next = merge(a->next, b, func);
	} else {
		r = b;
		r->next = merge(a, b->next, func);
	}

	r->next->prev = r;
	return r;
}

struct list *
merge_sort(struct list *list, int (*func) (void *, void *))
{
	struct list *a, *b;

	if (!list || !list->next)
		return list;

	a = list;
	b = list->next;

	while (b && b->next) {
		list = list->next;
		b = list->next->next;
	}

	b = list->next;
	list->next = NULL;

	return merge(merge_sort(a, func), merge_sort(b, func), func);
}

struct list *
list_append(struct list *list, void *data)
{
	struct list *trav;
	struct list *new = list_create(data);

	if (list) {
		for (trav = list; trav->next; trav = trav->next) ;
		trav->next = new;
		new->prev = trav;
		return list;
	} else {
		return new;
	}

}

struct list *
list_create(void *data)
{
	struct list *list = xcalloc(1, sizeof(struct list));
	list->data = data;
	return list;
}

struct list *
list_destroy(struct list *list, void (*func) (void *))
{
	struct list *tmp;

	while (list) {
		tmp = list;
		list = list->next;
		if (func)
			func(tmp->data);
		free(tmp);
	}

	return NULL;
}

int
list_length(struct list *list)
{
	struct list *t;
	int i;

	for (i = 0, t = list; t; t = t->next, i++);

	return i;
}

struct list *
list_prepend(struct list *list, void *data)
{
	struct list *new = list_create(data);
	new->next = list;

	if (list)
		list->prev = new;
	return new;
}

struct list *
list_sort(struct list *list, int (*func) (void *, void *))
{
	if (!list || !list->next || !func)
		return list;
	return merge_sort(list, func);
}
