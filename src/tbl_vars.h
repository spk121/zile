/*	$Id: tbl_vars.h,v 1.3 2003/05/06 22:28:42 rrt Exp $	*/

/*
 * Copyright (c) 1997-2002 Sandro Sigala.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you do any modification, please remember to keep in sync with the
 * documentation in the `../doc/zile.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("true" or "false");
 *   - "c" for color ("black", "white", "yellow", etc.);
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
#if ENABLE_NONTEXT_MODES
X("auto-font-lock",			"b", "true", "\
Automatically turn on Font Lock Mode when a C/C++ source file or shell\n\
script is opened.")
X("auto-font-lock-refresh",		"b", "true", "\
If enabled, the file will be reparsed by font lock on every C-l press.\n\
This may be disabled on slow machines.")
#endif
X("backup-directory",			"", "~/.backup", "\
Specify target backup directory.  Directory must be existent.\n\
This value is used only when the `backup-with-directory' value is true.")
X("backup-method",			"", "simple", "\
Specify the file backup method.\n\
\n\
Possible values are: none, simple and revision.\n\
\n\
 - If `none' is specified, Zile will not create backup files.\n\
 - If `simple' is specified, Zile will create a backup file with a\n\
   tilde `~' appended to the name (e.g.: on saving `foo.c' it will\n\
   create the backup `foo.c~').\n\
 - If `revision' is specified, Zile will create a new backup file on\n\
   each file saving preserving the old backups of the original file\n\
   (e.g.: on saving `foo.c' it will create the backup `foo.c~1~', then\n\
   on next save `foo.c~2~', etc.).")
X("backup-with-directory",		"b", "false", "\
If enabled Zile will backup files to a user specified directory;\n\
the directory must exist and must be specified in the\n\
variable `backup-directory'.")
X("beep",				"b", "true", "\
If enabled, a sound will be emitted on any error.")
X("colors",				"b", "true", "\
If your terminal supports colors, you should leave this enabled.")
X("display-time",			"b", "true", "\
If enabled the time is displayed in the status line.")
X("display-time-format",		"", "%I:%M%p", "\
The format of the displayed time in the status line.\n\
\n\
Conversion specifiers are introduced by a `%' character, and are replaced\n\
in the time format string as follows:\n\
\n\
   %a	The abbreviated weekday name according to the current locale.\n\
   %A	The full weekday name according to the current locale.\n\
   %b	The abbreviated month name according to the current locale.\n\
   %B	The full month name according to the current locale.\n\
   %c	The preferred date and time representation for the current locale.\n\
   %d	The day of the month as a decimal number (range 01 to 31).\n\
   %H	The hour as a decimal number using a 24-hour clock (range 00 to 23).\n\
   %I	The hour as a decimal number using a 12-hour clock (range 01 to 12).\n\
   %j	The day of the year as a decimal number (range 001 to 366).\n\
   %m	The month as a decimal number (range 01 to 12).\n\
   %M	The minute as a decimal number.\n\
   %p	Either `am' or `pm' according to the given time value, or the\n\
	corresponding strings for the current locale.\n\
   %S	The second as a decimal number.\n\
   %U	The week number of the current year as a decimal number, starting\n\
	with the first Sunday as the first day of the first week.\n\
   %W	The week number of the current year as a decimal number, starting\n\
	with the first Monday as the first day of the first week.\n\
   %w	The day of the week as a decimal, Sunday being 0.\n\
   %x	The preferred date representation for the current locale without\n\
	the time.\n\
   %X	The preferred time representation for the current locale without\n\
	the date.\n\
   %y	The year as a decimal number without a century (range 00 to 99).\n\
   %Y	The year as a decimal number including the century.\n\
   %Z	The time zone or name or abbreviation.\n\
   %%	A literal `%' character.")
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
#if ENABLE_NONTEXT_MODES
X("font-character",			"c", "green", "\
The color used in Font Lock Mode for characters.")
X("font-character-delimiters",		"c", "white", "\
The color used in Font Lock Mode for character delimiters.")
X("font-comment",			"c", "red", "\
The color used in Font Lock Mode for comments.")
X("font-directive",			"c", "blue", "\
The color used in Font Lock Mode for preprocessor directives.")
X("font-identifier",			"c", "white", "\
The color used in Font Lock Mode for identifiers.")
X("font-keyword",			"c", "magenta", "\
The color used in Font Lock Mode for keywords.")
#if ENABLE_MAIL_MODE
X("font-mail1",				"c", "green", "\
The color used in Font Lock / Mail Mode for quoting.")
X("font-mail2",				"c", "red", "\
The color used in Font Lock / Mail Mode for quoting.")
X("font-mail3",				"c", "cyan", "\
The color used in Font Lock / Mail Mode for quoting.")
X("font-mail4",				"c", "magenta", "\
The color used in Font Lock / Mail Mode for quoting.")
X("font-mail5",				"c", "blue", "\
The color used in Font Lock / Mail Mode for quoting.")
#endif
X("font-number",			"c", "cyan", "\
The color used in Font Lock Mode for numbers.")
X("font-other",				"c", "white", "\
The color used in Font Lock Mode for the text.")
X("font-string",			"c", "green", "\
The color used in Font Lock Mode for strings.")
X("font-string-delimiters",		"c", "white", "\
The color used in Font Lock Mode for string delimiters.")
#endif /* ENABLE_NOTEXT_MODES */
X("highlight-region",			"b", "true", "\
If enabled, highlight the current region with reversed colors.")
#if ENABLE_MAIL_MODE
X("mail-mode-auto-fill",		"b", "false", "\
If enabled, the Auto Fill Mode is automatically enabled in Mail Mode.")
X("mail-quoting-char",			"", ">", "\
The character prepended to quoted text in mails.\n\
Used by Font Lock in Mail Mode.")
#endif
X("novice-level",			"b", "true", "\
Enable this if you are novice to Emacs in general.\n\
Disabling this variable the Mini Help window and the message in\n\
the scratch buffer will be disabled.")
X("revisions-delete",			"", "ask", "\
Specify the action when the number of revisions exceed the value\n\
specified in `revisions-kept'.\n\
\n\
- If `ask' is specified, ask confirmation to delete.\n\
- If `noask' is specified, delete excess backup versions silently.\n\
- Any other value prevents any trimming.")
X("revisions-kept",			"", "5", "\
Number of oldest versions to keep when a new numbered backup is made\n\
(used only when `backup-method' is set to `revision').")
X("show-eob-marker",			"b", "true", "\
If enabled, a marker will be displayed at the end of the buffer.")
X("skip-splash-screen",			"b", "false", "\
If enabled, the splash screen at startup will be avoided.")
X("status-line-color",			"c", "cyan", "\
The color of the status line.")
X("tab-width",				"", "8", "\
The default tabulation width.")
X("text-mode-auto-fill",		"b", "false", "\
If enabled, the Auto Fill Mode is automatically enabled in Text Mode.")
