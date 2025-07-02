# Quake II Source Archive

This is an archive of Quake II game/mod sources. The repo will be used to compile cross-platform/cross-arch WASM binaries.
This is a sister project to https://github.com/Paril/quake2-wasm which provides an implementation of a game library that hosts a WebAssembly runtime, which can then load WASM binaries as game libraries. This allows game libraries to be cross-platform and compatible, no matter where they are run. As of this writing, the game DLL is the only implementation available, but the goal is to allow engines to implement this system down the road.

# Licenses

Licenses per-mod are included in the mods' source folders.

# How to Help

Submit sources to mods as issues, and we'll handle the rest. If the mod is actively maintained, submit the repository link.

# Sources in Repos

Currently, we're embedding the sources in this repo, however the plan for actively-maintained sources is to instead link them either via submodule or as a Python script to fetch their source, and use patches to fix any major code issues. We'll also likely submit patches back to the repo with explanations of our fixes, in case they are interested.

# How to Compile

## wasm only

You will need Python 3 and the [WASI SDK](https://github.com/WebAssembly/wasi-sdk) installed. For help installing the WASI SDK for use with this repository, check [the CI workflow](https://github.com/fpiesche/quake2-source-archive/blob/7e886d427086ba535541b57b6e33fd56c1d2aec1/.github/workflows/build-mod.yml#L65).

```bash
$ cd wasm
$ python3 ./compile_mods.py [debug|release]            # build all mods
$ python3 ./compile_mod.py <mod name> [debug|release]  # build a specific mod
```

## cmake

Ensure that the WASI SDK (see above), Python 3 and CMake are installed; [Ninja](https://ninja-build.org/) is an optional component that will speed up builds. If you want to be able to make cross-platform builds (eg. x86 built on an x86_64 machine), you'll also need the appropriate cross-compile packages. You can find a list of these (for Ubuntu running on x86_64) in [the GitHub workflow](https://github.com/fpiesche/quake2-source-archive/blob/7e886d427086ba535541b57b6e33fd56c1d2aec1/.github/workflows/build-mod.yml#L18).

```bash
$ mkdir build
$ cd build
$ cmake -GNinja ..                                          # Configure build for current platform
$ cmake -GNinja -DQ2_BUILD_ARCH=[x86|x86_64|armhf|aarch64]  # Configure builds for cross-compilation (eg. building 32bit x86 on a 64-bit system)
$ cmake --build .                                           # build all mods
$ cmake --build . --target <mod name>                       # build only a specific mod
```
