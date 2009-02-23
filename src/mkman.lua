-- Produce zile.1
--
-- Copyright (c) 2009 Free Software Foundation, Inc.
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

prog = {
  name = "mkman"
}

require "lib"
require "texinfo"
require "std"

h = io.open ("zile.1", "w")
assert (h)

-- FIXME: Put the following in a file
h:write (".\\\" Automatically generated file: DO NOT EDIT!\n" ..
         ".\\\" Generated from tbl_opts.h.\n" ..
         ".TH ZILE \"1\" \"February 2009\" \"Zile 2.3.1\" \"User Commands\"\n" ..
         ".SH NAME\n" ..
         "zile \\- Zile Is Lossy Emacs\n" ..
         ".SH SYNOPSIS\n" ..
         ".B zile\n" ..
         "[\\fIOPTION-OR-FILENAME\\fR]...\n" ..
         ".SH DESCRIPTION\n" ..
         ".PP\n" ..
         "Zile is a lightweight Emacs clone that provides a subset of Emacs's\n" ..
         "functionality suitable for basic editing.\n" ..
         "\n")

local required_argument = true

local vars = {}
for l in io.lines (arg[1]) do
  if string.find (l, "^[DOA] %(") then
    assert (loadstring (l)) ()
    local type = string.sub (l, 1, 1)
    if type == "D" then
      local text = unpack (xarg)
      if text ~= "" then
        h:write (text .. "\n")
      else
        h:write (".PP\n")
      end
    elseif type == "O" then
      local longname, shortname, arg, argstring, doc = unpack (xarg)
      h:write (".TP\n" ..
               "\\fB\\-\\-" .. longname .. "\\fR")
      if arg then
        h:write (", \\fB\\-" .. shortname .. "\\fR")
      end
      h:write (" ")
      if argstring ~= "" then
        h:write ("\\fI" .. argstring .. "\\fR")
        doc = string.gsub (doc, argstring, "\\fI" .. argstring .. "\\fR")
      end
      h:write ("\n")
      h:write (doc .. "\n")
    elseif type == "A" then
      local argstring, doc = unpack (xarg)
      doc = string.gsub (doc, argstring, "\\fI" .. argstring .. "\\fR")
      h:write (".TP\n" ..
               "\\fB" .. argstring .. "\\fR\n" ..
               doc .. "\n")
    end
  end
end

-- FIXME: Put the following in a file
h:write (".PP\n" ..
         "Exit status is 0 if OK, 1 if it cannot start up, for example because\n" ..
         "of an invalid command-line argument, and 2 if it crashes or runs out\n" ..
         "of memory.\n" ..
         ".SH FILES\n" ..
         "~/.zile \\(em user's Zile init file\n" ..
         "\n" ..
         pkgdatadir .."/dotzile.sample \\(em sample init file which also\n" ..
         "documents the default settings and contains some useful tips.\n" ..
         ".SH AUTHORS\n" ..
         "Zile was written by Sandro Sigala, David A. Capello and Reuben Thomas.\n" ..
         "The Lisp interpreter is based on code by Scott Lawrence.\n" ..
         ".SH \"REPORTING BUGS\"\n" ..
         "Please report bugs to <bug-zile@gnu.org>.\n" ..
         ".SH COPYRIGHT\n" ..
         "Copyright \\(co 2009 Free Software Foundation, Inc.\n" ..
         "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n" ..
         ".SH SEE ALSO\n" ..
         ".BR emacs (1)\n")

h:close ()
