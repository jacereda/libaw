#-*-Python-*-
import sys
import os
backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'native':
	target = sys.platform
else:
	tools = 'crossmingw'
	toolpath = '.'
if target == 'linux2':
	target = 'linux'

backends = {
	'darwin' : ['x11', 'cocoa'],
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

	def _SetCPPFlags(self):
		self.Append(CPPPATH=['include'])
		self.Append(LIBPATH='.') 

	def ForProgram(self):
		ret = self.Clone()
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		ret.Append(LIBS=['aw'])
		return ret

	def Prg(self, name, sources):
		self.Default(self.Program(name, sources))

	def CompileAs32Bits(self):
		self.Append(CCFLAGS=' -m32 ')
		self.Append(LINKFLAGS=' -m32 ')
		
	def ForPlugin(self):
		ret = self.Clone()
		ret.UsesOpenGL()
		ret.CompileAs32Bits()
		ret._SetCPPFlags()
		ret.Append(CPPDEFINES=['AWPLUGIN'])
		ret.Append(LIBS=['awplugin'])
		ret.Append(FRAMEWORKS=['WebKit', 'QuartzCore'])
		ret['SHLINKFLAGS'] = '$LINKFLAGS -bundle -flat_namespace'
		ret['SHLIBPREFIX'] = ''
		ret['SHLIBSUFFIX'] = ''
		return ret

	def Plg(self, name, sources):
		res = self.Command(
			name + '.rsrc', name + '.r', 
			'/Developer/Tools/Rez -o $TARGET -useDF $SOURCE')
		home = os.environ['HOME'] + '/'
		target = home + 'Library/Internet Plug-Ins/awplugin.webplugin/'
		plg = self.SharedLibrary(name, sources)
		self.Default(self.Install(target + 'Contents/', 
					      'Info.plist'))
		self.Default(self.Install(target + 'Contents/MacOS/', plg))
		self.Default(self.Install(target + 'Contents/Resources/', 
					      res))

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
