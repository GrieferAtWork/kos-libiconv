/* Copyright (c) 2019-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBICONV_MBCS_CP_MBCS_C
#define GUARD_LIBICONV_MBCS_CP_MBCS_C 1

#include "../api.h"
/**/

#include <hybrid/byteorder.h>
#include <hybrid/unaligned.h>

#include <kos/types.h>

#include <assert.h>
#include <errno.h>
#include <format-printer.h>
#include <stddef.h>
#include <unicode.h>

#include <libiconv/iconv.h>

#include "../codecs.h"
#include "cp-mbcs.h"

#if CODEC_MBCS_COUNT != 0
DECL_BEGIN

#define IS_ICONV_ERR_ERRNO(flags)                     (((flags) & ICONV_ERRMASK) == ICONV_ERR_ERRNO)
#define IS_ICONV_ERR_ERROR_OR_ERRNO(flags)            (((flags) & ICONV_ERRMASK) <= ICONV_ERR_ERROR)
#define IS_ICONV_ERR_ERROR_OR_ERRNO_OR_DISCARD(flags) (((flags) & ICONV_ERRMASK) <= ICONV_ERR_DISCARD)
#define IS_ICONV_ERR_DISCARD(flags)                   (((flags) & ICONV_ERRMASK) == ICONV_ERR_DISCARD)
#define IS_ICONV_ERR_REPLACE(flags)                   (((flags) & ICONV_ERRMASK) == ICONV_ERR_REPLACE)
#define IS_ICONV_ERR_IGNORE(flags)                    (((flags) & ICONV_ERRMASK) == ICONV_ERR_IGNORE)

#define decode_output_printer self->icd_output.ii_printer
#define decode_output_arg     self->icd_output.ii_arg
#define decode_output(p, s)   (*decode_output_printer)(decode_output_arg, p, s)
#define encode_output_printer self->ice_output.ii_printer
#define encode_output_arg     self->ice_output.ii_arg
#define encode_output(p, s)   (*encode_output_printer)(encode_output_arg, p, s)
#define DO(expr)                         \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0
#define DO_encode_output(p, s) DO(encode_output(p, s))
#define DO_decode_output(p, s) DO(decode_output(p, s))

/************************************************************************/
/* MBCS code page encode/decode functions.                              */
/************************************************************************/
PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) byte_t
NOTHROW(CC get_byte1_of_range)(struct iconv_mbcs_codepage const *cp,
                               struct iconv_mbcs_byte2_range const *range) {
	unsigned int result;
	for (result = 0; result <= 0xff; ++result) {
		char16_t uni = cp->imc_1byte[result];
		if (iconv_mbcs_codepage_hasbyte2(cp, uni) &&
		    iconv_mbcs_codepage_getbyte2(cp, uni) == range)
			break; /* Found it! */
	}
	/* TODO: Reverse engineer lead-up bytes for multi-byte prefixes */
	return (byte_t)result;
}

INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_mbcs_decode(struct iconv_decode *__restrict self,
                     /*cp???*/ char const *__restrict data, size_t size) {
	size_t utf8_len;
	char utf8_buf[UNICODE_UTF8_CURLEN];
	struct iconv_mbcs_codepage const *cp;
	ssize_t temp, result = 0;
	if unlikely(self->icd_flags & ICONV_HASERR)
		goto err_ilseq;
	cp = self->icd_data.idd_mbcs.mc_cp;
	for (; size; --size, ++data) {
		char32_t outch;
		unsigned char ch;
		ch = (unsigned char)*data;

		/* Switch on current state. */
		if (self->icd_data.idd_mbcs.mc_b2 != NULL) {
			struct iconv_mbcs_byte2_range const *range;
			range = self->icd_data.idd_mbcs.mc_b2;
			while (ch > range->imc_cpmax)
				range = iconv_mbcs_byte2_range_next(range);
			if unlikely(range->imc_type == ICONV_MBCS_BYTE2_RANGE_TYPE_EOF ||
			            ch < range->imc_cpmin) {
				/* Unmapped character. */
unmapped_multichar:
				if (IS_ICONV_ERR_ERROR_OR_ERRNO(self->icd_flags))
					goto err_ilseq;
				if (IS_ICONV_ERR_DISCARD(self->icd_flags)) {
					self->icd_data.idd_mbcs.mc_b2 = NULL;
					continue;
				}
				outch = '?';
				if (!IS_ICONV_ERR_REPLACE(self->icd_flags)) {
					byte_t byte1;
					/* TODO: Support for multi-byte lead-up */
					byte1 = get_byte1_of_range(cp, self->icd_data.idd_mbcs.mc_b2);
					outch = (char32_t)(char16_t)(((uint16_t)byte1 << 8) | ch);
				}
			} else {
				/* Switch on the range type to determine the unicode character. */
				switch (range->imc_type) {
				case ICONV_MBCS_BYTE2_RANGE_TYPE_LIN:
					outch = (char32_t)(char16_t)(uint16_t)((uint16_t)ch + range->imc_lin);
					break;
				case ICONV_MBCS_BYTE2_RANGE_TYPE_ARR:
					outch = range->imc_arr[ch - range->imc_cpmin];
					if unlikely(!outch)
						goto unmapped_multichar;
					break;
				case ICONV_MBCS_BYTE2_RANGE_TYPE_ARR32:
					outch = (char32_t)UNALIGNED_GET32(&range->imc_arr32[ch - range->imc_cpmin]);
					if unlikely(!outch)
						goto unmapped_multichar;
					break;
				case ICONV_MBCS_BYTE2_RANGE_TYPE_TAB: {
					/* Load the start of the follow-up range. */
					range = (struct iconv_mbcs_byte2_range const *)((byte_t const *)cp + range->imc_tab);
					self->icd_data.idd_mbcs.mc_b2 = range;
					continue;
				}	break;
				default: __builtin_unreachable();
				}
			}
			self->icd_data.idd_mbcs.mc_b2 = NULL;
		} else {
			outch = cp->imc_1byte[ch];
			if (iconv_mbcs_codepage_hasbyte2(cp, (char16_t)outch)) {
				/* Begin 2-byte sequence */
				struct iconv_mbcs_byte2_range const *range;
				range = iconv_mbcs_codepage_getbyte2(cp, (char16_t)outch);
				self->icd_data.idd_mbcs.mc_b2 = range;
				continue;
			}
			if unlikely(outch == 0 && ch != 0) {
				/* Invalid input character. */
				if (IS_ICONV_ERR_ERROR_OR_ERRNO(self->icd_flags))
					goto err_ilseq;
				if (IS_ICONV_ERR_DISCARD(self->icd_flags))
					continue;
				outch = '?';
				if (!IS_ICONV_ERR_REPLACE(self->icd_flags))
					outch = (char32_t)ch;
			}
		}
		utf8_len = (size_t)(unicode_writeutf8(utf8_buf, outch) - utf8_buf);
		DO_decode_output(utf8_buf, utf8_len);
	}
	return result;
err:
	return temp;
err_ilseq:
	self->icd_flags |= ICONV_HASERR;
	if (IS_ICONV_ERR_ERRNO(self->icd_flags))
		errno = EILSEQ;
	return -(ssize_t)size;
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
	struct iconv_mbcs_codepage const *cp;
	assert(self->icd_codec >= CODEC_MBCS_MIN &&
	       self->icd_codec <= CODEC_MBCS_MAX);
	cp = libiconv_mbcs_page(self->icd_codec);
	/* Initialize the decoder. */
	self->icd_data.idd_mbcs.mc_cp = cp;
	self->icd_data.idd_mbcs.mc_b2 = NULL;
	/* Initialize the input pipe. */
	input->ii_arg     = self;
	input->ii_printer = (pformatprinter)&libiconv_mbcs_decode;
}

INTERN NONNULL((1, 2)) void CC
libiconv_mbcs_encode_init(struct iconv_encode *__restrict self,
                          /*out*/ struct iconv_printer *__restrict input) {
	struct iconv_mbcs_codepage const *cp;
	assert(self->ice_codec >= CODEC_MBCS_MIN &&
	       self->ice_codec <= CODEC_MBCS_MAX);
	cp = libiconv_mbcs_page(self->ice_codec);
	/* Initialize the encoder. */
	self->ice_data.ied_cpmbcs = cp;
	/* Initialize the input pipe. */
	input->ii_arg     = self;
	input->ii_printer = (pformatprinter)&libiconv_mbcs_encode;
}


DEFINE_PUBLIC_ALIAS(iconv_mbcs_decode_init, libiconv_mbcs_decode_init);
DEFINE_PUBLIC_ALIAS(iconv_mbcs_encode_init, libiconv_mbcs_encode_init);

DECL_END
#endif /* CODEC_MBCS_COUNT != 0 */

#endif /* !GUARD_LIBICONV_MBCS_CP_MBCS_C */
