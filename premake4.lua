dofile "berlinunited/premake/premake_desktop.lua"
dofile "berlinunited/premake/premake_odroid_x2.lua"
dofile "berlinunited/premake/premake_darwin.lua"
dofile "berlinunited/premake/premake_nao.lua"
dofile "berlinunited/premake/premake_helper.lua"


--------------------------------------------------------------------------------

solution "FUmanoid"

	----------------------------------------------------------------------------
	-- We have release and debug builds for different platforms. This basically
	-- gives us a two-dimension matrix of possible targets.
	----------------------------------------------------------------------------

	configurations {
		"Release",
		"Debug"
	}

	platforms {
		  "Native"  -- we do not use this, instead we have "PC"
		, "PC"      -- for laptop/desktop (the host/build machine)
		, "Robot"   -- for FUmanoid robots (the only true robots :-))
		, "DARwIn"  -- for DARwIn OP robots
	--	, "Nao"     -- for NAO robot
	}


	----------------------------------------------------------------------------
	-- common settings across all projects/platforms/configurations
	----------------------------------------------------------------------------

	-- oh yes
	language "c++"

	-- let the BerlinUnited framework know that we make use of OpenCV
	defines { "USE_OPENCV" }

	-- premake warnings
	flags {
		-- make all warnings to errors (== -Werror)
		  "FatalWarnings"
	}

	-- compiler options
	buildoptions {
		-- enable support for c++11
		  "-std=c++11"

		-- add some include files as system files (we consider the
		-- protobuf generated  code and header files as system files
		-- to prevent gcc to output any warnings about them)
		, "-isystem berlinunited/src/messages"
		, "-isystem berlinunited/tools/armadillo/include"
		, "-isystem src/messages"
		, "-isystem berlinunited/tools"

	}

	dofile "berlinunited/premake/premake_gcc_flags.lua"

	-- include directories
	includedirs {
		  "src/"
		, "berlinunited/src/"
		, "tools/physics/ode/include"
	}


	----------------------------------------------------------------------------
	-- configuration specific settings across all platforms and projects
	----------------------------------------------------------------------------

	-- global configuration settings for RELEASE builds
	configuration "Release"
		-- set the NDEBUG (no debug) flag, this will remove e.g. ASSERT statements
		defines "NDEBUG"

		-- optimize for speed (translates to O3)
		flags { "OptimizeSpeed" }

		-- enable minimal debug information for backtrages for gdb by default
		buildoptions { "-ggdb1" }


	-- global configuration settings for DEBUG builds
	configuration "Debug"
		-- set the DEBUG flag
		defines "DEBUG"

		-- enable symbol inclusion, debug information
		flags { "Symbols" }
		buildoptions { "-ggdb3" }


	----------------------------------------------------------------------------
	-- platform specific settings across all configurations and projects
	----------------------------------------------------------------------------

	-- desktop/PC build (for local testing, not an actual robot)
	configuration "PC"
		targetdir "build/pc"
		defines      { "IMAGEFORMAT_YUV422", "ROBOT2013", "DESKTOP" }
		
		excludes { "src/**/**[Ss]imspark**" }

		-- never build an optimized version for PC (muahahaha)
		buildoptions { "-O0" }

		-- include our own protobuf version
		buildoptions { "-isystem berlinunited/tools/protobuf/include"          }
		linkoptions  { "-Lberlinunited/tools/protobuf/lib" .. os.bitcounter()  }

		-- include gtest (google test)
		buildoptions { "-isystem berlinunited/tools/gtest/include"             }
		linkoptions  { "-Lberlinunited/tools/gtest/lib" .. os.bitcounter()     }

		-- stuff for physics simulator
		linkoptions { "-Ltools/physics/ode/lib" ..os.bitcounter() }

		-- use sfexp for Simspark
		buildoptions { "-isystem berlinunited/tools/sfsexpr/include"}
		linkoptions  { "-Lberlinunited/tools/sfsexpr/lib" .. os.bitcounter()   }

		-- use libb64 for Simspark
		buildoptions { "-isystem berlinunited/tools/libbase64/include"}
		linkoptions  { "-Lberlinunited/tools/libbase64/lib" .. os.bitcounter() }


	-- FUmanoid robot
	configuration "Robot"
		targetdir "build/robot/"

		excludes { "src/tests/**", "berlinunited/src/tests/**", "src/**/**[Ss]imspark**" }

		defines      { "IMAGEFORMAT_YUV422", "ROBOT2013", "ODROID" }

		-- stuff for physics simulator
		linkoptions { "-Ltools/physics/ode/libarm" }

	-- DARwIn OP robot
	configuration "DARwIn"
		targetdir "build/robot/"

		excludes { "src/tests/**", "berlinunited/src/tests/**", "src/**/**[Ss]imspark**" }

		defines      { "IMAGEFORMAT_YUV422", "DARWIN" }
		buildoptions { "-march=atom -mtune=atom" }                  -- set platform


	-- NAO robot
	configuration "Nao"
		targetdir "build/robot/"

		excludes { "src/tests/**", "berlinunited/src/tests/**", "src/**/**[Ss]imspark**" }

		defines      { "IMAGEFORMAT_YUV422", "NAO" }
		buildoptions { "-m32 -march=i686 -msse -msse2 -mssse3" }    -- set platform


	----------------------------------------------------------------------------
	-- BerlinUnited Framework
	----------------------------------------------------------------------------

	project "BerlinUnited"
		kind "SharedLib"

		files {
			  "berlinunited/src/**.h"
			, "berlinunited/src/**.cpp"
			, "berlinunited/src/**.cc"
		}

		excludes {
			  "berlinunited/src/main.cpp"
			, "berlinunited/src/services.cpp"
			, "berlinunited/**/archive/**"
		}

		defines "BERLINUNITED_LIB"


	----------------------------------------------------------------------------
	-- Cognition modules
	----------------------------------------------------------------------------

	project "Cognition"
		kind "SharedLib"

		files {
			  "src/modules/cognition/**.h"
			, "src/modules/cognition/**.cpp"
			, "src/modules/cognition/**.cc"
		}

		excludes {
			  "**/archive/**"
		}


	----------------------------------------------------------------------------
	-- Motion modules
	----------------------------------------------------------------------------

	project "Motion"
		kind "SharedLib"

		files {
			  "src/modules/motion/**.h"
			, "src/modules/motion/**.cpp"
			, "src/modules/motion/**.cc"
		}

		excludes {
			  "**/archive/**"
		}


	----------------------------------------------------------------------------
	-- our project
	----------------------------------------------------------------------------

	project "FUmanoid"
		kind "ConsoleApp"

		-- what to compile (and what not)
		files {
			  "src/**.h"
			, "src/**.cpp"
			, "src/**.cc"
		}

		excludes {
			  "**/archive/**"
			, "src/modules/cognition/**"
			, "src/modules/motion/**"
		}

		-- libraries to link against
		links {
			  "blas"          -- required by armadillo (math)
			, "boost_iostreams"
			, "boost_serialization"
			, "dl"            -- required for backtrace
			, "espeak"        -- for spoken voice
			, "lapack"        -- required by armadillo (math)
			, "m"             -- math stuff
			, "opencv_core"
			, "opencv_highgui"
			, "opencv_imgproc"
			, "opencv_features2d"
			, "opencv_calib3d"
			, "opencv_objdetect"
			, "png"         -- for loading/saving png files
			, "protobuf"      -- protobuf
			, "pthread"       -- required to use threads
			, "rt"
			, "z"             -- compression, required by protobuf
		}
		
		linkoptions {
			-- we use the pthread library
			  "-pthread"

			-- include all symbols (required for backtrace)
			, "-rdynamic"

			-- on PC, add a runtime search path for the linker
			, (_OPTIONS["platform"] == "PC" and "-Wl,-rpath=pc" or "-Wl,-rpath=.")

			-- specify the location of our own shared libraries
			, "-Lbuild/" .. string.lower( _OPTIONS["platform"] )

			-- our own shared libraries
			, "-lBerlinUnited"
			, "-lCognition"
			, "-lMotion"
			, "-lode"          -- open dynamics engine
		}

		-- give some indication that we are done
		postbuildcommands { "@echo --------------- Compilation finished at `date` ----------------" }

		------------------------------------------------------------------------
		-- specific platform settings

		configuration "PC"
			links { "gtest", "sexp", "b64", "drawstuff", "GLU", "GL", "X11" }

		configuration "Robot"
			postbuildcommands { "@if [ \"x${DISPLAY}\" != 'x' ]; then cd $(TARGETDIR) && bash ../../bin/install.sh; fi" }

		configuration "DARwIn"
			postbuildcommands { "@if [ \"x${DISPLAY}\" != 'x' ]; then cd $(TARGETDIR) && bash ../../bin/install.sh; fi" }

