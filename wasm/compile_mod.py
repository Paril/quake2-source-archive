import sys
import subprocess
import os
from pathlib import Path
import hashlib

def compile(modname, debug):
	
	compile_args = [
		'-c',
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
		'-Wno-braced-scalar-init'
	]

	if not debug:
		compile_args += [
			'-O3'
		]
	else:
		compile_args += [
			'-O0'
		]
	
	linker_args = [
		'--no-entry',
		'-mwasm32',
		'-Lwasi-sysroot/lib/wasm32-wasi',
		'wasi-sysroot/lib/wasm32-wasi/libc.a',
		'wasi-sysroot/lib/wasm32-wasi/crt1-reactor.o',
		'libclang_rt.builtins-wasm32.a',
		'-z,stack-size=8388608',
		'--initial-memory=33554432',
		'--export=__heap_base',
		'--export=__data_end',
		'--export=malloc',
		'--export=free'
	]
	
	linker_postfix = [
		'-o',
		f'../bin/{modname}/game.wasm'
	]
	
	c_args = [
	]
	
	cpp_args = [
		'-fno-exceptions',
		'-std=c++20',
	]
	
	c_files = [ Path('./quake2-wasm/game/wasm.c') ]
	cpp_files = []

	for path in { p.resolve() for p in Path(f'../sources/{modname}').rglob("**/*") if p.suffix in [ ".c", ".cc", ".cpp" ] }:
		if path.suffix == '.c':
			c_files.append(path)
		else:
			cpp_files.append(path)

	if len(cpp_files):
		linker_args.append('wasi-sysroot/lib/wasm32-wasi/libc++abi.a');
		linker_args.append('wasi-sysroot/lib/wasm32-wasi/libc++.a');

	debug = 'Debug' if debug else 'Release'
	print(f'Compiling {modname}:{debug}...')
	print('Found ' + str(len(c_files)) + ' C files and ' + str(len(cpp_files)) + ' C++ files')
	
	os.makedirs(f'../bin/{modname}/obj', 0o777, True)

	errfile = f'../bin/{modname}.stderr'

	if os.path.exists(errfile):
		os.remove(errfile)

	err = open(errfile, 'a')
	
	obj_files = []
	
	for path in c_files + cpp_files:
		obj_file = f'../bin/{modname}/obj/{path.name}-{hashlib.md5(str(path).encode()).hexdigest()}.o'
		obj_files.append(obj_file)
		subprocess.run([ 'clang' ] + compile_args + (c_args if path.suffix == '.c' else cpp_args) + [ '-o', obj_file, path ], stderr=err)

	subprocess.run([ 'wasm-ld' ] + linker_args + obj_files + linker_postfix, stderr=err)

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
