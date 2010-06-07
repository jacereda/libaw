#-*-Python-*-
import sys
import os
backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'native':
	target = sys.platform
elif target == 'iphone':
	tools = 'iphone'
elif target == 'iphonesim':
	tools = 'iphonesim'
else:
	tools = 'crossmingw'

if target == 'linux2':
	target = 'linux'

if target == 'win32':
	comp = 'cl'
else:
	comp = 'gcc'

backends = {
	'iphone' : ['iphone'],
	'iphonesim' : ['iphone'],
	'darwin' : ['cocoa', 'x11'],
	'linux' : ['x11'],
	'win32' : ['nt'],
	}[target]

def genplist(target, source, env):
	name = str(target[0])
	name = name.split('/')[-1].split('.')[0]
	print name
	s = open(str(source[0]))
	d = open(str(target[0]), 'w')
	c = s.read().replace('NAME', name) 
	d.write(c)
	d.close()
	s.close()
	


class Env(Environment):
	def UsesOpenGL(self):
		if self['BACKEND'] == 'iphone':
			self.Append(FRAMEWORKS=['OpenGLES', 'QuartzCore', 'UIKit', 'Foundation'])
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
		if self['BACKEND'] in ['iphone', 'iphonesim']:
			return self._Objects(bdir, sources)
		return [self.SharedObject('sobj' + bdir + '/' + s + '.os', s) 
			for s in Split(sources)]

	def ForLib(self):
		return self.Clone()

	def Lib(self, name, sources):
		return self.Default(self.Library(name, 
						 self._Objects(name, sources)))

	def _SetCPPFlags(self):
		self.Append(CPPPATH=['include'])
		self.Append(LIBPATH='.') 

	def ForGLPrg(self):
		if self['BACKEND'] in ['iphone']:
			return None
		ret = self.Clone()
		ret.Append(LIBS=['aw'])
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		return ret

	def ForGLESPrg(self):
		if not self['BACKEND'] in ['iphone']:
			return None
		ret = self.Clone()
		ret.Append(LIBS=['aw'])
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		return ret


	def App(self, name, prg):
		if target != 'iphonesim':
			return
		dst = '#/../Library/Application Support/' +\
		'iPhone Simulator/User/Applications/%s/%s.app/' % (name, name)
		plist = self.Command(name + '.plist', 'template.plist',
				     genplist)
		self.Default(self.Install(dst, prg))
		self.Default(self.InstallAs(dst + 'Info.plist', 
					    name + '.plist'))
#		self.Default(self.Install(dst, name + '.png'))


	def Prg(self, name, sources):
		prg = self.Program(name, 
				   self._SharedObjects(name, sources))
		self.Default(prg)
		self.App(name, prg)

	def CompileAs32Bits(self):
		if comp == 'gcc':
			self.Append(CCFLAGS=' -m32 ')
			self.Append(LINKFLAGS=' -m32 ')

	def ForNPAPI(self):
		if self['BACKEND'] not in ['cocoa', 'nt']:
			return None
		ret = self.Clone()
		ret.Append(CPPDEFINES=['AWPLUGIN'])
		ret.Append(CPPPATH=[target])
		ret.CompileAs32Bits()
		if target == 'win32':
			ret.Append(CPPDEFINES=['XULRUNNER_SDK', 'WIN32', '_WINDOWS', 'XP_WIN32', 'MOZILLA_STRICT_API', 'XPCOM_GLUE', 'XP_WIN', '_X86_', 'NPSIMPLE_EXPORTS',])
			ret.Append(LIBS=['user32'])

		return ret
		
	def ForPlugin(self):
		if self['BACKEND'] not in ['cocoa', 'nt']:
			return None
		ret = self.Clone()
		ret.UsesOpenGL()
		ret.CompileAs32Bits()
		ret._SetCPPFlags()
		ret.Append(LIBS=['awnpapi'])
		ret.Append(FRAMEWORKS=['WebKit', 'QuartzCore'])
		if target in ['darwin']:
			ret['SHLINKFLAGS'] = '$LINKFLAGS' +\
			    ' -bundle -flat_namespace'
			ret['SHLIBPREFIX'] = ''
			ret['SHLIBSUFFIX'] = ''
		return ret

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
					 self._SharedObjects(name, sources) +\
					 platobjs)
		self.Default(plg)
		home = os.environ['HOME'] + '/'
		if target == 'win32':
			instarget = home + 'Mozilla/Plugins/'
			self.Default(self.Install(instarget, plg))
		if target in ['darwinnn']:
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
			  CPPDEFINES=confcppdefines[conf]+compcppdefines[comp],
			  LINKFLAGS=linkflags[comp][conf],
			  )
		if tools:
			cnf.Tool(tools, '.')
		dir = conf + '/' + backend
		cnf.BuildDir(dir, '.', duplicate=0)
		env = cnf.Clone()
		env['BACKEND'] = backend
		env.Append(CPPDEFINES=[['AWBACKEND', backend]])
		Export("env")
		env.SConscript(dir  + '/SConscript')
