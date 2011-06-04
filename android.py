import os
import os.path
import string

import SCons.Action
import SCons.Builder
import SCons.Tool
import SCons.Util
from SCons.Script import ARGUMENTS

def generate(env):
    ndk = ARGUMENTS.get('androidndk', 
                        os.environ['HOME'] + '/android/android-ndk-r5b')
    env['ANDROIDNDK'] = ndk
    sdk = ARGUMENTS.get('androidsdk', 
                        os.environ['HOME'] + '/android/android-sdk-mac_x86')
    ndkprefix = ndk + '/toolchains/arm-linux-androideabi-4.4.3/prebuilt/' +\
      'darwin-x86/bin/arm-linux-androideabi-'
    gnu_tools = ['gcc', 'g++', 'applelink', 'ar', 'gas']
    for tool in gnu_tools:
        SCons.Tool.Tool(tool)(env)
    sysroot = ' --sysroot=%s/platforms/android-9/arch-arm ' % ndk
    common = ' -ffunction-sections -fdata-sections -fvisibility=hidden' +\
      ' -Bdynamic -shared ' + sysroot
    env['ADB'] = sdk + '/platform-tools/adb'
    env['ANDROID'] = sdk + '/tools/android'
    env['SHLIBPREFIX'] = 'lib'
    env['SHLIBSUFFIX'] = '.so'
    env['AR'] = ndkprefix + 'ar'
    env['AS'] = ndkprefix + 'as'
    env['CC'] = ndkprefix + 'gcc'
    env['CXX'] = ndkprefix + 'g++'
    env['LINK'] = ndkprefix + 'gcc'
    env['RANLIB'] = ndkprefix + 'ranlib'
    env.Replace(CPPPATH=[
            ndk + '/platforms/android-9/arch-arm/usr/include/',
            ndk + '/sources/android/native_app_glue/',
            ])
    env.Replace(LINKPATH=[
            ndk + '/platforms/android-9/arch-arm/usr/lib/',
            ])
    env.Replace(CPPDEFINES=[
            '__ARM_ARCH_5__',
            '__ARM_ARCH_5T__',
            '__ARM_ARCH_5E__',
            '__ARM_ARCH_5TE__',
            'ANDROID',
            'SK_RELEASE',
            ])

    env.Replace(CCFLAGS='-fpic -ffunction-sections -funwind-tables -fstack-protector -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fno-strict-aliasing -finline-limit=64   -Wa,--noexecstack ')
    env.Append(LIBS=['log', 'android', 'EGL', 'GLESv1_CM', 'GLESv2', 'c', 'm', 'supc++'])
    env.Replace(LINKFLAGS=common + ' -Wl,-soname,${TARGET.file} -Wl,--no-undefined -Wl,-z,noexecstack ')
    env.Replace(SHLINKFLAGS=env['LINKFLAGS'])
    
def exists(env):
    return find(env)
