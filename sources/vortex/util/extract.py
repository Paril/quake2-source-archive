import re

settingsfile = open("settings.txt", "r")
luafile = open("list.lua", "w")
cfile = open("v_luasettings_game.c", "w")
chead = open("v_luasettings_game.h", "w")
cdecl = open("v_luasettings_decl.c", "w")
nomatch = open("nomatch.txt", "w")

for line in settingsfile:
	defmatch = re.match("#define[\s\t]+(.+)[\s\t]+(-?[0-9]+(\.+[0-9]+)?)", line)
	if defmatch != None:
		print defmatch.group(1).strip(), defmatch.group(2)
		if defmatch.group(2) != None:
			varname = defmatch.group(1).strip()
			varvalue = defmatch.group(2).strip()
			luafile.write ("{0} = {1}\n".format(varname.ljust(40), varvalue))
			cfile.write("{0} = Lua_GetVariable(\"{0}\", {1});\n".format(varname, varvalue))
			chead.write("extern double {0};\n".format(varname))
			cdecl.write("double {0};\n".format(varname))
		else:
			nomatch.write(line)
	else:
		nomatch.write(line)


