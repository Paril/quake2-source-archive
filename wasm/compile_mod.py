import sys
import subprocess
import os
from pathlib import Path
import hashlib
from string import Template
import re
import shutil
import multiprocessing

def compile_file(modname, compile_args, c_args, cpp_args, path):
	obj_file = f'../bin/{modname}/obj/{path.name}-{hashlib.md5(str(path).encode()).hexdigest()}'
	log = open(f'{obj_file}.txt', 'w')
	subprocess.run([ 'clang' ] + compile_args + (c_args if path.suffix == '.c' else cpp_args) + [ '-o', f'{obj_file}.o', path ], stderr=log)
	log.close()
	return obj_file

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
		'--stack-first',
		'-zstack-size=8388608',
		'--initial-memory=100663296',
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

	src_folder = Path(f'../sources/{modname}')

	exclude_re_file = src_folder.joinpath('exclude.opt')
	exclude_re = None
	
	if exclude_re_file.exists():
		with open(exclude_re_file, 'r') as content_file:
			exclude_re = re.compile(content_file.read())

	excluded = 0

	for path in { p.resolve() for p in src_folder.rglob("**/*") if p.suffix in [ ".c", ".cc", ".cpp" ] }:
		if exclude_re != None and exclude_re.search(str(path)):
			excluded += 1
		elif path.suffix == '.c':
			c_files.append(path)
		else:
			cpp_files.append(path)

	if len(cpp_files):
		linker_args.append('wasi-sysroot/lib/wasm32-wasi/libc++abi.a')
		linker_args.append('wasi-sysroot/lib/wasm32-wasi/libc++.a')

	def load_and_expand(path):
		if not os.path.exists(path):
			return False

		with open(path, 'r') as content_file:
			content = content_file.read()

		return Template(content).substitute(src_folder=str(src_folder)).split('\n')

	debug = 'Debug' if debug else 'Release'
	print('Found ' + str(len(c_files)) + ' C files and ' + str(len(cpp_files)) + ' C++ files')

	if excluded:
		print('Excluding ' + str(excluded))

	patch_file = src_folder.joinpath('wasm.patch')

	if patch_file.exists():
		patch_folder = src_folder.joinpath('src')
		print('Applying patches...')
		subprocess.run([ 'git', 'apply', '../wasm.patch' ], cwd=patch_folder)

	errfile = f'../bin/{modname}.stderr'

	print(f'Cleaning {modname}:{debug}...')

	if os.path.exists(errfile):
		os.remove(errfile)
	
	objs_path = Path(f'../bin/{modname}/obj')

	if objs_path.exists():
		shutil.rmtree(Path(f'../bin/{modname}/obj'))
	
	os.makedirs(str(objs_path), 0o777, True)
	
	outfile = Path(f'../bin/{modname}/game.wasm')
	
	if outfile.exists():
		os.remove(outfile)

	if (additional_compile_args := load_and_expand(f'../sources/{modname}/compiler.opt')) != False:
		compile_args += additional_compile_args

	err = open(errfile, 'a')

	term = shutil.get_terminal_size((80, 20))
	print(f'Compiling {modname}:{debug}...')
	
	all_paths = (c_files + cpp_files)
	all_paths.sort()

	results = []

	with multiprocessing.Pool(max(1, multiprocessing.cpu_count() - 1)) as pool:
		for path in all_paths:
			results.append(pool.apply_async(compile_file, (modname, compile_args, c_args, cpp_args, path)))
		obj_files = [ result.get() for result in results ]

	if patch_file.exists():
		patch_folder = src_folder.joinpath('src')
		print('Reverting patches...')
		subprocess.run([ 'git', 'apply', '-R', '../wasm.patch' ], cwd=patch_folder)

	for path in map(lambda f : Path(f'{f}.txt'), obj_files):
		if not path.exists() or not os.stat(path).st_size:
			continue
		with open(path, "r") as fr: 
			err.writelines(l for l in fr)
		err.flush()

	print(f'Linking {modname}:{debug}...')
	subprocess.run([ 'wasm-ld' ] + linker_args + list(map(lambda f : f'{f}.o', obj_files)) + linker_postfix, stderr=err)

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

	if outfile.exists():
		print(str(outfile) + ' compiled successfully');
	else:
		print('Linking failed; see errors above, or error file in bin.');
	
if __name__ == "__main__":
	compile(sys.argv[1], (sys.argv[2].lower() == 'debug') if len(sys.argv) > 2 else True)
