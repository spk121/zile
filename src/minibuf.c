/* Minibuffer facility functions
   Copyright (c) 1997-2004 Sandro Sigala.  All rights reserved.

   This file is part of Zile.

   Zile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Zile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with Zile; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*      $Id: minibuf.c,v 1.20 2004/05/20 22:34:50 rrt Exp $     */

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zile.h"
#include "extern.h"
#include "agetcwd.h"

static History files_history;

/*--------------------------------------------------------------------------
 * Minibuffer wrapper functions.
 *--------------------------------------------------------------------------*/

char *minibuf_format(const char *fmt, va_list ap)
{
        char *buf;
        vasprintf(&buf, fmt, ap);
        vsprintf(buf, fmt, ap);
        return buf;
}

void free_minibuf(void)
{
        free_history_elements(&files_history);
}

/*
 * Write the specified string in the minibuffer.
 */
void minibuf_write(const char *fmt, ...)
{
        va_list ap;
        char *buf;

        va_start(ap, fmt);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        term_minibuf_write(buf);
        free(buf);
}

/*
 * Write the specified error string in the minibuffer.
 */
void minibuf_error(const char *fmt, ...)
{
        va_list ap;
        char *buf;

        va_start(ap, fmt);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        term_minibuf_write(buf);
        free(buf);

        ding();
}

/*
 * Read a string from the minibuffer.
 */
char *minibuf_read(const char *fmt, const char *value, ...)
{
        va_list ap;
        char *buf, *p;

        va_start(ap, value);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        if (value != NULL)
                p = term_minibuf_read(buf, value, NULL, NULL);
        else
                p = term_minibuf_read(buf, "", NULL, NULL);
        free(buf);

        return p;
}

/*
 * Read a directory from the minibuffer.
 * The returned buffer must be freed by the caller.
 */
char *minibuf_read_dir(const char *fmt, const char *value, ...)
{
        va_list ap;
        char *buf, *p;
        Completion *cp;
        astr dir, fname, rbuf;

        va_start(ap, value);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        rbuf = astr_new();
        agetcwd(rbuf);
        dir = astr_new();
        fname = astr_new();
        expand_path(value, astr_cstr(rbuf), dir, fname);
        astr_delete(rbuf);
        astr_cat_cstr(dir, astr_cstr(fname));
        rbuf = compact_path(astr_cstr(dir));
        astr_delete(dir);
        astr_delete(fname);

        cp = new_completion(TRUE);
        p = term_minibuf_read(buf, astr_cstr(rbuf), cp, &files_history);
        free_completion(cp);
        astr_delete(rbuf);
        free(buf);

        if (p != NULL) {
                /* Add history element. */
                add_history_element(&files_history, p);

                rbuf = astr_new();
                agetcwd(rbuf);
                dir = astr_new();
                fname = astr_new();

                expand_path(p, astr_cstr(rbuf), dir, fname);
                astr_cpy(rbuf, dir);
                astr_cat(rbuf, fname);

                astr_delete(dir);
                astr_delete(fname);
                p = zstrdup(astr_cstr(rbuf));
                astr_delete(rbuf);
        }

        return p;
}

static int minibuf_read_forced(const char *fmt, const char *errmsg,
                        Completion *cp, ...)
{
        va_list ap;
        char *buf, *p;

        va_start(ap, cp);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        for (;;) {
                p = term_minibuf_read(buf, "", cp, NULL);
                if (p == NULL) { /* Cancelled. */
                        free(buf);
                        return -1;
                } else {
                        char *s;
                        astr as = astr_new();
                        /* Complete partial words if possible. */
                        astr_cpy_cstr(as, p);
                        if (cp->try(cp, as) == COMPLETION_MATCHED)
                                p = cp->match;
                        astr_delete(as);
                        for (s = alist_first(cp->completions); s != NULL;
                             s = alist_next(cp->completions))
                                if (!strcmp(p, s)) {
                                        free(buf);
                                        return alist_current_idx(cp->completions);
                                }
                        minibuf_error(errmsg);
                        waitkey(2 * 1000);
                }
        }

        assert(0);
        return FALSE;
}

int minibuf_read_yesno(const char *fmt, ...)
{
        va_list ap;
        char *buf;
        Completion *cp;
        int retvalue;

        va_start(ap, fmt);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        cp = new_completion(FALSE);
        alist_append(cp->completions, zstrdup("yes"));
        alist_append(cp->completions, zstrdup("no"));

        retvalue = minibuf_read_forced(buf, "Please answer yes or no.", cp);
        if (retvalue != -1) {
                /* The completions may be sorted by the minibuf completion
                   routines. */
                if (!strcmp(alist_at(cp->completions, retvalue), "yes"))
                        retvalue = TRUE;
                else
                        retvalue = FALSE;
        }
        free_completion(cp);
        free(buf);

        return retvalue;
}

int minibuf_read_boolean(const char *fmt, ...)
{
        va_list ap;
        char *buf;
        Completion *cp;
        int retvalue;

        va_start(ap, fmt);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        cp = new_completion(FALSE);
        alist_append(cp->completions, zstrdup("true"));
        alist_append(cp->completions, zstrdup("false"));

        retvalue = minibuf_read_forced(buf, "Please answer true or false.", cp);
        if (retvalue != -1) {
                /* The completions may be sorted by the minibuf completion
                   routines. */
                if (!strcmp(alist_at(cp->completions, retvalue), "true"))
                        retvalue = TRUE;
                else
                        retvalue = FALSE;
        }
        free_completion(cp);
        free(buf);

        return retvalue;
}

char *minibuf_read_color(const char *fmt, ...)
{
        va_list ap;
        char *buf;
        Completion *cp;
        int retvalue;
        unsigned int i;
        char *valid[] = {
                "black", "blue", "cyan", "green",
                "magenta", "red", "white", "yellow",
                "light-black", "light-blue", "light-cyan", "light-green",
                "light-magenta", "light-red", "light-white", "light-yellow"
        };

        va_start(ap, fmt);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        cp = new_completion(FALSE);
        for (i = 0; i < sizeof valid / sizeof(char *); ++i)
                alist_append(cp->completions, zstrdup(valid[i]));

        retvalue = minibuf_read_forced(buf, "Invalid color name.", cp);
        if (retvalue != -1) {
                /* The completions may be sorted by the minibuf completion
                   routines. */
                for (i = 0; i < sizeof valid / sizeof (char *); ++i)
                        if (!strcmp(alist_at(cp->completions, retvalue), valid[i])) {
                                retvalue = i;
                                break;
                        }
        }
        free_completion(cp);
        free(buf);

        if (retvalue == -1)
                return NULL;
        return valid[retvalue];
}

/*
 * Read a string from the minibuffer using a completion.
 */
char *minibuf_read_completion(const char *fmt, char *value, Completion *cp, History *hp, ...)
{
        va_list ap;
        char *buf, *p;

        va_start(ap, hp);
        buf = minibuf_format(fmt, ap);
        va_end(ap);

        p = term_minibuf_read(buf, value, cp, hp);
        free(buf);

        return p;
}

/*
 * Clear the minibuffer.
 */
void minibuf_clear(void)
{
        term_minibuf_clear();
}
