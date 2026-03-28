/* Copyright (c) 2019-2026 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2019-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBICONV_CONVERT_UTILS_H
#define GUARD_LIBICONV_CONVERT_UTILS_H 1

#include "api.h"
/**/

#ifndef LIBICONV_NO_SYSTEM_INCLUDES
#include <libiconv/iconv.h>
#endif /* !LIBICONV_NO_SYSTEM_INCLUDES */

DECL_BEGIN

#ifdef ICONV_ERR_ERRNO
#define IS_ICONV_ERR_ERRNO(flags) (((flags) & ICONV_ERRMASK) == ICONV_ERR_ERRNO)
#endif /* ICONV_ERR_ERRNO */
#ifndef ICONV_ERR_ERRNO
#define IS_ICONV_ERR_ERROR_OR_ERRNO(flags) (((flags) & ICONV_ERRMASK) == ICONV_ERR_ERROR)
#elif (ICONV_ERR_ERROR >= ICONV_ERR_ERRNO && ICONV_ERR_ERROR < ICONV_ERR_DISCARD && \
       ICONV_ERR_ERROR < ICONV_ERR_REPLACE && ICONV_ERR_ERROR < ICONV_ERR_IGNORE)
#define IS_ICONV_ERR_ERROR_OR_ERRNO(flags) (((flags) & ICONV_ERRMASK) <= ICONV_ERR_ERROR)
#else /* ... */
#define IS_ICONV_ERR_ERROR_OR_ERRNO(flags)           \
	(((flags) & ICONV_ERRMASK) == ICONV_ERR_ERRNO || \
	 ((flags) & ICONV_ERRMASK) == ICONV_ERR_ERROR)
#endif /* !... */

#if ((!defined(ICONV_ERR_ERRNO) || ICONV_ERR_DISCARD >= ICONV_ERR_ERRNO) && \
     ICONV_ERR_DISCARD >= ICONV_ERR_ERROR &&                                \
     ICONV_ERR_DISCARD < ICONV_ERR_REPLACE &&                               \
     ICONV_ERR_DISCARD < ICONV_ERR_IGNORE)
#define IS_ICONV_ERR_ERROR_OR_ERRNO_OR_DISCARD(flags) (((flags) & ICONV_ERRMASK) <= ICONV_ERR_DISCARD)
#else /* ... */
#define IS_ICONV_ERR_ERROR_OR_ERRNO_OR_DISCARD(flags) \
	(((flags) & ICONV_ERRMASK) == ICONV_ERR_ERRNO ||  \
	 ((flags) & ICONV_ERRMASK) == ICONV_ERR_ERROR ||  \
	 ((flags) & ICONV_ERRMASK) == ICONV_ERR_DISCARD)
#endif /* !... */

#define IS_ICONV_ERR_DISCARD(flags) (((flags) & ICONV_ERRMASK) == ICONV_ERR_DISCARD)
#define IS_ICONV_ERR_REPLACE(flags) (((flags) & ICONV_ERRMASK) == ICONV_ERR_REPLACE)
#define IS_ICONV_ERR_IGNORE(flags)  (((flags) & ICONV_ERRMASK) == ICONV_ERR_IGNORE)

#ifndef LIBICONV_SETERRNO
#define LIBICONV_SETERRNO(v) (errno = (v))
#endif /* !LIBICONV_SETERRNO */

DECL_END

#endif /* !GUARD_LIBICONV_CONVERT_UTILS_H */
