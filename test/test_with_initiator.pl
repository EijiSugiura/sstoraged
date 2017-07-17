#!/usr/bin/perl

use strict;
use JSON;
$|=0;
use constant SCENARIO	=> './test_with_initiator.json';

open(FILE,SCENARIO) or die "Can't open " . SCENARIO . " : $!";
my $texts = join('',<FILE>);
close(FILE);

my $jsobj = jsonToObj($texts);

foreach my $test (@{$$jsobj{prep}}){
    print $$test{desc} . "...\n";
    my $ret = system($$test{cmd});
    if($ret){
	print "\t\t\tfailed... skip tests\n";
	exit(1);
    }
    print "\t\t\t[OK]\n";
}

my $counter = 0;
my $ng=0;
print "Running "  . ($#{$$jsobj{test}}+1) . " tests";
foreach my $test (@{$$jsobj{test}}){
    ++$counter;
    my @logs = ();
    if($$test{log} eq 'stdout'){
	open(CMD,"$$test{cmd}|") or print "$$test{cmd} : $!";
	@logs = <CMD>;
	close(CMD);
    } elsif($$test{log} eq 'stderr'){
	open(CMD,"$$test{cmd} 2>&1 |") or print "$$test{cmd} : $!";
	@logs = <CMD>;
	close(CMD);
    } else {
	system($$test{cmd});
	open(LOG,$$test{log}) or print "$$test{cmd} : $!";
	@logs = <LOG>;
	close(LOG);
	rename $$test{log},$$test{log}.'.'.$counter;
    }
    my $ok = 0;
    foreach my $line (@logs){
	$ok = 1 if($line =~ /$$test{pattern}/);
    }
    if($ok){
	print ".";
    }else {
	print "\n" . $$test{desc} . " NG\n\"$$test{pattern}\" not found...\n@logs";
	$ng = 1;
	last;
    }
}
print "OK!\n" if(!$ng);

foreach my $test (@{$$jsobj{post}}){
    print $$test{desc} . "...\n";
    my $ret = system($$test{cmd});
    if($ret){
	print "\t\t\t[IGNORED]\n";
	next;
    }
    print "\t\t\t[OK]\n";
}

exit($ng);
