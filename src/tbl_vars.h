/*	$Id: tbl_vars.h,v 1.27 2006/07/25 12:27:02 rrt Exp $	*/

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
 * The third column specifies whether the variable becomes local when set.
 * The fifth column specifies the variable documentation.
 */

X("auto-fill-mode",			"b", "nil", FALSE, "\
If enabled, the Auto Fill Mode is automatically enabled.")
X("backup-directory",			"", "~/.backup", FALSE, "\
Specify target backup directory.  Directory must be existent.\n\
This value is used only when the `backup-with-directory' value is true.")
X("backup-method",			"", "simple", FALSE, "\
Specify the file backup method.\n\
\n\
Possible values are: none and simple.\n\
\n\
 - If `none' is specified, Zile will not create backup files.\n\
 - If `simple' is specified, Zile will create a backup file with a\n\
   tilde `~' appended to the name (e.g.: on saving `foo.c' it will\n\
   create the backup `foo.c~').")
X("backup-with-directory",		"b", "nil", FALSE, "\
If enabled Zile will backup files to a user specified directory;\n\
the directory must exist and must be specified in the\n\
variable `backup-directory'.")
X("beep",				"b", "t", FALSE, "\
If enabled, a sound will be emitted on any error.")
X("case-fold-search",			"b", "t", TRUE, "\
Non-nil means searches ignore case.")
X("case-replace",			"b", "t", FALSE, "\
Non-nil means `query-replace' should preserve case in replacements.")
X("fill-column",			"", "72", TRUE, "\
Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes buffer-local when set in any fashion.")
X("highlight-nonselected-windows",	"b", "nil", FALSE, "\
If enabled, highlight region even in nonselected windows.")
X("indent-tabs-mode",			"b", "nil", TRUE, "\
If enabled, insert-tab inserts `real' tabs; otherwise, it always inserts\n\
spaces.")
X("kill-whole-line",			"b", "nil", FALSE, "\
If enabled, `kill-line' with no arg at beg of line kills the whole line.")
X("skip-splash-screen",			"b", "nil", FALSE, "\
If enabled, the splash screen at startup will be avoided.")
X("standard-indent",			"", "4", FALSE, "\
Default number of columns for margin-changing functions to indent.")
X("tab-always-indent",			"b", "t", FALSE, "\
If enabled, the TAB key always indents, otherwise it inserts a `real'\n\
tab when point is in the left margin or the line's indentation.")
X("tab-width",				"", "8", TRUE, "\
Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes buffer-local when set in any fashion.")
X("transient-mark-mode",		"b", "t", FALSE, "\
If enabled, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
