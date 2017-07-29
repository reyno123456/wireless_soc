#! /usr/bin/perl
#use strict;

if ( @ARGV <1) {
	printf" bin2ini.pl infile";
	die " \n"	 ;
}

if ( ! open ITCM ,">","itcm_init.txt" ){
	die "Cant Open itcm_init.txt :$!" ;
}

if ( ! open COE ,">","itcm_init.coe" ){
	die "Cant Open itcm_init.coe :$!" ;
}

if ( ! open VIVADO ,">","vivado_sram_64x65536.mif" ){
	die "Cant Open vivado_init.mif :$!" ;
}







	 printf COE "MEMORY_INITIALIZATION_RADIX=16;\n";
	 printf COE "MEMORY_INITIALIZATION_VECTOR=\n";

$dd = 0 ;

while ( <>){
	chomp;
	if (/^\s*\w*\s*:\s*(\w*);/){
	 printf ITCM "$1\n";
	 printf COE  "$1,\n";
		$dd = hex($1);
	 printf VIVADO  "%064b\n",$dd;
	
	}
	
}

close ITCM;
close COE;
close VIVADO;

