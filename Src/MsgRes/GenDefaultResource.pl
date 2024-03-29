# Generate default message 

$msglist = "MsgIdList.txt";
$header = "MsgDef.h";
$cpp = "DefaultMsg.cpp";
$baseres = "MsgDef/TomboMsg_en.txt";

open(SRC, $msglist) || die;
open(INCL, ">$header") || die;

print INCL <<INCLHEAD ;
// This file is generated by GenMsgIncl.pl. DO NOT EDIT THIS FILE.

INCLHEAD

#######################

$i = 1;
while(<SRC>) {
	chop;
	/^MSG_(.*)$/;
	print INCL "#define MSG_ID_$1 $i\n";
	print INCL "#define $_ (g_mMsgRes.GetMsg(MSG_ID_$1))\n";
	print INCL "\n";
	$i++;
}

$i--;

print INCL <<INCLFOOT ;

#define NUM_MESSAGES $i
INCLFOOT

close(INCL);
close(SRC);

# ====================================================

open(LIST, $msglist) || die;
open(OUT, ">$cpp") || die;

open(SRC, $baseres) || die;
while(<SRC>) {
	chop;
	($key, $value) = split(/\t/);
	$msgpair{$key} = $value;
}
close(SRC);

print OUT <<HEADER ;
// This file is generated by GenBuildInResource.pl. Do not edit this file.
static LPCTSTR defaultMsg[] = {
HEADER

while(<LIST>) {
	chop;
	print OUT "	TEXT(\"$msgpair{$_}\"),\n";
}

print OUT <<FOOTER ;
};
FOOTER

close(LIST);
close(OUT);
