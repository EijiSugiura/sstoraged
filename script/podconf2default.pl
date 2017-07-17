#!/usr/bin/perl

use strict;
use Pod::Usage;
use Getopt::Long;

my($HELP, $FILENAME);
GetOptions("help|h"		=>	\$HELP,
	   "file|f=s"		=>	\$FILENAME) || pod2usage();
pod2usage(-verbose => 2) if ($HELP);
pod2usage() if(!defined($FILENAME));

open(FILE, $FILENAME) or die "Can't open $FILENAME : $!";
my @lines = <FILE>;
close(FILE);

my $state = 0;
foreach my $line (@lines){
    if($state == 0){
	++$state if($line =~ /^ Default/);
    }elsif($state == 1){
	if($line =~ /(\".+\")\s*:\s*(\"\S+\"),\s+/){
	    print "\tdefaultValue[$1] =\tstatic_cast<string>($2);\n";
	    $state = 0;
	}elsif($line =~ /(\".+\")\s*:\s*(\d+),\s+/){
	    print "\tdefaultValue[$1] =\tstatic_cast<uint64_t>($2);\n";
	    $state = 0;
	}elsif($line =~ /(\".+\")\s*:\s*(\w+),\s+/){
	    print "\tdefaultValue[$1] =\t$2;\n";
	    $state = 0;
	}
    }
}
exit(0);
__END__

=pod

=head1 NAME

podconf2default.pl

=head1 SYNOPSIS

podconf2default.pl [-h] [-f pod_file]

=head1 DESCRIPTION

B<podconf2default.pl> read pod_file, and output default attribue/value C++ source.

=cut
