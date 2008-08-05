#-*-Python-*-
debug = Environment(CCFLAGS=' -g -Wall ')
release = Environment(CCFLAGS=' -O2 ')
for conf,dir in [[debug, 'debug'], [release, 'release']]:
	conf.BuildDir(dir, '.', duplicate=0)
	env = conf.Copy()	
	Export("env")
	env.SConscript(dir + '/SConscript')
