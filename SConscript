#-*-Python-*-
Import('env')
aw = env.ForLib()
aw.UsesOpenGL()
backend = {
'android' : 'awandroid.c ',
'iphone' : 'awiphone.m awreportfile.c tlspthread.c',
'cocoa': 'awcocoa.m awmackeycodes.c awreportconsole.c',
'x11': 'awx.c awx11keycodes.c awreportconsole.c',
'nt': 'aww.c awntkeycodes.c awntevent.c awreportconsole.c',
}[aw['BACKEND']]

if aw['BACKEND'] == 'android':
    backend += aw['ANDROIDNDK'] + '/sources/android/native_app_glue/android_native_app_glue.c'

aw.Append(CPPPATH=['coroutine/source',])
if aw['BACKEND'] == 'nt':
    aw.Append(CPPDEFINES=['USE_FIBERS'])
else:
    aw.Append(CPPDEFINES=['USE_GUESSED_SETJMP'])
aw.Append(CPPDEFINES=['BUILDING_AW'])
aw.Lib('aw', 'aw.c co.c coroutine/source/Coro.c ' + backend)

aw.Install('include/aw', ['aw.h', 'awtypes.h', 'sysgl.h', 'sysglu.h'])
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
           awposixresolve.c awmackeycodes.c awcocoanpapi.m awreportfile.c tlspthread.c
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
    awplugin.Plg('cubeplugin', 'test/cube.c')


