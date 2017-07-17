#!/usr/bin/perl

use strict;
use Term::ReadLine;

use vars qw($TERM $OUT %CACHE);
$TERM = new Term::ReadLine 'sstorage configuration term';
$OUT = $TERM->OUT;

my $template = '/etc/sstorage/sstoraged.conf';
my $in;
$in = getYnLine("Modify \"$template\"") if(-f $template);
$template = getValue("Please specify template file path") if(!$in);
open(CONF, $template) || die "Can't open $template : $!";
my @lines = <CONF>;
close(CONF);
print "\nParsing $template...\n";

my @outputs = ();
my @MUST_ATTRS = ("GlobalTargetName",
		  "LocalTargetName");
my @LUNS = ();
my $section = "Global";
foreach my $line (@lines){
    push(@outputs, $line);
    $line =~ s/\s+//g;
    if($line =~ /^\"(.*)\":\"(.*)\"/ ||
       $line =~ /^\"(.*)\":\"(\d+)\"/){
	my ($attr, $val) = ($1, $2);
	for(my $index = 0; $index <= $#MUST_ATTRS; ++$index){
	    if($attr eq $MUST_ATTRS[$index]){
		$in = getYnLine("\"$attr\" is \"$val\"");
		$val = getValue("Please specify $attr") if(!$in);
		print $OUT "   \"$attr\" : \"$val\",\n\n";
		pop(@outputs);
		push(@outputs, "   \"$attr\" : \"$val\",\n");
		delete $MUST_ATTRS[$index];
	    }
	}
	$CACHE{$attr} = $val;
    }
    if($line =~ /^\"Target\"/){
	$section = "Target";
	next;
    } elsif($line =~ /^\"Initiator\"/){
	if(getyNLine("Add more \"$section\"")){
	    my @lines = addSection($section);
	    unshift(@lines, "      ,\n");
	    print $OUT "@lines\n";
	    insertLines(\@outputs, \@lines);
	}
	$section = "Initiator";
	next;
    } elsif($line =~ /^\"Volume\"/){
	if(getyNLine("Add more \"$section\"")){
	    my @lines = addSection($section);
	    unshift(@lines, "      ,\n");
	    print $OUT "@lines\n";
	    insertLines(\@outputs, \@lines);
	}
	$section = "Volume";
	next;
    }
    if($line =~ /^\{\}/){
	# empty array, must be add at least one.
	my @lines = ();
	if($section eq "Volume" ){
	    @lines = addVolumes();
	} else {
	    @lines = addSection($section);
	}
	print $OUT "@lines\n";
	pop(@outputs);
	push(@outputs, @lines);
    }
}

my @lines = addVolumes();
if($#lines >= 0){
    unshift(@lines, "      ,\n");
    print $OUT "@lines\n";
    insertLines(\@outputs, \@lines);
}
print $OUT "Generated configuration file...\n";
print $OUT "================================================================\n";
print $OUT @outputs;
print $OUT "================================================================\n";
if(!getyNLine("Overwrite $template")){
    $template = getValue("Please specify output file");
} else {
    print $OUT "Save original file to $template.org...";
    system("cp $template $template.org");
    print $OUT "done.\n";
}
print $OUT "Writing $template...";
open(OUTPUT, ">$template") or die "Can't write $template : $!";
print OUTPUT @outputs;
close(OUTPUT);
print $OUT "done.\n";
exit(0);

sub getYnLine {
    my ($prompt) = @_;
    $prompt .= "? [Y/n] : ";
    my $in = $TERM->readline($prompt);
    if(!defined($in)){
	print "empty : $!\n";
	exit(1);
    }
    $TERM->addhistory($in) if /\S/;
    if($in =~ /^y/i || length($in) == 0){
	$in = 1;
    } else {
	$in = 0;
    }
    return $in;
}

sub getyNLine {
    my ($prompt) = @_;
    $prompt .= "? [y/N] : ";
    my $in = $TERM->readline($prompt);
    if(!defined($in)){
	print "empty : $!\n";
	exit(1);
    }
    $TERM->addhistory($in) if /\S/;
    if($in =~ /^n/i || length($in) == 0){
	$in = 0;
    } else {
	$in = 1;
    }
    return $in;
}

sub getValue {
    my ($prompt) = @_;
    $prompt .= " : ";
retry_get_value:
    my $in = $TERM->readline($prompt);
    if(!defined($in)){
	print "empty : $!\n";
	exit(1);
    }
    $TERM->addhistory($in) if /\S/;
    goto retry_get_value if(length($in) == 0);
    return $in;
}

sub getValueYn{
    my ($attr, $val) = @_;
    $val = getValue("Please specify $attr") if(!getYnLine("\"$attr\" : \"$val\""));
    return $val;
}

sub insertLines{
    my ($outputs, $lines) = @_;
    my @cache = ();
    while(1){
	unshift(@cache, pop(@$outputs));
	last if($cache[0] =~ /\]/);
    }
    push(@$outputs, @$lines, @cache);
}

sub addSection{
    my ($section) = @_;
    print $OUT "\"$section\"\n";
    return addTarget() if($section =~ /Target/);
    return addInitiator() if($section =~ /Initiator/);
    die "Internal error";
}

sub addTarget{
    my @lines = ();

    my $targetname = $CACHE{LocalTargetName};
    $targetname = getValueYn("TargetName", $targetname);
    push(@lines, "      {\n         \"TargetName\" : \"$targetname\",\n");

    my @tokens = split(':', $targetname);
    my $targetalias = $tokens[$#tokens];
    $targetalias = getValueYn("TargetAlias", $targetalias);
    push(@lines, "         \"TargetAlias\" : \"$targetalias\",\n");

    open(HOSTNAME, "hostname|");
    my $targetaddress = <HOSTNAME>;
    close(HOSTNAME);
    chomp($targetaddress);
    die "Can't get hostname for TargetAddress" if(length($targetaddress) == 0);
    $targetaddress = "[" . $targetaddress . "]:3260";
    $targetaddress = getValueYn("TargetAddress", $targetaddress);
    push(@lines, "         \"TargetAddress\" : \"$targetaddress\"\n");
    push(@lines, "      }\n");

    return @lines;
}

sub addInitiator{
    my @lines = ();

    my $initiatorname = getValue("\"InitiatorName\"");
    push(@lines, "      {\n         \"InitiatorName\" : \"$initiatorname\",\n");

    my @tokens = split(':', $initiatorname);
    my $initiatoralias = $tokens[$#tokens];
    if(length($initiatoralias)){
	$initiatoralias = getValueYn("InitiatorAlias", $initiatoralias);
    } else {
	$initiatoralias = getValueYn("\"InitiatorAlias\"");
    }
    push(@lines, "         \"InitiatorAlias\" : \"$initiatoralias\",\n");

    my $lun = getValue("\"LUN\"");
    push(@lines, "         \"LUN\" : [\"$lun\"]\n      }\n");
    push(@LUNS, $lun);

    return @lines;
}

sub getVolumeSize{
    my $count = getValueYn("Volume Size[MB]", "4096");
    $count *= 1048576 / 512;
    $count = int($count);
    die "Illegal Volume Size" if($count <= 0 || !defined($count));
    return (0, $count);
}

sub addVolume{
    my ($volumename) = @_;
    my @lines = ();
    print $OUT "Configure \"$volumename\"...\n";
    push(@lines, "      {\n         \"VolumeName\" : \"$volumename\",\n");
    push(@lines, "         \"Main\" : [\n");

    push(@lines, "            {\n");
    my ($host,$path) = ($CACHE{LocalTargetName},
			getValueYn("Path", "/var/spool/sstoraged/".$volumename.".bin"));
    push(@lines, "               \"Host\" : \"$host\",\n");
    push(@lines, "               \"Path\" : \"$path\",\n");
    my ($start,$count) = getVolumeSize();
    push(@lines, "               \"Start\" : $start,\n");
    push(@lines, "               \"Count\" : $count\n");
    push(@lines, "            }\n");

    push(@lines, "         ]\n");
    push(@lines, "      }\n");
    return @lines;
}

sub addVolumes{
    my @lines = ();
    if($#LUNS >= 0){
	foreach my $lun (@LUNS){
	    push(@lines, "      ,\n") if($#lines >= 0);
	    push(@lines, addVolume($lun));
	}
	undef @LUNS;
    }
    return @lines;
}
