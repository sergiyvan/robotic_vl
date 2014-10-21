#!/usr/bin/perl
#
#############################################################################
# Ideas:
#  - Convert C-enums to XABSL enums automatically
#
#############################################################################
#
# XABSL symbol creator
# ********************
#
# Go through all source files and look for special markers to generate
# xabsl symbols automatically. This allows to define XABSL symbols in
# one (and only one) place, different to the manual approach which
# requires the symbols to be registered and implemented in C++, and
# registered again in XABSL.
#
# Usage
# *****
#
#   createXabslSymbols.pl <src> <moduleSource> <moduleHeader> <xabsl>
#
# where
#   <src>          is the source directory
#   <moduleSource> is the name of the module source file to be created,
#                  relative to <src>, that will include the symbol
#                  registrations and code
#   <moduleHeader> is the name of the module header file to be created,
#                  relative to <src>, that will include the module header
#   <xabsl>        is the name of the the XABSL file to be created
#
# When run without any arguments, default values will be used.
#
# How To
# ******
#
# At appropriate places in the source (recommended are the representations),
# you can define enums and symbols. For syntax details, check the macros and
# documentation in src/tools/xabslSymbolRegistration.h or look at already
# defined symbols.
#
# For documentation purposes, symbols can have a one-line comment DIRECTLY
# before the definition using /*: to start the comment and */ to finish it.

#############################################################################

BEGIN {
	eval 'use File::Find::Rule; 1'
		or die "Error: File::Find::Rule not installed. If on Ubuntu, do \"sudo aptitude install libfile-find-rule-perl\". Error: $@";
}

BEGIN {
	eval 'use List::MoreUtils qw/ uniq /; 1'
		or die "Error: List::MoreUtils not installed. If on Ubuntu, do \"sudo aptitude install liblist-moreutils-perl\". Error: $@";
}

use Cwd;

use warnings;


# List of hashes for required and provided representations. Each entry
# has values "name" for the name of the representation, and "file" for
# the source code file (including path)
my @requiredRepresentations;
my @providedRepresentations;

# List of hashes for symbols. Each entry has the following hash values:
# {name}      Name of the symbol
# {type}      Value type of the symbol (Decimal, Boolean, Enum)
# {function}  Name of the function (RepresentationNameSymbolName)
# {unit}      Measure unit (for documentation)
# {params}    Array of parameter hashes
#             {name}  name of parameter
#             {vtype} value type (Decimal, Boolean)
#             {ptype} parameter type (float, bool)
#             {unit}  Measure unit (for documentation)
#             {range} Range (for documentation)
# {codeGet}   Source code for getting the symbol
# {codeSet}   Source code for setting the symbol
# {comment}   Optional comment
my @symbols;

# List of enums
# {name}      Name of enum
# {values}    values of enum
#             {name}  name of value
#             {value} value
my @enums;

# list of include files
my @includes;

# user params
my $workingDir;
my $moduleSource;
my $moduleHeader;
my $xabsl;
my $baseClassHeader = "modules/cognition/behaviorLayer/xabsl/Symbols/SymbolsABC.h";
my $className       = "XabslSymbols";


#############################################################################
# Create list of files
#############################################################################

sub buildFileIndex {
	# Conditions
	my $excludeDirs  = File::Find::Rule->directory
	                     ->name('math', 'XabslEngine', 'messages', 'ModuleFramework')
	                     ->prune
	                     ->discard;
	my $includeFiles = File::Find::Rule->file
	                     ->name('*.h', '*.cpp');

	# scan current directory for files per the above rules
	my @files = File::Find::Rule->or($excludeDirs, $includeFiles)
	              ->in('.');

	return @files;
}


#############################################################################
# Given a XABSL symbol name, create a valid C function name
#############################################################################

sub makeFunctionName {
	$name = $_[0];

	# remove dots in the name and capitalise the following letter
	$name =~ s/\.([a-z]?)/uc($1)/ge;

	# also capitalize first letter
	$name =~ s/^(.)/uc($1)/ge;

	return $name;
}


#############################################################################
# Search symbol and remove from global symbol list
#############################################################################

sub getSymbol {
	$symbolName = $_[0];

	my $index = 0;
	foreach $symbol (@symbols) {
		if ($symbol && $symbol->{name} eq $symbolName) {
			splice(@symbols, $index, 1); # remove symbol
			return $symbol;
		}
		++$index;
	}

	return undef;
}


#############################################################################
# Scan a file for XABSL definitions
#############################################################################

sub scanForXabslSymbols {
	$filename  = $_[0];

	my $comment = "";

	my $representation;
	$representation->{file} = $filename;
	$representation->{name} = undef;

	open FILE, $filename or die "Error opening $filename\n";

	$lc = 0;
	while ($line=<FILE>) {
		++$lc;

		next if ($line =~ /^#define/);
		next if ($line =~ /^\/\//);

		# remember which class we are in, as this will be used
		# for the module requirements
		if ($line =~ /^class (\w*)/) {
			$representation->{name} = $1;
			next;
		}

		# allow overwriting of class
		elsif ($line =~ /XabslNoClass/) {
			$representation->{name} = undef;
			next;
		}

		# some required representations must be set explicitely
		#                              1 Name   2 Include
		elsif ($line =~ /XabslRequire\s*\((\w*),\s*(\S*)\)/) {
			my $name    = $1;
			my $include = $2;
			$include =~ s/"//g;

			# only add if not yet in list
			if (! grep { $_->{name} eq $name } @requiredRepresentations) {
				my $representation = { "name" => $name, "file" => $include };
				push @requiredRepresentations, $representation;
			}
		}

		# some provided representations must be set explicitely
		#                              1 Name   2 Include
		elsif ($line =~ /XabslProvide\s*\((\w*),\s*(\S*)\)/) {
			my $name    = $1;
			my $include = $2;
			$include =~ s/"//g;

			# only add if not yet in list
			if (! grep { $_->{name} eq $name } @providedRepresentations) {
				my $representation = { "name" => $name, "file" => $include };
				push @providedRepresentations, $representation;
			}
		}

		# some includes must be defined manually
		#                                    1 include
		elsif ($line =~ /XabslInclude\s*\(\s*\"?(\S*)\"?\s*\)/) {
			my $include = $1;
			$include =~ s/"//g;

			push @includes, $include;
		}

		# enum definition
		elsif ($line =~ /\s*XabslEnum\s*\(\s*(\w*)\s*,\s*{\s*$/) {
			my $enumName = $1;
			my $enum;
			$enum->{name}   = $enumName;
			$enum->{source} = "$filename:$lc";

			my @enumValues;
			while ($line=<FILE>) {
				++$lc;
				if ($line =~ m/}/) {
					last;
				}

				$line =~ s/\s//g; # remove whitespaces
				$line =~ s/,//g;  # remove comma
				($name, $value) = split("=", $line);
				my $enumValue;
				$enumValue->{name}  = $name;
				$enumValue->{value} = $value;
				push @enumValues, $enumValue;
			}

			$enum->{values} = [ @enumValues ];
			push @enums, $enum;
		}

		# symbol extraction
		#                            1                2                 3           4              5           6
		#                            Action      |    Name         |    Type     |  Unit         |   Params    | Code
		elsif ($line =~ /XabslSymbol(Get|Set)\s*\(\s*"([\.\w]*)"\s*,\s*([\w]*)\s*,\s*"([^"]*)"\s*,\s*([^{]*)?\s*{?(.*)$/) {
			# copy extracted information
			my $symbolAction = $1;
			my $symbolName   = $2;
			my $symbolType   = $3;
			my $symbolUnit   = $4;
			my $symbolParams = $5;
			my $symbolCode   = $6;
			my $enumType     = undef;

			# handle enum return type
			if ($symbolType !~ m/(Decimal|Boolean)/) {
				# type is an enum, so remember it
				$enumType = $symbolType;

				# remove [] from enum name
				$enumType =~ s/[\[\]]//g;

				# mark return type as enum
				$symbolType = "Enum";
			}


			# remove [""] from unit name (if any)
			$symbolUnit =~ s/(\["|"\])//g if $symbolUnit;

			# check if symbol already exists (get/set), if not create new symbol
			my $symbol = getSymbol($symbolName);
			if ($symbol) {
				# symbols may not change their type
				if ($symbol->{type} ne $symbolType) {
					die "SymbolSet/Get type mismatch for $symbolName in $filename:$lc\n";
				}
				# only one Set and one Get may be specified
				if (   ($symbolAction eq "Get" && $symbol->{codeGet})
				    || ($symbolAction eq "Set" && $symbol->{codeSet}))
				{
					die "Duplicate definition of SymbolSet or Get for $symbolName in $filename:$lc\n";
				}
				# parameter lists are not allowed if there is both a Set and a Get
				if ($symbol->{params}) {
					die "$symbolName is an Output symbol but the Get definition uses parameters, ref " . $symbol->{source} . "\n";
				}
			} else {
				# before we can define a SymbolSet, the Get must have been defined
				if ($symbolAction eq "Set") {
					die "SymbolGet:$symbolName required before SymbolSet:$symbolName in $filename:$lc\n";
				}
				$symbol->{source}   = "$filename:$lc";
				$symbol->{name}     = $symbolName;
				$symbol->{type}     = $symbolType;
				$symbol->{function} = makeFunctionName($symbolName);
				$symbol->{unit}     = $symbolUnit;
				$symbol->{params}   = ();
				$symbol->{enum}     = $enumType;
				$symbol->{comment}  = $comment;
			}

			# code may not finish on this line - continue parsing until the number of
			# opening braces matches the number of closing braces.
			my $code = "{" . $symbolCode . "\n";
			while (($code =~ tr/{//) != ($code =~ tr/}//) && ($line=<FILE>)) {
				++$lc;
				$code = $code . $line;
			}

			# the last closing curly brace will most likely have a parentheses at the end, remove it and anything after it (e.g. semicolons)
			$code =~ s/}[^}]*$/}/;

			# remove block comment sequences, as they could not possibly work anyway
			$code =~ s#(\s*\*/|\s*/\*)##g;

			# store code in correct slot
			if ($symbolAction eq "Get") {
				$symbol->{codeGet} = $code;
			} else {
				$symbol->{codeSet} = $code;
			}

			# clean params
			$symbolParams =~ s/,[^,]*$//;
			# split parameters
			my $paramStr = $symbolParams;
			if ($paramStr) {
				if ($symbolAction eq "Set") {
					die "Set function for $symbolName can not have parameters in $filename:$lc\n";
				}

				$paramStr =~ s/[\(\)]//g; # remove parentheses

				my @paramList = split(",", $paramStr);
				my @params;
				foreach $paramEntry (@paramList) {
					my $param;

					($paramType, $paramName, $paramUnit, $paramRange) = split(" ", $paramEntry);

					$param->{unit}  = $paramUnit;
					$param->{range} = $paramRange;

					if ($paramType eq "double") {
						$paramTypeXabsl = "Decimal";
					} elsif ($paramType eq "bool") {
						$paramTypeXabsl = "Boolean";
					} else {
						die "Unknown parameter type $paramType in $filename:$lc\n";
					}

					$param->{vtype} = $paramTypeXabsl;
					$param->{ptype} = $paramType;
					$param->{name}  = $paramName;
					push @params, $param;
				}
				$symbol->{params}   = [ @params ];
			}

			# remember representation
			if ($representation->{name}) {
				if ($symbolAction eq "Get") {
					# only add if not yet in list
					if (! grep { $_->{name} eq $representation->{name} } @requiredRepresentations) {
						my $reqRepresentation = { "name" => $representation->{name}, "file" => $representation->{file} };
						push(@requiredRepresentations, $reqRepresentation);
					}
				} else { # Set
					# only add if not yet in list
					if (! grep { $_->{name} eq $representation->{name} } @providedRepresentations) {
						push(@providedRepresentations, $representation);
					}
				}
			}

			push @symbols, $symbol;

		}

		elsif ($line =~ /XabslSymbol(Get|Set)/) {
			die("Unmatched XabslSymbol$1 operation at $filename:$lc\n");
		}

		# one-line XABSL comment
		elsif ($line =~ /^\s*\/\*:(.*)\*\//) {
			$comment = $1;
			$inComment = 0;
		}

		# multi-line comment start (not sure XABSL allows it, at least it won't display prettily)
		elsif ($line =~ /^\s*\/\*:(.*)$/) {
			$comment = $1;
			$inComment = 1;
		}

		# multi-line comment end
		elsif ($line =~ /^(.*)\*\// && $inComment == 1) {
			$comment .= $1;
			$inComment = 0;
		}

		# in multi-line comment
		elsif ($inComment) {
			$comment .= $line;
		}

		# everything else
		else {
			# clear comment
			$comment = "";
			$inComment = 0;
		}
	}

	close FILE;
}


#############################################################################
# Print values for debugging purposes.
#############################################################################

sub printData {
	foreach $enum (@enums) {
		print $enum->{source} . "\n";
		print "--> " . $enum->{name} . "\n";
		foreach $enumValue (@{$enum->{values}}) {
			print "\t" . $enumValue->{name} . ' = ' . $enumValue->{value} . "\n";
		}
	}
}


#############################################################################
# Print warning in comment.
#############################################################################

sub printWarning {
	$file = $_[0];
	print $file "/* DO NOT EDIT THIS FILE                                        */\n";
	print $file "/* This file was created automatically and will be overwritten. */\n";
	print $file "/* To regenerate, run bin/createXabslSymbols.pl                 */\n";
	print $file "\n";
}


#############################################################################
# Output symbol module (Source)
#############################################################################

sub outputSymbolModuleSource {
	open FILE, ">" . $_[0] or die "Could not open file $_[0]\n";
	printWarning(FILE);

	print FILE '#include "' . $moduleHeader . '"' . "\n";
	print FILE "\n";
	print FILE "$className* " . $className . "::theInstance = NULL;\n";
	print FILE "\n";
	print FILE "void " . $className . "::registerSymbols(xabsl::Engine& engine) {\n";

	# register enums
	foreach $enum (@enums) {
		print FILE "	// " . $enum->{source} . "\n";
		foreach $enumValue (@{$enum->{values}}) {
			print FILE '	engine.registerEnumElement("' . $enum->{name} . '", ' .
			           '"' . $enum->{name} . "." . $enumValue->{name} . '", ' . $enumValue->{value} . ");\n";
		}
		print FILE "\n";
	}

	# register symbols
	foreach $symbol (@symbols) {
		my $direction = "Input";
		if ($symbol->{codeSet}) {
			$direction = "Output";
		}

		print FILE "	// " . $symbol->{source} . "\n";
		if ($symbol->{type} eq "Enum") {;
			print FILE '	engine.registerEnumerated' . $direction . 'Symbol(' .
			           '"' . $symbol->{name} . '", ' .
			           '"' . $symbol->{enum} . '", ';
		} else {
			print FILE '	engine.register' . $symbol->{type} . $direction . 'Symbol(' .
			           '"' . $symbol->{name} . '", ';
		}
		if ($direction eq "Output") {
			print FILE '&set' . $symbol->{function} . ", ";
		}
		print FILE '&get' . $symbol->{function} . ');' . "\n";
		foreach $param (@{$symbol->{params}}) {
			print FILE '	engine.register' . $symbol->{type} . $direction . 'Symbol' .
			      $param->{vtype} . 'Parameter(' .
			      '"' . $symbol->{name} . '", ' .
			      '"' . $symbol->{name} . "." . $param->{name} . '", ' .
			      ' &' . $symbol->{function} . "_" . $param->{name} . ");\n";
		}

		print FILE "\n";
	}

	print FILE "}\n\n";

	# symbol
	foreach $symbol (@symbols) {
		print FILE "// " . $symbol->{source} . "\n";

		foreach $param (@{$symbol->{params}}) {
			print FILE $param->{ptype} . " " . $className . "::" . $symbol->{function} . "_" . $param->{name} . ";\n";
		}

		print FILE "double " if ($symbol->{type} eq "Decimal");
		print FILE "bool "   if ($symbol->{type} eq "Boolean");
		print FILE "int "    if ($symbol->{type} eq "Enum");

		print FILE $className . "::get" . $symbol->{function} . "()\n";
		print FILE $symbol->{codeGet};
		print FILE "\n";

		if ($symbol->{codeSet}) {
			print FILE "void " . $className . "::set" . $symbol->{function} . "(";
			print FILE "double " if ($symbol->{type} eq "Decimal");
			print FILE "bool "   if ($symbol->{type} eq "Boolean");
			print FILE "int "    if ($symbol->{type} eq "Enum");
			print FILE "value) ";
			print FILE $symbol->{codeSet};
			print FILE "\n";
		}

		print FILE "\n";
	}
}


#############################################################################
# Output symbol module (Header)
#############################################################################

sub outputSymbolModuleHeader {
	open FILE, ">" . $_[0] or die "Could not open file $_[0]\n";
	printWarning(FILE);

	print FILE "#ifndef XABSLSYMBOLS_H_\n";
	print FILE "#define XABSLSYMBOLS_H_\n";
	print FILE "\n";
	print FILE '#include "ModuleFramework/Module.h"' . "\n";
	print FILE '#include "' . $baseClassHeader . '"' . "\n";
	print FILE "\n";

	# print necessary include statements
	foreach $include (@includes) {
		print FILE '#include "' . $include . '"' . "\n";
	}
	print FILE "\n";

	# create module definition
	print FILE "BEGIN_DECLARE_MODULE($className)\n";
	foreach $representation (@requiredRepresentations) {
		print FILE '	REQUIRE(' . $representation->{name} . ")\n";
	}
	foreach $representation (@providedRepresentations) {
		print FILE '	PROVIDE(' . $representation->{name} . ")\n";
	}
	print FILE "END_DECLARE_MODULE($className)\n";
	print FILE "\n";

	# create class
	print FILE "class $className : public " . $className . "Base, public SymbolsABC {\n";
	print FILE "protected:\n";
	print FILE "	static $className* theInstance;\n";
	print FILE "public:\n";
	print FILE "	$className() {\n";
	print FILE "		theInstance = this;\n";
	print FILE "	}\n";
	print FILE "	virtual ~$className() {}\n";
	print FILE "\n";
	print FILE "    virtual void init() {}\n";
	print FILE "	virtual void execute() {}\n";
	print FILE "	virtual void registerSymbols(xabsl::Engine& engine);\n";
	print FILE "private:\n";

	foreach $symbol (@symbols) {
		print FILE "	// " . $symbol->{source} . "\n";
		print FILE "	static ";
		print FILE "double " if ($symbol->{type} eq "Decimal");
		print FILE "bool "   if ($symbol->{type} eq "Boolean");
		print FILE "int "    if ($symbol->{type} eq "Enum");
		print FILE "get" . $symbol->{function} . "();\n";

		if ($symbol->{codeSet}) {
			print FILE "	static void set" . $symbol->{function} . "(";
			print FILE "double " if ($symbol->{type} eq "Decimal");
			print FILE "bool "   if ($symbol->{type} eq "Boolean");
			print FILE "int "   if ($symbol->{type} eq "Enum");
			print FILE "value);\n";
		}

		foreach $param (@{$symbol->{params}}) {
			print FILE "	static " . $param->{ptype} . " " . $symbol->{function} . "_" . $param->{name} . ";\n";
		}
		print FILE "\n";
	}

	print FILE "};\n";
	print FILE "#endif\n";
}


#############################################################################
# Output XABSL file
#############################################################################

sub outputXabsl {
	open FILE, ">" . $_[0] or die "Could not open file $_[0]\n";

	printWarning(FILE);
	print FILE 'namespace XABSLSymbols("XABSL Symbols") {' . "\n";

	# register enums
	foreach $enum (@enums) {
		print FILE "	// " . $enum->{source} . "\n";
		print FILE "	enum " . $enum->{name} . " {\n";
		my $count = 0;
		foreach $enumValue (@{$enum->{values}}) {
			print FILE "		";
			if ($count++ > 0) {
				print FILE ", ";
			} else {
				print FILE "  ";
			}
			print FILE $enumValue->{name} . "\n";
		}
		print FILE "	};\n";
	}

	foreach $symbol (@symbols) {
		if ($symbol->{comment} eq "") {
			print FILE "	/** Symbol defined at " . $symbol->{source} . " */\n";
		} else {
			print FILE "	/** " . $symbol->{comment} . " (Symbol defined at " . $symbol->{source} . ") */\n";
		}
		print FILE "	float "  if ($symbol->{type} eq "Decimal");
		print FILE "	bool "   if ($symbol->{type} eq "Boolean");
		print FILE "	enum " . $symbol->{enum} . " " if ($symbol->{type} eq "Enum");

		my $direction = "input";
		if ($symbol->{codeSet}) {
			$direction = "output";
		}

		print FILE "$direction " . $symbol->{name};
		print FILE ' "' . $symbol->{unit} . '"' if $symbol->{unit};
		if ($#{$symbol->{params}}+1 > 0) {
			print FILE "(\n";
			foreach $param (@{$symbol->{params}}) {
				print FILE "		";
				print FILE "float "  if ($param->{ptype} eq "double");
				print FILE "bool "   if ($param->{ptype} eq "Boolean");
				print FILE $param->{name};
				print FILE " " . $param->{range} if $param->{range};
				print FILE ' ' . $param->{unit}  if $param->{unit};
				print FILE ";\n";
			}
			print FILE "	);\n"
		} else {
			print FILE ";\n";
		}
	}

	print FILE "}\n";
}


#############################################################################
# Main
#############################################################################

if ($#ARGV+1 == 0) {
	push @ARGV, "src";
	push @ARGV, "modules/cognition/behaviorLayer/xabsl/Symbols/symbols.cpp";
	push @ARGV, "modules/cognition/behaviorLayer/xabsl/Symbols/symbols.h";
	push @ARGV, "modules/cognition/behaviorLayer/xabsl/Symbols/symbols.xabsl";
} elsif ($#ARGV+1 != 4) {
	print "\nUsage: createXabslSymbols.pl <srcdirectory> <outputModuleSource> <outputModuleHeader> <outputXabsl>\n";
	exit;
}

# remember arguments
$workingDir   = $ARGV[0];
$moduleSource = $ARGV[1];
$moduleHeader = $ARGV[2];
$xabsl        = $ARGV[3];

# change current directory to the working directory path
chdir($workingDir);

# remove current output files (this is necessary as in case of syntax errors we may have
# accidentially created output files that still contain marker code and would thus be
# parsed again in the next, i.e. this, run)
unlink $moduleSource;
unlink $moduleHeader;

# create file index
@files = buildFileIndex();

# scan each file found
for $file (@files) {
	scanForXabslSymbols($file) if $file !~ /xabslSymbolRegistration.h/;
}

# collect include files and unify
foreach $representation (@requiredRepresentations, @providedRepresentations) {
	push @includes, $representation->{file};
}
@includes = uniq @includes;

# remove duplicate representations
@requiredRepresentations = uniq @requiredRepresentations;
@providedRepresentations = uniq @providedRepresentations;

# remove representations from REQUIRED if they are PROVIDED
my %isProvided = map {$_->{name} => 1} @providedRepresentations;
@requiredRepresentations = grep{ not $isProvided{$_->{name}} } @requiredRepresentations;

# sort symbols by source, in order to have a reproducable result
@symbols = sort { $a->{source} cmp $b->{source} } @symbols;
@enums = sort { $a->{source} cmp $b->{source} } @enums;

# output symbol files
outputSymbolModuleHeader($moduleHeader);
outputSymbolModuleSource($moduleSource);
outputXabsl($xabsl)

