static void updateTitle(aw * w, int x, int y) {
	char buf[16] = {0};
	snprintf(buf, sizeof(buf), "%dx%d", x, y);
	awSetTitle(w, buf);
}


#if !defined NO_HANDLER
static int processEvents(aw ** w, ac * c) {
	int keepgoing = 1;
	const awEvent * awe;
	while ((awe = awNextEvent(*w))) switch (awe->type) {
	case AW_EVENT_DOWN:
		if (awe->u.down.which == 'f') {
			awMakeCurrent(*w, 0);
			awClose(*w);
			*w = awOpenFullscreen();
		}
		if (awe->u.down.which == 'w') {
			awMakeCurrent(*w, 0);
			awClose(*w);
			*w = awOpen(100, 100, 300, 400);
		}
		if (awe->u.down.which == 'q')
			keepgoing = 0;
		break;
	case AW_EVENT_RESIZE:
		reshape(awe->u.resize.w, awe->u.resize.h);
		updateTitle(*w, awe->u.resize.w, awe->u.resize.h);
		break;
	case AW_EVENT_CLOSE:
		keepgoing = 0; 
		break;
	default: break;
	}
	return keepgoing;
}
#endif


static void loop(aw ** w, ac * c) {
	while (processEvents(w, c)) {
		awMakeCurrent(*w, c);
		display();
		awSwapBuffers(*w);
	}
}

static void go(aw * w, ac * c) {
	awMakeCurrent(w, c);
	init();
	loop(&w, c);
	awMakeCurrent(w, 0);
	awClose(w);
	acDel(c);
	awEnd();
}

int main(int argc, char ** argv) {
	aw * w = 0;
	ac * c = 0;
	if (awInit())
		w = awOpen(100, 100, 500, 500);
	else
		Log("unable to initialize AW");
	if (!w)
		Log("unable to open window (is DISPLAY set?)");
	if (w)
		c = acNew(0);
	if (c)
		acSetInterval(c, 1);
	if (c) 
		go(w, c);
	return w == 0;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
