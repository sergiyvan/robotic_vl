
-- small function to find out what kind of processor we have (32 vs 64 bit)
function os.bitcounter()
	local f = assert(io.popen("uname -m", 'r'))
	local s = assert(f:read('*a'))
	f:close()
	if (s == "x86_64\n" or s == "amd64\n") then
		return 64
	else
		return 32
	end
end

