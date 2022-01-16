/* Copyright (c) 2019-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef _LIBICONV_TRANSLITERATE_H
#define _LIBICONV_TRANSLITERATE_H 1

#include "api.h"

#include <__stdinc.h>

#include <bits/types.h>

/* Transliteration is the process of enumerating alternative (possibly multi-character)
 * representations of some given unicode character. Using this, it is possible to  e.g.
 * convert a German "ß" into "ss" or "ä" into "a".
 * This kind of functionality can be enabled for iconv encoders via `ICONV_ERR_TRANSLIT',
 * but  can  also  be  used  explicitly  via  the  function  exported  from  this header. */


/* Max # of new unicode characters generated by any possible transliteration. */
#define ICONV_TRANSLITERATE_MAXLEN 8


#ifdef __CC__
__DECL_BEGIN


/* Generate the `nth' transliteration for `uni_ch' and store the results
 * in `result', returning  the #  of stored characters.  When no  (more)
 * transliterations exist for `uni_ch' (where available ones are indexed
 * via `nth', starting at `0'), return `(size_t)-1'
 * Note that in the case of multi-character transliterations, all possible
 * transliterations for replacement  characters are  already attempted  by
 * this  function itself,  meaning that in  these cases all  those are all
 * tried as well.
 * Note that this function may or may not re-return  `uni_ch && return==1'.
 * When this is the case, simply ignore the call any try again with `nth+1'
 *
 * @param: result: Result buffer for transliterated text.
 * @param: uni_ch: The character that should be transliterated.
 * @param: nth:    Specifies  that the nth transliteration of `uni_ch' be
 *                 generated, starting at `0' and ending as soon as  this
 *                 function returns `(size_t)-1' to indicate that no more
 *                 possible transliterations are available.
 * @param: what:   What to try (set of `ICONV_TRANSLITERATE_F_*')
 * @return: (size_t)-1: No (more) transliterations available.
 * @return: * : The # of characters written to `result' (may be 0). */
typedef __ATTR_WUNUSED __ATTR_NONNULL((1)) __size_t
/*__NOTHROW_NCX*/ (LIBICONV_CC *PICONV_TRANSLITERATE)(__CHAR32_TYPE__ result[ICONV_TRANSLITERATE_MAXLEN],
                                                      __CHAR32_TYPE__ uni_ch, __size_t nth,
                                                      unsigned int what);
#ifdef LIBICONV_WANT_PROTOTYPES
LIBICONV_DECL __ATTR_WUNUSED __ATTR_NONNULL((1)) __size_t
__NOTHROW_NCX(LIBICONV_CC iconv_transliterate)(__CHAR32_TYPE__ result[ICONV_TRANSLITERATE_MAXLEN],
                                               __CHAR32_TYPE__ uni_ch, __size_t nth,
                                               unsigned int what);
#endif /* LIBICONV_WANT_PROTOTYPES */

/* Possible flags for `iconv_transliterate::what' */
#define ICONV_TRANSLITERATE_F_LOWER    0x0001 /* Flag: attempt to use `unicode_tolower()' */
#define ICONV_TRANSLITERATE_F_UPPER    0x0002 /* Flag: attempt to use `unicode_toupper()' */
#define ICONV_TRANSLITERATE_F_TITLE    0x0004 /* Flag: attempt to use `unicode_totitle()' */
#define ICONV_TRANSLITERATE_F_FOLD     0x0008 /* Flag: attempt to use `unicode_fold()' */
#define ICONV_TRANSLITERATE_F_TRANSLIT 0x0010 /* Flag: attempt to replace with similar-looking character(s) */
#define ICONV_TRANSLITERATE_F_ALL      0xffff /* Attempt everything. (used by `ICONV_ERR_TRANSLIT') */



__DECL_END
#endif /* __CC__ */

#endif /* !_LIBICONV_TRANSLITERATE_H */
