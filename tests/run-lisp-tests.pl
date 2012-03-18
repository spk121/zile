# run-lisp-tests
#
# Copyright (c) 2009-2012 Free Software Foundation, Inc.
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
use File::Path;


# N.B. Tests that use execute-kbd-macro must note that keyboard input
# is only evaluated once the script has finished running.

# The following are defined in the environment for a build
my $abs_srcdir = $ENV{abs_srcdir} || `pwd`;
chomp $abs_srcdir;
my $srcdir = $ENV{srcdir} || ".";
my $builddir = $ENV{builddir} || ".";

my $zile_pass = 0;
my $zile_fail = 0;
my $emacs_pass = 0;
my $emacs_fail = 0;

my @zile_cmd = ("$builddir/src/zile");
unshift @zile_cmd, (split ' ', $ENV{VALGRIND}) if $ENV{VALGRIND};

sub run_test {
  my ($test, $name, $editor_name, $edit_file, @args) = @_;
  copy("$srcdir/tests/test.input", $edit_file);
  chmod 0644, $edit_file;
  if (system(@args) == 0) {
    if (system("diff", "$test.output", $edit_file) == 0) {
      unlink $edit_file, "$edit_file~";
      return 1;
    } else {
      print STDERR "$editor_name failed to produce correct output for test `$name'\n";
      return 0;
    }
  } else {
    print STDERR "$editor_name failed to run test `$name' with error code $?\n";
    return 0;
  }
}

for my $test (@ARGV) {				# ../tests/zile-only/backward_delete_char.el
  $test =~ s/\.el$//;				# ../tests/zile-only/backward_delete_char
  my $name = $test;
  $name =~ s|^\Q$srcdir/tests/||;		# zile-only/backward_delete_char
  my $edit_file = "$test.input";
  $edit_file =~ s/^\Q$srcdir/$builddir/e;	# ./tests/zile-only/backward_delete_char.input
  my $lisp_file = "$test.el";
  $lisp_file =~ s/^\Q$srcdir/$abs_srcdir/e;
  my @args = ("--no-init-file", $edit_file, "--load", $lisp_file);

  mkpath(dirname($edit_file));

  if ($ENV{EMACSPROG}) {
    if (run_test($test, $name, "Emacs", $edit_file, $ENV{EMACSPROG}, @args, "--batch", "--quick")) {
      $emacs_pass++;
    } else {
      $emacs_fail++;
      rename $edit_file, "$edit_file-emacs";
      rename "$edit_file~", "$edit_file-emacs~";
    }
  }

  if (run_test($test, $name, "Zile", $edit_file, @zile_cmd, @args)) {
    $zile_pass++;
  } else {
    $zile_fail++;
  }
}

print STDERR "Zile: $zile_pass pass(es) and $zile_fail failure(s)\n";
print STDERR "Emacs: $emacs_pass pass(es) and $emacs_fail failure(s)\n";

exit $zile_fail + $emacs_fail;
