static void loop(aw * w) {
		while (processEvents(w)) {
				display();
				awSwapBuffers(w);
		}
}

static void go(aw * w) {
		awPushCurrent(w);
		init();
		loop(w);
		awPopCurrent(w);
		awClose(w);
		awEnd();
}

int main(int argc, char ** argv) {
		aw * w = 0;
		if (!awInit())
				Log("unable to initialize AW");
		else
				w = awOpen(100, 100, 500, 500);
		if (w) 
				go(w);
		else
				Log("unable to open window (is DISPLAY set?)");
		return w == 0;
}


/* 
   Local variables: **
   c-file-style: "bsd" **
   c-basic-offset: 8 **
   End: **
*/
