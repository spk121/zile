-- Produce funcs.texi and tbl_funcs.lua
--
-- Copyright (c) 2009 Free Software Foundation, Inc.
-- Copyright (c) 2006, 2007 Reuben Thomas.
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
  name = "mkfuncs"
}

require "lib"
require "texinfo"
require "std"

dir = arg[1]
table.remove (arg, 1)

h1 = io.open ("funcs.texi", "w")
assert (h1)

h2 = io.open ("tbl_funcs.h", "w")
assert (h2)

h1:write ("@c Automatically generated file: DO NOT EDIT!\n")
h1:write ("@table @code\n")

h2:write ("/*\n")
h2:write (" * Automatically generated file: DO NOT EDIT!\n")
h2:write (" * " .. PACKAGE_NAME .. " command to C function bindings and docstrings.\n")
h2:write (" */\n")
h2:write ("\n")

local funcs = {}

for i in ipairs (arg) do
  if arg[i] then
    lines = io.lines (dir .. "/" .. arg[i])
    for l in lines do
      if string.sub (l, 1, 5) == "DEFUN" then
        local _, _, name = string.find (l, "%(\"(.-)\"")
        if name == nil or name == "" then
          die ("invalid DEFUN syntax `" .. l .. "'")
        end

        local interactive = string.sub (l, 1, 12) ~= "DEFUN_HIDDEN"

        local state = 0
        local doc = ""
        for l in lines do
          if state == 1 then
            if string.sub (l, 1, 3) == "+*/" then
              state = 0
              break
            end
            doc = doc .. l .. "\n"
          elseif string.sub (l, 1, 3) == "/*+" then
            state = 1
          end
        end

        if doc == "" then
          die ("no docstring for " .. name)
        elseif state == 1 then
          die ("unterminated docstring for " .. name)
        end

        table.insert (funcs, {name = name,
                              doc = doc,
                              interactive = interactive})
      end
    end
  end
end

table.sort (funcs,
            function (a, b)
              return a.name < b.name
            end)

for _, f in pairs (funcs) do
  local cdoc = string.gsub (f.doc, "\n", "\\n\\\n")
  cdoc = texi (cdoc)
  h1:write ("@item " .. f.name .. "\n" .. f.doc)

  local cname = string.gsub (f.name, "-", "_")
  h2:write ("X(\"" .. f.name .. "\", " .. cname .. ", " ..
            (f.interactive and "true" or "false") .. ", \"\\\n")
  h2:write (cdoc)
  h2:write ("\")\n")
end

h1:write ("@end table\n")
h1:close ()

h2:close ()
