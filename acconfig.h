/* Define this to support the xterm resizing. */
#undef SUPPORT_XTERM_RESIZE

/* Define this to remove the debugging assertions.  */
#undef NDEBUG

/* Define this to include debugging code.  */
#undef DEBUG

/* Define if you have the ncurses library (-lncurses).  */
#undef HAVE_LIBNCURSES

/* Define this to use the ncurses screen handling library.  */
#undef USE_NCURSES

/* The date of compilation.  */
#undef CONFIGURE_DATE

/* The host of compilation.  */
#undef CONFIGURE_HOST

@BOTTOM@

#ifdef USE_NCURSES
#ifndef HAVE_LIBNCURSES
#error ncurses is required for compilation.
#endif
#endif

#include <stddef.h>

#ifndef HAVE_XMALLOC
extern void *xmalloc(size_t);
#endif

#ifndef HAVE_XREALLOC
extern void *xrealloc(void *, size_t);
#endif

#ifndef HAVE_XSTRDUP
extern char *xstrdup(const char *);
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif
