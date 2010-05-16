#-*-Python-*-
import sys
import os
backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'native':
	target = sys.platform
	if target == 'win32':
		tools = 'mingw'
		toolpath = None
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

	def Objects(self, bdir, sources):
		return [self.Object('obj' + bdir + '/' + s + '.o', s) 
			for s in Split(sources)]
		
	def SharedObjects(self, bdir, sources):
		return [self.SharedObject('sobj' + bdir + '/' + s + '.o', s) 
			for s in Split(sources)]

	def Lib(self, name, sources):
		return self.Default(self.Library(name, 
						 self.Objects(name, sources)))

	def _SetCPPFlags(self):
		self.Append(CPPPATH=['include'])
		self.Append(LIBPATH='.') 

	def ForProgram(self):
		ret = self.Clone()
		ret.Append(LIBS=['aw'])
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		return ret

	def Prg(self, name, sources):
		self.Default(self.Program(name, 
					  self.SharedObjects(name, sources)))

	def CompileAs32Bits(self):
		self.Append(CCFLAGS=' -m32 ')
		self.Append(LINKFLAGS=' -m32 ')
		
	def ForPlugin(self):
		ret = self.Clone()
		ret.UsesOpenGL()
		ret.CompileAs32Bits()
		ret._SetCPPFlags()
		ret.Append(CPPDEFINES=['AWPLUGIN'])
		ret.Append(LIBS=['awnpapi'])
		ret.Append(FRAMEWORKS=['WebKit', 'QuartzCore'])
		if target == 'darwin':
			ret['SHLINKFLAGS'] = '$LINKFLAGS' +\
			    ' -bundle -flat_namespace'
			ret['SHLIBPREFIX'] = ''
			ret['SHLIBSUFFIX'] = ''
		return ret


	def ShLib(self, name, sources):
		return self.Library(name, self.SharedObjects(name, sources))

	def Plg(self, name, sources):
		plg = self.SharedLibrary(
			name, 
			self.SharedObjects(name + 'plugin', sources))
		self.Default(plg)
		if target == 'darwin':
			res = self.Command(
				name + '.rsrc', name + '.r', 
				'/Developer/Tools/Rez -o $TARGET' +
				' -useDF $SOURCE')
			home = os.environ['HOME'] + '/'
			instarget = home + 'Library/Internet Plug-Ins/' +\
			name + '.webplugin/'
			self.Default(
				self.Install(instarget + 'Contents/', 
					     'Info.plist'))
			self.Default(
				self.Install(instarget + 'Contents/MacOS/', 
					     plg))
			print plg
			self.Default(
				self.Install(instarget + 'Contents/Resources/',
					     res))

confCCFLAGS = {
	'debug': '-g -Wall ',
	'release': '-O2 -Wall ',
}

confCPPDEFINES = {
	'debug': [],
	'release': ['NDEBUG'],
}	

targetCCFLAGS = {
	'win32' : '',
	'darwin' : '-fvisibility=hidden',
	'linux' : '-fvisibility=hidden',
}


for backend in backends:
	for conf in ['debug']:
		cnf = Env(CCFLAGS=confCCFLAGS[conf] + targetCCFLAGS[target],
			  CPPDEFINES=confCPPDEFINES[conf],
			  )
		if tools:
			cnf.Tool(tools, toolpath)
		dir = conf + '/' + backend
		cnf.BuildDir(dir, '.', duplicate=0)
		env = cnf.Clone()
		env['BACKEND'] = backend
		env.Append(CPPDEFINES=[['AWBACKEND', backend]])
		Export("env")
		env.SConscript(dir  + '/SConscript')
