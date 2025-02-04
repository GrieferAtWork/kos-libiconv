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
#ifndef GUARD_LIBICONV_MBCS_CP_MBCS_H
#define GUARD_LIBICONV_MBCS_CP_MBCS_H 1

#include "../api.h"
/**/

#include <hybrid/byteorder.h>

#include <kos/types.h>

#include <format-printer.h>

#include <libiconv/iconv.h>

#include "../codecs.h"

/* HINT: MBCS == MultiByteCharacterSet */

#if CODEC_MBCS_COUNT != 0
DECL_BEGIN

struct iconv_mbcs_byte2_range {
#define ICONV_MBCS_BYTE2_RANGE_TYPE_EOF   0 /* Range list end (sentinel) */
#define ICONV_MBCS_BYTE2_RANGE_TYPE_LIN   1 /* Linear unicode mapping */
#define ICONV_MBCS_BYTE2_RANGE_TYPE_ARR   2 /* 16-bit array of unicode ordinals */
#define ICONV_MBCS_BYTE2_RANGE_TYPE_ARR32 3 /* 32-bit array of unicode ordinals */
#define ICONV_MBCS_BYTE2_RANGE_TYPE_TAB   4 /* Another table (offset from code-page start to another array
                                             * of `struct iconv_mbcs_byte2_range' to-be used for decoding) */
	byte_t imc_type;  /* Type of range (one of ICONV_MBCS_BYTE2_RANGE_TYPE_*). */
	/* NOTE: Ranges are sorted by `imc_cpmin' and don't overlap, meaning that
	 *       when searching for the correct  range, you can always stop  once
	 *       the next range's `imc_cpmin >= SEARCHED_BYTE' */
	byte_t imc_cpmax; /* Last code-page byte mapped by this range. (255 for ICONV_MBCS_BYTE2_RANGE_TYPE_EOF) */
	/* All of the following are only valid when `imc_type != ICONV_MBCS_BYTE2_RANGE_TYPE_EOF' */
	byte_t imc_cpmin; /* First code-page byte mapped by this range. */
	byte_t imc_size;  /* Size  of  this  range  structure  (divided  by 2)
	                   * Multiply by 2 to calculate the offset to the next
	                   * element in the range list. */
	union {
		/* Range-specific data.
		 * NOTE: The arrays aren't actually of length 256, but if flexible arrays
		 *       are used, then GCC bickers because they're used in a union. (Why
		 *       dafuq aren't I allowed to use flexible arrays in unions?) */
		uint16_t imc_lin;    /* [ICONV_MBCS_BYTE2_RANGE_TYPE_LIN] Linear   unicode   mapping
		                      * >> RESULT = (char16_t)(uint16_t)((uint16_t)BYTE2 + imc_lin); */
		uint32_t imc_arr[256];   /* [ICONV_MBCS_BYTE2_RANGE_TYPE_ARR] Array unicode mapping
		                          * >> RESULT = imc_arr[BYTE2 - imc_cpmin];
		                          * NOTE: Undefined ordinals appear as `0' in this list! */
		/* WARNING: The 32-bit integer array is only aligned by 2 bytes! */
		uint32_t imc_arr32[256]; /* [ICONV_MBCS_BYTE2_RANGE_TYPE_ARR32] Array       unicode       mapping
		                          * >> RESULT = (char32_t)UNALIGNED_GET32(&imc_arr32[BYTE2 - imc_cpmin]);
		                          * NOTE: Undefined ordinals appear as `0' in this list! */
		uint16_t imc_tab;    /* [ICONV_MBCS_BYTE2_RANGE_TYPE_TAB] Linear   unicode   mapping
		                      * >> RESULT = (char16_t)(uint16_t)((uint16_t)BYTE2 + imc_lin); */
	};
};

/* Return a pointer to the next range. */
#define iconv_mbcs_byte2_range_next(self) \
	((struct iconv_mbcs_byte2_range const *)((byte_t const *)(self) + (self)->imc_size * 2))


struct iconv_mbcs_codepage {
	char16_t imc_1byte[256]; /* First-byte  codepage. These map to either unicode ordinals,
	                          * or  fall  into `imc_offidx_min...imc_offidx_max',  in which
	                          * case the ordinal is an index into second-byte offset table. */
	char16_t imc_offidx_min; /* Lower bound for offset indices */
	char16_t imc_offidx_max; /* Upper bound for offset indices */
	COMPILER_FLEXIBLE_ARRAY(uint16_t, imc_offtab); /* [(imc_offidx_max - imc_offidx_min) + 1]
	                                                * Array of offsets to `iconv_mbcs_byte2_range',
	                                                * originating from the start of this structure. */
	/* This is  where  all  of  the  iconv_mbcs_byte2_range-structures
	 * exist. They must be accessed via the offsets from `imc_offtab'! */
	/* TODO: Encode data (probably in  a separate database since  there
	 *       is no way of calculating the total sizeof() the codepage's
	 *       encode data (without going through all ranges)). Also, the
	 *       database file for encode data is already ~2.5MiB large, so
	 *       it stands to reason to put the rest into a separate  file. */
};

/* Check if `byte1_uniord'  has an  associated table,  where
 * `byte1_uniord' is the result of `self->imc_1byte[BYTE1]'. */
#define iconv_mbcs_codepage_hasbyte2(self, byte1_uniord) \
	((byte1_uniord) >= (self)->imc_offidx_min && (byte1_uniord) <= (self)->imc_offidx_max)
/* Return the first range holding decode information about the second byte after `byte1_uniord' */
#define iconv_mbcs_codepage_getbyte2(self, byte1_uniord)                         \
	((struct iconv_mbcs_byte2_range const *)((byte_t const *)(self) +            \
	                                         (self)->imc_offtab[(byte1_uniord) - \
	                                                            (self)->imc_offidx_min]))

typedef uint32_t libiconv_mbcs_offset_t;
struct iconv_mbcs_database;
INTDEF struct iconv_mbcs_database const libiconv_mbcs_db;
INTDEF libiconv_mbcs_offset_t const libiconv_mbcs_offsets[];

/* Return the mbcs code page associated with `codec'.
 * The caller must ensure that `codec >= CODEC_MBCS_MIN && codec <= CODEC_MBCS_MAX' */
#define libiconv_mbcs_page(codec)                                            \
	(struct iconv_mbcs_codepage const *)((byte_t const *)&libiconv_mbcs_db + \
	                                     libiconv_mbcs_offsets[(codec)-CODEC_MBCS_MIN])


/************************************************************************/
/* MBCS code page encode/decode functions.                              */
/************************************************************************/
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_mbcs_decode(struct iconv_decode *__restrict self, /*cp???*/ char const *__restrict data, size_t size);
INTDEF NONNULL((1, 2)) ssize_t FORMATPRINTER_CC libiconv_mbcs_encode(struct iconv_encode *__restrict self, /*utf-8*/ char const *__restrict data, size_t size);


/* Initialize a mbcs encoder/decoder. */
INTDEF NONNULL((1, 2)) void CC libiconv_mbcs_decode_init(struct iconv_decode *__restrict self, /*out*/ struct iconv_printer *__restrict input);
INTDEF NONNULL((1, 2)) void CC libiconv_mbcs_encode_init(struct iconv_encode *__restrict self, /*out*/ struct iconv_printer *__restrict input);

DECL_END
#endif /* CODEC_MBCS_COUNT != 0 */

#endif /* !GUARD_LIBICONV_MBCS_CP_MBCS_H */
