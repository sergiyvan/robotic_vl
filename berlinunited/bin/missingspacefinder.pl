#!/usr/bin/perl

BEGIN {
	eval 'use File::Find::Rule; 1'
		or die "Error: File::Find::Rule not installed. If on Ubuntu, do \"sudo aptitude install libfile-find-rule-perl\". Error: $@";
}

use Cwd;

sub buildFileIndex {
	$directory = $_[0];

	# Conditions
	my $excludeDirs  = File::Find::Rule->directory
	                     ->name('math', 'XabslEngine', 'messages', 'ModuleFramework')
	                     ->prune
	                     ->discard;
	my $includeFiles = File::Find::Rule->file
	                     ->name('*.h', '*.cpp');

	my @files = File::Find::Rule->or($excludeDirs, $includeFiles)
	              ->in($directory);

	return @files;
}

sub checkSpaces {
	$filename = $_[0];

	open FILE, $filename or die "Error opening $filename\n";

	$lc = 0;
	while ($line=<FILE>) {
		++$lc;

		# remove strings and comments
		$line =~ s/\"[^\"]*"/\"\"/g;
		$line =~ s/^\s*\*.*$//g;
		$line =~ s/\/\/.*$//g;
		$line =~ s/\/\*.*$//g;

		# Opening braces ({) need to be prefaced with a whitespace,
		# i.e. instead of "if (true){" we want "if (true) {".
		if ($line =~ /(\)|\w)\{/) {
			print "$filename:$lc - Missing space before opening brace: $line";
		}

		# After keywords (NOT function names) we want a space before the
		# opening parentheses. I.e. instead of "if(true)" we want "if (true)"
		if ($line =~ /(for\(|if\(|while\(|do\{)/) {
			print "$filename:$lc - Missing space after keyword: $line";
		}

		# We want a space after a comma, i.e. instead of "min(a,b)" we want
		# to have "min(a, b)"
		if ($line =~ /,\S/) {
			print "$filename:$lc - Missing space after comma: $line";
		}

		# Comparison or logical operators should use spaces BEFORE
		if ($line =~ /\w(<=|=>|==|!=|\&\&|\|\|)/) {
			print "$filename:$lc - Missing space before comparison or logical operator: $line";
		}

		# Comparison or logical operators should use spaces AFTER
		if ($line =~ /(<=|=>|==|!=|\&\&|\|\|)\w/) {
			print "$filename:$lc - Missing space after comparison or logical operator: $line";
		}
	}

	close FILE;
}

my $cwd = getcwd();
@files = buildFileIndex("$cwd/src");

for $file (@files) {
	checkSpaces($file)
}

