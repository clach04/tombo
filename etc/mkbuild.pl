#
# TOMBO Build program for ActivePerl(Win32)

$version = "Tombo_2_0b4";

##########################################################################
# Tool definition
##########################################################################

use File::Glob ':glob';
use File::Path;
use File::Copy;
#use File::DosGlob 'glob';

$zippg = "\"C:\\Program Files\\mzp\\mzp.exe\"";

##########################################################################
# File definition
##########################################################################

$buildroot = "C:\\work2\\SF\\TOMBO";
$docroot = "C:\\work2\\SF\\Docs";
$builddir  = "C:\\temp\\Tombo-Rel";

@docsJP = (
	["", "$docroot\\Jp\\COPYING-2_0.txt"],
	["", "$docroot\\Jp\\Readme.txt"],
	["", "$docroot\\DocBooks\\UserGuide_jp.chm"],
	["", "$buildroot\\etc\\vfolder.xml"],
	["", "$buildroot\\etc\\TOMBO_vfolder.dtd"],
);

@docsEN = (
	["", "$docroot\\En\\COPYING-2_0.txt"],
	["", "$docroot\\En\\Readme.txt"],
	["", "$docroot\\DocBooks\\UserGuide_en.chm"],
	["", "$buildroot\\etc\\vfolder.xml"],
	["", "$buildroot\\etc\\TOMBO_vfolder.dtd"],
);

@bin_ce_all_jp = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
	["Bin\\HPCPro\\ARM", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\RegErase\\ARMRel\\RegErase.exe"],

	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\oniguruma\\mips\\oniguruma.dll"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\RegErase\\MIPSRel\\RegErase.exe"],

	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
	["Bin\\HPCPro\\SH3", "$buildroot\\oniguruma\\sh3\\oniguruma.dll"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\RegErase\\SH3Rel\\RegErase.exe"],

	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
	["Bin\\HPCPro\\SH4", "$buildroot\\oniguruma\\sh4\\oniguruma.dll"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\RegErase\\SH4Rel\\RegErase.exe"],

	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\Tombo.mips.CAB"],

	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\Tombo.sh3.CAB"],

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_jp.arm.CAB"],

	
	["Bin\\WindowsMobile5", "$buildroot\\WindowsMobile5PPC\\CabWiz\\Tombo_jp.arm.CAB"],

	["Bin\\BE500", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\BE500", "$buildroot\\Src\\MsgRes\\TomboMsg\\jp\\TomboMsg.txt"],
	["Bin\\BE500", "$buildroot\\oniguruma\\mips\\oniguruma.dll"],
	["Bin\\BE500", "$buildroot\\HpcPro\\RegErase\\MIPSRel\\RegErase.exe"],

	@docsJP

);

@bin_pktpc_arm_jp = (
	["", "$buildroot\\PocketPC\\CabWiz\\Tombo.arm.CAB"],
	["", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_jp.arm.CAB"],

	@docsJP

);

@bin_wm5_arm_jp = (
	["", "$buildroot\\WindowsMobile5PPC\\CabWiz\\Tombo_jp.arm.CAB"],
	@docsJP
);

@bin_win32_jp = (
	["", "C:\\temp\\TomboSetupJ\\*.*"],
	["Files", "C:\\temp\\TomboSetupJ\\Files\\*.*"],
	["", "$docroot\\Jp\\Readme.txt"],
);

@bin_ce_all_en = (
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\Tombo\\ARMRel\\Tombo.exe"],
	["Bin\\HPCPro\\ARM", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
	["Bin\\HPCPro\\ARM", "$buildroot\\oniguruma\\arm\\oniguruma.dll"],
	["Bin\\HPCPro\\ARM", "$buildroot\\HpcPro\\RegErase\\ARMRel\\RegErase.exe"],

	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\oniguruma\\mips\\oniguruma.dll"],
	["Bin\\HPCPro\\MIPS", "$buildroot\\HpcPro\\RegErase\\MIPSRel\\RegErase.exe"],

	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\Tombo\\SH3Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH3", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
	["Bin\\HPCPro\\SH3", "$buildroot\\oniguruma\\sh3\\oniguruma.dll"],
	["Bin\\HPCPro\\SH3", "$buildroot\\HpcPro\\RegErase\\SH3Rel\\RegErase.exe"],

	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\Tombo\\SH4Rel\\Tombo.exe"],
	["Bin\\HPCPro\\SH4", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
	["Bin\\HPCPro\\SH4", "$buildroot\\oniguruma\\sh4\\oniguruma.dll"],
	["Bin\\HPCPro\\SH4", "$buildroot\\HpcPro\\RegErase\\SH4Rel\\RegErase.exe"],

	["Bin\\PocketPC\\MIPS", "$buildroot\\PocketPC\\CabWiz\\TomboE.mips.CAB"],

	["Bin\\PocketPC\\SH3", "$buildroot\\PocketPC\\CabWiz\\TomboE.sh3.CAB"],

	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],
	["Bin\\PocketPC\\ARM", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_en.arm.CAB"],

	["Bin\\WindowsMobile5", "$buildroot\\WindowsMobile5PPC\\CabWiz\\Tombo_en.arm.CAB"],

	["Bin\\BE300", "$buildroot\\BE300\\Tombo\\MIPSRel\\Tombo.exe"],
	["Bin\\BE300", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
	["Bin\\BE300", "$buildroot\\oniguruma\\mips\\oniguruma.dll"],
	["Bin\\BE300", "$buildroot\\HpcPro\\RegErase\\MIPSRel\\RegErase.exe"],

	@docsEN
);

@bin_pktpc_arm_en = (
	["", "$buildroot\\PocketPC\\CabWiz\\TomboE.arm.CAB"],
	["", "$buildroot\\PocketPC\\CabWiz\\TomboVGA_en.arm.CAB"],

	@docsEN
);

@bin_wm5_arm_en = (
	["", "$buildroot\\WindowsMobile5PPC\\CabWiz\\Tombo_en.arm.CAB"],
	@docsEN
);

@bin_win32_en = (
	["", "C:\\temp\\TomboSetupE\\*.*"],
	["Files", "C:\\temp\\TomboSetupE\\Files\\*.*"],
	["", "$docroot\\En\\Readme.txt"],
#	["", "$buildroot\\Win32\\Tombo\\Release\\Tombo.exe"],
#	["", "$buildroot\\Src\\MsgRes\\TomboMsg\\en\\TomboMsg.txt"],
#	["", "$buildroot\\oniguruma\\win32\\oniguruma.dll"],
#	["", "$buildroot\\Win32\\RegErase\\Release\\RegErase.exe"],
#	["", "$buildroot\\expat\\lib\\Win32\\libexpatw.dll"],

	@docsEN
);

##########################################################################
# Program section
##########################################################################

@archlist = (
	[\@bin_ce_all_jp, "CE-JP", "_ce_all_jp"],
	[\@bin_ce_all_en, "CE-EN", "_ce_all_en"],
	[\@bin_win32_jp, "Win32-JP", "_desktop_jp"],
	[\@bin_win32_en, "Win32-EN", "_desktop_en"],

	[\@bin_pktpc_arm_jp,"PKTPC-ARM-JP", "_pktpc_arm_jp"],
	[\@bin_pktpc_arm_en,"PKTPC-ARM-EN", "_pktpc_arm_en"],

	[\@bin_wm5_arm_jp, "WM5-ARM-JP", "_wm5_jp"],
	[\@bin_wm5_arm_en, "WM5-ARM-EN", "_wm5_en"],

#	[\@bin_sig3, "SIG3", "_sig3"],

);

foreach $arch (@archlist) {
	my ($al, $dl, $suffix) = @{$arch};
	&cpfile ("$builddir\\$version\\$dl\\$version", $al);
	&zipfile("$builddir\\$version\\$dl", $suffix, $version);
}

exit 0;

##########################################################################
# Subroutines
##########################################################################

sub cpfile {
	my ($buildto, $list) = @_;
	my ($i, $num);

	$num = 0;

	foreach $i (@{$list}) {
		($dst, $src) = @{$i};

		# Create directory if not exist.
		mkpath("$buildto\\$dst") || die "mkdir $buildto\\$dst : $!" unless (-d "$buildto\\$dst");

		# Check file existance and copy it.
		foreach $f (bsd_glob($src)) {
			die "$f not exist!" unless (-f "$f");
			print "$f -> $buildto\\$dst\n";
			copy("$f", "$buildto\\$dst\\") || die "$f : $!";
			$num++;
		}
	}
	print "Copy $num files.\n";
}

sub zipfile {
	my ($dir, $suffix, $target) = @_;
	chdir($dir);
	system("$zippg -r ..\\$version$suffix.zip $version");
}
