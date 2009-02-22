-- Produce vars.texi and dotzile.sample
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
require "std"

h1 = io.open ("vars.texi", "w")
assert (h1)

h2 = io.open ("dotzile.sample", "w")
assert (h2)

h3 = io.open ("dotzile.texi", "w")
assert (h3)

h1:write ("@c Automatically generated file: DO NOT EDIT!\n")
h1:write ("@table @code\n")

h2:write ("; ." .. PACKAGE .. " configuration\n\n")

h3:write ("@c Automatically generated file: DO NOT EDIT!\n")
h3:write ("@setfilename dotzile.info\n")
h3:write ("@example\n")

local vars = {}
for l in io.lines (arg[1]) do
  if string.find (l, "^X %(") then
    assert (loadstring (l)) ()
    local name, defval, local_when_set, doc = unpack (xarg)
    if doc == "" then
      die ("empty docstring for " .. name)
    end

    h2:write ("; " .. string.gsub (texi (doc), "(\n", "\n; ") .. "\n")
    h2:write ("; Default value is " .. defval .. ".\n")
    h2:write ("(setq " .. name .. " " .. defval .. ")\n")
    h2:write ("\n")

    -- Record lines so they can be sorted for vars.texi
    table.insert (vars, {name = name,
                         defval = defval,
                         local_when_set = local_when_set,
                         doc = doc})
  end
end
table.sort (vars,
            function (a, b)
              return a.name < b.name
            end)

for _, v in ipairs (vars) do
  h1:write ("@item " .. v.name .. "\n" .. v.doc .. "\n")
  if v.local_when_set then
    h1:write ("Automatically becomes buffer-local when set in any fashion.\n")
  end
  h1:write ("Default value is @samp{" .. v.defval .. "}.\n")
end

h1:write ("@end table\n")
h1:close ()

-- Add extra key rebindings to sample configuration file
h2:write ("; Rebind keys\n; (global-set-key \"key\" 'func)\n\n; Better bindings for when backspace generates C-h\n;(global-set-key \"\\BACKSPACE\"  'backward-delete-char)\n;(global-set-key \"\\C-h\"        'backward-delete-char)\n;(global-set-key \"\\M-:\"        'mark-paragraph)\n;(global-set-key \"\\M-hb\"       'list-bindings)\n;(global-set-key \"\\M-hd\"       'describe-function)\n;(global-set-key \"\\M-hf\"       'describe-function)\n;(global-set-key \"\\M-hF\"       'view-zile-FAQ)\n;(global-set-key \"\\M-hk\"       'describe-key)\n;(global-set-key \"\\M-hlr\"      'list-registers)\n;(global-set-key \"\\M-hs\"       'help-config-sample)\n;(global-set-key \"\\M-ht\"       'help-with-tutorial)\n;(global-set-key \"\\M-hw\"       'where-is)\n;(global-set-key \"\\M-hv\"       'describe-variable)\n")
h2:close ()

h3:write ("@end example\n")
h3:close ()
