typedef struct _co co;
co * coMain();
co * coNew(void (*)(void*), void *);
void coSwitch(co *, co *);
co * coCurrent();
void * coData(co *);
