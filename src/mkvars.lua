-- Produce dotzile.sample
--
-- Copyright (c) 2006, 2007, 2009 Free Software Foundation, Inc.
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
  name = "mkvars"
}

require "lib"
require "texinfo"

h1 = io.open ("tbl_vars.h", "w")
assert (h1)

h2 = io.open ("dotzile.sample", "w")
assert (h2)

h2:write ("; ." .. PACKAGE .. " configuration\n\n")
-- Don't note where the contents of this file comes from or that it's
-- auto-generated, because it's ugly in a user configuration file.

function escape_for_C (s)
  s = string.gsub (s, "\n", "\\n")
  s = string.gsub (s, "\"", "\\\"")
  return s
end

local vars = {}
for l in io.lines (arg[1]) do
  if string.find (l, "^X %(") then
    assert (loadstring (l)) ()
    local name, defval, local_when_set, doc = unpack (xarg)
    doc = texi (doc)

    h1:write ("X (\"" .. name .. "\", \"" .. defval .. "\", " ..
              tostring (local_when_set) .. ", \"" ..
              escape_for_C (doc) .. "\")\n")

    h2:write ("; " .. string.gsub (doc, "(\n", "\n; ") .. "\n")
    h2:write ("; Default value is " .. defval .. ".\n")
    h2:write ("(setq " .. name .. " " .. defval .. ")\n")
    h2:write ("\n")
  end
end

h1:close ()
h2:close ()
