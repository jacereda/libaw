#-*-Python-*-
debug = Environment(CCFLAGS=' -g ')
for conf,dir in [[debug, 'debug']]:
	conf.BuildDir(dir, '.', duplicate=0)
	env = conf.Copy()	
	Export("env")
	env.SConscript(dir + '/SConscript')
