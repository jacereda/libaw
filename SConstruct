#-*-Python-*-
import sys
import os
backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'native':
	target = sys.platform
#	if target == 'win32':
#		tools = 'mingw'
#		toolpath = None
else:
	tools = 'crossmingw'
	toolpath = '.'
if target == 'linux2':
	target = 'linux'

if target == 'win32':
	comp = 'cl'
else:
	comp = 'gcc'

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
			self.Append(LIBS=['opengl32', 'gdi32', 'glu32', 'user32'])

	def _Objects(self, bdir, sources):
		return [self.Object('obj' + bdir + '/' + s + '.o', s) 
			for s in Split(sources)]
		
	def _SharedObjects(self, bdir, sources):
		return [self.SharedObject('sobj' + bdir + '/' + s + '.os', s) 
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
					  self._SharedObjects(name, sources)))

	def CompileAs32Bits(self):
		if comp == 'gcc':
			self.Append(CCFLAGS=' -m32 ')
			self.Append(LINKFLAGS=' -m32 ')

	def ForNPAPI(self):
		ret = self.Clone()
		ret.Append(CPPDEFINES=['AWPLUGIN'])
		ret.Append(CPPPATH=[target])
		ret.CompileAs32Bits()
		if target == 'win32':
			ret.Append(CPPDEFINES=['XULRUNNER_SDK', 'WIN32', '_WINDOWS', 'XP_WIN32', 'MOZILLA_STRICT_API', 'XPCOM_GLUE', 'XP_WIN', '_X86_', 'NPSIMPLE_EXPORTS',])
			ret.Append(LIBS=['user32'])

		return ret
		
	def ForPlugin(self):
		ret = self.Clone()
		ret.UsesOpenGL()
		ret.CompileAs32Bits()
		ret._SetCPPFlags()
		ret.Append(LIBS=['awnpapi'])
		ret.Append(FRAMEWORKS=['WebKit', 'QuartzCore'])
		if target == 'darwin':
			ret['SHLINKFLAGS'] = '$LINKFLAGS' +\
			    ' -bundle -flat_namespace'
			ret['SHLIBPREFIX'] = ''
			ret['SHLIBSUFFIX'] = ''
		return ret

	def ForShLib(self):
		ret = self.Clone()
		if target == 'win32':
			ret.Append(LIBS=['user32'])
		return ret

	def ShLib(self, name, sources):
		return self.SharedLibrary(name, self._SharedObjects(name, sources))

	def ShLinkLib(self, name, sources):
		return self.Library(name, self._SharedObjects(name, sources))

	def Plg(self, name, sources):
		prefix = ''
		platobjs = []
		if target == 'win32':
			prefix = 'np'
			res = self.Command(name + '.res', name + '.rc', 
					   'rc /fo $TARGET $SOURCE')
			platobjs = ['awnpapi.def', res[0]]
		plg = self.SharedLibrary(prefix + name, 
				   self._SharedObjects(name, sources) + platobjs)
		self.Default(plg)
		home = os.environ['HOME'] + '/'
		if target == 'win32':
			instarget = home + 'Mozilla/Plugins/'
			self.Default(self.Install(instarget, plg))
		if target == 'darwin':
			res = self.Command(
				name + '.rsrc', name + '.r', 
				'/Developer/Tools/Rez -o $TARGET' +
				' -useDF $SOURCE')
			instarget = home + 'Library/Internet Plug-Ins/' +\
			name + '.webplugin/'
			self.Default(
				self.Install(instarget + 'Contents/', 
					     'Info.plist'))
			self.Default(
				self.Install(instarget + 'Contents/MacOS/', 
					     plg))
			self.Default(
				self.Install(instarget + 'Contents/Resources/',
					     res))

ccflags = {
	'gcc': {
		'debug': '-g -Wall -fvisibility=hidden',
		'release': '-O2 -Wall -fvisibility=hidden',
		},
	'cl': {
		'debug': ' /EHs-c- /MTd /DEBUG /Z7 /Od ',
		'release': ' /MT /O2 ',
		},
}

linkflags = {
	'gcc': {
		'debug': '',
		'release': '',
		},
	'cl': {
		'debug': ' /DEBUG ',
		'release': '',
		},
}

confcppdefines = {
	'debug': [],
	'release': ['NDEBUG'],
}

compcppdefines= {
	'gcc': [],
	'cl' : [['snprintf', '_snprintf']],
}


for backend in backends:
	for conf in ['debug']:
		cnf = Env(CCFLAGS=ccflags[comp][conf],
			  CPPDEFINES=confcppdefines[conf] + compcppdefines[comp],
			  LINKFLAGS=linkflags[comp][conf],
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
