/* Define to 1 if you have the <assert.h> header file. */
#define HAVE_ASSERT_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define as 0 or 1 according to the floating point format suported by the
   machine */
#define HAVE_IEEEFP 1

/* Define to 1 if you have the `jbg_newlen' function. */
#define HAVE_JBG_NEWLEN 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#if !defined(__ANDROID__) && !defined(__IOS__) && !defined(_WIN32) && !defined(_WIN64)
#define HAVE_IO_H 1
#endif

/* Define to 1 if you have the <search.h> header file. */
#if !defined(__ANDROID__) || (__ANDROID_API__ >= 20)
	# define HAVE_SEARCH_H 1
#endif

/* Define to 1 if you have the `setmode' function. */
#define HAVE_SETMODE 1

/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* Signed 64-bit type formatter */
#define TIFF_INT64_FORMAT "%ll"

/* Signed 64-bit type */
#define TIFF_INT64_T int64_t

/* Unsigned 64-bit type formatter */

#define TIFF_UINT64_FORMAT "%llu"

/* Unsigned 64-bit type */
#define TIFF_UINT64_T uint64_t

/* Set the native cpu bit order */
#define HOST_FILLORDER FILLORDER_LSB2MSB

//#define snprintf _snprintf

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
# ifndef inline
#  define inline __inline
# endif
#endif

#define lfind _lfind
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */
