#-*-Python-*-
Import('env')
import os
aw = env.Clone()
aw.UsesOpenGL()
backend = {
'cocoa': ['awcocoa.m', 'awmackeycodes.c'],
'x11': ['awx.c'],
'nt': ['aww.c'],
}[aw['BACKEND']]
aw.Append(CPPDEFINES=['BUILDING_AW'])
aw.Lib('aw', ['aw.c'] + backend)

aw.Install('include/aw', ['aw.h', 'sysgl.h', 'sysglu.h'])
if env['BACKEND'] in ['cocoa']:
    awnpapi = env.Clone()
    awnpapi.CompileAs32Bits()
    awnpapi.Append(CPPPATH=['include'])
    awnpapi.Append(CPPDEFINES=['AWPLUGIN'])
    if awnpapi['BACKEND'] == 'nt':
        awnpapi.Append(CPPDEFINES=['HAVE_WINCONFIG_H'])
    else:
        awnpapi.Append(CPPDEFINES=['HAVE_CONFIG_H'])
    awnpapi.Append(CPPPATH=['pcl', 'pcl/include'])
    backend = {
        'cocoa': 'awplugin.m awmackeycodes.c',
        'x11': 'awplugin.c',
        'nt': 'awplugin.c',
        }[awnpapi['BACKEND']]
    plg = awnpapi.ShLib('awnpapi', backend + '''
        aw.c pcl/pcl/pcl.c pcl/pcl/pcl_private.c pcl/pcl/pcl_version.c
        ''')

awtest = env.ForProgram()
awtest.Prg('awtest', 'test/awtest.c')
awtest.Prg('hello', 'test/hello.c')
awtest.Prg('robot', 'test/robot.c')
awtest.Prg('picksquare', 'test/picksquare.c')
awtest.Prg('cube', 'test/cube.c')
awtest.Prg('multi', 'test/multi.c')
awtest.Prg('sharing', 'test/sharing.c')

if env['BACKEND'] in ['cocoa']:
    awplugin = env.ForPlugin()
    awplugin.Plg('awplugin', 'test/awtest.c')
