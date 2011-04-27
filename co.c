#include <Coro.h>
#include <stdlib.h>
#include "tls.h"
#include "co.h"

struct _co {
	Coro * co;
	void * data;
	void (*func)(void*);
};

static tls * g_curr = 0;

co * coMain(void * data) {
	co * co = malloc(sizeof(*co));
	co->co = Coro_new();
	co->data = data;
	co->func = 0;
	Coro_initializeMainCoro(co->co);
	if (!g_curr)
		g_curr = tlsNew();
	tlsSet(g_curr, co);
	return co;
}

co * coNew(void (*func)(void*), void * data) {
	co * co = malloc(sizeof(*co));
	co->co = Coro_new();
	Coro_setStackSize_(co->co, 8*1024*1024);
	co->data = data;
	co->func = func;
	return co;
}

void coDel(co * co) {
	Coro_free(co->co);
	free(co);
}

void coSwitchTo(co * next) {
	co * curr;
	curr = coCurrent();
	tlsSet(g_curr, next);
	if (next->func) {
		Coro_startCoro_(curr->co, next->co,
				next->data, next->func);
		next->func = 0;
	}
	else
		Coro_switchTo_(curr->co, next->co);
}

co * coCurrent() {
	return tlsGet(g_curr);
}

void * coData(co * co) {
	return co->data;
}
