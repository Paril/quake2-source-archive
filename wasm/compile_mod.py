import sys
import subprocess
import os
from pathlib import Path

def compile(modname, debug):
	
	if not debug:
		args = [
			'-O3'
		]
	else:
		args = [ ]
	
	args += [
		'-Wl,-z,stack-size=8388608',
		'-Wl,--initial-memory=33554432',
		'--target=wasm32-wasi',
		'--sysroot=wasi-sysroot',
		#'-Wl,--export=__heap_end',
		#'-Wl,--export=__data_base',
		'-Wl,--export=malloc',
		'-Wl,--export=free',
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

	debug = 'Debug' if debug else 'Release'

	print(f'Compiling {modname}:{debug}...')

	errfile = f'../bin/{modname}.stderr'
	err = open(errfile, 'w')
	subprocess.run([ "clang" ] + args, stderr=err)
	elen = err.tell()
	err.close()
	
	if not elen:
		os.remove(errfile)
	else:
		err = open(errfile, 'r')
		lines = err.readlines()
		err.close()

		for line in lines:
			if line.find("error:") != -1 or line.find("warning:") != -1:
				print('  ' + line.strip())
	
if __name__ == "__main__":
	compile(sys.argv[1], (sys.argv[2].lower() == 'debug') if len(sys.argv) > 2 else True)
