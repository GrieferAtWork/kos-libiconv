/* Copyright (c) 2019-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2019-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifdef __INTELLISENSE__
#include "convert.c"
#endif /* __INTELLISENSE__ */

#include <assert.h>

DECL_BEGIN

/*[[[deemon
import util;
import * from deemon;
local base64_ord2chr = Tuple({
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/',
}.each.ord());
local base64_chr2ord = [0xff] * 256;
for (local ord, chr: util.enumerate(base64_ord2chr))
	base64_chr2ord[chr] = ord;
base64_chr2ord['='.ord()] = 0;
print("static char const base64_ord2chr[64] = {");
for (local line: base64_ord2chr.segments(8))
	print("	", ", ".join(for (local x: line) f"'{string.chr(x)}'"), ",");
print("};");

print("static byte_t const base64_chr2ord[256] = {");
for (local line: base64_chr2ord.segments(16))
	print("	", ", ".join(for (local x: line) x.hex(2)), ",");
print("};");
]]]*/
static char const base64_ord2chr[64] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};
static byte_t const base64_chr2ord[256] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e, 0xff, 0xff, 0xff, 0x3f,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
	0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
/*[[[end]]]*/

INTERN NONNULL((1)) ssize_t
NOTHROW_NCX(CC libiconv_base64_encode_flush)(struct iconv_encode *__restrict self) {
	ssize_t result;
	char buf[COMPILER_STRLEN(".==")], *p = buf;

	/* Check for unwritten, pending data to form whole 6-bit words */
	switch (self->ice_data.ied_base64.b64_pend & _ICONV_ENCODE_BASE64_PEND_SIZE) {

	case _ICONV_ENCODE_BASE64_PEND_S0:
		return 0;

	case _ICONV_ENCODE_BASE64_PEND_S2:
		/* 2 bits: must encode 3 chars from 2 (nul-)input bytes to align
		 * >> ??1111
		 * >> 111122  (=)
		 * >> 222222  (=) */
		*p++ = base64_ord2chr[self->ice_data.ied_base64.b64_pend & 0x30];
		*p++ = '=';
		*p++ = '=';
		break;

	case _ICONV_ENCODE_BASE64_PEND_S4:
		/* 4 bits: must encode 2 chars from 1 (nul-)input byte to align
		 * >> ????11
		 * >> 111111  (=) */
		*p++ = base64_ord2chr[self->ice_data.ied_base64.b64_pend & 0x3c];
		*p++ = '=';
		break;

	default: __builtin_unreachable();
	}
	result = (*self->ice_output.ii_printer)(self->ice_output.ii_arg, buf, (size_t)(p - buf));
	if likely(result >= 0)
		self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S0;
	return result;
}


INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_base64_encode(struct iconv_encode *__restrict self,
                       /*bytes*/ char const *__restrict data,
                       size_t size) {
	char buf[128], *p = buf;
	ssize_t temp, result = 0;

	/* Check for unwritten, pending data to form whole 6-bit words */
	switch (self->ice_data.ied_base64.b64_pend & _ICONV_ENCODE_BASE64_PEND_SIZE) {

	case _ICONV_ENCODE_BASE64_PEND_S0:
		break;

	case _ICONV_ENCODE_BASE64_PEND_S2:
		/* 2 bits: must encode 3 chars from 2 input bytes to align
		 * >> ??1111
		 * >> 111122
		 * >> 222222 */
		if (size >= 2) {
			byte_t d1 = (byte_t)data[0];
			byte_t d2 = (byte_t)data[1];
			*p++ = base64_ord2chr[(self->ice_data.ied_base64.b64_pend & 0x30) | ((d1 & 0xf0) >> 4)];
			*p++ = base64_ord2chr[(d1 & 0x0f) << 2 | (d2 & 0xc0) >> 6];
			*p++ = base64_ord2chr[d2 & 0x3f];
			size -= 2;
			data += 2;
		} else {
			byte_t d1;
			if unlikely(!size)
				goto done;
			/* >> ??1111
			 * >> 1111?? */
			d1 = (byte_t)data[0];
			*p++ = base64_ord2chr[(self->ice_data.ied_base64.b64_pend & 0x30) | ((d1 & 0xf0) >> 4)];
			self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S4 | ((d1 & 0x0f) << 2);
			goto done_flush;
		}
		self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S0;
		break;

	case _ICONV_ENCODE_BASE64_PEND_S4: {
		byte_t d1;
		/* 4 bits: must encode 2 chars from 1 input bytes to align
		 * >> ????11
		 * >> 111111 */
		if unlikely(!size)
			goto done;
		d1 = (byte_t)data[0];
		*p++ = base64_ord2chr[(self->ice_data.ied_base64.b64_pend & 0x3c) | ((d1 & 0xc0) >> 6)];
		*p++ = base64_ord2chr[d1 & 0x3f];
		self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S0;
		size -= 1;
		data += 1;
	}	break;

	default: __builtin_unreachable();
	}

	while (size >= 3) {
		byte_t d1, d2, d3;
		/* Output whole words:
		 * >> 111111
		 * >> 112222
		 * >> 222233
		 * >> 333333 */
		if (p > (COMPILER_ENDOF(buf) - 4)) {
			/* Flush buffer. */
			DO_encode_output(buf, (size_t)(p - buf));
			p = buf;
		}
		d1 = (byte_t)data[0];
		d2 = (byte_t)data[1];
		d3 = (byte_t)data[2];
		*p++ = base64_ord2chr[((d1 & 0xfc) >> 2)];
		*p++ = base64_ord2chr[((d1 & 0x03) << 4) | ((d2 & 0xf0) >> 4)];
		*p++ = base64_ord2chr[((d2 & 0x0f) << 2) | ((d3 & 0xc0) >> 6)];
		*p++ = base64_ord2chr[((d3 & 0x3f))];
		data += 3;
		size -= 3;
	}

	/* Remember unwritten bytes */
	switch (size) {

	case 0:
		break;

	case 1: {
		/* >> 111111
		 * >> 11???? */
		byte_t d1 = data[0];
		if (p > (COMPILER_ENDOF(buf) - 1)) {
			/* Flush buffer. */
			DO_encode_output(buf, (size_t)(p - buf));
			p = buf;
		}
		*p++ = base64_ord2chr[((d1 & 0xfc) >> 2)];
		self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S2 | ((d1 & 0x03) << 4);
	}	break;

	case 2: {
		/* >> 111111
		 * >> 112222
		 * >> 2222?? */
		byte_t d1 = data[0];
		byte_t d2 = data[1];
		if (p > (COMPILER_ENDOF(buf) - 2)) {
			/* Flush buffer. */
			DO_encode_output(buf, (size_t)(p - buf));
			p = buf;
		}
		*p++ = base64_ord2chr[((d1 & 0xfc) >> 2)];
		*p++ = base64_ord2chr[((d1 & 0x03) << 4) | ((d2 & 0xf0) >> 4)];
		self->ice_data.ied_base64.b64_pend = _ICONV_ENCODE_BASE64_PEND_S4 | ((d2 & 0x0f) << 2);
	}	break;

	default: __builtin_unreachable();
	}

	/* Flush written, but buffered data */
done_flush:
	DO_encode_output(buf, (size_t)(p - buf));
done:
	return result;
err:
	return temp;
}



INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_base64_decode(struct iconv_decode *__restrict self,
                       /*ascii*/ char const *__restrict data,
                       size_t size) {
	char const *end = data + size;
	byte_t buf[128], *p = buf;
	ssize_t temp, result = 0;
	if unlikely(self->icd_flags & ICONV_HASERR)
		goto err_ilseq;
	/* Skip line-feeds */
again:
	while (data < end && (*data == '\r' || *data == '\n'))
		++data;
	if likely(data < end) {
		switch (__builtin_expect(self->icd_data.idd_base64.b64_state,
		                         _ICONV_DECODE_BASE64_T0)) {

		case _ICONV_DECODE_BASE64_T0: {
			size_t rem;
			/* Consume all caller-given 4-char-to-3-byte sequences */
			while (end >= data + 4) {
				/* NOTE: '=' gets mapped to `0x00' by `base64_chr2ord' */
				byte_t v0 = base64_chr2ord[(unsigned char)data[0]];
				byte_t v1 = base64_chr2ord[(unsigned char)data[1]];
				byte_t v2 = base64_chr2ord[(unsigned char)data[2]];
				byte_t v3 = base64_chr2ord[(unsigned char)data[3]];
				if unlikely((v0 | v1 | v2 | v3) == 0xff) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					/* Handle error (and case where there is a line-feed) */
					DO(libiconv_base64_decode(self, data + 0, 1));
					DO(libiconv_base64_decode(self, data + 1, 1));
					DO(libiconv_base64_decode(self, data + 2, 1));
					DO(libiconv_base64_decode(self, data + 3, 1));
					data += 4;
					p = buf;
					goto again;
				}
				if unlikely(p > (COMPILER_ENDOF(buf) - 3)) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					p = buf;
				}
				*p++ = (v0 << 2) | ((v1 & 0x30) >> 4);
				*p++ = ((v1 & 0x0f) << 4) | ((v2 & 0x3c) >> 2);
				*p++ = ((v2 & 0x03) << 6) | v3;
				if unlikely((v0 & v1 & v2 & v3) == 0x00) {
					/* Check if there is a '=' somewhere in there. */
					if (data[0] == '=' || data[1] == '=')
						goto err_ilseq_pm3;
					if (data[2] == '=') {
						if (data[3] != '=')
							goto err_ilseq_pm3;
						/* XX== */
						if (p[-2] != 0x00) /* Can only take away NUL-bytes! */
							goto err_ilseq_pm3;
						p -= 2;
						data += 4;
						self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EOF;
						goto again;
					}
					if (data[3] == '=') {
						/* XXX= */
						if (p[-1] != 0x00) /* Can only take away NUL-bytes! */
							goto err_ilseq_pm3;
						p -= 1;
						data += 4;
						self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EOF;
						goto again;
					}
				}
				data += 4;
			}
			rem = (size_t)(end - data);
			assert(rem >= 0 && rem <= 3);
			switch (rem) {

			case 0:
				assert(self->icd_data.idd_base64.b64_state == _ICONV_DECODE_BASE64_T0);
				break;

			case 1: {
				byte_t v0 = base64_chr2ord[(unsigned char)data[0]];
				if unlikely((v0) == 0xff)
					goto err_ilseq;
				if unlikely((v0) == 0x00 && data[0] == '=')
					goto err_ilseq;
				self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T6;
				self->icd_data.idd_base64.b64_vprev = v0 << 2;
				data += 1;
			}	break;

			case 2: {
				byte_t v0 = base64_chr2ord[(unsigned char)data[0]];
				byte_t v1 = base64_chr2ord[(unsigned char)data[1]];
				if unlikely(((v0 | v1) == 0xff) ||
				            ((v0 & v1) == 0x00 && (data[0] == '=' || data[1] == '='))) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					DO(libiconv_base64_decode(self, data + 0, 1));
					DO(libiconv_base64_decode(self, data + 1, 1));
					data += 2;
					p = buf;
					goto again;
				}
				if unlikely(p > (COMPILER_ENDOF(buf) - 1)) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					p = buf;
				}
				*p++ = (v0 << 2) | ((v1 & 0x30) >> 4);
				self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T12;
				self->icd_data.idd_base64.b64_vprev = v1 << 4;
				data += 2;
			}	break;

			case 3: {
				byte_t v0 = base64_chr2ord[(unsigned char)data[0]];
				byte_t v1 = base64_chr2ord[(unsigned char)data[1]];
				byte_t v2 = base64_chr2ord[(unsigned char)data[2]];
				if unlikely(((v0 | v1 | v2) == 0xff) ||
				            ((v0 & v1) == 0x00 && (data[0] == '=' || data[1] == '='))) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					DO(libiconv_base64_decode(self, data + 0, 1));
					DO(libiconv_base64_decode(self, data + 1, 1));
					DO(libiconv_base64_decode(self, data + 2, 1));
					data += 3;
					p = buf;
					goto again;
				}
				if unlikely(p > (COMPILER_ENDOF(buf) - 1)) {
					DO_decode_output((char *)buf, (size_t)(p - buf));
					p = buf;
				}
				*p++ = (v0 << 2) | ((v1 & 0x30) >> 4);
				*p++ = ((v1 & 0x0f) << 4) | ((v2 & 0x3c) >> 2);
				if (v2 == 0 && data[2] == '=') {
					/* Special case: 'XX=' (with next not-yet-given character needing to be a '=') */
					if (p[-1] != 0x00) /* Can only take away NUL-bytes! */
						goto err_ilseq_pm2;
					p -= 1;
					self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EQ2;
				} else {
					self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T18;
					self->icd_data.idd_base64.b64_vprev = v2 << 6;
				}
				data += 3;
			}	break;

			default: __builtin_unreachable();
			}

		}	break;

		case _ICONV_DECODE_BASE64_T6: {
			byte_t v1 = base64_chr2ord[(unsigned char)data[0]];
			if unlikely(v1 == 0xff)
				goto err_ilseq;
			if unlikely(v1 == 0x00 && data[0] == '=')
				goto err_ilseq;
			if unlikely(p > (COMPILER_ENDOF(buf) - 1)) {
				DO_decode_output((char *)buf, (size_t)(p - buf));
				p = buf;
			}
			*p++ = self->icd_data.idd_base64.b64_vprev | ((v1 & 0x30) >> 4);
			self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T12;
			self->icd_data.idd_base64.b64_vprev = (v1 & 0x0f) << 4;
			++data;
			goto again;
		}	break;

		case _ICONV_DECODE_BASE64_T12: {
			byte_t v2 = base64_chr2ord[(unsigned char)data[0]];
			if unlikely(v2 == 0xff)
				goto err_ilseq;
			if unlikely(v2 == 0x00 && data[0] == '=') {
				/* Special case: XX== */
				if unlikely(self->icd_data.idd_base64.b64_vprev != 0x00)
					goto err_ilseq;
				self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EQ2;
				++data;
				goto again;
			}
			if unlikely(p > (COMPILER_ENDOF(buf) - 1)) {
				DO_decode_output((char *)buf, (size_t)(p - buf));
				p = buf;
			}
			*p++ = self->icd_data.idd_base64.b64_vprev | (v2 >> 2);
			self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T18;
			self->icd_data.idd_base64.b64_vprev = (v2 & 0x03) << 6;
			++data;
			goto again;
		}	break;

		case _ICONV_DECODE_BASE64_T18: {
			byte_t v3 = base64_chr2ord[(unsigned char)data[0]];
			if unlikely(v3 == 0xff)
				goto err_ilseq;
			if unlikely(v3 == 0x00 && data[0] == '=') {
				/* Special case: XXX= */
				if unlikely(self->icd_data.idd_base64.b64_vprev != 0x00)
					goto err_ilseq;
				self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EOF;
				++data;
				goto again;
			}
			if unlikely(p > (COMPILER_ENDOF(buf) - 1)) {
				DO_decode_output((char *)buf, (size_t)(p - buf));
				p = buf;
			}
			*p++ = self->icd_data.idd_base64.b64_vprev | v3;
			self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_T0;
			DBG_memset(&self->icd_data.idd_base64.b64_vprev, 0xcc,
			           sizeof(self->icd_data.idd_base64.b64_vprev));
			++data;
			goto again;
		}	break;

		case _ICONV_DECODE_BASE64_EQ2:
			if likely(*data == '=') {
				++data;
				self->icd_data.idd_base64.b64_state = _ICONV_DECODE_BASE64_EOF;
				goto again;
			}
			goto err_ilseq;

		case _ICONV_DECODE_BASE64_EOF:
			/* No further input should arrive once EOF has been reached */
			goto err_ilseq;

		default: __builtin_unreachable();
		}
	}
	if likely(p > buf)
		DO_decode_output((char *)buf, (size_t)(p - buf));
	return result;
err:
	return temp;
err_ilseq_pm3:
	p -= 1;
err_ilseq_pm2:
	p -= 2;
err_ilseq:
	DO_decode_output((char *)buf, (size_t)(p - buf));
	self->icd_flags |= ICONV_HASERR;
	if (IS_ICONV_ERR_ERRNO(self->icd_flags))
		errno = EILSEQ;
	return -(ssize_t)(size_t)(end - data);
}


DECL_END
