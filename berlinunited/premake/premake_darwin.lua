dofile "platform.lua"

local compilerDir = "/opt/darwin/compiler/"
local stageDir = "/opt/darwin/sdk/"

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
    name = "DARwIn",
    description = "Darwin Robot build",
    gcc = {
        cc = PREFIX .. compilerDir .. "/bin/i586-fumanoids-linux-gnu-gcc",
        cxx = PREFIX .. compilerDir .. "/bin/i586-fumanoids-linux-gnu-g++",
        ar = PREFIX .. compilerDir .. "/bin/i586-fumanoids-linux-gnu-ar",
        cppflags = "-isystem " .. includeDir .. " -isystem " .. includeDir .. "/i386-linux-gnu/" .. " -Wno-psabi",
        ldflags  = " -L " .. stageDir .. "/usr/lib" .. " -L " .. stageDir .. "/lib" .. " -Wl,-rpath-link=" .. stageDir .. "/usr/lib" .. " -L " .. stageDir .. "/usr/lib/i386-linux-gnu" .. " -Wl,-rpath-link=" .. stageDir .. "/usr/lib/i386-linux-gnu" .. " -L " .. stageDir .. "/lib/i386-linux-gnu" .. " -Wl,-rpath-link=" .. stageDir .. "/lib/i386-linux-gnu"
    }
}

