dofile "platform.lua"

local compilerDir = "/opt/igepv2/compiler/"
local stageDir    = "/opt/igepv2/sdk/"

local includeDir = stageDir .. "/usr/include"

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

-- define new platform and compiler
newplatform {
	name = "Robot",
	description = "IGEPv2 build",
	gcc = {
		-- compilation is based on the Ubuntu ARM cross toolchain (4.7) or similar
		cc  = PREFIX .. compilerDir .. "/bin/arm-linux-gnueabihf-gcc",
		cxx = PREFIX .. compilerDir .. "/bin/arm-linux-gnueabihf-g++",
		ar  = PREFIX .. compilerDir .. "/bin/arm-linux-gnueabihf-ar",
		cppflags = "-isystem " .. includeDir .. " -isystem" .. includeDir .. "/arm-linux-gnueabihf -Wno-psabi -D_GLIBCXX_USE_SCHED_YIELD -D_GLIBCXX_USE_NANOSLEEP",
		ldflags  = " -L " .. stageDir ..
				   " -L " .. stageDir .. "/lib" ..
				   " -L " .. stageDir .. "/lib/pulseaudio" ..
				   " -L " .. stageDir .. "/lib/arm-linux-gnueabihf" ..
				   " -L " .. stageDir .. "/usr/lib" ..
				   " -L " .. stageDir .. "/usr/lib/arm-linux-gnueabihf" ..
				   " -Wl,-rpath-link=" .. stageDir .. "/lib" ..
				   " -Wl,-rpath-link=" .. stageDir .. "/lib/pulseaudio" ..
				   " -Wl,-rpath-link=" .. stageDir .. "/lib/arm-linux-gnueabihf" ..
				   " -Wl,-rpath-link=" .. stageDir .. "/usr/lib" ..
				   " -Wl,-rpath-link=" .. stageDir .. "/usr/lib/arm-linux-gnueabihf"
	}
}

