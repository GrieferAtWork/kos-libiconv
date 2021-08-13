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
#ifndef GUARD_LIBICONV_STDICONV_C
#define GUARD_LIBICONV_STDICONV_C 1
#define _KOS_SOURCE 1

#include "api.h"
/**/

#include <__crt.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unicode.h>

#include <libiconv/iconv.h>
#include <libiconv/stdiconv.h>

#include "codecs.h"
#include "iconv.h"
#include "stdiconv.h"

DECL_BEGIN

/* Opaque object used by the functions below. */
struct stdiconv {
	struct iconv_decode  si_decode;        /* Decoder */
	struct iconv_encode  si_encode;        /* Encoder */
	struct iconv_printer si_decode_input;  /* [const] Input printer for decoding. */
	struct iconv_printer si_encode_input;  /* [const] Input printer for encoding. */
	mbstate_t            si_converted_mbs; /* Internal mb-state used for `si_converted' */
	size_t               si_converted;     /* # of characters converted. */
	char                *si_outbuf;        /* [0..si_outsiz] Pointer to the output buffer. */
	size_t               si_outsiz;        /* # of bytes left in the output buffer. */
};

PRIVATE ssize_t FORMATPRINTER_CC
stdiconv_count_characters_and_forward(struct stdiconv *__restrict self,
                                      /*utf-8*/ char const *__restrict data,
                                      size_t datalen) {
	/* While  also supporting byte-wise processing through use of
	 * a shift-state, counter the # of unicode characters as data
	 * is passed to out function  before forwarding said data  to
	 * the input printer of the encoder. */
	char const *ptr = data;
	size_t len      = datalen;
	while (len) {
		char32_t c32;
		size_t status;
		status = unicode_c8toc32(&c32, ptr, len, &self->si_converted_mbs);
		if unlikely((ssize_t)status < 0) {
			if (status == (size_t)-1) {
				errno = EILSEQ;
				return -1;
			} else if (status == (size_t)-2) {
				break;
			}
		}
		ptr += status;
		len -= status;
		++self->si_converted;
	}
	return (*self->si_encode_input.ii_printer)(self->si_encode_input.ii_arg,
	                                           data, datalen);
}

PRIVATE ssize_t FORMATPRINTER_CC
stdiconv_copy_to_outbuf(struct stdiconv *__restrict self,
                        /*ABSTRACT*/ char const *__restrict data,
                        size_t datalen) {
	if (self->si_outsiz < datalen)
		return -2; /* E2BIG */
	memcpy(self->si_outbuf, data, datalen);
	self->si_outbuf += datalen;
	self->si_outsiz -= datalen;
	return (ssize_t)datalen;
}


/* Create a new conversion descriptor for converting `fromcode' --> `tocode'. */
INTERN WUNUSED struct stdiconv *
NOTHROW_NCX(LIBCCALL libiconv_stdiconv_open)(const char *tocode,
                                             const char *fromcode) {
	unsigned int input_codec;
	unsigned int output_codec;
	struct stdiconv *result;
	uintptr_t decode_flags = ICONV_ERR_ERRNO;
	uintptr_t encode_flags = ICONV_ERR_ERRNO;

	/* Figure out codec names and parse additional flags. */
	input_codec  = libiconv_codec_and_flags_byname(fromcode, &decode_flags);
	output_codec = libiconv_codec_and_flags_byname(tocode, &encode_flags);
	if (input_codec == 0 || output_codec == 0)
		return NULL;

	result = (struct stdiconv *)malloc(sizeof(struct stdiconv));
	if unlikely(!result)
		goto err;

	/* Set-up  printers. Note that we need to inject an intermediate printer
	 * between the decoder and encoder just so we can count the # of unicode
	 * characters */
	result->si_encode.ice_output.ii_printer = (pformatprinter)&stdiconv_count_characters_and_forward;
	result->si_encode.ice_output.ii_arg     = result;
	result->si_encode.ice_flags             = encode_flags;
	result->si_decode.icd_output.ii_printer = (pformatprinter)&stdiconv_copy_to_outbuf;
	result->si_decode.icd_output.ii_arg     = result;
	result->si_decode.icd_flags             = decode_flags;
	__mbstate_init(&result->si_converted_mbs);

	/* Initialize the decoder and encoder. */
	if unlikely(libiconv_decode_init(&result->si_decode,
	                                 &result->si_decode_input,
	                                 input_codec) != 0)
		goto err_r;
	if unlikely(libiconv_encode_init(&result->si_encode,
	                                 &result->si_encode_input,
	                                 output_codec) != 0)
		goto err_r;

	/* And that's it! */
	return result;
err_r:
	free(result);
err:
	return (struct stdiconv *)(iconv_t)-1;
}

/* Close an iconv descriptor, as previously opened by `iconv_open(3)'. */
INTERN int
NOTHROW_NCX(LIBCCALL libiconv_stdiconv_close)(struct stdiconv *self) {
	if unlikely(!self || self == (struct stdiconv *)(iconv_t)-1) {
		errno = EBADF;
		return -1;
	}
	free(self);
	return 0;
}


/* Convert data from one codec into another.
 *
 * Author's  note: don't use this interface if  you're targeting KOS. Use the much
 * better pformatprinter-based interface instead! Also: technically speaking, this
 * one is also  just a wrapper  around that one.  (though one that  adds a lot  of
 * overhead for functionality you probably don't actually be need)
 *
 * @param: self:         Conversion controller. (s.a. `iconv_open(3)')
 * @param: inbuf:        [0..*inbytesleft][0..1] When NULL (or pointing to NULL),
 *                       do nothing. Otherwise, pointer to the start of data that
 *                       has yet to be converted. (updated during the call)
 * @param: inbytesleft:  [1..1][valid_if(inbuf)]
 *                       [in]  The max # of bytes to read from `*inbuf'
 *                       [out] # of bytes not taken  from `*inbuf'.
 *                             Always set to `0' on success.
 * @param: outbuf:       [1..*outbytesleft][1..1][valid_if(inbuf)]
 *                       Pointer to the start of a memory region that should be
 *                       filled with data to-be converted.
 * @param: outbytesleft: [1..1][valid_if(inbuf)] # of bytes available in `outbuf'.
 *                       Updated over the course of the call.
 * @return: * : The # of decoded+encoded unicode characters. (technically only the
 *              # of decoded characters, but that  should normally equal the #  of
 *              encoded ones, and should they not match, the specs don't say  what
 *              should be returned then, so...)
 * @return: (size_t)-1: [errno=EILSEQ] Invalid sequence  in input  (*inbuf now  points
 *                                     to the start of said sequence, but *inbytesleft
 *                                     is left unchanged)
 * @return: (size_t)-1: [errno=EINVAL] Incomplete multi-byte sequence encountered. This
 *                                     error doesn't happen  on KOS because  incomplete
 *                                     sequences are  handled by  remembering the  part
 *                                     already encountered and  expecting a later  call
 *                                     to  provide  the missing  part. If  that doesn't
 *                                     happen,  then it's an invalid sequence (EILSEQ).
 * @return: (size_t)-1: [errno=E2BIG]  Output buffer is too small. (Arguments are left
 *                                     unchanged). */
INTERN size_t
NOTHROW_NCX(LIBCCALL libiconv_stdiconv)(struct stdiconv *self,
                                        char **__restrict inbuf, size_t *__restrict inbytesleft,
                                        char **__restrict outbuf, size_t *__restrict outbytesleft) {
	uintptr_t saved_encode_flags;
	uintptr_t saved_decode_flags;
	union iconv_decode_data saved_decode_data;
	union iconv_encode_data saved_encode_data;
	mbstate_t saved_converted_mbs;
#define STDICONV_SAVE()                               \
	(saved_decode_flags  = self->si_decode.icd_flags, \
	 saved_decode_data   = self->si_decode.icd_data,  \
	 saved_encode_flags  = self->si_encode.ice_flags, \
	 saved_encode_data   = self->si_encode.ice_data,  \
	 saved_converted_mbs = self->si_converted_mbs)
#define STDICONV_LOAD()                              \
	(self->si_decode.icd_flags = saved_decode_flags, \
	 self->si_decode.icd_data  = saved_decode_data,  \
	 self->si_encode.ice_flags = saved_encode_flags, \
	 self->si_encode.ice_data  = saved_encode_data,  \
	 self->si_converted_mbs    = saved_converted_mbs)

	ssize_t status;
	if unlikely(!self || self == (struct stdiconv *)(iconv_t)-1) {
		errno = EBADF;
		return -1;
	}

	if unlikely(!inbuf || !*inbuf)
		return 0; /* no-op */

	/* Fill in information about where data should go. */
	self->si_outbuf = *outbuf;
	self->si_outsiz = *outbytesleft;

	/* Save a couple of fields we may need if we have to rewind. */
	STDICONV_SAVE();

	/* Reset the conversion counter because we only care about the current instance. */
	self->si_converted = 0;

	/* (try) to do the actual conversion all in one step. */
	status = (*self->si_decode_input.ii_printer)(self->si_decode_input.ii_arg,
	                                             *inbuf, *inbytesleft);

	/* Check for errors. */
	if (status < 0) {
		if (status == -1) {
			/* EILSEQ: For compliance, we have to  fix-up `inbuf' so that  it
			 *         points to the start of the bad sequence. We do this by
			 *         feeding input one byte at a time until it breaks. */
			STDICONV_LOAD();

			/* Prevent data from making it out of the encoder */
			if (self->si_encode_input.ii_printer == (pformatprinter)&stdiconv_copy_to_outbuf) {
				/* This can happen in case the encode function is a no-op (e.g. `tocode' is `utf-8') */
				self->si_encode_input.ii_printer = &format_length;
			} else {
				assert(self->si_decode.icd_output.ii_printer == (pformatprinter)&stdiconv_copy_to_outbuf);
				self->si_decode.icd_output.ii_printer = &format_length;
			}

			/* Search for the specific location where the error happens. */
			while (*inbytesleft) {
				status = (*self->si_decode_input.ii_printer)(self->si_decode_input.ii_arg,
				                                             *inbuf, 1);
				if (status < 0)
					break;
				++*inbuf;
				--*inbytesleft; /* Hey! Look at that: we're nice enough to keep this one valid, too! */
			}

			/* Restore the correct printer for output to buffers. */
			if (self->si_encode_input.ii_printer == &format_length) {
				/* This can happen in case the encode function is a no-op (e.g. `tocode' is `utf-8') */
				self->si_encode_input.ii_printer = (pformatprinter)&stdiconv_copy_to_outbuf;
			} else {
				assert(self->si_decode.icd_output.ii_printer == &format_length);
				self->si_decode.icd_output.ii_printer = (pformatprinter)&stdiconv_copy_to_outbuf;
			}

			/* Restore the decoder into its original configuration. */
return_with_EILSEQ:
			STDICONV_LOAD();
			return (size_t)-1;
		} else if (status == -2) {
			/* E2BIG: Restore iconv state and set E2BIG. */
			STDICONV_LOAD();

			/* In order to properly update the caller-given pointers, we need
			 * to figure out the exact position at which we're no longer able
			 * to  write to the  output buffer. For  this purpose, start over
			 * but only offer one byte  at a time so  we can return with  the
			 * iconv state such that only a single additional character  will
			 * result in an overflow of the output buffer. */
			self->si_outbuf = *outbuf;
			self->si_outsiz = *outbytesleft;
			self->si_converted = 0;
			for (;;) {
				if unlikely(!*inbytesleft)
					goto success; /* Shouldn't happen... */
				status = (*self->si_decode_input.ii_printer)(self->si_decode_input.ii_arg,
				                                             *inbuf, 1);
				if (status < 0) {
					if unlikely(status == -1) {
						/* Shouldn't happen... (EILSEQ during second pass?) */
						goto return_with_EILSEQ;
					}
					assertf(status == -2, "status = %" PRIdSIZ, status);
					break;
				}
				++*inbuf;
				--*inbytesleft;

				/* We're still good, so create another back-up. */
				STDICONV_SAVE();

#ifndef __OPTIMIZE_SIZE__
				/* Optimization check: if the output is entirety filled, we can stop now. */
				if (self->si_outsiz == 0) {
					if unlikely(!*inbytesleft)
						goto success; /* Shouldn't happen... */
					break;
				}
#endif /* !__OPTIMIZE_SIZE__ */
			}

			/* Load the last-valid state prior to the buffer overflow happening. */
			STDICONV_LOAD();

			/* `*inbuf' and `*inbytesleft'  were already updated  in the loop  above.
			 * And since we  know that trying  to write the  next byte (`*inbuf')  to
			 * the decode function's input will result  in a buffer overflow, we  can
			 * now  update the output-buffer  pointers to indicate  how much space is
			 * still  unused. Usually,  nothing will be  unused, but in  case we need
			 * to write a multi-byte character into  the output, it may be  non-empty
			 * but still too small to be able to hold that character in its entirety. */
			*outbuf       = self->si_outbuf;
			*outbytesleft = self->si_outsiz;

			/* Return with E2BIG. */
			errno = E2BIG;
			return (size_t)-1;
		}
	}

	/* Update buffer pointers to indicate that all was converted! */
	*inbuf += *inbytesleft;
	*inbytesleft  = 0;
success:
	*outbuf       = self->si_outbuf;
	*outbytesleft = self->si_outsiz;

	/* Must return the # of converted unicode characters. */
	return self->si_converted;
#undef STDICONV_LOAD
#undef STDICONV_SAVE
}


DEFINE_PUBLIC_ALIAS(iconv_open, libiconv_stdiconv_open);
DEFINE_PUBLIC_ALIAS(iconv_close, libiconv_stdiconv_close);
DEFINE_PUBLIC_ALIAS(iconv, libiconv_stdiconv);

DECL_END

#endif /* !GUARD_LIBICONV_STDICONV_C */
