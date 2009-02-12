-- Zee-specific library functions
--
-- Copyright (c) 2009 Free Software Foundation, Inc.
-- Copyright (c) 2006, 2007, 2008 Reuben Thomas.
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


-- Parse re-usable C headers
function X(...)
  xarg = {...}
end

-- Extract the docstrings from tbl_funcs.lua
docstring = {}
function def(t)
  local name, s
  for i, v in pairs(t) do
    if type(i) == "string" then
      name = i
      _G[name] = v
    elseif type(i) == "number" and i == 1 then
      s = v
    end
  end
  if name then
    docstring[name] = s
  end
end

-- Load a file of Lua, aborting if not found.
function require(f)
  local ok, err = pcall(dofile, f .. ".lua")
  if not ok then
    io.stderr:write(string.format(PACKAGE_NAME .. " is not properly installed: could not load file `%s'\n%s\n", f, err))
    die(1)
  end
end
