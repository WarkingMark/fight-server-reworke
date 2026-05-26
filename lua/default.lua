-- modifed: igorpauk 2017-18

-- Default bot control function
function default()
	if my.status == FS_PS_ACTIVE then
		ATTACK(math.random(1,3))
	end
end
