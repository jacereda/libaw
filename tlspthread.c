#include <pthread.h>
#include <stdlib.h>
#include "aw.h"
#include "awos.h"
#include "tls.h"

struct _tls {
	pthread_key_t key;
};

tls * tlsNew() {
	tls * tls = malloc(sizeof(*tls));
	pthread_key_create(&tls->key, 0);
	debug("created key %d", (int)tls->key);
	return tls;
}

void * tlsGet(tls * tls) {
	void * ret;
	ret = pthread_getspecific(tls->key);
	debug("get %d -> %p", (int)tls->key, ret);
	return ret;
}

void tlsSet(tls * tls, void * val) {
	debug("set %d <- %p", (int)tls->key, val);	
	pthread_setspecific(tls->key, val);
}
