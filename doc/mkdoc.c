/*
 * A Quick & Dirty tool to produce the AUTODOC file.
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* #include other sources so this program can be easily built on the
   build host when cross-compiling */
#include "strrstr.c"
#include "vasprintf.c"
#include "zmalloc.c"
#include "astr.c"

struct fentry {
  char	*name;
  char	*key1;
  char	*key2;
  char	*key3;
  char	*key4;
  astr	doc;
} fentry_table[] = {
#define X0(zile_name, c_name) \
  { zile_name, NULL, NULL, NULL, NULL, NULL },
#define X1(zile_name, c_name, key1) \
  { zile_name, key1, NULL, NULL, NULL, NULL },
#define X2(zile_name, c_name, key1, key2) \
  { zile_name, key1, key2, NULL, NULL, NULL },
#define X3(zile_name, c_name, key1, key2, key3) \
  { zile_name, key1, key2, key3, NULL, NULL },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
};

#define fentry_table_size (sizeof fentry_table  / sizeof fentry_table[0])

struct ventry {
  char	*name;
  char	*defvalue;
  int	local;
  char	*doc;
} ventry_table[] = {
#define X(name, defvalue, local, doc) \
	{ name, defvalue, local, doc },
#include "tbl_vars.h"
#undef X
};

#define ventry_table_size (sizeof ventry_table / sizeof ventry_table[0])

FILE *input_file;
FILE *output_file;

static void fdecl(const char *name)
{
  astr doc = astr_new();
  astr buf;
  unsigned s = 0, i;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (s == 1) {
      if (!strncmp(astr_cstr(buf), "+*/", (size_t)3))
        break;
      astr_cat(doc, buf);
      astr_cat_char(doc, '\n');
    }
    if (!strncmp(astr_cstr(buf), "/*+", (size_t)3))
      s = 1;
    else if (astr_cstr(buf)[0] == '{')
      break;
    astr_delete(buf);
  }

  for (i = 0; i < fentry_table_size; ++i)
    if (!strcmp(name, fentry_table[i].name))
      fentry_table[i].doc = doc;
}

static void parse(void)
{
  astr buf;

  while ((buf = astr_fgets(input_file)) != NULL) {
    if (!strncmp(astr_cstr(buf), "DEFUN(", (size_t)6)) {
      int i, j;
      astr sub;
      i = astr_find_cstr(buf, "\"");
      j = astr_rfind_cstr(buf, "\"");
      if (i < 0 || j < 0 || i == j) {
        fprintf(stderr, "mkdoc: invalid DEFUN() syntax\n");
        exit(1);
      }
      sub = astr_substr(buf, i + 1, (size_t)(j - i - 1));
      astr_cpy(buf, sub);
      astr_delete(sub);
      fdecl(astr_cstr(buf));
    }
    astr_delete(buf);
  }
}

static void dump_help(void)
{
  unsigned int i;
  for (i = 0; i < fentry_table_size; ++i) {
    astr doc = fentry_table[i].doc;
    if (doc != NULL)
      fprintf(output_file, "\fF_%s\n%s",
              fentry_table[i].name, astr_cstr(doc));
  }
  for (i = 0; i < ventry_table_size; ++i)
    fprintf(output_file, "\fV_%s\n%s\n%s\n",
            ventry_table[i].name, ventry_table[i].defvalue,
            ventry_table[i].doc);
}

static void process_file(char *filename)
{
  if (filename != NULL && strcmp(filename, "-") != 0) {
    if ((input_file = fopen(filename, "r")) == NULL) {
      fprintf(stderr, "mkdoc:%s: %s\n",
              filename, strerror(errno));
      exit(1);
    }
  } else
    input_file = stdin;

  parse();

  if (input_file != stdin)
    fclose(input_file);
}

/*
 * Stub to make zmalloc &c. happy.
 */
void zile_exit(int doabort)
{
  (void)doabort;
  exit(2);
}

int main(int argc, char **argv)
{
  input_file = stdin;
  output_file = stdout;

  if (argc < 1)
    process_file(NULL);
  else
    while (*argv)
      process_file(*argv++);

  dump_help();

  return 0;
}
