#include <sys/types.h>
#include "co.h"
#include "aw.h"
#include "awos.h"
#if defined __APPLE__
#define NP_NO_CARBON
#include <WebKit/npapi.h>
#include <WebKit/npfunctions.h>
#include <QuartzCore/QuartzCore.h>
#else
#include <npapi.h>
#include <npfunctions.h>
#endif

struct _aw {
        awHeader hdr;
	void * handle;
};

struct _ac {
        acHeader hdr;
	void * handle;
};

typedef struct _ins ins;

typedef struct _insHeader {
	aw w;
	ac c;
	co * comain;
	co * coaw;
} insHeader;

ins * awosNew(NPNetscapeFuncs *, NPP);
void awosDel(ins *);
void awosUpdate(ins *);
void awosSetWindow(ins *, void *);
int awosMakeCurrentI(ins *);
int awosClearCurrentI(ins *);
NPError awosEvent(ins *, void *);
NPError awosGetValue(NPP, NPPVariable, void *); 


void * awosSelf();
void * awosResolve(void * in, const char * name);

const char * awosResourcesPath(ins *);

#undef EXPORTED
#define EXPORTED