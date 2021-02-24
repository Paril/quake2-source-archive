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
In the `wasm` folder, do `py compile_mod.py <name> [debug|release]` to compile an individual mod. By default, debug is implied, but anything other than debug will result in a release build. To compile every mod, do `py compile_mods.py [debug|release]`.
