# Produce dotzile.sample
#
# Copyright (c) 2010 Free Software Foundation, Inc.
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

use Zile;

open VARS, ">src/tbl_vars.h" or die;
open SAMPLE, ">src/dotzile.sample" or die;

print SAMPLE <<EOF;
;;;; .$ENV{PACKAGE} configuration

;; Rebind keys with:
;; (global-set-key "key" 'func)

EOF

# Don't note where the contents of this file comes from or that it's
# auto-generated, because it's ugly in a user configuration file.

sub escape_for_C {
  my ($s) = @_;
  $s =~ s/\n/\\n/g;
  $s =~ s/\"/\\\"/g;
  return $s;
}

sub comment_for_lisp {
  my ($s) = @_;
  $s =~ s/\n/\n; /g;
  return $s;
}

open IN, "<$ARGV[0]";
while (<IN>) {
  if (/^X \(/) {
    eval $_ or die "Error evaluating:\n$_\n";
    my ($name, $defval, $local_when_set, $doc) = @xarg;
    $doc = texi($doc);

    print VARS "X (\"$name\", \"$defval\", " .
                     ($local_when_set ? "true" : "false") . ", \"" .
                       escape_for_C($doc) . "\")\n";

    print SAMPLE "; " . comment_for_lisp($doc) . "\n" .
      "; Default value is $defval.\n" .
        "(setq $name $defval)\n\n";
  }
}
