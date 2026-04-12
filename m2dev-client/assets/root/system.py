import sys
import app
import dbg
import builtins
import _frozen_importlib as _bootstrap
import _frozen_importlib_external as _bootstrap_external
import marshal
import pack
import __main__
import re

# Keep import paths deterministic for the embedded runtime.
sys.path = [p for p in sys.path if isinstance(p, str)]
if "lib" not in sys.path:
	sys.path.append("lib")

class TraceFile:
	def write(self, msg):
		dbg.Trace(msg)

	def flush(self):
		pass

class TraceErrorFile:
	def write(self, msg):
		dbg.TraceError(msg)
		dbg.RegisterExceptionString(msg)

	def flush(self):
		pass

class LogBoxFile:
	def __init__(self):
		self.stderrSave = sys.stderr
		self.msg = ""

	def __del__(self):
		self.restore()

	def restore(self):
		if sys:
			sys.stderr = self.stderrSave

	def write(self, msg):
		self.msg = self.msg + msg

	def show(self):
		dbg.LogBox(self.msg,"Error")

sys.stdout = TraceFile()
sys.stderr = TraceErrorFile()

#
# pack file support (must move to system.py, systemrelease.pyc)
#

class pack_file_iterator(object):
	def __init__(self, packfile):
		self.pack_file = packfile
		
	def __next__(self):
		tmp = self.pack_file.readline()
		if tmp:
			return tmp
		raise StopIteration

_chr = builtins.chr

_SOURCE_ENCODING_RE = re.compile(br"coding[:=]\s*([-\w.]+)")

def _extract_source_encoding(data):
	for line in data.splitlines()[:2]:
		match = _SOURCE_ENCODING_RE.search(line)
		if not match:
			continue
		try:
			return match.group(1).decode("ascii")
		except Exception:
			return None
	return None

def _decode_pack_text(data):
	if isinstance(data, str):
		return data

	if isinstance(data, bytearray):
		data = bytes(data)
	elif not isinstance(data, bytes):
		data = bytes(data)

	encodings = []
	source_encoding = _extract_source_encoding(data)
	if source_encoding:
		encodings.append(source_encoding)
	encodings.extend(("utf-8-sig", "cp949", "latin-1"))

	seen = set()
	for encoding in encodings:
		if encoding in seen:
			continue
		seen.add(encoding)
		try:
			return data.decode(encoding)
		except (LookupError, UnicodeDecodeError):
			pass

	return data.decode("utf-8", "replace")

class pack_file(object):

	def __init__(self, filename, mode = 'rb'):
		assert mode in ('r', 'rb')
		if not pack.Exist(filename):
			raise IOError('No file or directory')
		self.mode = mode
		self.data = pack.Get(filename)
		if mode == 'r':
			self.data = _decode_pack_text(self.data)
			self.data = self.data.replace(_chr(13) + _chr(10), _chr(10)).replace(_chr(13), _chr(10))

	def __iter__(self):
		return pack_file_iterator(self)

	def read(self, length = None):
		empty = b'' if self.mode == 'rb' else ''
		if not self.data:
			return empty
		if length:
			tmp = self.data[:length]
			self.data = self.data[length:]
			return tmp
		else:
			tmp = self.data
			self.data = b'' if self.mode == 'rb' else ''
			return tmp

	def readline(self):
		newline = b'\n' if self.mode == 'rb' else _chr(10)
		return self.read(self.data.find(newline)+1)

	def readlines(self):
		return [x for x in self]

builtins.pack_open = pack_open = pack_file

old_open = open
def open(filename, mode = 'rb'):
	try:
		if mode in ('r', 'rb') and pack.Exist(filename):
			return pack_file(filename, mode)
	except Exception:
		pass
	return old_open(filename, mode)

builtins.open = open
builtins.old_open = old_open
builtins.new_open = open

_ModuleType = type(sys)

module_do = lambda x:None
currentExecName = ""

class custom_import_hook(object):
	def _pack_filename(self, name):
		filename = name + ".py"
		if pack.Exist(filename):
			return filename
		return None

	def find_spec(self, fullname, path=None, target=None):
		filename = self._pack_filename(fullname)
		if not filename:
			return None
		return _bootstrap.spec_from_loader(fullname, self, origin=filename)

	def create_module(self, spec):
		return None

	def exec_module(self, module):
		global currentExecName
		name = module.__name__
		filename = self._pack_filename(name)
		if not filename:
			raise ImportError(name)
		dbg.Trace('importing from pack %s\\n' % name)
		currentExecName = name
		execfile(filename, module.__dict__)
		module_do(module)

	def find_module(self, name, path=None):
		if self._pack_filename(name):
			return self
		return None

	def load_module(self, name):
		if name in sys.modules:
			return sys.modules[name]

		module = _ModuleType(name)
		sys.modules[name] = module
		self.exec_module(module)
		return sys.modules[name]

def splitext(p):
	root, ext = '', ''
	for c in p:
		if c in ['/']:
			root, ext = root + ext + c, ''
		elif c == '.':
			if ext:
				root, ext = root + ext, c
			else:
				ext = c
		elif ext:
			ext = ext + c
		else:
			root = root + c
	return root, ext

class PythonExecutioner: 

	def Run(kPESelf, sFileName, kDict): 
		if kPESelf.__IsCompiledFile__(sFileName): 
			kCode=kPESelf.__LoadCompiledFile__(sFileName) 
		else: 
			kCode=kPESelf.__LoadTextFile__(sFileName) 

		exec(kCode, kDict) 

	def __IsCompiledFile__(kPESelf, sFileName): 

		sBase, sExt = splitext(sFileName) 
		sExt=sExt.lower() 

		if sExt==".pyc" or sExt==".pyo": 
			return 1 
		else: 
			return 0 

	def __LoadTextFile__(kPESelf, sFileName): 
		sText=pack_open(sFileName,'r').read() 
		return compile(sText, sFileName, "exec") 

	def __LoadCompiledFile__(kPESelf, sFileName): 
		kFile=pack_open(sFileName)

		magic = kFile.read(4)
		if isinstance(magic, str):
			magic = magic.encode("latin1")

		if magic != _bootstrap_external.MAGIC_NUMBER:
			raise 

		kFile.read(4) 

		kData=kFile.read() 
		if isinstance(kData, str):
			kData = kData.encode("latin1")
		return marshal.loads(kData) 

def execfile(fileName, dict): 
	kPE=PythonExecutioner() 
	kPE.Run(fileName, dict) 

def exec_add_module_do(mod):
	global execfile
	mod.__dict__['execfile'] = execfile

module_do = exec_add_module_do

meta_hook = custom_import_hook()
if meta_hook not in sys.meta_path:
	sys.meta_path.insert(0, meta_hook)

"""
#
# PSYCO installation (must move to system.py, systemrelease.pyc)
#
try:
	import psyco
	#from psyco.classes import *

	def bind_me(bindable_list):
		try:
			for x in bindable_list:
				try:
					psyco.bind(x)
				except:
					pass
		except:
			pass		

	_prev_psyco_old_module_do = module_do
	def module_bind(module):
		_prev_psyco_old_module_do(module)
		#print 'start binding' + str(module)
		try:
			psyco.bind(module)
		except:
			pass
		for x in module.__dict__.itervalues():
			try:
				psyco.bind(x)
			except:
				pass		
		#print 'end binding'

	dbg.Trace("PSYCO installed\\n")

except Exception, msg:
	bind_me = lambda x:None
	dbg.Trace("No PSYCO support : %s\\n" % msg)
"""

def GetExceptionString(excTitle):
	(excType, excMsg, excTraceBack)=sys.exc_info()
	excText=""
	excText+=_chr(10)

	import traceback
	traceLineList=traceback.extract_tb(excTraceBack)

	for traceLine in traceLineList:
		if traceLine[3]:
			excText+="%s(line:%d) %s - %s" % (traceLine[0], traceLine[1], traceLine[2], traceLine[3])
		else:
			excText+="%s(line:%d) %s"  % (traceLine[0], traceLine[1], traceLine[2])

		excText+=_chr(10)
	
	excText+=_chr(10)
	excText+="%s - %s:%s" % (excTitle, excType, excMsg)		
	excText+=_chr(10)

	return excText

def ShowException(excTitle):
	excText=GetExceptionString(excTitle)
	dbg.TraceError(excText)
	app.Abort()

	return 0

def RunMainScript(name):
	try:		
		source = open(name, "rb").read()
		if isinstance(source, (bytes, bytearray)):
			source = _decode_pack_text(source)
		exec(compile(source, name, 'exec'), __main__.__dict__)
	except RuntimeError as msg:
		msg = str(msg)

		import locale
		if locale.error:
			msg = locale.error.get(msg, msg)

		dbg.LogBox(msg)
		app.Abort()

	except:	
		msg = GetExceptionString("Run")
		dbg.LogBox(msg)
		app.Abort()
	
try:
	import debugInfo
except ImportError:
	import debuginfo as debugInfo
debugInfo.SetDebugMode(__DEBUG__)

loginMark = "-cs"

RunMainScript("prototype.py")

