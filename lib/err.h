/*	$Id: err.h,v 1.1 2001/01/19 22:08:00 ssigala Exp $	*/

#ifndef __ERR_H
#define __ERR_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void err(int status, const char *fmt, ...);
extern void verr(int status, const char *fmt, va_list ap);
extern void errx(int status, const char *fmt, ...);
extern void verrx(int status, const char *fmt, va_list ap);
extern void warn(const char *fmt, ...);
extern void vwarn(const char *fmt, va_list ap);
extern void warnx(const char *fmt, ...);
extern void vwarnx(const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* !__ERR_H */
