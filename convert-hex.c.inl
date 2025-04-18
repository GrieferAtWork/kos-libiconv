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
#ifdef __INTELLISENSE__
#include "convert.c"
#endif /* __INTELLISENSE__ */

DECL_BEGIN

INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_hex_upper_encode(struct iconv_encode *__restrict self,
                          /*bytes*/ char const *__restrict data,
                          size_t size) {
	ssize_t temp, result = 0;
	while (size) {
		unsigned char ch;
		char buf[2];
		ch = (unsigned char)*data;
		buf[0] = _itoa_upper_digits[(ch & 0xf0) >> 4];
		buf[1] = _itoa_upper_digits[ch & 0x0f];
		DO_encode_output(buf, 2);
		++data;
		--size;
	}
	return result;
err:
	return temp;
}

INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_hex_lower_encode(struct iconv_encode *__restrict self,
                          /*bytes*/ char const *__restrict data,
                          size_t size) {
	ssize_t temp, result = 0;
	while (size) {
		unsigned char ch;
		char buf[2];
		ch = (unsigned char)*data;
		buf[0] = _itoa_lower_digits[(ch & 0xf0) >> 4];
		buf[1] = _itoa_lower_digits[ch & 0x0f];
		DO_encode_output(buf, 2);
		++data;
		--size;
	}
	return result;
err:
	return temp;
}


INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_hex_decode(struct iconv_decode *__restrict self,
                    /*ascii*/ char const *__restrict data, size_t size) {
	ssize_t temp, result = 0;
	if unlikely(self->icd_flags & ICONV_HASERR)
		goto err_ilseq;
	while (size) {
		uint8_t nibble;
		unsigned char ch;
		ch = (unsigned char)*data;
		if (ch >= '0' && ch <= '9') {
			nibble = ch - '0';
		} else if (ch >= 'a' && ch <= 'f') {
			nibble = 10 + ch - 'a';
		} else if (ch >= 'A' && ch <= 'F') {
			nibble = 10 + ch - 'A';
		} else {
			if (IS_ICONV_ERR_ERROR_OR_ERRNO(self->icd_flags))
				goto err_ilseq;
			if (!IS_ICONV_ERR_DISCARD(self->icd_flags)) {
				if (IS_ICONV_ERR_REPLACE(self->icd_flags))
					ch = '?';
				DO_decode_output((char const *)&ch, 1);
			}
			goto next_data;
		}
		if (self->icd_data.idd_hex == 0x01) {
			self->icd_data.idd_hex = nibble << 4;
		} else {
			nibble |= self->icd_data.idd_hex;
			self->icd_data.idd_hex = 0x01;
			DO_decode_output((char const *)&nibble, 1);
		}
next_data:
		++data;
		--size;
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



DECL_END
