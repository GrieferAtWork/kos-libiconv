/* Copyright (c) 2019-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBICONV_MBCS_CP_MBCS_C
#define GUARD_LIBICONV_MBCS_CP_MBCS_C 1

#include "../api.h"
/**/

#include <hybrid/byteorder.h>

#include <kos/types.h>

#include <format-printer.h>
#include <stddef.h>

#include <libiconv/iconv.h>

#include "../codecs.h"
#include "cp-mbcs.h"

#if CODEC_MBCS_COUNT != 0
DECL_BEGIN

/* TODO */

/************************************************************************/
/* MBCS code page encode/decode functions.                              */
/************************************************************************/
INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_mbcs_decode(struct iconv_decode *__restrict self,
                     /*cp???*/ char const *__restrict data, size_t size) {
	(void)self;
	(void)data;
	(void)size;
	COMPILER_IMPURE();
	/* TODO */
	return 0;
}

INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_mbcs_encode(struct iconv_encode *__restrict self,
                     /*utf-8*/ char const *__restrict data, size_t size) {
	(void)self;
	(void)data;
	(void)size;
	COMPILER_IMPURE();
	/* TODO */
	return 0;
}


/* Initialize a mbcs encoder/decoder. */
INTERN NONNULL((1, 2)) void CC
libiconv_mbcs_decode_init(struct iconv_decode *__restrict self,
                          /*out*/ struct iconv_printer *__restrict input) {
	(void)self;
	(void)input;
	COMPILER_IMPURE();
	/* TODO */
}

INTERN NONNULL((1, 2)) void CC
libiconv_mbcs_encode_init(struct iconv_encode *__restrict self,
                          /*out*/ struct iconv_printer *__restrict input) {
	(void)self;
	(void)input;
	COMPILER_IMPURE();
	/* TODO */
}


DEFINE_PUBLIC_ALIAS(iconv_mbcs_decode_init, libiconv_mbcs_decode_init);
DEFINE_PUBLIC_ALIAS(iconv_mbcs_encode_init, libiconv_mbcs_encode_init);

DECL_END
#endif /* CODEC_MBCS_COUNT != 0 */

#endif /* !GUARD_LIBICONV_MBCS_CP_MBCS_C */
