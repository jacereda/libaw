typedef struct _tls tls;
tls * tlsNew();
void * tlsGet(tls *);
void tlsSet(tls *, void *);
