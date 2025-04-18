/* Copyright (c) 2019-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBICONV_CONVERT_H
#define GUARD_LIBICONV_CONVERT_H 1

#include "api.h"
/**/

#include <format-printer.h>
#include <kos/types.h>

#include "codecs.h"

DECL_BEGIN

/************************************************************************/
/* ASCII                                                                */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_ascii_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_ascii_decode(struct iconv_decode *__restrict self, /*ASCII*/ char const *__restrict data, size_t size);

/************************************************************************/
/* UTF-8 w/ BOM                                                         */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf8_bom_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf8_bom_decode(struct iconv_decode *__restrict self, /*utf-8-bom*/ char const *__restrict data, size_t size);


/************************************************************************/
/* UTF-16                                                               */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16le_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16be_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16le_decode(struct iconv_decode *__restrict self, /*utf-16*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16be_decode(struct iconv_decode *__restrict self, /*utf-16*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16le_bom_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16be_bom_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16le_bom_decode(struct iconv_decode *__restrict self, /*utf-16*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf16be_bom_decode(struct iconv_decode *__restrict self, /*utf-16*/ void const *__restrict data, size_t size);


/************************************************************************/
/* UTF-32                                                               */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32le_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32be_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32le_decode(struct iconv_decode *__restrict self, /*utf-32*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32be_decode(struct iconv_decode *__restrict self, /*utf-32*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32le_bom_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32be_bom_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32le_bom_decode(struct iconv_decode *__restrict self, /*utf-32*/ void const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_utf32be_bom_decode(struct iconv_decode *__restrict self, /*utf-32*/ void const *__restrict data, size_t size);


/************************************************************************/
/* Special handling for latin-1 (for the same of performance)           */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_latin1_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_latin1_decode(struct iconv_decode *__restrict self, /*latin1*/ char const *__restrict data, size_t size);


#if CODEC_CP_COUNT != 0
/************************************************************************/
/* Generic code-page                                                    */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp_decode(struct iconv_decode *__restrict self, /*cp???*/ char const *__restrict data, size_t size);
#endif /* CODEC_CP_COUNT != 0 */


#if CODEC_CP7L_COUNT != 0
/************************************************************************/
/* 7L-code-page encode/decode                                           */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp7l_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp7l_decode(struct iconv_decode *__restrict self, /*cp???*/ char const *__restrict data, size_t size);
#endif /* CODEC_CP7L_COUNT != 0 */


#if CODEC_CP7H_COUNT != 0
/************************************************************************/
/* 7H-code-page encode/decode                                           */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp7h_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp7h_decode(struct iconv_decode *__restrict self, /*cp???*/ char const *__restrict data, size_t size);
#endif /* CODEC_CP7H_COUNT != 0 */


#if CODEC_ISO646_COUNT != 0
/************************************************************************/
/* iso646 code page encode/decode functions.                            */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp646_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_cp646_decode(struct iconv_decode *__restrict self, /*cp???*/ char const *__restrict data, size_t size);
#endif /* CODEC_ISO646_COUNT != 0 */



/************************************************************************/
/* Custom codecs.                                                       */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_c_escape_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_c_escape_decode(struct iconv_decode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_xml_escape_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_xml_escape_decode(struct iconv_decode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_uri_escape_encode(struct iconv_encode *__restrict self, /*bytes*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_uri_escape_decode(struct iconv_decode *__restrict self, /*ascii*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_hex_lower_encode(struct iconv_encode *__restrict self, /*bytes*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_hex_upper_encode(struct iconv_encode *__restrict self, /*bytes*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_hex_decode(struct iconv_decode *__restrict self, /*ascii*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_base64_encode(struct iconv_encode *__restrict self, /*bytes*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_base64_decode(struct iconv_decode *__restrict self, /*ascii*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1)) ssize_t NOTHROW_NCX(CC libiconv_base64_encode_flush)(struct iconv_encode *__restrict self);



DECL_END

#endif /* !GUARD_LIBICONV_CONVERT_H */
