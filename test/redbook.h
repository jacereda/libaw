static void updateTitle(aw * w, int x, int y) {
	char buf[16] = {0};
	snprintf(buf, sizeof(buf), "%dx%d", x, y);
	awSetTitle(w, buf);
}


#if !defined NO_HANDLER
static int processEvents(ag * g, aw ** w, ac * c) {
	int keepgoing = 1;
	const ae * e;
	while ((e = awNextEvent(*w))) switch (aeType(e)) {
		case AW_EVENT_DOWN:
			if (aeWhich(e) == 'f') {
				awMakeCurrent(*w, 0);
				awClose(*w);
				*w = awOpenFullscreen(g);
			}
			if (aeWhich(e) == 'w') {
				awMakeCurrent(*w, 0);
				awClose(*w);
				*w = awOpen(g);
				awGeometry(*w, 100, 100, 300, 400);
			}
			if (aeWhich(e) == 'q')
				keepgoing = 0;
			break;
		case AW_EVENT_RESIZE:
			reshape(aeWidth(e), aeHeight(e));
			updateTitle(*w, aeWidth(e), aeHeight(e));
			break;
		case AW_EVENT_CLOSE:
			keepgoing = 0; 
			break;
		default: break;
		}
	return keepgoing;
}
#endif


static void loop(ag * g, aw ** w, ac * c) {
	while (processEvents(g, w, c)) {
		awMakeCurrent(*w, c);
		display();
		awSwapBuffers(*w);
	}
}

static void go(ag * g, aw * w, ac * c) {
	awMakeCurrent(w, c);
	init();
	loop(g, &w, c);
	awMakeCurrent(w, 0);
	awClose(w);
	acDel(c);
	agDel(g);
}

int main(int argc, char ** argv) {
	ag * g = 0;
	aw * w = 0;
	ac * c = 0;
	g = agNew("redbook");
	if (g)
		w = awOpen(g);
	else
		Log("unable to initialize AW");
	if (!w)
		Log("unable to open window (is DISPLAY set?)");
	if (w)
		c = acNew(g, 0);
	if (w)
		awGeometry(w, 100, 100, 500, 500);
	if (w)
		awShow(w);
	if (w)
		awSetInterval(w, 1);
	if (c) 
		go(g, w, c);
	return w == 0;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
