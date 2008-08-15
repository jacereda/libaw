#-*-Python-*-
import sys
import os
backends = []
if sys.platform == 'darwin':
	backends += ['cocoa']
if os.name == 'posix':
	backends += ['x11']
if os.name == 'nt':
	backends += ['nt']

class Env(Environment):
	def UsesOpenGL(self):
		if self['BACKEND'] == 'cocoa':
			self.Append(FRAMEWORKS=['OpenGL', 'AppKit'])
		if self['BACKEND'] == 'x11':
			self.Append(LIBPATH=['/usr/X11R6/lib'])
			self.Append(CPPPATH=['/usr/X11R6/include'])
			self.Append(LIBS=['GL', 'GLU', 'X11'])
		if self['BACKEND'] == 'nt':
			print "XXX TODO"

confCCFLAGS = {
	'debug': '-g -Wall',
	'release': '-O2'
}

confCPPDEFINES = {
	'debug': [''],
	'release': ['NDEBUG'],
}	

for backend in backends:
	for conf in ['debug', 'release']:
		cnf = Env(CCFLAGS=confCCFLAGS[conf],
			  CPPDEFINES=confCPPDEFINES[conf])
		dir = conf + '/' + backend
		cnf.BuildDir(dir, '.', duplicate=0)
		env = cnf.Copy()
		env['BACKEND'] = backend
		env.Append(CPPDEFINES=[['AWBACKEND', backend]])
		Export("env")
		env.SConscript(dir  + '/SConscript')
