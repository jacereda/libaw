#-*-Python-*-
Import('env')
aw = env.ForLib()
aw.UsesOpenGL()
backend = {
'iphone' : 'awiphone.m co.c coroutine/source/Coro.c awreportfile.c',
'cocoa': 'awmain.c awcocoa.m awmackeycodes.c awreportconsole.c',
'x11': 'awmain.c awx.c awx11keycodes.c awreportconsole.c',
'nt': 'awmain.c aww.c awntkeycodes.c awntevent.c awreportconsole.c',
}[aw['BACKEND']]

if aw['BACKEND'] == 'iphone':
    aw.Append(CPPPATH=['coroutine/source',])
    aw.Append(CPPDEFINES=['USE_SETJMP',])

aw.Append(CPPDEFINES=['BUILDING_AW'])
aw.Lib('aw', 'aw.c ' + backend)

aw.Install('include/aw', ['aw.h', 'sysgl.h', 'sysglu.h'])
awnpapi = env.ForNPAPI()
if awnpapi:
    awnpapi.CompileAs32Bits()
    awnpapi.Append(CPPPATH=['include'])
    awnpapi.Append(CPPDEFINES=['AWPLUGIN'])
    awnpapi.Append(CPPPATH=['coroutine/source',])
    awnpapi.UsesOpenGL()
    if awnpapi['BACKEND'] == 'nt':
        awnpapi.Append(CPPDEFINES=['USE_FIBERS',])
    elif awnpapi['BACKEND'] == 'x11':
        awnpapi.Append(CPPDEFINES=['USE_UCONTEXT',])        
    else:
        awnpapi.Append(CPPDEFINES=['USE_SETJMP',])        
    backend = {
        'cocoa': '''
           awposixresolve.c awmackeycodes.c awcocoanpapi.m awreportfile.c
        ''',
        'nt': '''
           awntresolve.c awntkeycodes.c awntevent.c awntnpapi.c awreportfile.c
        ''',
        'x11': '''
           awposixresolve.c awx11keycodes.c awxembednpapi.c awreportconsole.c 
        ''',
        }[awnpapi['BACKEND']]
    plg = awnpapi.ShLinkLib('awnpapi', backend + '''
        aw.c awnpapi.c co.c coroutine/source/Coro.c
        ''')

awtest = env.ForGLPrg()
if awtest:
    awtest.Prg('awtest', 'test/awtest.c')
    awtest.Prg('hello', 'test/hello.c')
    awtest.Prg('robot', 'test/robot.c')
    awtest.Prg('picksquare', 'test/picksquare.c')
    awtest.Prg('cube', 'test/cube.c')
    awtest.Prg('multi', 'test/multi.c')
    awtest.Prg('sharing', 'test/sharing.c')

awestest = env.ForGLESPrg()
if awestest:
    awestest.Prg('awtest', 'test/awtest.c')
    awestest.Prg('gles', 'test/gles.c')

awplugin = env.ForPlugin()
if awplugin:
    awplugin.Plg('awplugin', 'test/awtest.c')


