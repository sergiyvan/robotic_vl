----------------------------------------------------------------------------
-- The recommended GCC flags you should use.
----------------------------------------------------------------------------

if BERLINUTED_GCC_FLAGS_SET ~= 1 then
	BERLINUTED_GCC_FLAGS_SET = 1

	language "c++"

	-- premake warnings
	flags {
		-- make all warnings to errors (== -Werror)
		  "FatalWarnings"
	}

		                

	-- compiler options
	buildoptions {
		-- enable support for c++11
		  "-std=c++11"

		-- Documentation for warnings can be found online at
		-- https://gcc.gnu.org/onlinedocs/gcc-4.8.3/gcc/Warning-Options.html#Warning-Options

		-- This option causes the compiler to abort compilation on the first
		-- error occurred rather than trying to keep going and printing further
		-- error messages.
		-- This should avoid confusion of C++-newbies because they only get
		-- one error msg. They won't try to fix a problem which does not exist.
		,  "-Wfatal-errors"

		-- enable "all" warnings ("all" == "warnings about constructions that some users consider questionable")
		,  "-Wall"

		-- enable (some) extra warnings (still not all)
		, "-Wextra"

		-- be really pedantic
--		, "-Wpedantic"

		-- warn about anything that depends on the “size of” a function type or of void
		, "-Wpointer-arith"

		-- warn whenever a pointer is cast so as to remove a type qualifier from the target type
		, "-Wcast-qual"

		-- warn against hiding a base class's virtual function
		, "-Woverloaded-virtual"

		-- warn if a precompiled header is found in the search path but can't be used
		, "-Winvalid-pch"

		-- don't use C-style casts
--		, "-Wold-style-cast"

		-- warn about violations of some style guidelines from Scott Meyers' Effective C++ book
--		, "-Weffc++"

		-- Warn also about the use of an uncasted NULL as sentinel
		, "-Wstrict-null-sentinel"

		-- Warn when overload resolution chooses a promotion from unsigned or
		-- enumerated type to a signed type, over a conversion to an unsigned
		-- type of the same size. Previous versions of G++ would try to pre-
		-- serve unsignedness, but the standard mandates the current behavior.
		, "-Wsign-promo"

		-- warn for implicit conversions that may change the sign of an integer value,
		-- like assigning a signed integer expression to an unsigned integer variable
--		, "-Wsign-conversion"

		-- warn about suspicious uses of logical operators in expressions. This
		-- includes using logical operators in contexts where a bit-wise operator
		-- is likely to be expected.
		, "-Wlogical-op"

		-- also disable some warnings that are enabled by Wall/extra/default
		, "-Wno-long-long"                    -- don't complain about long long
		, "-Wno-variadic-macros"              -- we like variadic macros
		, "-Wno-multichar"                    -- we use some multichars
		, "-Wno-unused-parameter"             -- 
		, "-Wno-missing-field-initializers"   -- allow implicit zero initialization

		-- create user header file dependencies file (.d)
		, "-MMD -MP -fpch-deps"
	}


	linkoptions {
		-- we use the pthread library
		  "-pthread"

		-- include all symbols (required for backtrace)
		, "-rdynamic"

		-- on PC, add a runtime search path for the linker
		, (_OPTIONS["platform"] == "PC" and "-Wl,-rpath=pc" or "-Wl,-rpath=.")
	}
end
