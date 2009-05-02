#-*-Python-*-
Import('env')
aw = env.Clone()
aw.UsesOpenGL()
backend = {
'cocoa': 'awcocoa.m',
'x11': 'awx.c',
'nt': 'aww.c',
}[aw['BACKEND']]
aw.Append(CPPDEFINES=['BUILDING_AW'])
aw.SharedLibrary('aw', ['aw.c', backend])
aw.Install('include/aw', ['aw.h', 'sysgl.h', 'sysglu.h'])
awtest = env.Clone()
awtest.UsesOpenGL()
awtest.Append(LIBS=['aw'])
awtest.Append(CPPPATH=['include'])
awtest.Append(LIBPATH='.') 
awtest.Program('awtest', 'test/awtest.c')
awtest.Program('hello', 'test/hello.c')
awtest.Program('robot', 'test/robot.c')
awtest.Program('picksquare', 'test/picksquare.c')
awtest.Program('cube', 'test/cube.c')
awtest.Program('multi', 'test/multi.c')
