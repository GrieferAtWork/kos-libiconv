/*[[[magic
options["COMPILE.language"] = "c";
local gcc_opt = options.setdefault("GCC.options", []);
gcc_opt.remove("-g"); // Disable debug informations for this file!
]]]*/
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
#ifndef GUARD_LIBICONV_CP_ISO646_C
#define GUARD_LIBICONV_CP_ISO646_C 1

#include "api.h"
/**/

#include <hybrid/byteorder.h>

#include <kos/types.h>

#include <libiconv/iconv.h>

#include "codecs.h"
#include "cp-iso646.h"

#if CODEC_ISO646_COUNT != 0
DECL_BEGIN

/*[[[deemon (printCpIso646Database from .iconvdata.iconvdata)();]]]*/
STATIC_ASSERT(CODEC_ISO646_MIN + 0 == CODEC_DK_US);
STATIC_ASSERT(CODEC_ISO646_MIN + 1 == CODEC_ISO646_BASIC);
STATIC_ASSERT(CODEC_ISO646_MIN + 2 == CODEC_ISO646_CA);
STATIC_ASSERT(CODEC_ISO646_MIN + 3 == CODEC_ISO646_CA2);
STATIC_ASSERT(CODEC_ISO646_MIN + 4 == CODEC_ISO646_CN);
STATIC_ASSERT(CODEC_ISO646_MIN + 5 == CODEC_ISO646_CU);
STATIC_ASSERT(CODEC_ISO646_MIN + 6 == CODEC_ISO646_DE);
STATIC_ASSERT(CODEC_ISO646_MIN + 7 == CODEC_ISO646_DK);
STATIC_ASSERT(CODEC_ISO646_MIN + 8 == CODEC_ISO646_DK_NO_ALT_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 9 == CODEC_ISO646_DK_NO_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 10 == CODEC_ISO646_ES);
STATIC_ASSERT(CODEC_ISO646_MIN + 11 == CODEC_ISO646_ES2);
STATIC_ASSERT(CODEC_ISO646_MIN + 12 == CODEC_ISO646_FI_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 13 == CODEC_ISO646_FR);
STATIC_ASSERT(CODEC_ISO646_MIN + 14 == CODEC_ISO646_FR1);
STATIC_ASSERT(CODEC_ISO646_MIN + 15 == CODEC_ISO646_GB);
STATIC_ASSERT(CODEC_ISO646_MIN + 16 == CODEC_ISO646_GB_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 17 == CODEC_ISO646_HU);
STATIC_ASSERT(CODEC_ISO646_MIN + 18 == CODEC_ISO646_IE);
STATIC_ASSERT(CODEC_ISO646_MIN + 19 == CODEC_ISO646_INIS_SUBSET);
STATIC_ASSERT(CODEC_ISO646_MIN + 20 == CODEC_ISO646_INV);
STATIC_ASSERT(CODEC_ISO646_MIN + 21 == CODEC_ISO646_IRV_1983);
STATIC_ASSERT(CODEC_ISO646_MIN + 22 == CODEC_ISO646_IS);
STATIC_ASSERT(CODEC_ISO646_MIN + 23 == CODEC_ISO646_IT);
STATIC_ASSERT(CODEC_ISO646_MIN + 24 == CODEC_ISO646_JP);
STATIC_ASSERT(CODEC_ISO646_MIN + 25 == CODEC_ISO646_JP_OCR_B);
STATIC_ASSERT(CODEC_ISO646_MIN + 26 == CODEC_ISO646_KR);
STATIC_ASSERT(CODEC_ISO646_MIN + 27 == CODEC_ISO646_LATIN_GR_MIXED);
STATIC_ASSERT(CODEC_ISO646_MIN + 28 == CODEC_ISO646_MT);
STATIC_ASSERT(CODEC_ISO646_MIN + 29 == CODEC_ISO646_NATS_DANO);
STATIC_ASSERT(CODEC_ISO646_MIN + 30 == CODEC_ISO646_NL);
STATIC_ASSERT(CODEC_ISO646_MIN + 31 == CODEC_ISO646_NL_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 32 == CODEC_ISO646_NO);
STATIC_ASSERT(CODEC_ISO646_MIN + 33 == CODEC_ISO646_NO2);
STATIC_ASSERT(CODEC_ISO646_MIN + 34 == CODEC_ISO646_PL);
STATIC_ASSERT(CODEC_ISO646_MIN + 35 == CODEC_ISO646_PT);
STATIC_ASSERT(CODEC_ISO646_MIN + 36 == CODEC_ISO646_PT2);
STATIC_ASSERT(CODEC_ISO646_MIN + 37 == CODEC_ISO646_SE);
STATIC_ASSERT(CODEC_ISO646_MIN + 38 == CODEC_ISO646_SE2);
STATIC_ASSERT(CODEC_ISO646_MIN + 39 == CODEC_ISO646_SEFI_NATS);
STATIC_ASSERT(CODEC_ISO646_MIN + 40 == CODEC_ISO646_SE_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 41 == CODEC_ISO646_SWI_NRCS);
STATIC_ASSERT(CODEC_ISO646_MIN + 42 == CODEC_ISO646_T_61);
STATIC_ASSERT(CODEC_ISO646_MIN + 43 == CODEC_ISO646_VIEWDATA);
STATIC_ASSERT(CODEC_ISO646_MIN + 44 == CODEC_ISO646_YU);
STATIC_ASSERT(CODEC_ISO646_MIN + 45 == CODEC_ISO_IR_2);
STATIC_ASSERT(CODEC_ISO646_MIN + 46 == CODEC_ISO_IR_8_1);
STATIC_ASSERT(CODEC_ISO646_MIN + 47 == CODEC_US_DK);
INTERN_CONST struct iconv_iso646_codepage const libiconv_iso646_pages[48] = {
	/* [CODEC_DK_US - CODEC_ISO646_MIN]                 = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c4, 0x00d6, 0x005d, 0x005e, 0x005f, 0x0060, 0x00e4, 0x00f6, 0x007d, 0x007e } },
	/* [CODEC_ISO646_BASIC - CODEC_ISO646_MIN]          = */ { { 0x0021, 0x0022, 0x0000, 0x0000, 0x0026, 0x003a, 0x003f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x005f, 0x0000, 0x007b, 0x007c, 0x007d, 0x007e } },
	/* [CODEC_ISO646_CA - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00e0, 0x00e2, 0x00e7, 0x00ea, 0x00ee, 0x005f, 0x00f4, 0x00e9, 0x00f9, 0x00e8, 0x00fb } },
	/* [CODEC_ISO646_CA2 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00e0, 0x00e2, 0x00e7, 0x00ea, 0x00c9, 0x005f, 0x00f4, 0x00e9, 0x00f9, 0x00e8, 0x00fb } },
	/* [CODEC_ISO646_CN - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x00a5, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_CU - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x00a1, 0x00d1, 0x005d, 0x00bf, 0x005f, 0x0060, 0x00b4, 0x00f1, 0x005b, 0x00a8 } },
	/* [CODEC_ISO646_DE - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00a7, 0x00c4, 0x00d6, 0x00dc, 0x005e, 0x005f, 0x0060, 0x00e4, 0x00f6, 0x00fc, 0x00df } },
	/* [CODEC_ISO646_DK - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c6, 0x00d8, 0x00c5, 0x00dc, 0x005f, 0x0060, 0x00e6, 0x00f8, 0x00e5, 0x00fc } },
	/* [CODEC_ISO646_DK_NO_ALT_NRCS - CODEC_ISO646_MIN] = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c6, 0x00d8, 0x00c5, 0x005e, 0x005f, 0x0060, 0x00e6, 0x00f8, 0x00e5, 0x007e } },
	/* [CODEC_ISO646_DK_NO_NRCS - CODEC_ISO646_MIN]     = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00c4, 0x00c6, 0x00d8, 0x00c5, 0x00dc, 0x005f, 0x00e4, 0x00e6, 0x00f8, 0x00e5, 0x00fc } },
	/* [CODEC_ISO646_ES - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00a7, 0x00a1, 0x00d1, 0x00bf, 0x005e, 0x005f, 0x0060, 0x00b0, 0x00f1, 0x00e7, 0x007e } },
	/* [CODEC_ISO646_ES2 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x2022, 0x00a1, 0x00d1, 0x00c7, 0x00bf, 0x005f, 0x0060, 0x00b4, 0x00f1, 0x00e7, 0x00a8 } },
	/* [CODEC_ISO646_FI_NRCS - CODEC_ISO646_MIN]        = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c4, 0x00d6, 0x00c5, 0x00dc, 0x005f, 0x00e9, 0x00e4, 0x00f6, 0x00e5, 0x00fc } },
	/* [CODEC_ISO646_FR - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00e0, 0x00b0, 0x00e7, 0x00a7, 0x005e, 0x005f, 0x00b5, 0x00e9, 0x00f9, 0x00e8, 0x00a8 } },
	/* [CODEC_ISO646_FR1 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00e0, 0x00b0, 0x00e7, 0x00a7, 0x005e, 0x005f, 0x0060, 0x00e9, 0x00f9, 0x00e8, 0x00a8 } },
	/* [CODEC_ISO646_GB - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_GB_NRCS - CODEC_ISO646_MIN]        = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x007e } },
	/* [CODEC_ISO646_HU - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x00c1, 0x00c9, 0x00d6, 0x00dc, 0x005e, 0x005f, 0x00e1, 0x00e9, 0x00f6, 0x00fc, 0x02dd } },
	/* [CODEC_ISO646_IE - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00d3, 0x00c9, 0x00cd, 0x00da, 0x00c1, 0x005f, 0x00f3, 0x00e9, 0x00ed, 0x00fa, 0x00e1 } },
	/* [CODEC_ISO646_INIS_SUBSET - CODEC_ISO646_MIN]    = */ { { 0x0000, 0x0000, 0x0000, 0x0024, 0x0000, 0x003a, 0x0000, 0x0000, 0x005b, 0x0000, 0x005d, 0x0000, 0x0000, 0x0000, 0x0000, 0x007c, 0x0000, 0x0000 } },
	/* [CODEC_ISO646_INV - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0000, 0x0000, 0x0026, 0x003a, 0x003f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x005f, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 } },
	/* [CODEC_ISO646_IRV_1983 - CODEC_ISO646_MIN]       = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x007e } },
	/* [CODEC_ISO646_IS - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00d0, 0x00de, 0x005c, 0x00c6, 0x00d6, 0x005f, 0x00f0, 0x00fe, 0x007c, 0x00e6, 0x00f6 } },
	/* [CODEC_ISO646_IT - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00a7, 0x00b0, 0x00e7, 0x00e9, 0x005e, 0x005f, 0x00f9, 0x00e0, 0x00f2, 0x00e8, 0x00ec } },
	/* [CODEC_ISO646_JP - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x00a5, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_JP_OCR_B - CODEC_ISO646_MIN]       = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x2329, 0x00a5, 0x232a, 0x005e, 0x005f, 0x0000, 0x007b, 0x007c, 0x007d, 0x0000 } },
	/* [CODEC_ISO646_KR - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x20a9, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_LATIN_GR_MIXED - CODEC_ISO646_MIN] = */ { { 0x039e, 0x0022, 0x0393, 0x00a4, 0x0026, 0x03a8, 0x03a0, 0x0394, 0x03a9, 0x0398, 0x03a6, 0x039b, 0x03a3, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_MT - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x0121, 0x017c, 0x0127, 0x005e, 0x005f, 0x010b, 0x0120, 0x017b, 0x0126, 0x010a } },
	/* [CODEC_ISO646_NATS_DANO - CODEC_ISO646_MIN]      = */ { { 0x0021, 0x00ab, 0x00bb, 0x0024, 0x0026, 0x003a, 0x003f, 0x3000, 0x00c6, 0x00d8, 0x00c5, 0x25a0, 0x005f, 0x2007, 0x00e6, 0x00f8, 0x00e5, 0x2013 } },
	/* [CODEC_ISO646_NL - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO646_NL_NRCS - CODEC_ISO646_MIN]        = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x00be, 0x0133, 0x00bd, 0x007c, 0x005e, 0x005f, 0x0060, 0x00a8, 0x0192, 0x00bc, 0x00b4 } },
	/* [CODEC_ISO646_NO - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c6, 0x00d8, 0x00c5, 0x005e, 0x005f, 0x0060, 0x00e6, 0x00f8, 0x00e5, 0x203e } },
	/* [CODEC_ISO646_NO2 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x00a7, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c6, 0x00d8, 0x00c5, 0x005e, 0x005f, 0x0060, 0x00e6, 0x00f8, 0x00e5, 0x007c } },
	/* [CODEC_ISO646_PL - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x007a, 0x0026, 0x003a, 0x003f, 0x0119, 0x017a, 0x005c, 0x0144, 0x015b, 0x005f, 0x0105, 0x00f3, 0x0142, 0x017c, 0x0107 } },
	/* [CODEC_ISO646_PT - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00a7, 0x00c3, 0x00c7, 0x00d5, 0x005e, 0x005f, 0x0060, 0x00e3, 0x00e7, 0x00f5, 0x00b0 } },
	/* [CODEC_ISO646_PT2 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00b4, 0x00c3, 0x00c7, 0x00d5, 0x005e, 0x005f, 0x0060, 0x00e3, 0x00e7, 0x00f5, 0x007e } },
	/* [CODEC_ISO646_SE - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c4, 0x00d6, 0x00c5, 0x005e, 0x005f, 0x0060, 0x00e4, 0x00f6, 0x00e5, 0x203e } },
	/* [CODEC_ISO646_SE2 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x00c9, 0x00c4, 0x00d6, 0x00c5, 0x00dc, 0x005f, 0x00e9, 0x00e4, 0x00f6, 0x00e5, 0x00fc } },
	/* [CODEC_ISO646_SEFI_NATS - CODEC_ISO646_MIN]      = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x3000, 0x00c4, 0x00d6, 0x00c5, 0x25a0, 0x005f, 0x2007, 0x00e4, 0x00f6, 0x00e5, 0x2013 } },
	/* [CODEC_ISO646_SE_NRCS - CODEC_ISO646_MIN]        = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x00c9, 0x00c4, 0x00d6, 0x00c5, 0x00dc, 0x005f, 0x00e9, 0x00e4, 0x00f6, 0x00e5, 0x00fc } },
	/* [CODEC_ISO646_SWI_NRCS - CODEC_ISO646_MIN]       = */ { { 0x0021, 0x0022, 0x00f9, 0x0024, 0x0026, 0x003a, 0x003f, 0x00e0, 0x00e9, 0x00e7, 0x00ea, 0x00ee, 0x00e8, 0x00f4, 0x00e4, 0x00f6, 0x00fc, 0x00fb } },
	/* [CODEC_ISO646_T_61 - CODEC_ISO646_MIN]           = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x0000, 0x005d, 0x0000, 0x005f, 0x0000, 0x0000, 0x007c, 0x0000, 0x0000 } },
	/* [CODEC_ISO646_VIEWDATA - CODEC_ISO646_MIN]       = */ { { 0x0021, 0x0022, 0x00a3, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x2190, 0x00bd, 0x2192, 0x2191, 0x2317, 0x2015, 0x00bc, 0x2016, 0x00be, 0x00f7 } },
	/* [CODEC_ISO646_YU - CODEC_ISO646_MIN]             = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x017d, 0x0160, 0x0110, 0x0106, 0x010c, 0x005f, 0x017e, 0x0161, 0x0111, 0x0107, 0x010d } },
	/* [CODEC_ISO_IR_2 - CODEC_ISO646_MIN]              = */ { { 0x0021, 0x0022, 0x0023, 0x00a4, 0x0026, 0x003a, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x203e } },
	/* [CODEC_ISO_IR_8_1 - CODEC_ISO646_MIN]            = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0xe002, 0x00c4, 0x00d6, 0x00c5, 0x25a0, 0x005f, 0xe003, 0x00e4, 0x00f6, 0x00e5, 0x2013 } },
	/* [CODEC_US_DK - CODEC_ISO646_MIN]                 = */ { { 0x0021, 0x0022, 0x0023, 0x0024, 0x0026, 0x003a, 0x003f, 0x0040, 0x00c4, 0x00d6, 0x00c5, 0x005e, 0x005f, 0x0060, 0x00e4, 0x00f6, 0x00e5, 0x007e } },
};
/*[[[end]]]*/



/* Index into `iic_override' for which override to use for which character.
 * When no  override  should  be used,  returned  index  is  out-of-bounds. */
INTERN_CONST uint8_t const libiconv_iso646_override[128] = {
/*[[[deemon
import util;
local overrides = [0x21, 0x22, 0x23, 0x24, 0x26, 0x3a, 0x3f, 0x40, 0x5b,
                   0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x7b, 0x7c, 0x7d, 0x7e];
local indices = [#overrides] * 128;
for (local i, o: util.enumerate(overrides))
	indices[o] = i;
for (local i, o: util.enumerate(indices)) {
	if ((i % 16) == 0)
		print("\t"),;
	print(o, ","),;
	if ((i % 16) == 15) {
		print;
	} else {
		print(" "),;
	}
}
]]]*/
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 0, 1, 2, 3, 18, 4, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 5, 18, 18, 18, 18, 6,
	7, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 8, 9, 10, 11, 12,
	13, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
	18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 14, 15, 16, 17, 18,
/*[[[end]]]*/
};

/* Evaluate to the ordinal indices at which each of the overrides applies.
 * Coincidentally, these indices are also equal to the default  overrides,
 * though since every iso646 codepage  specifies all overrides, that  fact
 * doesn't actually matter. */
INTERN_CONST uint8_t const libiconv_iso646_override_chars[ISO646_OVERRIDE_COUNT] = {
	0x21, 0x22, 0x23, 0x24, 0x26, 0x3a, 0x3f, 0x40, 0x5b,
	0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x7b, 0x7c, 0x7d, 0x7e
};


DECL_END
#endif /* CODEC_ISO646_COUNT != 0 */

#endif /* !GUARD_LIBICONV_CP_ISO646_C */
