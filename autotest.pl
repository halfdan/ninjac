#!/usr/bin/perl
use strict;
use warnings;
use File::Copy;
use File::Basename;
use IO::Handle;
use Term::ANSIColor qw(:constants);

############################################################
## autotest.pl - for ninja oo compiler
## =========================================================
## author:                         Fabian Müller
##
## compiliert alle testfiles aus test_fm und vergleicht sie
## erwartetem output
############################################################

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
## global variables and constants
#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

my $author='Fabian Mueller';
my $email='fabian.mueller@mni.fh-giessen.de';
my $version='1.0';
my $created='2011-01-15';

my $pwd=`pwd`;                                         # current directory
chomp($pwd);
my $ref_dir=$pwd.'/test_fm';                          # location of all references (files and output)
my @testfiles = glob($ref_dir.'/*.nj');
my $failed = 0;
my $won = 0;

sub Autotest_Main() {
    # alle Testfiles compilieren
    #use Data::Dumper;
    #print Dumper(@testfiles);
    foreach my $testfile (@testfiles) {
#        print $testfile . "\n";
        my $basename = basename($testfile, ".nj");
        my $refoutput = "$ref_dir/$basename.ref";
        my $tmpout = "$ref_dir/$basename.tmp";
        my $output = "$ref_dir/$basename.out";
        open my $oldout, ">&STDOUT" or die "Can't duplicate STDOUT: $!";
#        close STDOUT;
        
        # redirect STDOUT
        open(TMPOUT, ">", "$tmpout") or die("Konnte '$tmpout' nicht öffnen: $!");
        STDOUT->fdopen(\*TMPOUT, 'w') or die("$!");

        # compile testfile
        system("./njc $testfile");

        # restore STDOUT
        STDOUT->fdopen(\*$oldout, 'w') or die("$!");
        close TMPOUT;

        # filter tmp-Output
        open(TMPOUT, "<", $tmpout) or die("Konnte '$tmpout' nicht öffnen: $!");
        open(OUTPUT, ">", $output) or die("Konnte '$output' nicht öffnen: $!");
        foreach my $line (<TMPOUT>) {
            next if $line =~ /Compiling/;
            $line =~ s/(.*?)(\/.*\/)(.*)(\'.*)/$1$3$4/;
            print OUTPUT $line;
        }
        close(TMPOUT);
        close(OUTPUT);

        # compare if diff was successfully
        print "testing $basename...\t";
        if ( 0 == system("diff $refoutput $output > /dev/null") ) {
            print GREEN, "PASS" . "\n", RESET;
            unlink($tmpout);
            unlink($output);
            $won++;
        } else {
            print RED, "FAIL" . "\n", RESET;
            $failed++;
        }
    }
    print "\n";
    print "Tests passed: $won\n";
    print "Tests failed: $failed\n";
}

Autotest_Main();















