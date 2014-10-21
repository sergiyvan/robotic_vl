#!/usr/bin/perl

# Author: S.G.G.
# eMail: gene@staubsaal.de

use strict;
use warnings;

use Data::Dumper;
use Storable 'dclone';
use Getopt::Long;

my $emptymodule = { "REP" => { "REQUIRE" => [], "PROVIDE" => [], "RECYCLE" => []},
					"NAME" => "", 'LAYER' => "UNKNOWN",	"DISPLAY" => "0", "ENABLED" => "", "SHADOW" => []};
my $emptyrepresentation = { "NAME" => "", "DISPLAY" => 0, "REP" => { "REQUIRE" => [], "PROVIDE" => [], "RECYCLE" => []} };
my %swap_rep = ("REQUIRE" => "PROVIDE", "PROVIDE" => "REQUIRE", "RECYCLE" => "RECYCLE" );

sub analyseCode
{
	my $module_map = shift();
	my $rep_map = shift();
	# Search all .h and .cpp for data
	my $currentModule = "";
	open (FILES, 'find . | sort | grep -E "^.*\.(h|cpp)\$" | xargs cat |');
	while(<FILES>)
	{
		if (/^BEGIN_DECLARE_MODULE\((.*)\)$/)
		{
			my $module = $1;
			$module_map->{$module} = dclone $emptymodule unless exists($module_map->{$module});
			$module_map->{$module}{'NAME'} = $module;

			while(defined($_) and $_ = <FILES>)
			{
				if	(/^\s*(REQUIRE|PROVIDE|RECYCLE)S?\((.*)\)/)
				{
					$rep_map->{$2} = dclone $emptyrepresentation unless exists($rep_map->{$2});
					$rep_map->{$2}{'NAME'} = $2;

					push($module_map->{$module}{'REP'}{$1}, $rep_map->{$2});
					push($rep_map->{$2}{'REP'}{$swap_rep{$1}}, $module_map->{$module});
				}
				undef $_ if (/^END_DECLARE_MODULE\(.*\)$/);
			}
		}
		elsif (/^REGISTER_MODULE\(\s*([^\s,]*)\s*,\s*([^,\s]*)\s*,\s*([^,\s]*)\s*,.*$/)
		{
			my $en = $3;
			my $la = $1;
			$en = "ENABLED" if ($en eq "true");
			$en = "" if ($en eq "false");
			$la = "MOTION" if ($la eq "Motion");
			$la = "COGNITION" if ($la eq "Cognition");

			$module_map->{$2} = dclone $emptymodule unless exists($module_map->{$2});
			@{$module_map->{$2}}{'NAME', 'LAYER', 'ENABLED'} = ($2, $la, $en);
		}
		elsif(/^(\S*)::.*$/) 
		{
			$currentModule = $1;
		} elsif(/^[^=]*=\s*registerModule<(\S*)>\(\S*(\)|,);$/ or /^\s*REGISTERSYMBOLMODULE\((\S*)\);?$/) {
			$module_map->{$1} = dclone $emptymodule unless exists($module_map->{$1});
			$module_map->{$1}{'NAME'} = $1;
			@{$module_map->{$1}}{'LAYER', 'ENABLED'} = @{$module_map->{$currentModule}}{'LAYER', 'ENABLED'};

			push($module_map->{$currentModule}{'SHADOW'}, $module_map->{$1});
		}
	}
	close (FILES);
}
sub printLayer
{
	my ($dot, $layer, $highlight, $noshadowlink) = (shift(),shift(), shift(), shift());
	my $module_map = shift();
	print $dot "subgraph cluster$layer {$/label=\"$layer\";$/";

	for (values %{$module_map})
	{
		#next if $_->{'ENABLED'} eq '' or $_->{'DISPLAY'} == 0;
		next if $_->{'DISPLAY'} == 0;
		next unless $_->{'LAYER'} eq $layer;

		my $pre = $_->{'LAYER'};
		my $nodename = $_->{'LAYER'}.$_->{'NAME'};
		my $color = 0;
		$color = 1 if $_->{'DISPLAY'} == 2 and $highlight == 1;

		unless ($noshadowlink)
		{
			for (@{$_->{'SHADOW'}}) {
				next if $_->{'DISPLAY'} == 0;
				print $dot "$nodename -> ".$_->{'LAYER'}.$_->{'NAME'}." [style=dashed]$/";
			}
		}

		print $dot "$nodename [label=\"".$_->{'NAME'}."\", shape=box, style=filled, fillcolor=\"#55CC55\"]$/" if $color == 1;
		print $dot "$nodename [label=\"".$_->{'NAME'}."\", shape=box, style=filled, fillcolor=\"#CCCCCC\"]$/" unless $color == 1;

		my %print_format = ("PROVIDE" => "$nodename -> %s %s$/",
		                    "REQUIRE" => "%s  -> $nodename %s$/",
			                "RECYCLE" => "%s  -> $nodename %s$/");
		for my $type (keys %print_format)
		{
			for (@{$_->{'REP'}{$type}})
			{
				next if ($_->{'DISPLAY'} == 0);
				
				my $nodename = $pre.$_->{'NAME'};
				my $color = 0;
				$color = 1 if $_->{'DISPLAY'} == 2 and $highlight == 1;
				print $dot "$nodename [label=\"".$_->{'NAME'}."\", style=filled, fillcolor=\"#55CC55\"]$/" if $color == 1;
				print $dot "$nodename [label=\"".$_->{'NAME'}."\"]$/" unless $color == 1;

				my $extra = "";
				$extra = "[constraint=false style=\"bold\"]" if ($type eq "RECYCLE");

				print $dot sprintf($print_format{$type}, $nodename, $extra);
			}
		}
	}
	print $dot "}$/$/";
}
sub printAllModules
{
	my $module_map = shift();
	my $rep_map = shift();

	my ($highlight, $file, $keyword, $level, $enabled_only, $ignore, $noshadowlink) =
	(shift(),shift(),shift(),shift(),shift(),shift(),shift());

	$file =~ /\.(...)$/;
	my $filetype = $1;

	open my $dot, "| dot -x -T$filetype > $file";
	print $dot "digraph {$/ graph [rankdir=\"TB\"]$/";

	for my $layer("UNKNOWN","MOTION","COGNITION")
	{
		$_->{'DISPLAY'} = 0 for (values $module_map);
		$_->{'DISPLAY'} = 0	for (values $rep_map);

		for my $type ("REQUIRE", "PROVIDE")
		{
			for (grep{/^$keyword$/i}(keys %{$module_map}))
			{
				$module_map->{$_}{'DISPLAY'} = 2;
				&analyseModule($_, $type, $layer, $level+1, $enabled_only, $ignore, $module_map, $rep_map);
			}
			for (grep {/^$keyword$/i}(keys %{$rep_map}))
			{
				$rep_map->{$_}{'DISPLAY'} = 2;
				&analyseRep($_, $type, $layer, $level+1, $enabled_only, $ignore, $module_map, $rep_map);
			}
		}
		&printLayer($dot, $layer, $highlight, $noshadowlink, $module_map, $rep_map);
	}
	print $dot "}$/";
	close($dot)
}

sub analyseModule
{	
	my ($module, $type, $layer, $level, $enabled_only, $ignore) = (shift(),shift(), shift(), shift(), shift(), shift());
	my $module_map  = shift();
	my $rep_map    = shift();
	return if (not $module_map->{$module}{'ENABLED'} eq "ENABLED" and $enabled_only);
	return if ($module =~ /^$ignore$/i);
	return unless ($module_map->{$module}{'LAYER'} =~ /$layer/i and $level > 0);
	
	$module_map->{$module}{'DISPLAY'} = 1 if $module_map->{$module}{'DISPLAY'} == 0;

	for (@{$module_map->{$module}{'SHADOW'}}) {
		&analyseModule($_->{'NAME'}, $type, $layer, $level-1, $enabled_only, $ignore, $module_map, $rep_map);
	}
		
	&analyseRep($_->{'NAME'}, $type, $layer, $level-1, $enabled_only, $ignore, $module_map, $rep_map, $ignore) for(@{$module_map->{$module}{'REP'}{$type}});


	if ($type eq "REQUIRE") {
		&analyseRep($_->{'NAME'}, $type, $layer, $level-1, $enabled_only, $ignore, $module_map, $rep_map, $ignore, ) for(@{$module_map->{$module}{'REP'}{"RECYCLE"}});
	}
}
sub analyseRep
{
	my ($rep, $type, $layer, $level, $enabled_only, $ignore) = (shift(),shift(),shift(),shift(), shift(), shift());
	my $module_map = shift();
	my $rep_map    = shift();
	return unless ($level > 0);
	return if ($rep =~ /^$ignore$/i);

	$rep_map->{$rep}->{'DISPLAY'} = 1 if $rep_map->{$rep}{'DISPLAY'} == 0;
	&analyseModule($_->{'NAME'}, $type, $layer, $level-1, $enabled_only, $ignore, $module_map, $rep_map, $ignore) for(@{$rep_map->{$rep}{'REP'}{$type}});

	if ($type eq "REQUIRE") {
		&analyseModule($_->{'NAME'}, $type, $layer, $level-1, $enabled_only, $ignore, $module_map, $rep_map, $ignore) for(@{$rep_map->{$rep}{'REP'}{"RECYCLE"}});
	}

}

sub printHelp
{
	print "Usage:$/printModules.pl [--depthmax=<int>] [--file=<file>] [--nohighlight] [--help]
	[--enabled-only] [--ignore=<regex>] [--noshadowlink] <regex-node>$/";
	print "creates a diagram around modules and representations that matches <regex-node> expression$/$/";
	print "-d --depthmax           default:2$/\t\t\t".
	                              "Maximal distance to a given node.$/";
	print "-f --file               default:diagram.png$/\t\t\t".
	                              "File to save the diagram to.$/\t\t\t".
	                              "Fhe file should have the extension png,svg,gif or pdf.$/";
	print "--noh --nohighlight     Setting this option will prevent marking choosen nodes green.$/";
	print "--help | -h             shows this help$/";
	print "--enabled-only          prints only modules that are by default enabled$/";
	print "--ignore                regex of representations and modules name to ignore$/";
	print "--noshadowlink          Do not draw the dashed line between two modules, if one module$/\t\t\t".
	                              "is executing the other one$/";
	print "<regex-node>            A perl regex expression to select the nodes of interest.$/\t\t\t".
	                              "Cases are ignored.$/";

	print "Examples:$/";
	print "./bin/printModules.pl gyroreader$/\tCreates a diagram around the module gyroreader$/$/";
	print "./bin/printModules.pl gyrodata$/\tCreates a diagram around the representation gyrodata$/$/";

	print "./bin/printModules.pl --nohighlight gyrodata$/\tCreates a diagram around the
	representation gyrodata without highlighting$/$/";

	print "./bin/printModules.pl --file=diagram.svg gyrodata$/\tCreates a diagram around the
	representation gyrodata exporting it as a svg file$/$/";

	print "./bin/printModules.pl \"gyro.*\"$/\tCreates a diagram around any
	module and/or representation starting with gyro$/$/"; 

	print "./bin/printModules.pl \".*reader\"$/\tCreates a diagram around any
	module and/or representation ending with reader$/$/"; 

	print "./bin/printModules.pl -d 1 --nohighlight \".*\"$/\tCreates a full diagram with all
	modules and representations$/";
}


my $level = 2;
my $nohighlight = 0;
my $help = 0;
my $errorparsing = 0;
my $enabled_only = 0;
my $ignore_exp = "";
my $noshadowlink = 0;
my $file = "diagram.png";
$help = 1 unless GetOptions ("depthmax|d=i"     => \$level,    # numeric
	       					 "file|f=s"         => \$file,
 	                         "nohighlight|noh"  => \$nohighlight,
							 "enabled-only"     => \$enabled_only,
							 "ignore=s"         => \$ignore_exp,
							 "noshadowlink"     => \$noshadowlink,
		     	        	 "help|h"           => \$help);  # flag

my $keyword = shift;
$help = 1 unless defined $keyword;
if ($help) {
	&printHelp;
	exit;
}
my %module_map;
my %rep_map;
&analyseCode(\%module_map, \%rep_map);
&printAllModules(\%module_map, \%rep_map, 1-$nohighlight, $file, $keyword, $level, $enabled_only, $ignore_exp, $noshadowlink);
