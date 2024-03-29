# check differences between MsgIDList and each resource definitions.

if ($#ARGV != 1) {
	print "Usage : CheckRes.pl <resource file> <outfile>\n";
	exit 0;
}

# read MsgIDList

open(ID, "MsgIDList.txt") || die;
while(<ID>) {
	chop;
	$listinfo{$_} = "u";
}
close(ID);

# compare to resource file
open(RES, $ARGV[0]) || die;
$line = 1;
while(<RES>) {
	chop;
	($key, $value) = split(/	/);

	if ($listinfo{$key} eq "u") {
		# the ID is exists in list and occures first time.
		$listinfo{$key} = "e";
	} elsif (defined($listinfo{$key})) {
		$listinfo{$key} = "d";
	} else {
		# unknown ID
		$listinfo{$key} = "x";
	}
	$msgline{$key} = $value;
	$line++;
}
close(RES);

$excessive="";
$insuff="";
$dup="";

# print result
foreach $i (keys(%listinfo)) {
	next if ($listinfo{$i} eq "e");

	if ($listinfo{$i} eq "x") {
		$excessive .= " $i";
	} elsif ($listinfo{$i} eq "d") {
		$dup .= " $i";
	} elsif ($listinfo{$i} eq "u") {
		$insuff .= " $i";
	}
}

print "Excessive list = $excessive\n" if ($excessive ne "");
print "Insufficent list = $insuff\n" if ($insuff ne "");
print "Duplicate list = $dup\n" if ($dup ne "");

if ($excessive ne "" || $insuff ne "" || $dup ne "") {
	exit 1;
}

print "OK.\n";


open(ID, "MsgIDList.txt") || die;
open(OUT, ">$ARGV[1]") || die;
while(<ID>) {
	chop;
	print OUT "$msgline{$_}\n";
}
close(OUT);
close(ID);
