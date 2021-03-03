	Here is the source for Vanilla CTF v0.9.  The only wrinkle is that
the file zbot.c is not included (so as not to reveal what I'm doing to the
people hacking the zbot).  However, I have included object files zbot.obj
and zbot.o for compiling for Win32 and Linux respectively.  This should allow
you to build a fully functional version, although you might need to tweak
the project files / makefile a bit to get it to work correctly.  
Alternatively, its trivial to remove the references to the zbot code and
compile a copy without zbot detection.  Have fun.