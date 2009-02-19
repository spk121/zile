-- Turn texinfo markup into plain text
--
-- Copyright (c) 2009 Free Software Foundation, Inc.
-- Copyright (c) 2007 Reuben Thomas.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.


function texi (s)
  s = string.gsub (s, "@i{([^}]+)}", function (s) return string.upper (s) end)
  s = string.gsub (s, "@kbd{([^}]+)}", "%1")
  s = string.gsub (s, "@samp{([^}]+)}", "%1")
  s = string.gsub (s, "@itemize%s[^\n]*\n", "")
  s = string.gsub (s, "@end%s[^\n]*\n", "")
  return s
end
