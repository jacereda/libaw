import os
import os.path
import string

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util

def generate(env):
    sdkprefix = \
        '/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/'
    gnu_tools = ['gcc', 'g++', 'applelink', 'ar', 'gas']
    for tool in gnu_tools:
        SCons.Tool.Tool(tool)(env)

    env['CC'] = sdkprefix + 'gcc'
    env['CXX'] = sdkprefix + 'g++'
    env['LINK'] = sdkprefix + 'gcc'
    env.Append(CPPDEFINES=[['__IPHONE_OS_VERSION_MIN_REQUIRED', 30000]])
    env.Append(LINKFLAGS=' -arch i386 -m32 ')
    env.Append(CFLAGS=' -arch i386 -m32 ')
    env.Append(CFLAGS=' -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.1.sdk ')
    env.Append(LINKFLAGS=' -isysroot /Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator3.1.sdk ')
    env.Append(CFLAGS=' -mmacosx-version-min=10.5 ')
    env.Append(LINKFLAGS=' -mmacosx-version-min=10.5 ')
    env['AS'] = sdkprefix + 'as'
    
def exists(env):
    return find(env)
