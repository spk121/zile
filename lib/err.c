/*	$Id: err.c,v 1.1 2001/01/19 22:08:00 ssigala Exp $	*/

#include "config.h"

/*
 * 4.4BSD err(), warn() functions reimplementation.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"

/* The name of the running program. */
extern char *progname;

void err(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verr(status, fmt, ap);
	va_end(ap);
}

void verr(int status, const char *fmt, va_list ap)
{
	int olderrno = errno;

	fprintf(stderr, "%s: ", progname);
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(olderrno));

	exit(status);
}

void errx(int status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verrx(status, fmt, ap);
	va_end(ap);
}

void verrx(int status, const char *fmt, va_list ap)
{
	fprintf(stderr, "%s: ", progname);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);

	exit(status);
}

void warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarn(fmt, ap);
	va_end(ap);
}

void vwarn(const char *fmt, va_list ap)
{
	int olderrno = errno;

	fprintf(stderr, "%s: ", progname);
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(olderrno));
}

void warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

void vwarnx(const char *fmt, va_list ap)
{
	fprintf(stderr, "%s: ", progname);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
}
