mytable = {}

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function check_table(ln)

    for x,n in ipairs(mytable) do

		if ln == n then
		   return false
		end
	end

	return true

end

function process_file(file)
    dofile(file)

	for a,b in ipairs(maplist) do

		if check_table(b) then
		    table.insert(mytable, b)
		end

	end

end


process_file("maplist_ctf.txt")
process_file("maplist_dom.txt")
process_file("maplist_ffa.txt")
process_file("maplist_inh.txt")
process_file("maplist_inv.txt")
process_file("maplist_pvm.txt")
process_file("maplist_pvp.txt")
process_file("maplist_tbi.txt")
process_file("maplist_tra.txt")
process_file("maplist_vhw.txt")

mapfound = false
mapsnotfound = {}

for q,w in ipairs(mytable) do

	if file_exists("D:/juegos/Quake 2/vortex/maps/" .. w .. ".bsp") then
		print("cp " .. "D:/juegos/Quake 2/vortex/maps/" .. w .. ".bsp " .. " D:/juegos/Quake 2/vortex\\ml\\")
	    os.execute("copy " .. "D:/juegos/Quake 2/vortex/maps/" .. w .. ".bsp " .. " D:/juegos/Quake 2/vortex\\ml\\")
		mapfound = true
	end

	if file_exists("../../baseq2/maps/" .. w .. ".bsp") then
		os.execute("cp " .. "../../baseq2/maps/" .. w .. ".bsp" .. " D:/juegos/Quake 2/vortex\\ml")
		mapfound = true
	end

	if not mapfound then
		table.insert(mapsnotfound, w)
	end
	mapfound = false
end

for q,w in ipairs(mapsnotfound) do
	print (w .. " was not found")
end

print ("press enter to finish")

io.read()
