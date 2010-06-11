#include <assert.h>
#include <string.h>
#include "awnpapios.h"

typedef void (*awmethod)(void);

static NPNetscapeFuncs * s_browser = 0;
static NPObject * s_so = 0;
static char s_plgname[256];

static awmethod resolve(NPIdentifier method) {
	static void * me = 0;
	char * mname = s_browser->utf8fromidentifier(method);
	awmethod ret;
	if (!me)
		me = awosSelf(s_plgname);
	ret = awosResolve(me, mname);
	debug("resolve %s %p", mname, ret);
	// leak mname?
	return ret;
}

static bool hasmethod(NPObject* obj, NPIdentifier method) {
	return resolve(method) != 0;
}

static bool invoke(NPObject* obj, NPIdentifier method, 
		   const NPVariant *args, uint32_t nargs, NPVariant *res) {
	awmethod func = resolve(method);
	if (func)
		func();
	else {
		debug("no such method");
		s_browser->setexception(obj, "no such method");
	}
	return func != 0;
}

static bool invokedefault(NPObject *obj, 
			  const NPVariant *args, uint32_t nargs, 
			  NPVariant *res) {
	debug("invokedefault");
	return 0;
}

static bool hasproperty(NPObject *obj, NPIdentifier prop) {
	debug("hasproperty");
	return 0;
}

static bool getproperty(NPObject *obj, NPIdentifier prop, NPVariant *res) {
	debug("getproperty");
	return 0;
}

static NPClass scriptable = {
	NP_CLASS_STRUCT_VERSION,
	0,
	0,
	0,
	hasmethod,
	invoke,
	invokedefault,
	hasproperty,
	getproperty,
	0,
	0,
};

static void ev(ins * o, int ev, int x, int y) {
	insHeader * h = (insHeader*)o;
	got(&h->w, ev, x, y);
	debug("event %d %d %d", ev, x, y);
}

insHeader * getHeader() {
	return (insHeader*)coData(coCurrent());
}

void awentry(void * data) {
	extern int main(int argc, char ** argv);
	int argc = 1;
	char * argv0 = "awnpapi";
	debug("entry");
	main(argc, &argv0);
	getHeader()->awdone = 1;
	coSwitchTo(getHeader()->comain);
	assert(0);
}

static NPError nnew(NPMIMEType type, NPP i,
		    uint16_t mode, int16_t argc, char* argn[],
		    char* argv[], NPSavedData* saved) {
	ins * o;
	insHeader * h;
	unsigned j;
        snprintf(s_plgname, sizeof s_plgname - 1, "%s", argv[0]);
	debug("new");
	for (j = 0; j < argc; j++) {
		debug("  %s", argv[j]);
		debug("  %s", argn[j]);
	}
	o = awosNew(s_browser, i);
	h = (insHeader*)o;
	memset(&h->w, 0, sizeof(h->w));
	memset(&h->c, 0, sizeof(h->c));
	i->pdata = o;
	h->comain = coMain(o);
	debug("comain: %p", h->comain);
	h->coaw = coNew(awentry, o);
	h->awdone = 0;
	return NPERR_NO_ERROR;
}

static NPError setwindow(NPP i, NPWindow* w) {
	ins * o = (ins*)i->pdata;
	debug("setwindow %p", w->window);
	debug("%dx%d", w->width, w->height);
	ev(o, AW_EVENT_RESIZE, w->width, w->height);
	awosSetWindow(o, w);
	return NPERR_NO_ERROR;
}

static NPError destroy(NPP i, NPSavedData **save) {
	ins * o = (ins*)i->pdata;
	debug("destroy");
	ev(o, AW_EVENT_CLOSE, 0, 0);
	debug("destroy>aw");
	while (!getHeader()->awdone) 
		coSwitchTo(getHeader()->coaw);
	debug("aw>destroy");
	coDel(getHeader()->coaw);
	coDel(getHeader()->comain);
	awosDel(o);
        i->pdata = 0;
	if(s_so)
		s_browser->releaseobject(s_so);
	return NPERR_NO_ERROR;
}

static NPError getvalue(NPP i, NPPVariable variable, void *v) {
	ins * o;
	NPError ret = NPERR_NO_ERROR;
	debug("getvalue");
	o = i? (ins*)i->pdata : 0;
	switch(variable) {
	default: 
		debug("getval default"); 
		ret = awosGetValue(i, variable, v);
		break;
	case NPPVpluginNameString: 
		debug("getval pluginname"); 
		*(char **)v = "plugin name";
		break;
	case NPPVpluginDescriptionString:
		debug("getval plugindesc"); 
		*(char **)v = "plugin description";
		break;
	case NPPVpluginScriptableNPObject:
		debug("getval scriptable");
		if(!s_so)
			s_so = s_browser->createobject(i, &scriptable);
		s_browser->retainobject(s_so);
		*(void**)v = s_so;
		break;
	case NPPVpluginWindowBool:
		*(int*)v = 1;
		break;
	}
	return ret;
}

static NPError hevent(NPP i, void * ve) {
	ins * o = (ins*)i->pdata;
	debug("event");
	return awosEvent(o, ve);
}

EXPORTED NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* f) {
	debug("getentrypoints %p", f);
	f->size = sizeof (NPPluginFuncs);
	f->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	f->newp = nnew;
	f->setwindow = setwindow;
	f->destroy = destroy;
	f->getvalue = getvalue;
	f->event = hevent;
	debug("/getentrypoints %p", f);
	return NPERR_NO_ERROR;
}

EXPORTED NPError OSCALL NP_Initialize(NPNetscapeFuncs* f
#if defined XP_UNIX && !defined XP_MACOSX
                                      ,NPPluginFuncs * funcs
#endif
        ) {
	debug("initialize");
	s_browser = f;
	if(!f) {
                debug("invalid functable");
		return NPERR_INVALID_FUNCTABLE_ERROR;
        }
	if(((f->version & 0xff00) >> 8) > NP_VERSION_MAJOR) {
                debug("incompatible");
                return NPERR_INCOMPATIBLE_VERSION_ERROR;
        }
#if defined XP_UNIX && !defined XP_MACOSX
        NP_GetEntryPoints(funcs);
#endif
	return NPERR_NO_ERROR;
}

EXPORTED NPError OSCALL NP_Shutdown() {
	debug("shutdown");
	return NPERR_NO_ERROR;
}

EXPORTED char * NP_GetMIMEDescription(void) {
        static char buf[256];
        char tmp[256];
        const char * modpath = awosModPath();
        const char * last = strrchr(modpath, '/');
        if (!last)
                last = strrchr(modpath, '\\');
        snprintf(tmp, sizeof tmp - 1, "%s", last+1);
        *strrchr(tmp, '.') = 0;
        snprintf(buf, sizeof buf - 1, 
                 "application/%s::xx@foo.bar", 
                 0 == strncmp(buf, "lib", 3)? tmp+3 : 
                 0 == strncmp(buf, "np", 2)? tmp+2 : 
                 tmp);
        report("getmimedesc %s", buf);
        return buf;
}

EXPORTED NPError OSCALL NP_GetValue(
#if defined(XP_UNIX)
        void *
#else
        NPP
#endif
        npp, 
                                    NPPVariable variable, void *val) {
	debug("npgetvalue");
	return getvalue(npp, variable, val);
}

int awosInit() { 
	debug("awosInit");
	return 1; 
}

int awosEnd() { 
	debug("awosEnd");
	return 1; 
}

aw * awosOpen(int x, int y, int width, int height, int fs, int bl) {
	insHeader * h = getHeader();
	debug("awosOpen");
	return &h->w;
}

int awosSetTitle(aw * w, const char * t) {
	debug("awosSetTitle");
	return 1;
}

int awosClose(aw * w) {
	debug("awosClose");
	return 1;
}

int awosMakeCurrent(aw * w, ac * c) {
	debug("awosMakeCurrent");
	return awosMakeCurrentI((ins*)w);
}

int awosClearCurrent(aw * w) {
	debug("awosClearCurrent");
	return awosClearCurrentI((ins*)w);
}

int awosSwapBuffers(aw * w) {
	insHeader * hdr = getHeader();
	debug("aw>main %p", hdr->comain);
	coSwitchTo(hdr->comain);
	debug("main>aw");
	awosUpdate((ins*)hdr);
	return 1;
}

int awosShow(aw * w) {
	debug("awosShow");
	return 1;
}

int awosHide(aw * w) {
	debug("awosHide");
	return 1;
}

void awosPollEvent(aw * w) {
	debug("awosPollEvent");
}

int awosSetSwapInterval(aw * w, int interval) {
	debug("awosSetSwapInterval");
	return 1;
}

int acosInit() {
	debug("acosInit");
	return 1;
}

int acosEnd() {
	debug("acosEnd");
	return 1;
}

ac * acosNew(ac * c) {
	insHeader * h = getHeader();
	debug("acosNew");
	return &h->c;
}

int acosDel(ac * c) {
	debug("acosDel");
	return 1;
}

const char * awResourcesPath() {
	insHeader * hdr = getHeader();
	return awosResourcesPath((ins*)hdr);
}

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/
