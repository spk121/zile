# Zile-specific library functions
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


# Parse re-usable C headers
sub false { 0; }
sub true { 1; }
sub X { @xarg = @_; }
$D = $O = $A = \&X;

# Turn texinfo markup into plain text
sub texi {
  my ($s) = @_;
  $s =~ s/\@i\{([^}]+)}/uc($1)/ge;
  $s =~ s/\@kbd\{([^}]+)}/\1/g;
  $s =~ s/\@samp\{([^}]+)}/\1/g;
  $s =~ s/\@itemize%s[^\n]*\n//g;
  $s =~ s/\@end%s[^\n]*\n//g;
  return $s;
}
