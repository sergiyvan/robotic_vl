dofile "platform.lua"

local crossDir = "/opt/fumanoids-nao/compiler/i686-fumanoids-linux-gnu"
local stageDir = "/opt/fumanoids-nao/staging"

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
    name = "Nao",
    description = "Robot build for NAO",
    gcc = {
        cc  = PREFIX .. crossDir .. "/bin/i686-fumanoids-linux-gnu-gcc",
        cxx = PREFIX .. crossDir .. "/bin/i686-fumanoids-linux-gnu-g++",
        ar  = PREFIX .. crossDir .. "/bin/i686-fumanoids-linux-gnu-ar",
        cppflags = " --sysroot=" .. crossDir .. "/i686-fumanoids-linux-gnu/sysroot/" ..
                   " -isystem " .. crossDir .. "/i686-fumanoids-linux-gnu/sysroot/usr/include/" .. 
                   " -isystem " .. crossDir .. "/i686-fumanoids-linux-gnu/include/c++/4.7.1/" .. 
                   " -I " .. stageDir .. "/usr/include/" ..
                   " -D_GLIBCXX_USE_SCHED_YIELD -D_GLIBCXX_USE_NANOSLEEP",
        ldflags  = " -L " .. stageDir .. "/usr/lib" ..
                   " -L " .. stageDir .. "/lib" ..
                   " -L " .. crossDir .. "/i686-fumanoids-linux-gnu/sysroot/usr/lib/"  ..
                   " -lgfortran -lportaudio"
    }
}
