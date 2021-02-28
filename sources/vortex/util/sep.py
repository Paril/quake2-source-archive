import re

inserts = open("ins.sql", "r")
outs = open("out.sql", "w")
tablename = "runes_mods"

outs.write("BEGIN TRANSACTION;")

for line in inserts:
	mlist = line.split("),(")
	
	for atext in mlist:
		print atext
		outs.write("INSERT INTO `{}` VALUES ({});\n".format(tablename, atext))
		
	
outs.write("COMMIT;")
