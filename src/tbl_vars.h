/*	$Id: tbl_vars.h,v 1.14 2004/10/08 13:30:45 rrt Exp $	*/

/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you do any modification, please remember to keep in sync with the
 * documentation in the `../doc/zile.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("true" or "false");
 *   - "" (empty string) for non-fixed format.
 * The third column specifies the default value.
 * The forth column specifies the variable documentation.
 */

X("alternative-bindings",		"b", "false", "\
Remap the help functions (bound by default to `C-h') to `M-h'.  This may\n\
be useful when `C-h' is already bound to Backspace or Delete.\n\
\n\
Please note that changing this variable at run-time has no effect; you\n\
need instead to modify your `~/.zilerc' configuration file and restart Zile.")
X("backup-directory",			"", "~/.backup", "\
Specify target backup directory.  Directory must be existent.\n\
This value is used only when the `backup-with-directory' value is true.")
X("backup-method",			"", "simple", "\
Specify the file backup method.\n\
\n\
Possible values are: none and simple.\n\
\n\
 - If `none' is specified, Zile will not create backup files.\n\
 - If `simple' is specified, Zile will create a backup file with a\n\
   tilde `~' appended to the name (e.g.: on saving `foo.c' it will\n\
   create the backup `foo.c~').")
X("backup-with-directory",		"b", "false", "\
If enabled Zile will backup files to a user specified directory;\n\
the directory must exist and must be specified in the\n\
variable `backup-directory'.")
X("beep",				"b", "true", "\
If enabled, a sound will be emitted on any error.")
X("displayable-characters",		"", "0x20-0x7e", "\
Specify the set of characters that can be shown as-is on the screen.\n\
The characters not included in this set are shown as octal\n\
sequences (like \\261).\n\
\n\
The set syntax is the following:\n\
\n\
    set       ::= range | value ( ',' range | value )*\n\
    range     ::= value '-' value\n\
    value     ::= hex_value | oct_value | dec_value\n\
    hex_value ::= '0x'[0-9a-fA-F]+\n\
    oct_value ::= '0'[0-7]*\n\
    dec_value ::= [1-9][0-9]*\n\
\n\
For example, the following are valid sets:\n\
\n\
    0x20-0x7e			(standard English-only character set)\n\
    0x20-0x7e,0xa1-0xff		(typical European character set)\n\
    0-15,17,0xef-0xff\n\
    012,015-0x50		(two correct but useless character sets)")
X("expand-tabs",			"b", "false", "\
If disabled, Zile will insert hard tabs (the character `\\t'),\n\
otherwise it will insert spaces.")
X("fill-column",			"", "72", "\
The default fill column (used in Auto Fill Mode).")
X("highlight-nonselected-windows",	"b", "false", "\
If enabled, highlight region even in nonselected windows.")
X("novice-level",			"b", "true", "\
Enable this if you are new to Emacs in general.\n\
Setting this variable to false disables the Mini Help window and\n\
the message in the scratch buffer.")
X("skip-splash-screen",			"b", "false", "\
If enabled, the splash screen at startup will be avoided.")
X("standard-indent",			"", "4", "\
Default number of columns for margin-changing functions to indent.")
X("tab-width",				"", "8", "\
The default tabulation width.")
X("auto-fill-mode",			"b", "false", "\
If enabled, the Auto Fill Mode is automatically enabled.")
X("transient-mark-mode",		"b", "true", "\
If enabled, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
