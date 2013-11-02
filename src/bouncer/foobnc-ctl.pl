#!/usr/bin/perl

use LWP::Simple;
use LWP::UserAgent;
use HTTP::Cookies;
use URI::Escape;


#
# new useragent that overrides the credentials method.
#
package newua;

@ISA = qw(LWP::UserAgent);

sub new {
        my $type = shift;
        my $self = new LWP::UserAgent;
        bless $self, $type;
}

sub get_basic_credentials {
        return ($::ARGV[1], $::ARGV[2]);
}

#
# main loop.
#
package main;



die "Syntax: $0 <bouncer ip:port> <bouncer name> <bouncer password> <config file>\n" unless @ARGV == 4;

# loca config file.
open FH, $ARGV[3];
my $newcfg = join "", <FH>;
close FH;

my @content = ('command=configure',
			   'config='.uri_escape($newcfg, "^A-Za-z"));

my $content = join '&', @content;

my $header  = new HTTP::Headers('Content-Type' => 'application/x-www-form-urlencoded',
								'Content-Length' => length($content));

my $request = new HTTP::Request(POST => "http://$ARGV[0]/foobnc.html",
								$header, $content);

my $ua = new newua();
my $response = $ua->request($request);

if (!$response->is_success) {
	printf "[error] %s\n", $response->content();
}
else {
	printf "[response] %s\n", $response->content();
}
