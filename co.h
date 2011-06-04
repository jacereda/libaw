typedef struct _co co;
co * coMain(void *);
co * coNew(void (*)(void*), void *, size_t);
void coDel(co *);
void coSwitchTo(co *);
co * coCurrent();
void * coData(co *);
