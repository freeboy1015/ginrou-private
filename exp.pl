#!/usr/bin/perl

use 5.010;

$dir = 'expDir/';
chdir "$dir" or die "cannnot chdir to ./expDir $!";

opendir DIR, "./";
@jpgFiles = grep{ /.*\d\.JPG$/} readdir DIR;

@roi = qw\359 396 1272 829\;
foreach (@jpgFiles) {
  $in = $out = $_;
  $out =~ s/.JPG/_little\.JPG/;
  system("./imageResize.out $in $out @roi") unless -e $out;
  $_ = $out;
}
closedir DIR;
print "@jpgFiles\n";


for (1..2) {
  $inRight = shift @jpgFiles;
  $inLeft = shift @jpgFiles;
  $apLeft = 'Zhou2011.png';
  $apRight = 'Zhou2011.png';
  @pL = qw\0.25 0.0\;
  @pR = qw\0.25 0.0\;
  $debugDir = "debugImages$_";
  mkdir( $debugDir ,0755) unless( -d $debugDir );
  $dispmap = 'DisparityMap.png';
  $deblurred = 'deblurred.png';
  @args = ();
  push @args, $inLeft;
  push @args, $inRight;
  push @args, $apLeft;
  push @args, $apRight;
  push @args, @pL;
  push @args, @pR;
  push @args, $dir.$debugDir;
  push @args, $dispmap;
  push @args, $deblurred;


  print "input arguments are \n";
  @newArgs = ();
  foreach (@args) {
    $_ = $dir.$_ if(/.*\.[a-zA-z]/);
    push @newArgs, $_;
    print "$_\n";
  }

  chdir "../" or die "$!";
  system("./main.out @newArgs");

}
