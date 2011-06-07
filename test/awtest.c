#include <aw/aw.h>
#include <aw/sysgl.h>
#include "log.h"

static int g_exit;
static const char * g_progname;
static ap * g_p = 0;

// To test the browser plugin
EXPORTED void hello() {
        Log("Hello World!");
}

static void resize(aw * w, int ww, int wh) {
        char buf[256] = {0};
        snprintf(buf, sizeof(buf), "%s %dx%d", g_progname, ww, wh);
        awSetTitle(w, buf);
}

static void deletePointer(aw * w) {
        if (g_p) {
                awPointer(w, 0);
                apDel(g_p);
        }
}

static void hideCursor(ag * g, aw * w) {
        unsigned i;
        unsigned char rgba[32*32*4];
        for (i = 0; i < 32*32; i++) {
                rgba[0] = rgba[1] = rgba[2] = 0;
                rgba[3] = 0xff;
        }
        deletePointer(w);
        g_p = apNew(g, rgba, 10, 5);
        awPointer(w, g_p);        
}

static void setCursor(ag * g, aw * w) {
        unsigned char rgba[32*32*4];
        int x, y;

        deletePointer(w);   

        for (y = 0; y < 32; y++) {
                for (x = 0; x < 32; x++) {
                        unsigned char * c = rgba + (y*32 + x) * 4;
            
                        c[0] = 0;
                        c[1] = 0;
                        c[2] = 0;
                        c[3] = 0xff;

                        if ((x < 8) || (y < 8))
                                c[0] = 0xff;
                        else if ((x < 16) || (y < 16))
                                c[1] = 0xff;
                        else if ((x < 24) || (y < 24))
                                c[2] = 0xff;
                        else
                                c[3] = 0;
                }
        }

        g_p = apNew(g, rgba, 10, 5);
        awPointer(w, g_p);
}

static void draw() {
        static int i = 0;
        glClearColor((i++ & 0xff) / 255.0f, 0.f, 1.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFlush();
}

static aw * processEvents(ag * g, aw * w, ac * c) {
        const ae * e;
        agTick(g);
        if ((e = awNextEvent(w))) 
                switch (aeType(e)) {
                case AW_EVENT_RESIZE:
                        resize(w, aeWidth(e), aeHeight(e));
                        Log("Resized to %d %d", aeWidth(e), aeHeight(e));
                        break;
                case AW_EVENT_POSITION:
                        Log("Moved to %d %d", aeX(e), aeY(e));
                        break;
                case AW_EVENT_UNICODE:
                        Log("Unicode: %s", aeKeyName(e));
                        break;
                case AW_EVENT_DOWN:
                        Log("Down: %s", aeKeyName(e));
                        break;
                case AW_EVENT_UP:
                        Log("Up: %s", aeKeyName(e));
                        switch (aeWhich(e)) {
                        case AW_KEY_8: awPointer(w, 0); break;
                        case AW_KEY_9: setCursor(g, w); break;
                        case AW_KEY_0: hideCursor(g, w); break;
                        case AW_KEY_S: awShowBorders(w); break;
                        case AW_KEY_H: awHideBorders(w); break;
                        case AW_KEY_M: awMaximize(w); break;
                        case AW_KEY_N: awNormalize(w); break;
                        case AW_KEY_V: {
                                static int v = 0;
                                v = 1 - v;
                                awGeometry(w, 100-v*20, 200-v*20, 
                                           300+v*40, 400+v*40);
                        } break;
                        case  AW_KEY_Q: 
                                g_exit = 1; break;
                        case AW_KEY_MOUSELEFT:
                                if (awMouseX(w) < 80 && awMouseY(w) < 80) {
                                        static int shown = 0;
                                        if (shown)
                                                awHideKeyboard(w);
                                        else
                                                awShowKeyboard(w);
                                        shown = !shown;
                                        setCursor(g, w);
                                }
                                break;
                        default: break;
                        }
                        break;
                case AW_EVENT_MOTION:
                        Log("Motion: %d,%d", aeX(e), aeY(e));
                        break;
                case AW_EVENT_EXPOSED:
                        Log("Exposed");
                        break;
                case AW_EVENT_KILLFOCUS:
                        Log("Kill focus");
                        break;
                case AW_EVENT_SETFOCUS:
                        Log("Set focus");
                        break;
                case AW_EVENT_DROP:
                        Log("Drop %s", aePath(e));
                        break;
                case AW_EVENT_CLOSE:
                        Log("Exit requested");
                        g_exit = 1; 
                        break;
                default: break;
                }
        if (awPressed(w, AW_KEY_A))
                Log("a pressed");
        if (awPressed(w, AW_KEY_MOUSELEFT))
                Log("mleft pressed");
        if (awPressed(w, AW_KEY_MOUSERIGHT))
                Log("mright pressed");

        return w;
}

int main(int argc, char ** argv) {
        ag * g = 0;
        aw * w = 0;
        ac * c = 0;

        g_progname = argv[0];
        g = agNew("awtest");
        if (g)
                w = awNew(g);
        if (g)
                c = acNew(g, 0);
        awSetInterval(w, 1);
        awMakeCurrent(w, c);
        awGeometry(w, 100, 200, 300, 400);
        awShow(w);
        g_exit = 0;
        while (!g_exit) {
//  Log("Z order: %d", awOrder(w));
                w = processEvents(g, w, c);
                if (!g_exit) {
                        draw();
                        awSwapBuffers(w);
                }
        }
        awMakeCurrent(w, 0);
        deletePointer(w);
        acDel(c);
        awDel(w);
        agDel(g);
        Log("leaving");
        return 0;
}

/*
  Local variables: **
  c-file-style: "bsd" **
  indent-tabs-mode: nil **
  End: **
*/
