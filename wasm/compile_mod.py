import sys
import subprocess
import os
from pathlib import Path

def compile(modname):
	args = [
		'-Wl,-z,stack-size=8388608',
		'-Wl,--initial-memory=33554432',
		'--target=wasm32-wasi',
		'--sysroot=wasi-sysroot',
		'-W',
		'-Wall',
		'-Wno-tautological-constant-out-of-range-compare',
		'-Wno-unused-parameter',
		'-Wno-unused-value',
		'-Wno-missing-braces',
		'-Wno-unused-variable',
		'-Wno-sign-compare',
		'-Wno-absolute-value',
		'-Wno-dangling-else',
		'-Wno-constant-conversion',
		'-Wno-switch',
		'-Wno-missing-field-initializers',
		'-Wno-unused-function',
		'-Wno-misleading-indentation',
		'-Wno-macro-redefined',
		'-Wno-braced-scalar-init',
		'-D_DEBUG',
		'-Dstricmp=Q_stricmp',
		'-o',
		f'../bin/{modname}/game.wasm',
		'./quake2-wasm/game/wasm.c'
	]

	for path in Path(f'../sources/{modname}').rglob('*.c'):
		args.append(path)
	
	os.makedirs(f'../bin/{modname}', 0o777, True)

	print(f'Compiling {modname}...')

	errfile = f'../bin/{modname}.stderr'
	err = open(errfile, 'wb')
	subprocess.run([ "clang" ] + args, stderr=err)
	elen = err.tell()
	err.close()
	
	if not elen:
		os.remove(errfile)
	else:
		err = open(errfile, 'rb')
		line = ''
		offset = -1
		while 1:
			err.seek(offset, os.SEEK_END)
			offset = offset - 1
			c = chr(err.read(1)[0])
			
			if c.isprintable():
				line = c + line
			
			if offset == 0 or (c == '\n' and len(line) > 0):
				break
		print('  (last message): ' + line.strip())
		err.close()
	
if __name__ == "__main__":
	compile(sys.argv[1])
