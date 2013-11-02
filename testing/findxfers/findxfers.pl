#!/bin/perl

my @m = qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec);
my %m = (Jan => 1,
	 Feb => 2,
	 Mar => 3,
	 Apr => 4,
	 May => 5,
	 Jun => 6,
	 Jul => 7,
	 Aug => 8,
	 Sep => 9,
	 Oct => 10,
	 Nov => 11,
	 Dec => 12);

my $f = '/ftp-data/logs/xferlog';
my @start = split /-/, $ARGV[0];
my $user = $ARGV[1];

$|++;

if (scalar @start != 3) {
	print "syntax; SITE FINDXFERS <period-start>

Eg: SITE FINDXFERS 2001-01-01

";
	exit 0;
}

$go = 1 unless scalar @start == 3;

print "Loading xferlog.. Hold..\n";

open FH, $f;
while (my $l = <FH>) {
    $l =~ s/ +/ /gs;
    $l =~ s/(\r|\n)//gs;

    my @a = split(/ /, $l);

    next unless scalar @a == 17;

#    next unless ($user eq $a[13]);

    if ($go == 0) {

	if (($a[4] >= $start[0]) && ($m{$a[1]} >= $start[1]) && ($a[2] >= $start[2])) {
	    print ".. Found first transfer in period .. $a[0] $a[1] $a[2] $a[3] $a[4]\n";
	    $go = 1;
	}
    }

    next unless $go > 0;

    my $dir = $a[11];

    $data{$a[13]}->{"$dir,f"}++;
    $data{$a[13]}->{"$dir,b"} += $a[7];
    $data{$a[13]}->{"$dir,s"} += $a[5];
}
close FH;


if (scalar @start == 3) {
    $periodstart = sprintf("%4d-%02d-%02d", @start);
} else {
    $periodstart = "Alltime";
}

print <<EOHEAD;

Period start: $periodstart
------------------------------------------------------------------------
EOHEAD

foreach my $tk (sort keys %data) {
    my $i = $data{$tk};

    my $ospeed = $i->{"o,b"} / ($i->{"o,s"} + 1);
    my $ispeed = $i->{"i,b"} / ($i->{"i,s"} + 1);

    printf <<EOOUT, $tk, $i->{"o,f"}, $i->{"o,b"}/(1024*1024), $ospeed/1024, $i->{"i,f"}, $i->{"i,b"}/(1024*1024), $ispeed/1024;
%-12.12s--> Download  : %6d file/s, %10.1f Mbytes, %7.1f k/s
            `-> Upload    : %6d file/s, %10.1f Mbytes, %7.1f k/s
EOOUT

}

