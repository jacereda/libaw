#-*-Python-*-
import sys
import os

domain = ARGUMENTS.get('domain', 'com.example')

backends = []

tools = 0

target = ARGUMENTS.get('target', 'native')
if target == 'native':
	target = sys.platform
elif target == 'iphone':
	tools = 'iphone'
elif target == 'iphonesim':
	tools = 'iphonesim'
elif target == 'android':
	tools = 'android'
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
	'darwin' : ['cocoa'],
	'android' : ['android'],
	'linux' : ['x11'],
	'win32' : ['nt'],
	'mingw' : ['nt'],
	}[target]

def filtertemplate(target, source, env):
	name = str(target[0])
	name = name.split(os.path.sep)[-1].split('.')[0]
	s = open(str(source[0]))
	d = open(str(target[0]), 'w')
	c = s.read().replace('NAME', name) 
	d.write(c)
	d.close()
	s.close()

def filtermanifest(target, source, env):
	name = str(target[0])
	name = name.split(os.path.sep)[-2]
	s = open(str(source[0]))
	d = open(str(target[0]), 'w')
	c = s.read().replace('{NAME}', name).replace('{DOMAIN}', domain)
	d.write(c)
	d.close()
	s.close()
	
def outputfrom(cmd):
	p = os.popen(cmd)
	ret = p.read()
	p.close
	return ret


class Env(Environment):
	def UsesOpenGL(self):
		if self['BACKEND'] == 'iphone':
			self.Append(FRAMEWORKS=['OpenGLES', 'QuartzCore', 'UIKit', 'Foundation'])
#		if self['BACKEND'] == 'android':
#			self.Append(LIBS=['EGL', 'GLESv1_CM', 'GLESv2'])
		if self['BACKEND'] == 'cocoa':
			self.Append(FRAMEWORKS=['OpenGL', 'AppKit'])
		if self['BACKEND'] == 'x11':
			self.Append(LIBPATH=['/usr/X11R6/lib'])
			self.Append(CPPPATH=['/usr/X11R6/include',
					     '/usr/include/gtk-2.0/'])
			self.Append(LIBS=['GL', 'GLU', 'X11'])
		if self['BACKEND'] == 'nt':
			self.Append(LIBS=['opengl32', 'gdi32', 'glu32', 'user32', 'imm32'])

	def _Objects(self, bdir, sources):
		if self['BACKEND'] in ['android']:
			return self._SharedObjects(bdir, sources)

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
		if self['BACKEND'] in ['iphone', 'android']:
			return None
		ret = self.Clone()
		ret.Append(LIBS=['aw'])
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		return ret

	def ForGLESPrg(self):
		if not self['BACKEND'] in ['iphone', 'android']:
			return None
		ret = self.Clone()
		ret.Append(LIBS=['aw'])
		ret.UsesOpenGL()
		ret._SetCPPFlags()
		return ret


	def App(self, name, prg):
		dst = '#/../Library/Application Support/' +\
		'iPhone Simulator/User/Applications/%s/%s.app/' % (name, name)
		plist = self.Command(name + '.plist', 'template.plist',
				     filtertemplate)
		self.Default(self.Install(dst, prg))
		self.Default(self.InstallAs(dst + 'Info.plist', plist))
#		self.Default(self.Install(dst, name + '.png'))

		
	def Apk(self, name, prg):
		manifest = self.Command('%s/AndroidManifest.xml' % name,
					'AndroidManifest-template.xml',
					filtermanifest)
		buildxml = self.Command(
			'%s/build.xml' % name, manifest,
			'%s update project -p ${TARGET.dir} -t 1' 
			% self['ANDROID'])
		apk = self.Command(
			'%s/bin/%s-debug.apk' % (name, name), 
			[buildxml, prg],
			'ant debug -f ${SOURCE} && ' +\
			'mv ${TARGET.dir}/NativeActivity-debug.apk $TARGET')
		self.Default(self.Command(
				name + '-install', apk,
				'%s uninstall %s && %s install $SOURCE'
				%(env['ADB'], domain+'.'+name, env['ADB'])))

	def Prg(self, name, sources):
		if self['BACKEND'] in ['android']:
			prg = self.SharedLibrary(
				'%s/libs/armeabi/%s' % (name,name),
				self._SharedObjects(name, sources))
		else:
			prg = self.Program(name,
					   self._SharedObjects(name, sources))
		self.Default(prg)
		if target == 'iphonesim':
			self.App(name, prg)
		if target == 'android':
			self.Apk(name, prg)

	def CompileAs32Bits(self):
		if comp == 'gcc':
			self.Append(CCFLAGS=' -m32 ')
			self.Append(LINKFLAGS=' -m32 ')

	def ForNPAPI(self):
		if self['BACKEND'] not in ['cocoa', 'nt', 'x11']:
			return None
		ret = self.Clone()
		ret.Append(CPPDEFINES=['AWPLUGIN'])
		ret.Append(CPPPATH=['npapi'])
		ret.CompileAs32Bits()
		if target == 'win32':
			ret.Append(CPPDEFINES=['XULRUNNER_SDK', 'WIN32', '_WINDOWS', 'XP_WIN32', 'MOZILLA_STRICT_API', 'XPCOM_GLUE', 'XP_WIN', '_X86_', 'NPSIMPLE_EXPORTS',])
			ret.Append(LIBS=['user32'])
		if target == 'linux':
			ret.Append(CFLAGS=outputfrom(
					'pkg-config --cflags gtk+-2.0'))
			ret.Append(LINK=outputfrom(
					'pkg-config --libs gtk+-2.0'))
			ret.Append(CPPDEFINES=['XULRUNNER_SDK', 'XP_UNIX', 'MOZ_X11'])
		return ret
		
	def ForPlugin(self):
		if self['BACKEND'] not in ['cocoa', 'nt', 'x11']:
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
		return # XXX Broken when running as out-of-process
		prefix = ''
		platobjs = []
		if target == 'win32':
			prefix = 'np'
			ress = self.Command(name + '.rc', 'template.rc',
					    filtertemplate)
			res = self.Command(name + '.res', ress, 
					   'rc /fo $TARGET $SOURCE')
			platobjs = ['awnpapi.def', res[0]]
		plg = self.SharedLibrary(prefix + name, 
					 self._SharedObjects(name, sources) +\
					 platobjs)
		self.Default(plg)
		home = os.environ['HOME'] + '/'
		if target == 'win32' and self['CONF'] == 'release':
			instarget = home + 'Mozilla/Plugins/'
			self.Default(self.Install(instarget, plg))
		if target == 'linux' and self['CONF'] == 'release':
			instarget = home + '.mozilla/plugins/'
			self.Default(self.Install(instarget, plg))
		if target == 'darwin' and self['CONF'] == 'release':
			ress = self.Command(name + '.r', 'template.r',
					    filtertemplate)
			res = self.Command(
				name + '.rsrc', ress, 
				'/Developer/Tools/Rez -o $TARGET' +
				' -useDF $SOURCE')
			instarget = home + 'Library/Internet Plug-Ins/' +\
			name + '.webplugin/'
			plist = self.Command(name + '.plist',
					     'webtemplate.plist',
					     filtertemplate)
			self.Default(
				self.InstallAs(instarget +\
						       'Contents/Info.plist', 
					       plist))
			self.Default(
				self.Install(instarget + 'Contents/MacOS/', 
					     plg))
			self.Default(
				self.Install(instarget + 'Contents/Resources/',
					     res))

ccflags = {
	'gcc': {
		'debug': '-g -Wall ',
		'release': '-g -O2 -Wall ',
		},
	'cl': {
		'debug': ' /EHs-c- /MTd /DEBUG /Z7 /Od ',
		'release': ' /MT /O2 ',
		},
}

linkflags = {
	'gcc': {
		'debug': '-g',
		'release': '-g',
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
	for conf in ARGUMENTS.get('conf', 'debug,release').split(','):
		cf = ccflags[comp][conf]
		lf = linkflags[comp][conf]
		if not 'nt' in backends:
			cf += ' -fvisibility=hidden '
			lf += ' -fvisibility=hidden '
		cnf = Env(CCFLAGS=cf,
			  CPPDEFINES=confcppdefines[conf]+compcppdefines[comp],
			  LINKFLAGS=lf,
			  )
		if tools:
			cnf.Tool(tools, '.')
		dir = conf + '/' + backend
		cnf.BuildDir(dir, '.', duplicate=0)
		cnf.SConsignFile(dir)
		cnf['CONF'] = conf
		env = cnf.Clone()
		env['BACKEND'] = backend
		env.Append(CPPDEFINES=[['AWBACKEND', backend]])
		Export("env")
		env.SConscript(dir  + '/SConscript')
