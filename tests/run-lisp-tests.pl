# run-lisp-tests
#
# Copyright (c) 2009-2011 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
#
# GNU Zile is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Zile is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Zile; see the file COPYING.  If not, write to the
# Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
# MA 02111-1301, USA.

use strict;
use warnings;

use File::Basename;
use File::Copy;


# N.B. Tests that use execute-kbd-macro must note that keyboard input
# is only evaluated once the script has finished running.

# srcdir and builddir are defined in the environment for a build
my $abs_srcdir = $ENV{abs_srcdir} || `pwd`;
chomp $abs_srcdir;
my $srcdir = $ENV{srcdir} || ".";
my $builddir = $ENV{builddir} || ".";

my $zile_pass = 0;
my $zile_fail = 0;
my $emacs_pass = 0;
my $emacs_fail = 0;

# If TERM is not set to a terminal type, choose a default
if (-z $ENV{TERM} || $ENV{TERM} eq "unknown") {
  $ENV{TERM} = "vt100";
}

for my $test (@ARGV) {				# ../tests/zile-only/backward_delete_char.el
  $test =~ s/\.el$//;				# ../tests/zile-only/backward_delete_char
  my $name = basename($test);			# backward_delete_char
  my $edit_file = "$test.input";
  $edit_file =~ s/$srcdir/$builddir/e;		# ./tests/zile-only/backward_delete_char.input
  my $lisp_file = "$test.el";
  $lisp_file =~ s/$srcdir/$abs_srcdir/e;

  my @args = ("--no-init-file", $edit_file, "--load", $lisp_file);

  if ($ENV{EMACSPROG}) {
    copy("$srcdir/tests/test.input", $edit_file);
    chmod 0644, $edit_file;
    if (system($ENV{EMACSPROG}, "--quick", "--batch", @args) == 0) {
      if (system("diff", "$test.output", $edit_file) == 0) {
        $emacs_pass++;
        unlink $edit_file, "$edit_file~";
      } else {
        print STDERR "Emacs $name failed to produce correct output\n";
        $emacs_fail++;
      }
    } else {
      print STDERR "Emacs $name failed to run with error code $?\n";
      $emacs_fail++;
    }
  }

  copy("$srcdir/tests/test.input", $edit_file);
  chmod 0644, $edit_file;
  my @zile_cmd = ("$builddir/src/zile");
  unshift @zile_cmd, (split ' ', $ENV{VALGRIND}) if $ENV{VALGRIND};
  if (system(@zile_cmd, @args) == 0) {
    if (system("diff", "$test.output", $edit_file) == 0) {
      $zile_pass++;
      unlink $edit_file, "$edit_file~";
    } else {
      print STDERR "Zile $name failed to produce correct output\n";
      $zile_fail++;
    }
  } else {
    print STDERR "Zile $name failed to run with error code $?\n";
    $zile_fail++
  }
}

print STDERR "Zile: $zile_pass pass(es) and $zile_fail failure(s)\n";
print STDERR "Emacs: $emacs_pass pass(es) and $emacs_fail failure(s)\n";

exit $zile_fail + $emacs_fail;
