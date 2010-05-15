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
awplugin = env.Clone()
awplugin.CompileAs32Bits()
awplugin.Append(CPPPATH=['include'])
awplugin.Append(CPPDEFINES=['AWPLUGIN', 'HAVE_CONFIG_H'])
awplugin.Append(CPPPATH=['pcl', 'pcl/include'])
if aw['BACKEND'] == 'cocoa':
    plg = awplugin.Lib('awplugin', [
        'awplugin.m', 
        'awmackeycodes.c',
        'aw.c', 
        'pcl/pcl/pcl.c',
        'pcl/pcl/pcl_private.c',
        'pcl/pcl/pcl_version.c',
        ])

awtest = env.ForProgram()
awtest.Prg('awtest', 'test/awtest.c')
awtest.Prg('hello', 'test/hello.c')
awtest.Prg('robot', 'test/robot.c')
awtest.Prg('picksquare', 'test/picksquare.c')
awtest.Prg('cube', 'test/cube.c')
awtest.Prg('multi', 'test/multi.c')
awtest.Prg('sharing', 'test/sharing.c')

if aw['BACKEND'] == 'cocoa':
    awplugintest = env.ForPlugin()
    awplugintest.Plg('awplugin', 'test/awtest.c')
