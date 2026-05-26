-- modifed: igorpauk 2017-18

function sprintf(...)
	local r, s = pcall(string.format,...)
	return s
end;

function printf(...)
	print(sprintf(...))
end

-- Dump the given value
function dump(v,n)
	if n == nil then n = 0 end
	local r
	local p = string.rep("  ",n)
	local t = type(v)
	if t == "table" then
		local i = 0
		r = "{"
		for tk, tv in pairs(v) do
			r = r..string.format("\n%s  %s => %s",p,dump(tk,n+1),dump(tv,n+1))
			i = i + 1
		end
		if i > 0 then r = r.."\n"..p end
		r = r.."}"
	elseif t == "string" then
		r = '"'..v..'"'
	elseif (t ~= "number") and (t ~= "boolean") then
		r = "("..tostring(v)..")"
	else
		r = tostring(v)
	end
	return r
end

------------------------------------------------------------------------------------------------------

math.randomseed(os.time()*123456789 % 987654321)	-- Init random seed

aux = {}

-- Roll a chance
function aux.randRoll(p)
	return math.random() < p
end

-- Get random array value
function aux.getRandVal(t, n)
	local s = #t
	if s == 0 then return end
	if n == nil then
		return t[math.random(s)]
	else
		local tCopy = t
		local tNew = {}
		if s <= n then
			n = s
		end
		for _ = 1, n do
			table.insert(tNew,table.remove(tCopy,math.random(#tCopy)))
		end
		return tNew
	end
end

-- Get random table key
function aux.getRandKey(t)
	local keys = {}
	for k in pairs(t) do
		table.insert(keys,k)
	end
	local s = #keys
	if s == 0 then return end
	return keys[math.random(s)]
end

-- Check whether 'a' is in table 't'
function aux.inTable(a, t)
	for _,v in pairs(t) do
		if v == a then return true end
	end
	return false
end

-- Use a usable random effect, filter before if necessary
function aux.useEffect(filter, targetPtr, prob)
	if (prob ~= nil) and not aux.randRoll(prob) then return false end
	local effList = {}
	local dofilter = (type(filter) == "table")
	for _,effPtr in pairs(EFF_LIST(my.persPtr)) do
		if EFF_ISUSABLE(effPtr) then
			if not dofilter then
				table.insert(effList,effPtr)
			else
				local doinsert = true
				eff = EFF(effPtr)
				for k,v in pairs(filter) do
					if eff[k] ~= v then
						doinsert = false
						break
					end
				end
				if doinsert then
					table.insert(effList,effPtr)
				end
			end
		end
	end
	if #effList == 0 then return end
	return USE_EFFECT(aux.getRandVal(effList),targetPtr)
end

-- Active effects, filtered if necessary
function aux.activeEffects(persPtr, filter)
	local effList = {}
	local dofilter = (type(filter) == "table")
	for _,effPtr in pairs(EFF_LIST(persPtr)) do
		if EFF_ISACTIVE(effPtr) then
			if not dofilter then
				table.insert(effList,effPtr)
			else
				local doinsert = true
				eff = EFF(effPtr)
				for k,v in pairs(filter) do
					if eff[k] ~= v then
						doinsert = false
						break
					end
				end
				if doinsert then
					table.insert(effList,effPtr)
				end
			end
		end
	end
	return effList
end

-- Drop effects, filtered if necessary
function aux.dropEffects(persPtr, filter)
	local dofilter = (type(filter) == "table")
	for _,effPtr in pairs(EFF_LIST(persPtr)) do
		if EFF_ISACTIVE(effPtr) then
			if dofilter then
				eff = EFF(effPtr)
				for k,v in pairs(filter) do
					if eff[k] ~= v then
						effPtr = nil
						break
					end
				end
			end
			if effPtr ~= nil then
				DROP_EFFECT(effPtr)
			end
		end
	end
end

-- The opposite team number
function aux.oppTeamNum()
	if my.teamNum == 1 then
		return 2
	else
		return 1
	end
end

-- Get filtered personage list
function aux.getPersList(teamNum, alive, except, bot, n)
	local newPersList = {}
	for _,persPtr in pairs(PERS_LIST(teamNum, alive, except)) do
		if (bot == nil) or (bot == PERS_ISBOT(persPtr)) then
			table.insert(newPersList,persPtr)
		end
	end
	if n ~= nil then return aux.getRandVal(newPersList,n) end
	return newPersList
end

function bot_CheckHiLevelPers(oppList, intLevel, bot_HiLevel2LowLevel)
	for _,persPtr in pairs(oppList) do   -- перебираем всех живых союзников
		local pers = PERS(persPtr)  -- массив выбранного противника

		if (pers.level > intLevel) then -- если уровень больше
			local rashod = pers.level - intLevel;
			if(rashod == 1) then
				if (#aux.activeEffects(persPtr,{artId = 5051}) == 0 and #aux.activeEffects(persPtr,{artId = 5052}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5051},persPtr) -- используем понижение
					aux.useEffect({artId = 5052},persPtr) -- используем понижение
				end
			end
			if(rashod == 2) then
				if (#aux.activeEffects(persPtr,{artId = 5049}) == 0 and #aux.activeEffects(persPtr,{artId = 5050}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5049},persPtr) -- используем понижение
					aux.useEffect({artId = 5050},persPtr) -- используем понижение
				end
			end
			if(rashod == 3) then
				if (#aux.activeEffects(persPtr,{artId = 5047}) == 0 and #aux.activeEffects(persPtr,{artId = 5048}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5047},persPtr) -- используем понижение
					aux.useEffect({artId = 5048},persPtr) -- используем понижение
				end
			end
			if(rashod == 4) then
				if (#aux.activeEffects(persPtr,{artId = 5045}) == 0 and #aux.activeEffects(persPtr,{artId = 5046}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5045},persPtr) -- используем понижение
					aux.useEffect({artId = 5046},persPtr) -- используем понижение
				end
			end
			if(rashod == 5) then
				if (#aux.activeEffects(persPtr,{artId = 5043}) == 0 and #aux.activeEffects(persPtr,{artId = 5044}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5043},persPtr) -- используем понижение
					aux.useEffect({artId = 5044},persPtr) -- используем понижение
				end
			end
			if(rashod == 6) then
				if (#aux.activeEffects(persPtr,{artId = 5041}) == 0 and #aux.activeEffects(persPtr,{artId = 5042}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5041},persPtr) -- используем понижение
					aux.useEffect({artId = 5042},persPtr) -- используем понижение
				end
			end
			if(rashod == 7) then
				if (#aux.activeEffects(persPtr,{artId = 5039}) == 0 and #aux.activeEffects(persPtr,{artId = 5040}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5039},persPtr) -- используем понижение
					aux.useEffect({artId = 5040},persPtr) -- используем понижение
				end
			end
			if(rashod == 8) then
				if (#aux.activeEffects(persPtr,{artId = 5037}) == 0 and #aux.activeEffects(persPtr,{artId = 5038}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5037},persPtr) -- используем понижение
					aux.useEffect({artId = 5038},persPtr) -- используем понижение
				end
			end
			if(rashod == 9) then
				if (#aux.activeEffects(persPtr,{artId = 5035}) == 0 and #aux.activeEffects(persPtr,{artId = 5036}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5035},persPtr) -- используем понижение
					aux.useEffect({artId = 5036},persPtr) -- используем понижение
				end
			end
			if(rashod == 10) then
				if (#aux.activeEffects(persPtr,{artId = 5033}) == 0 and #aux.activeEffects(persPtr,{artId = 5034}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5033},persPtr) -- используем понижение
					aux.useEffect({artId = 5034},persPtr) -- используем понижение
				end
			end
			if(rashod == 11) then
				if (#aux.activeEffects(persPtr,{artId = 5031}) == 0 and #aux.activeEffects(persPtr,{artId = 5032}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5031},persPtr) -- используем понижение
					aux.useEffect({artId = 5032},persPtr) -- используем понижение
				end
			end
			if(rashod == 12) then
				if (#aux.activeEffects(persPtr,{artId = 5029}) == 0 and #aux.activeEffects(persPtr,{artId = 5030}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5029},persPtr) -- используем понижение
					aux.useEffect({artId = 5030},persPtr) -- используем понижение
				end
			end
			if(rashod == 13) then
				if (#aux.activeEffects(persPtr,{artId = 5027}) == 0 and #aux.activeEffects(persPtr,{artId = 5028}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5027},persPtr) -- используем понижение
					aux.useEffect({artId = 5028},persPtr) -- используем понижение
				end
			end
			if(rashod == 14) then
				if (#aux.activeEffects(persPtr,{artId = 5025}) == 0 and #aux.activeEffects(persPtr,{artId = 5026}) == 0) then -- Если на противнике нет эффектов
					aux.useEffect({artId = 5025},persPtr) -- используем понижение
					aux.useEffect({artId = 5026},persPtr) -- используем понижение
				end
			end
		end
	end
end

objects = {}
function lookupParam(to, param, id)
local ok, err = pcall(lookupParamX, to, param, id)

if not ok then
	return 0
else
	DEBUG(lookupParamX(to, param, id))
	return lookupParamX(to, param, id)
end

end

function lookupParamX(...)
	local i = 0
	local to
	local param
	local id
	for _, v in ipairs{...} do
	  if(i == 0) then to = v end
	  if(i == 1) then param = v end
	  if(i == 2) then id = v end
	  i = i + 1
    end
	
	if(to == "Pers") then
		if(objects[id][param]) then
			return objects[id][param]
		else
			return 0
		end
	end
	if (to == "Fight") then
		if(_G[param]) then
			return _G[param]
		else
			return 0
		end
	end
end

function abil_MountHeal()

end

-- Write funtion bot_CheckHiLevelPers
