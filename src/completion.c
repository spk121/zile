/* Completion facility functions
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

/*      $Id: completion.c,v 1.14 2005/01/09 18:11:13 rrt Exp $   */

#include "config.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <assert.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zile.h"
#include "extern.h"
#include "agetcwd.h"

/*----------------------------------------------------------------------
 *                       Completion functions
 *----------------------------------------------------------------------*/

/*
 * Allocate a new completion structure.
 */
Completion *new_completion(int fileflag)
{
        Completion *cp;

        cp = (Completion *)zmalloc(sizeof(Completion));
        memset(cp, 0, sizeof(Completion));

        cp->completions = list_new();
        cp->matches = list_new();

        if (fileflag) {
                cp->path = astr_new();
                cp->fl_dir = TRUE;
        }

        return cp;
}

/*
 * Dispose an completion structure.
 */
void free_completion(Completion *cp)
{
        list p;
        
        for (p = list_first(cp->completions); p != cp->completions; p = list_next(p))
                free(p->item);
        list_delete(cp->completions);
        list_delete(cp->matches);
        if (cp->fl_dir)
                astr_delete(cp->path);
        free(cp);
}

/*
 * Scroll completions up.
 */
void completion_scroll_up(Completion *cp)
{
        Window *wp, *old_wp = cur_wp;

        (void)cp;
        wp = find_window("*Completions*");
        assert(wp != NULL);
        set_current_window(wp);
        if (cur_bp->pt.n == cur_bp->num_lines || !FUNCALL(scroll_up))
                gotobob();
        set_current_window(old_wp);

        term_redisplay();
}

/*
 * Scroll completions down.
 */
void completion_scroll_down(Completion *cp)
{
        Window *wp, *old_wp = cur_wp;

        (void)cp;
        wp = find_window("*Completions*");
        assert(wp != NULL);
        set_current_window(wp);
        if (cur_bp->pt.n == 0 || !FUNCALL(scroll_down)) {
                gotoeob();
                resync_redisplay();
        }
        set_current_window(old_wp);

        term_redisplay();
}

/*
 * Calculate the maximum length of the completions.
 */
static int calculate_max_length(list l, int size)
{
        int len, i, max = 0;
        list p;

        for (p = list_first(l), i = 0; p != l && i < size; p = list_next(p), i++)
                if ((len = strlen(p->item)) > max)
                        max = len;

        return max;
}

/*
 * Print the list of completions in a set of columns.
 */
static void completion_print(list l, int size)
{
        int i, j, col, max, numcols;
        list p;

        max = calculate_max_length(l, size) + 5;
        numcols = (cur_wp->ewidth - 1) / max;

        bprintf("Possible completions are:\n");
        for (p = list_first(l), i = col = 0; p != l && i < size; p = list_next(p), i++) {
                if (col >= numcols) {
                        col = 0;
                        insert_newline();
                }
                insert_string(p->item);
                for (j = max - strlen(p->item); j > 0; --j)
                        insert_char(' ');
                ++col;
        }
}

/*
 * Popup the completion window.
 */
static void popup_completion(Completion *cp, int allflag, int num)
{
        Window *wp, *old_wp = cur_wp;
        Buffer *bp;

        cp->fl_poppedup = 1;

        if ((wp = find_window("*Completions*")) == NULL) {
                if (head_wp->next == NULL)
                        cp->fl_close = 1;
                set_current_window(popup_window ());
                if (!cp->fl_close)
                        cp->old_bp = cur_bp;
                bp = find_buffer("*Completions*", TRUE);
                switch_to_buffer(bp);
        } else {
                set_current_window(wp);
        }

        zap_buffer_content();
        cur_bp->flags = BFLAG_NEEDNAME | BFLAG_NOSAVE | BFLAG_NOUNDO;
        set_temporary_buffer(cur_bp);

        if (allflag)
                completion_print(cp->completions, list_length(cp->completions));
        else
                completion_print(cp->matches, num);

        gotobob();

        cur_bp->flags |= BFLAG_READONLY;

        set_current_window(old_wp);

        term_redisplay();
}

static int hcompar(const void *p1, const void *p2)
{
        return strcmp(*(const char **)p1, *(const char **)p2);
}

/*
 * Reread directory for completions.
 */
static int completion_reread(Completion *cp, astr as)
{
        astr buf, pdir, fname;
        DIR *dir;
        struct dirent *d;
        struct stat st;
        list p;
        int i;

        for (p = list_first(cp->completions); p != cp->completions; p = list_next(p))
                free(p->item);
        list_delete(cp->completions);

        cp->completions = list_new();
        cp->fl_sorted = 0;

        buf = astr_new();
        pdir = astr_new();
        fname = astr_new();

        for (i = 0; i < astr_len(as); i++){
                if (*astr_char(as, i) == '/') {
                        if (*astr_char(as, i + 1) == '/') {
                                /* Got `//'; restart from this point. */
                                while (*astr_char(as, i + 1) == '/')
                                        i++;
                                astr_truncate(buf, 0);
                                /* Final '/' remains to be copied below. */
                        }
                }
                astr_cat_char(buf, *astr_char(as, i));
        }
        astr_cpy(as, buf);

        agetcwd(buf);
        if (!expand_path(astr_cstr(as), astr_cstr(buf), pdir, fname))
                return FALSE;

        if ((dir = opendir(astr_cstr(pdir))) == NULL)
                return FALSE;

        astr_cpy(as, fname);

        while ((d = readdir(dir)) != NULL) {
                astr_cpy(buf, pdir);
                astr_cat_cstr(buf, d->d_name);
                if (stat(astr_cstr(buf), &st) != -1) {
                        astr_cpy_cstr(buf, d->d_name);
                        if (S_ISDIR(st.st_mode))
                                astr_cat_cstr(buf, "/");
                } else
                        astr_cpy_cstr(buf, d->d_name);
                list_append(cp->completions, zstrdup(astr_cstr(buf)));
        }
        closedir(dir);

        astr_delete(cp->path);
        cp->path = compact_path(astr_cstr(pdir));

        astr_delete(buf);
        astr_delete(pdir);
        astr_delete(fname);

        return TRUE;
}

/*
 * Match completions.
 */
int completion_try(Completion *cp, astr search, int popup_when_complete)
{
        int i, j, ssize, fullmatches = 0, partmatches = 0;
        char c;
        list p;

        list_delete(cp->matches);
        cp->matches = list_new();

        if (cp->fl_dir)
                if (!completion_reread(cp, search))
                        return COMPLETION_NOTMATCHED;

        if (!cp->fl_sorted) {
                list_sort(cp->completions, hcompar);
                cp->fl_sorted = 1;
        }

        ssize = astr_len(search);

        if (ssize == 0) {
                cp->match = list_first(cp->completions)->item;
                if (list_length(cp->completions) > 1) {
                        cp->matchsize = 0;
                        popup_completion(cp, TRUE, 0);
                        return COMPLETION_NONUNIQUE;
                } else {
                        cp->matchsize = strlen(cp->match);
                        return COMPLETION_MATCHED;
                }
        }

        for (p = list_first(cp->completions); p != cp->completions; p = list_next(p))
                if (!strncmp(p->item, astr_cstr(search), ssize)) {
                        ++partmatches;
                        list_append(cp->matches, p->item);
                        if (!strcmp(p->item, astr_cstr(search)))
                                ++fullmatches;
                }

        if (partmatches == 0)
                return COMPLETION_NOTMATCHED;
        else if (partmatches == 1) {
                cp->match = list_first(cp->matches)->item;
                cp->matchsize = strlen(cp->match);
                return COMPLETION_MATCHED;
        }

        if (fullmatches == 1 && partmatches > 1) {
                cp->match = list_first(cp->matches)->item;
                cp->matchsize = strlen(cp->match);
                if (popup_when_complete)
                        popup_completion(cp, FALSE, partmatches);
                return COMPLETION_MATCHEDNONUNIQUE;
        }

        for (j = ssize; ; ++j) {
                list p = list_first(cp->matches);
                char *s = p->item;
                
                c = s[j];
                for (i = 1; i < partmatches; ++i) {
                        p = list_next(p);
                        s = p->item;
                        if (s[j] != c) {
                                cp->match = list_first(cp->matches)->item;
                                cp->matchsize = j;
                                popup_completion(cp, FALSE, partmatches);
                                return COMPLETION_NONUNIQUE;
                        }
                }
        }

        assert(0);
        return COMPLETION_NOTMATCHED;
}
