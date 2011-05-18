import os
import os.path
import string

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util

def generate(env):
    sdkprefix = \
        '/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/'
    gnu_tools = ['gcc', 'g++', 'applelink', 'ar', 'gas']
    for tool in gnu_tools:
        SCons.Tool.Tool(tool)(env)

    env['CC'] = sdkprefix + 'gcc'
    env['CXX'] = sdkprefix + 'g++'
    env['LINK'] = sdkprefix + 'gcc'
    env.Append(CPPDEFINES=[['__IPHONE_OS_VERSION_MIN_REQUIRED', 30000]])
    env.Append(CPPDEFINES=[['TARGET_OS_IPHONE', 1]])
    env.Append(LINKFLAGS=' -arch armv6 ')
    env.Append(CFLAGS=' -arch armv6 ')
    env.Append(CFLAGS=' -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.1.sdk ')
    env.Append(LINKFLAGS=' -isysroot /Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS3.1.sdk ')
#    env.Append(CFLAGS=' -mmacosx-version-min=10.6 ')
#    env.Append(LINKFLAGS=' -mmacosx-version-min=10.6 ')
    env['AS'] = sdkprefix + 'as'
    
def exists(env):
    return find(env)
