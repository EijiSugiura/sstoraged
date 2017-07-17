#!/usr/bin/perl

use strict;
use Pod::Usage;
use Getopt::Long;

my($HELP, $FILENAME);
GetOptions("help|h"		=>	\$HELP,
	   "file|f=s"		=>	\$FILENAME) || pod2usage();
pod2usage(-verbose => 2) if ($HELP);
pod2usage() if(!defined($FILENAME));

open(FILE, "cat $FILENAME | pod2text |") or die "Can't open $FILENAME : $!";
my @lines = <FILE>;
close(FILE);

print "// sstoraged.conf - sstoraged configuration file\n";
print "{\n";

my $output = 0;
my $default = 0;
my $comment = 0;
my $cache = "";
my $indent = 0;
foreach my $line (@lines){
    if($line =~ /^General/){
	$output = 1;
	$comment = 1;
	$indent = 1;
	next;
    }
    if($line =~ /^iSCSI/){
	if(length($cache)){
	    for(my $counter = 0; $counter < $indent; $counter++){
		print "   ";
	    }
	    print "// " . $cache . "\n";
	    $cache = "";
	}
	$output = 1;
	$indent = 1;
	next;
    }
    if($line =~ /^Target/){
	if(length($cache)){
	    for(my $counter = 0; $counter < $indent; $counter++){
		print "   ";
	    }
	    print "// " . $cache . "\n";
	    $cache = "";
	}
	print "\n   \"Target\" : [\n";
	$output = 0;
	$indent = 2;
	next;
    }
    if($line =~ /^Initiator/){
	if(length($cache)){
	    for(my $counter = 0; $counter < $indent; $counter++){
		print "   ";
	    }
	    print $cache . "\n";
	    $cache = "";
	} else {
	    print "      {}\n";
	}
	print "   ],\n";
	print "\n   \"Initiator\" : [\n";
	$output = 0;
	$indent = 2;
	next;
    }
    if($line =~ /^Volume/){
	if(length($cache)){
	    for(my $counter = 0; $counter < $indent; $counter++){
		print "   ";
	    }
	    print $cache . "\n";
	    $cache = "";
	} else {
	    print "      {}\n";
	}
	print "   ],\n";
	print "   \"Volume\" : [\n";
	$output = 0;
	$indent = 2;
	next;
    }
    if($line =~ /^EXAMPLES/){
	if(length($cache)){
	    print "\n";
	    for(my $counter = 0; $counter < $indent; $counter++){
		print "   ";
	    }
	    print $cache . "\n";
	    $cache = "";
	} else {
	    print "      {}\n";
	}
	print "   ]\n";
	last;
    }
    if($output && $line =~ /^     Default/){
	$default = 1;
	next;
    }
    if($default){
	$cache = $line;
	$cache =~ s/\s+//;
	$default = 0;
    }elsif($comment == 0) {
	next;
    }elsif( $line =~ /^  \S+/ && length($cache)) {
	for(my $counter = 0; $counter < $indent; $counter++){
	    print "   ";
	}
	if($cache =~/TargetName/){
	    print "$cache\n";
	} else {
	    print "// " . $cache . "\n";
	}
	for(my $counter = 0; $counter < $indent; $counter++){
	    print "   ";
	}
	print "// " . $line;
	$cache = "";
    }elsif( $line =~ /\S+/ ) {
	for(my $counter = 0; $counter < $indent; $counter++){
	    print "   ";
	}
	print "// " . $line;
    }else{
#	print $line;
    }
}

print "}\n";

exit(0);
__END__

=pod

=head1 NAME

podconf2example.pl

=head1 SYNOPSIS

podconf2example.pl [-h] [-f pod_file]

=head1 DESCRIPTION

B<podconf2example.pl> read pod_file, and output configuration example.

=cut
