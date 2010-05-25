typedef struct _co co;
co * coMain(void *);
co * coNew(void (*)(void*), void *);
void coSwitchTo(co *);
co * coCurrent();
void * coData(co *);
