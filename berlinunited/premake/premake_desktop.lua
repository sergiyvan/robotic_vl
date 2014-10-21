dofile "platform.lua"

-- use various performance enhancement tools
local PREFIX = ""
if (os.getenv("USE_CCACHE") == "1") then
	PREFIX = "ccache "
	if (os.getenv("USE_DISTCC") == "1") then
		PREFIX = "CCACHE_PREFIX=distcc ccache "
	end
elseif (os.getenv("USE_DISTCC") == "1") then
	PREFIX = "distcc "
end

if (os.getenv("USE_DISTCC") == "1") then
	COMPILERSUFFIX="-4.7"
else
	COMPILERSUFFIX=""
end

-- define new platform and compiler
newplatform {
    name = "PC",
    description = "PC build",
    gcc = {
        cc = PREFIX .. "gcc" .. COMPILERSUFFIX,
        cxx = PREFIX .. "g++" .. COMPILERSUFFIX,
        cppflags = " -D_GLIBCXX_USE_SCHED_YIELD -D_GLIBCXX_USE_NANOSLEEP"
    }
}

