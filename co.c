#include <Coro.h>
#include <stdlib.h>
#include "co.h"

struct _co {
	Coro * co;
	void * data;
	void (*func)(void*);
};

static co * g_curr;

co * coMain(void * data) {
	co * co = malloc(sizeof(*co));
	co->co = Coro_new();
	co->data = data;
	co->func = 0;
	Coro_initializeMainCoro(co->co);
	g_curr = co;
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
	co * curr = coCurrent();
	g_curr = next;
	if (next->func) {
		void (*func)(void*) = next->func;
		next->func = 0;
		Coro_startCoro_(curr->co, next->co,
				next->data, func);
	}
	else
		Coro_switchTo_(curr->co, next->co);
}

co * coCurrent() {
	return g_curr;
}

void * coData(co * co) {
	return co->data;
}
