static void loop(aw * w) {
	while (processEvents(w)) {
		display();
		awSwapBuffers(w);
	}
}

static void go(aw * w, ac * c) {
	awMakeCurrent(w, c);
	init();
	loop(w);
	awMakeCurrent(w, 0);
	acDel(c);
	awClose(w);
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
