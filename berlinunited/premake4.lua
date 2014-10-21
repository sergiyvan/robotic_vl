--[[
	Premake script to create the makefiles for BerlinUnited
	
	----> "./bin/premake --platform=ABC gmake" <----

--]]


dofile "premake/premake_desktop.lua"
dofile "premake/premake_helper.lua"

-----------------------------------------------------------------------------------------

solution "BerlinUnited"
	configurations {
		"Release",
		"Debug"
	}
	platforms {
		"Native",
		"PC"
	}

	defines "BERLINUNITEDSTANDALONE"
	defines "IMAGEFORMAT_YUV422"
	defines "USE_OPENCV"

	-- global configuration settings
	configuration "Release"
		defines "NDEBUG"
		flags { "OptimizeSpeed" }

	configuration "Debug"
		defines "DEBUG"
		flags { "Symbols" }

	-- platform settings
	configuration "PC"
		targetdir "build/pc"
		defines      { "DESKTOP" }

		buildoptions { "-m" .. os.bitcounter() }

		buildoptions { "-isystem tools/protobuf/include"         }
		linkoptions  { "-Ltools/protobuf/lib" .. os.bitcounter() }

		buildoptions { "-isystem tools/gtest/include"            }
		linkoptions  { "-Ltools/gtest/lib" .. os.bitcounter()    }

		-- armadillo options
		buildoptions {
			  "-isystem tools/armadillo/include"
			, "-isystem tools/"
		}

	-- our project
	project "BerlinUnited"
		kind "ConsoleApp"
		language "c++"

		-- what to compile (and what not)
		files {
			"src/**.h",
			"src/**.cpp",
			"src/**.cc"
		}
		excludes {
			"excluded.file"
		}

		-- include directories
		includedirs {
			"src/",
		}

		-- protobuf generated code and header files are system files
		buildoptions {
			"-isystem src/messages",
		}

		dofile "premake/premake_gcc_flags.lua"

		-- libraries to link against
		links {
			"blas",
			"boost_iostreams",
			"boost_serialization",
			"dl",
			"espeak",
			"lapack",
			"m",
			"opencv_core",
			"opencv_highgui",
			"opencv_imgproc",
			"opencv_features2d",
			"opencv_calib3d",
			"opencv_objdetect",
			"png12",
			"protobuf",
			"pthread",
			"rt",
			"z"
		}

		-- give some indication that we are done
		postbuildcommands { "@echo --------------- Compilation finished at `date` ----------------" }


		------------------------------------------------------------------------------------------------------
		-- Individual configuration settings
		------------------------------------------------------------------------------------------------------

		-- Generic PC configuration
		configuration { "PC" }
			buildoptions { "-ggdb3" }
			links { "gtest" }
