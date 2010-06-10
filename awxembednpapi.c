#include "awnpapios.h"
#include <stdlib.h>
#include <X11/X.h>
#include <GL/glx.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

struct _ins {
	insHeader h;
        Window w;
        Display * dpy;
        GLXContext ctx;
        GtkWidget * plug;
};

ins * awosNew(NPNetscapeFuncs * browser, NPP i) {
	return calloc(1, sizeof(ins));
}

static XVisualInfo * chooseVisual(Display * dpy, int screen) {
        int att[64];
        int * p = att;
        *p++ = GLX_RGBA;
        *p++ = GLX_DOUBLEBUFFER;
        *p++ = GLX_RED_SIZE; *p++ = 1;
        *p++ = GLX_GREEN_SIZE; *p++ = 1;
        *p++ = GLX_BLUE_SIZE; *p++ = 1;
        *p++ = GLX_DEPTH_SIZE; *p++ = 1;
        *p++ = None;
        return glXChooseVisual(dpy, screen, att);
}

static unsigned mapbutton(unsigned b) {
        unsigned ret;
        switch (b) {
        default: ret = AW_KEY_NONE; break;
        case 1: ret = AW_KEY_MOUSELEFT; break;
        case 2: ret = AW_KEY_MOUSEMIDDLE; break;
        case 3: ret = AW_KEY_MOUSERIGHT; break;
        }
        return ret;
}

static gboolean event(GtkWidget * wid, GdkEvent * ev, gpointer data) {
        extern unsigned mapkeycode(unsigned);
        ins * o = (ins*)data;
        aw * w = &o->h.w;
        int ret = TRUE;
        switch (ev->type) {
        case GDK_KEY_PRESS: {
                GdkEventKey * kev = (GdkEventKey*)ev;
                got(w, AW_EVENT_DOWN, mapkeycode(kev->hardware_keycode), 0);
                break;
        }
        case GDK_KEY_RELEASE: {
                GdkEventKey * kev = (GdkEventKey*)ev;
                got(w, AW_EVENT_UP, mapkeycode(kev->hardware_keycode), 0);
                break;
        }
        case GDK_BUTTON_PRESS: {
                GdkEventButton * bev = (GdkEventButton*)ev;
                gtk_widget_grab_focus(o->plug);
                got(w, AW_EVENT_DOWN, mapbutton(bev->button), 0);
                break;
        }
        case GDK_BUTTON_RELEASE: {
                GdkEventButton * bev = (GdkEventButton*)ev;
                got(w, AW_EVENT_UP, mapbutton(bev->button), 0);
                break;
        }
        case GDK_MOTION_NOTIFY: {
                GdkEventMotion * mev = (GdkEventMotion*)ev;
                got(w, AW_EVENT_MOTION, mev->x, mev->y);
                break;
        }
        default: ret = FALSE; break;
        }
        return ret;
}


static gboolean update(gpointer data) {
        ins * o = (ins*)data;
	coSwitchTo(o->h.coaw);
        glXSwapBuffers(o->dpy, o->w);
        return TRUE;
}

void awosSetWindow(ins * o, NPWindow * win) {
        if (!o->plug) {
                NPSetWindowCallbackStruct * info = 
                        (NPSetWindowCallbackStruct*)win->ws_info;
                XVisualInfo * vinfo;
                GtkWidget * plug;
                plug = gtk_plug_new((GdkNativeWindow)(win->window));
                GTK_WIDGET_SET_FLAGS(GTK_WIDGET(plug), GTK_CAN_FOCUS);
                gtk_widget_add_events(plug, 0
                                      | GDK_BUTTON_PRESS_MASK 
                                      | GDK_BUTTON_RELEASE_MASK
                                      | GDK_KEY_PRESS_MASK
                                      | GDK_KEY_RELEASE_MASK
                                      | GDK_POINTER_MOTION_MASK
                        );
                g_signal_connect(G_OBJECT(plug), "event", 
                                 G_CALLBACK(event), o);
                gtk_widget_show_all(plug);
                gtk_widget_grab_focus(plug);
                o->dpy = info->display;
                o->plug = plug;
                o->w = gtk_plug_get_id(GTK_PLUG(o->plug));
                g_timeout_add(10, update, o);
                vinfo = chooseVisual(o->dpy, 0);
                o->ctx = glXCreateContext(o->dpy, vinfo, 0, True);
                XFree(vinfo);
                glXMakeCurrent(o->dpy, o->w, o->ctx);
        }
}

void awosDel(ins * o) {
	free(o);
}

int awosMakeCurrentI(ins * o) {
        return glXMakeCurrent(o->dpy, o->w, o->ctx);
}

int awosClearCurrentI(ins * o) {
        return glXMakeCurrent(o->dpy, 0, 0);
}

void awosUpdate(ins * o) {
        update(o);
}

NPError awosEvent(ins * o, void * ev) {
//	NPEvent * e = (NPEvent *)ev;
	report("osevent");
	return NPERR_NO_ERROR;
}

const char * awosResourcesPath(ins * o) {
	return ".";
}

NPError awosGetValue(NPP i, NPPVariable var, void * v) {
//	ins * o = (ins*)i->pdata;
	NPError ret = NPERR_NO_ERROR;
	report("os getvalue"); 
	switch(var) {
        case NPPVpluginNeedsXEmbed:
                *(int*)v = 1;
                break;
	default: 
		report("os getval default"); 
		ret = NPERR_GENERIC_ERROR; 
		break;
	}
	return ret;
} 

/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   indent-tabs-mode: nil **
   End: **
*/

