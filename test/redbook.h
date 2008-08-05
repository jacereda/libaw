
static void loop(aw * w) {
	while (processEvents(w)) {
		display();
		awSwapBuffers(w);
	}
}

static void go(aw * w, const char * title) {
	awSetTitle(w, title);
	awResize(w, 500, 500);
	awShow(w);
	awPushCurrent(w);
	init();
	loop(w);
	awPopCurrent(w);
	awClose(w);
	awEnd();
}

int main(int argc, char ** argv) {
	aw * w;
	awInit();
	w = awOpen();
	if (w) 
		go(w, argv[0]);
	else
		Log("unable to open window (is DISPLAY set?)");
	return w == 0;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   End: **
*/
