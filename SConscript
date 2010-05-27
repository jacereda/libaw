#-*-Python-*-
Import('env')
aw = env.ForShLib()
aw.UsesOpenGL()
backend = {
'cocoa': 'awcocoa.m awmackeycodes.c',
'x11': 'awx.c',
'nt': 'aww.c awntkeycodes.c awntevent.c',
}[aw['BACKEND']]
aw.Append(CPPDEFINES=['BUILDING_AW'])
aw.ShLib('aw', 'aw.c ' + backend)

aw.Install('include/aw', ['aw.h', 'sysgl.h', 'sysglu.h'])
if env['BACKEND'] in ['cocoa', 'nt']:
    awnpapi = env.ForNPAPI()
    awnpapi.CompileAs32Bits()
    awnpapi.Append(CPPPATH=['include'])
    awnpapi.Append(CPPDEFINES=['AWPLUGIN'])
    awnpapi.Append(CPPPATH=['coroutine/source',])
    if awnpapi['BACKEND'] == 'nt':
        awnpapi.Append(CPPDEFINES=['USE_FIBERS',])
    else:
        awnpapi.Append(CPPDEFINES=['USE_SETJMP',])        
    backend = {
        'cocoa': 'awposixresolve.c awmackeycodes.c awcocoanpapi.m',
        'x11': 'awposixresolve.c',
        'nt': 'awntresolve.c awntkeycodes.c awntevent.c awntnpapi.c',
        }[awnpapi['BACKEND']]
    plg = awnpapi.ShLinkLib('awnpapi', backend + '''
        aw.c awnpapi.c co.c coroutine/source/Coro.c
        ''')

awtest = env.ForProgram()
awtest.Prg('awtest', 'test/awtest.c')
awtest.Prg('hello', 'test/hello.c')
awtest.Prg('robot', 'test/robot.c')
awtest.Prg('picksquare', 'test/picksquare.c')
awtest.Prg('cube', 'test/cube.c')
awtest.Prg('multi', 'test/multi.c')
awtest.Prg('sharing', 'test/sharing.c')

if env['BACKEND'] in ['cocoa', 'nt']:
    awplugin = env.ForPlugin()
    awplugin.Plg('awplugin', 'test/awtest.c')
#    awplugin.Prg('cotest', 'cotest.c')

