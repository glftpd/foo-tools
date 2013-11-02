#!/usr/bin/perl

die "syntax; g2gl.pl <passwd> <shadow> <userfiledir> <outputdir>\n" unless scalar @ARGV == 4;

# read passwd file.
open PASSWD, $ARGV[0] || die "error opening passwd $ARGV[0]\n";
while (my $l = <PASSWD>) {
	$l =~ s/(\r|\n)//gs;

	my @u = split(/:/, $l);

	next unless scalar @u == 7;

	# make creation time gl-stylah.
	$u[4] =~ s!/!-!g;
	$u[5] = '/site';
	$u[6] = '/bin/false';

	$users{$u[0]} = \@u;
}
close PASSWD;

# get passwords from shadow.
open SHADOW, $ARGV[1] || die "error opening shadow: $ARGV[1]\n";
while (my $l = <SHADOW>) {
	$l =~ s/(\r|\n)//gs;

	my @s = split /:/, $l;

	next unless $users{$s[0]};
	
	$users{$s[0]}->[1] = $s[1];
}
close SHADOW;

# read the userfiles.
opendir DH, $ARGV[2] || die "error opening userfiledir: $ARGV[2]\n";
my @users = grep !/^\./, readdir DH;
closedir DH;

foreach my $user (@users) {
	my @ugrps = convert_userfile("$ARGV[2]/$user", "$ARGV[3]/$user");

	foreach (@ugrps) {
		$groups{$_}++;
	}

	next unless $users{$user};

	$users{$user}->[3] = $ugrps[0];
}

# write groups.
open GROUP, ">group.new";
my $i = 100;
foreach my $grp (keys %groups) {

	print GROUP "$grp:$grp:$i:\n";

	foreach (keys %users) {
		next if ($users{$_}->[3] ne $grp);

		$users{$_}->[3] = $i;
	}

	$i += 100;
}
close GROUP;

open PASSWD, ">passwd.new";
foreach my $user (keys %users) {
	print PASSWD join(":", @{$users{$user}})."\n";
}

exit 0;


sub convert_userfile {
	my ($gf, $glf) = @_;

	my %data;

	open GF, $gf || die "couldnt read file $gf\n";

	while (my $l = <GF>) {
		$l =~ s/(\r|\n)//gs;

		if (my ($p, $q) = $l =~ /^(ratio|num_logins|credits|files_up|files_down|bytes_up|bytes_down|seconds_up|seconds_down|files_up_wk|files_down_wk|bytes_up_wk|bytes_down_wk|seconds_up_wk|seconds_down_wk|bytes_up_m|bytes_down_m|files_up_m|files_down_m|seconds_up_m|seconds_down_m|login_times|last_on|time_limit|last_nuked|weeklimit) (\d+)$/) {
			$data{$p} = $q;
		}
		elsif (my ($p, $q) = $l =~ /^(tagline) (.*)/) {
			$data{$p} = $q;
		}
		elsif (my ($p, $q) = $l =~ /^(group_user|ip) (.*)/) {
			push @{$data{$p}}, $q;
		}
	}

	close GF;

	open GLF, ">$glf" || die "couldnt open $glf for writing\n";
	print GLF <<EOGLF;
USER added by g2gl converter
GENERAL 0,0 $data{num_logins} 0 0
TIMEFRAME 0 0
FLAGS 3
TAGLINE $data{tagline}
DIR /
CREDITS $data{credits}
RATIO $data{ratio}
ALLUP $data{files_up} $data{bytes_up} $data{seconds_up}
ALLDN $data{files_down} $data{bytes_down} $data{seconds_down}
WKUP $data{files_up_wk} $data{bytes_up_wk} $data{seconds_up_wk}
WKDN $data{files_down_wk} $data{bytes_down_wk} $data{seconds_down_wk}
DAYUP 0 0 0
DAYDN 0 0 0
MONTHUP $data{files_up_m} $data{bytes_up_m} $data{seconds_up_m}
MONTHDN $data{files_down_m} $data{bytes_down_m} $data{seconds_down_m}
NUKE $data{last_nuked} 0 0
TIME $data{login_times} $data{last_on} 0 0
SLOTS -1 -1
EOGLF

	foreach (@{$data{group_user}}) {
		print GLF "GROUP $_\n";
	}
	foreach (@{$data{ip}}) {
		print GLF "IP $_\n";
	}

	close GLF;

	return @{$data{group_user}};
}
