#!/usr/bin/env perl

$hash_func = 'python /Users/sakane/work/lang/python/test_sha1.py';

$prefix = 'http://fiap.tanu.org/test/alps';

@id = qw/e7 f1 f7 f6/;
@obj = qw@
temperature
temperature/max
temperature/min
temperature/avr
humidity
humidity/max
humidity/min
humidity/avr
pressure
light
water
battery
rssi
@;
&pmap(\@id, \@obj);

@id = (d6);
@obj = qw@
current
current/max
current/min
current/avr
battery
rssi
@;
&pmap(\@id, \@obj);

sub pmap
{
	my ($id, $obj) = @_;

	for $i (@$id) {
		for $j (@$obj) {
			$pid = sprintf("%s/%s/%s", $prefix, $i, $j);
			chomp($hash = `$hash_func $pid`);
			print <<EOD;
{ "$pid",
  "$hash" },
EOD
		}
	}
}
