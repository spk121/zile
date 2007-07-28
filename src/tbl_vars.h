/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you do any modification, please remember to keep in sync with the
 * documentation in the `../doc/zile.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the default value.
 * The third column specifies whether the variable becomes local when set.
 * The fourth column specifies the variable documentation.
 */

X("auto-fill-mode",			"nil", FALSE, "\
If non-nil, Auto Fill Mode is automatically enabled.")
X("backup-directory",			"nil", FALSE, "\
The directory for backup files, which must exist.\n\
If this variable is nil, the backup is made in the original file's\n\
directory.\n\
This value is used only when `make-backup-files' is `t'.")
X("case-fold-search",			"t", TRUE, "\
Non-nil means searches ignore case.")
X("case-replace",			"t", FALSE, "\
Non-nil means `query-replace' should preserve case in replacements.")
X("fill-column",			"72", TRUE, "\
Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes buffer-local when set in any fashion.")
X("highlight-nonselected-windows",	"nil", FALSE, "\
If non-nil, highlight region even in nonselected windows.")
X("indent-tabs-mode",			"nil", TRUE, "\
If non-nil, insert-tab inserts \"real\" tabs; otherwise, it always inserts\n\
spaces.")
X("inhibit-splash-screen",		"nil", FALSE, "\
Non-nil inhibits the startup screen.\n\
It also inhibits display of the initial message in the `*scratch*' buffer.")
X("kill-whole-line",			"nil", FALSE, "\
If non-nil, `kill-line' with no arg at beg of line kills the whole line.")
X("make-backup-files",			"t", FALSE, "\
Non-nil means make a backup of a file the first time it is saved.\n\
This is done by appending `~' to the file name. ")
X("ring-bell",				"t", FALSE, "\
Non-nil means ring the terminal bell on any error.")
X("standard-indent",			"4", FALSE, "\
Default number of columns for margin-changing functions to indent.")
X("tab-always-indent",			"t", FALSE, "\
If non-nil, the TAB key always indents, otherwise it inserts a \"real\"\n\
tab when point is in the left margin or in the line's indentation.")
X("tab-width",				"8", TRUE, "\
Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes buffer-local when set in any fashion.")
X("transient-mark-mode",		"t", FALSE, "\
If non-nil, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
