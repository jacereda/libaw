static void awentry(void * data) {
	extern int main(int argc, char ** argv);
	int argc = 1;
	char * argv0 = "awnpapi";
	debug("entry");
	main(argc, &argv0);
	getHeader()->awdone = 1;
	coSwitchTo(getHeader()->comain);
	assert(0);
}

int main(int argc, char ** argv) {
        extern int fakemain(int, char **);
        g_comain = coMain(0);
	g_coaw = coNew(awentry, 0);
        return fakemain(argc, argv);
}
