/*	$Id: tbl_vars.h,v 1.24 2006/07/15 01:44:57 rrt Exp $	*/

/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you do any modification, please remember to keep in sync with the
 * documentation in the `../doc/zile.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("t" or "nil");
 *   - "" (empty string) for non-fixed format.
 * The third column specifies the default value.
 * The forth column specifies the variable documentation.
 */

X("alternative-bindings",		"b", "nil", "\
Remap the help functions (bound by default to `C-h') to `M-h'.  This may\n\
be useful when `C-h' is already bound to Backspace or Delete.\n\
\n\
Please note that changing this variable at run-time has no effect; you\n\
need instead to modify your `~/.zile' configuration file and restart Zile.")
X("auto-fill-mode",			"b", "nil", "\
If enabled, the Auto Fill Mode is automatically enabled.")
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
X("backup-with-directory",		"b", "nil", "\
If enabled Zile will backup files to a user specified directory;\n\
the directory must exist and must be specified in the\n\
variable `backup-directory'.")
X("beep",				"b", "t", "\
If enabled, a sound will be emitted on any error.")
X("case-replace",			"b", "t", "\
Non-nil means `query-replace' should preserve case in replacements.")
X("fill-column",			"", "72", "\
Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes buffer-local when set in any fashion.")
X("indent-tabs-mode",			"b", "nil", "\
If enabled, insert-tab inserts `real' tabs; otherwise, it always inserts\n\
spaces.")
X("highlight-nonselected-windows",	"b", "nil", "\
If enabled, highlight region even in nonselected windows.")
X("kill-whole-line",			"b", "nil", "\
If enabled, `kill-line' with no arg at beg of line kills the whole line.")
X("skip-splash-screen",			"b", "nil", "\
If enabled, the splash screen at startup will be avoided.")
X("standard-indent",			"", "4", "\
Default number of columns for margin-changing functions to indent.")
X("tab-always-indent",			"b", "t", "\
If enabled, the TAB key always indents, otherwise it inserts a `real'\n\
tab when point is in the left margin or the line's indentation.")
X("tab-width",				"", "8", "\
Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes buffer-local when set in any fashion.")
X("transient-mark-mode",		"b", "t", "\
If enabled, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
