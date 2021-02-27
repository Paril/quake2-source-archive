q2print("Lua: Assigning nolag cvar.\n")

is_pvm = ((cvar_get("pvm", "0") ~= "0") or (cvar_get("invasion", "0") ~= "0"))
is_invasion = (cvar_get("invasion", "0") ~= "0")

UsePathfinding = 0
q2print("Pathfinding is disabled.\n")

cvar_set("vrx_over10mult", "0.75")
cvar_set("vrx_sub10mult", "1.2")
cvar_set("vrx_pvppointmult", "2.3")
cvar_set("vrx_pvpcreditmult", "2")

if is_pvm then
    cvar_set("nolag", "1")
	q2dofile("variables_pvm")

	cvar_set("vrx_over10mult", "0.6")
	cvar_set("vrx_sub10mult", "0.8")
	cvar_set("vrx_pvmpointmult", "0.8")

	if is_invasion then
		-- q2print("INFO: Using grid pathfinding.\n")
		-- UsePathfinding = 1
		cvar_set("vrx_over10mult", "0.45")
		cvar_set("vrx_sub10mult", "0.65")
		cvar_set("vrx_pvmpointmult", "0.6")

		-- so much exp in this map. Better nerf it.
		if cvar_get("mapname", "") ~= "wasted_inv3" then
			cvar_set("vrx_over10mult", "0.3")
			cvar_set("vrx_sub10mult", "0.5")
		end
	end

else
	cvar_set("nolag", "0")
	q2dofile("variables_pvp")
	cvar_set("vrx_pvmpointmult", "1.5") -- ffa case
end

reloadvars()

--[[
function string:split(sep)
        local sep, fields = sep or ":", {}
        local pattern = string.format("([^%s]+)", sep)
        self:gsub(pattern, function(c) fields[#fields+1] = c end)
        return fields
end

function parse_maplist(name)
	local ret_table = {}

	for line in io.lines(name) do
			local qsplit = line:split(",");
			print(qsplit[0])
			table.insert(ret_table, qsplit[0])
	end
	return ret_table;
end

parse_maplist("vortex/settings/maplist_pvp.txt")
]]

