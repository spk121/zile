/*	$Id: mkdoc.c,v 1.2 2003/04/24 15:11:17 rrt Exp $	*/

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct fentry {
	char	*name;
	char	*key1;
	char	*key2;
	char	*key3;
	char	*key4;
	char	*doc;
} fentry_table[] = {
#define X0(zile_name, c_name) \
	{ zile_name, NULL, NULL, NULL, NULL, NULL },
#define X1(zile_name, c_name, key1) \
	{ zile_name, key1, NULL, NULL, NULL, NULL },
#define X2(zile_name, c_name, key1, key2) \
	{ zile_name, key1, key2, NULL, NULL, NULL },
#define X3(zile_name, c_name, key1, key2, key3) \
	{ zile_name, key1, key2, key3, NULL, NULL },
#define X4(zile_name, c_name, key1, key2, key3, key4) \
	{ zile_name, key1, key2, key3, key4, NULL },
#include "tbl_funcs.h"
#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
};

#define fentry_table_size (sizeof fentry_table  / sizeof fentry_table[0])

struct ventry {
	char	*name;
	char	*fmt;
	char	*defvalue;
	char	*doc;
} ventry_table[] = {
#define X(name, fmt, defvalue, doc) \
	{ name, fmt, defvalue, doc },
#include "tbl_vars.h"
#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
};

#define ventry_table_size (sizeof ventry_table  / sizeof ventry_table[0])


FILE *input_file;
FILE *output_file;

static void fdecl(char *name)
{
	char *doc, buf[1024];
	int size, maxsize, s = 0;
	unsigned int i;

	size = 0;
	maxsize = 30;
	doc = (char *)xmalloc(maxsize);
	doc[0] = '\0';

	while (fgets(buf, 1024, input_file) != NULL) {
		if (s == 1) {
			if (!strncmp(buf, "+*/", 3))
				break;
			i = strlen(buf);
			if (size + (int)i >= maxsize) {
				maxsize += i + 10;
				doc = (char *)xrealloc(doc, maxsize);
			}
			strcat(doc, buf);
			size += i;
		}
		if (!strncmp(buf, "/*+", 3))
			s = 1;
		else if (buf[0] == '{')
			break;
	}

	for (i = 0; i < fentry_table_size; ++i)
		if (!strcmp(name, fentry_table[i].name))
			fentry_table[i].doc = doc;
}

static void parse(void)
{
	char buf[1024];

	while (fgets(buf, 1024, input_file) != NULL) {
		if (!strncmp(buf, "DEFUN(", 6)) {
			*strchr(strchr(buf, '"') + 1, '"') = '\0';
			fdecl(strchr(buf, '"') + 1);
		}
	}
}

static void dump_help(void)
{
	unsigned int i;
	for (i = 0; i < fentry_table_size; ++i)
		fprintf(output_file, "\fF_%s\n%s",
			fentry_table[i].name, fentry_table[i].doc);
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
 * Output the program syntax then exit.
 */
static void usage(void)
{
	fprintf(stderr, "usage: mkdoc [-d] [-o file] [file ...]\n");
	exit(1);
}

char *progname;

int main(int argc, char **argv)
{
	int c;

	progname = argv[0];
	input_file = stdin;
	output_file = stdout;

	while ((c = getopt(argc, argv, "o:")) != -1)
		switch (c) {
		case 'o':
			if (output_file != stdout)
				fclose(output_file);
			if ((output_file = fopen(optarg, "w")) == NULL) {
				fprintf(stderr, "mkdoc:%s: %s\n",
					optarg, strerror(errno));
				return 1;
			}
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		process_file(NULL);
	else
		while (*argv)
			process_file(*argv++);

	dump_help();

	return 0;
}
