co * coMain(void *);
co * coNew(void (*)(void*), void *, size_t);
void coDel(co *);
void coSwitchTo(co *);
co * coCurrent();
void coSetData(co *, void *);
void * coData(co *);
