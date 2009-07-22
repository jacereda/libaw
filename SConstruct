#-*-Python-*-
import sys
import os
backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'linux2':
	target = 'linux'
else:
	tools = 'crossmingw'
	toolpath = '.'
if target == 'native':
	target = sys.platform

backends = {
	'darwin' : ['cocoa', 'x11'],
	'linux' : ['x11'],
	'win32' : ['nt'],
	}[target]

class Env(Environment):
	def UsesOpenGL(self):
		if self['BACKEND'] == 'cocoa':
			self.Append(FRAMEWORKS=['OpenGL', 'AppKit'])
		if self['BACKEND'] == 'x11':
			self.Append(LIBPATH=['/usr/X11R6/lib'])
			self.Append(CPPPATH=['/usr/X11R6/include'])
			self.Append(LIBS=['GL', 'GLU', 'X11'])
		if self['BACKEND'] == 'nt':
			self.Append(LIBS=['opengl32', 'gdi32', 'glu32'])

confCCFLAGS = {
	'debug': '-g -Wall -fvisibility=hidden',
	'release': '-O2'
}

confCPPDEFINES = {
	'debug': [],
	'release': ['NDEBUG'],
}	

for backend in backends:
	for conf in ['debug']:
		cnf = Env(CCFLAGS=confCCFLAGS[conf],
			  CPPDEFINES=confCPPDEFINES[conf])
		if tools:
			cnf.Tool(tools, toolpath)
		if ARGUMENTS.get('useclang', 0):
			cnf.Replace(CC='~/llvm/bld/Release/bin/clang')
			cnf.Append(LINKFLAGS=' -L/usr/lib/gcc/i686-apple-darwin8/4.0.1/ ')
			cnf.Append(CCFLAGS=' -Os ')
			
		dir = conf + '/' + backend
		cnf.BuildDir(dir, '.', duplicate=0)
		env = cnf.Clone()
		env['BACKEND'] = backend
		env.Append(CPPDEFINES=[['AWBACKEND', backend]])
		Export("env")
		env.SConscript(dir  + '/SConscript')
