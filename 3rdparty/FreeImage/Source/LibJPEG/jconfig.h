/* jconfig.h.  Generated by configure.  */
/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */
#define JPEG_LIB_VERSION 80

/* libjpeg-turbo version */
#define LIBJPEG_TURBO_VERSION 1.4.0

/* Support arithmetic encoding */
#define C_ARITH_CODING_SUPPORTED 1

/* Support arithmetic decoding */
#define D_ARITH_CODING_SUPPORTED 1

/*
 * Define BITS_IN_JSAMPLE as either
 *   8   for 8-bit sample values (the usual setting)
 *   12  for 12-bit sample values
 * Only 8 and 12 are legal data precisions for lossy JPEG according to the
 * JPEG standard, and the IJG code does not support anything else!
 * We do not support run-time selection of data precision, sorry.
 */

#define BITS_IN_JSAMPLE  8      /* use 8 or 12 */

/* Support in-memory source/destination managers */
/* #undef MEM_SRCDST_SUPPORTED */

/* Compiler supports function prototypes. */
#define HAVE_PROTOTYPES 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Compiler supports 'unsigned char'. */
#define HAVE_UNSIGNED_CHAR 1

/* Compiler supports 'unsigned short'. */
#define HAVE_UNSIGNED_SHORT 1

/* Compiler does not support pointers to unspecified structures. */
/* #undef INCOMPLETE_TYPES_BROKEN */

/* Compiler has <strings.h> rather than standard <string.h>. */
/* #undef NEED_BSD_STRINGS */

/* Linker requires that global names be unique in first 15 characters. */
/* #undef NEED_SHORT_EXTERNAL_NAMES */

/* Need to include <sys/types.h> in order to obtain size_t. */
#define NEED_SYS_TYPES_H 1

/* Broken compiler shifts signed values as an unsigned shift. */
/* #undef RIGHT_SHIFT_IS_UNSIGNED */

/* Use accelerated SIMD routines. */
#if !defined(WITH_SIMD) && (defined(_WIN32) || defined(__WIN32__))
#	define WITH_SIMD 1
#endif

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
