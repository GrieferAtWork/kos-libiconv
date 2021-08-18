/*[[[magic
local gcc_opt = options.setdefault("GCC.options", []);
gcc_opt.removeif([](x) -> x.startswith("-O"));
gcc_opt.append("-Os");
]]]*/
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
#ifndef GUARD_LIBICONV_CONVERT_XML_C
#define GUARD_LIBICONV_CONVERT_XML_C 1
#define _KOS_SOURCE 1

#include "api.h"
/**/

#include <hybrid/overflow.h>

#include <kos/types.h>

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unicode.h>

#include <libiconv/iconv.h>

#include "convert.h"

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


/*****************************************************************************
 *
 * Entity database format (tightly packed and sorted lexicographically):
 *    [0x00]       // Only if the first entry has a mandatory ";"
 *    {
 *        [0x01]   // Only iff the trailing ";" is optional for this entry
 *        <name>   // Name, with the leading "&" removed and no trailing ";"
 *        0x00     // NUL
 *        <ord>    // Ordinal replacement, encoded as UTF-8. When parsed
 *                 // as  unicode,  this  is a  1-character  long string
 *        0x00     // NUL
 *    } [Repeat for every entity, sorted by <name>]
 *    0x2E         // '.'
 *    0x2E         // '.'
 *    0x00         // NUL
 *    0x2E         // '.'
 *    0x00         // NUL
 *
 * Even  though entries are variable-length, it is possible to find the
 * start/end of any entry if given the pointer to any part within. This
 * is because every entry consists of 2 C-strings, meaning that the end
 * of both strings can always be located (in the above graphic, that is
 * one of the 0x00 aka  NUL fields). The two  strings can then be  told
 * apart from each other by the fact  that <name> is always at least  2
 * characters long, and <ord> is always exactly 1 character long. Thus,
 * it's also always possible  to find the start  of an entry (for  this
 * purpose, said start is considered to be the beginning of <name>)
 *
 * Because the entire database is sorted lexicographically by <name>,
 * it now becomes possible to perform a binary search where each step
 * is adjusted to located the start of the center-most entry.
 *
 * In turn, searching the database still only takes O(log2(N)), where
 * N is the constant number of entities within.
 *
 * NOTES:
 *  - The offset to <name> of the first entry is always `1'
 *  - The offset to the end of the last entry is always `sizeof(db) - 5'
 *
 *****************************************************************************/




/*[[[deemon
import * from deemon;
import xe = XML_ENTITIES from ".iconvdata.xml-entities";

// In order for the special database encoding we're using
// to work, all keys must have a length of at least 2!
local keys = xe.keys.sorted();
assert (keys.each.length < ...) >= 2;
local longestKeyLen = keys.each.length > ...;

local fp = File.Writer();
if (!xe[keys.first][1])
	fp << "\0";
for (local key: keys) {
	local ord, semicolonIsOptional = xe[key]...;
	if (semicolonIsOptional)
		fp << "\1";
	fp << key << "\0" << string.chr(ord).encode("utf-8") << "\0";
}
fp << "..\0.\0";
fp = fp.string[:-1]; // Implicit trailing NUL isn't needed
print("#define XML_ENTITY_MAXLEN ", longestKeyLen);
print("PRIVATE char const xml_entity_db[", (#fp + 1).hex(), "] = \"\\");
for (local line: fp.segments(32))
	print(line.encode("c-escape"), "\\");
print("\";");
]]]*/
#define XML_ENTITY_MAXLEN 31
PRIVATE char const xml_entity_db[0x5f23] = "\
\1AElig\0\xC3\x86\0\1AMP\0&\0\1Aacute\0\xC3\x81\0Abre\
ve\0\xC4\x82\0\1Acirc\0\xC3\x82\0Acy\0\xD0\x90\0Afr\0\xF0\x9D\x94\x84\0\
\1Agrave\0\xC3\x80\0Alpha\0\xCE\x91\0Amacr\0\xC4\x80\0And\
\0\xE2\xA9\x93\0Aogon\0\xC4\x84\0Aopf\0\xF0\x9D\x94\xB8\0ApplyFun\
ction\0\xE2\x81\xA1\0\1Aring\0\xC3\x85\0Ascr\0\xF0\x9D\x92\x9C\0As\
sign\0\xE2\x89\x94\0\1Atilde\0\xC3\x83\0\1Auml\0\xC3\x84\0Bac\
kslash\0\xE2\x88\x96\0Barv\0\xE2\xAB\xA7\0Barwed\0\xE2\x8C\x86\0B\
cy\0\xD0\x91\0Because\0\xE2\x88\xB5\0Bernoullis\0\xE2\x84\xAC\
\0Beta\0\xCE\x92\0Bfr\0\xF0\x9D\x94\x85\0Bopf\0\xF0\x9D\x94\xB9\0Brev\
e\0\xCB\x98\0Bscr\0\xE2\x84\xAC\0Bumpeq\0\xE2\x89\x8E\0CHcy\0\xD0\xA7\
\0\1COPY\0\xC2\xA9\0Cacute\0\xC4\x86\0Cap\0\xE2\x8B\x92\0Capi\
talDifferentialD\0\xE2\x85\x85\0Cayleys\0\xE2\x84\xAD\
\0Ccaron\0\xC4\x8C\0\1Ccedil\0\xC3\x87\0Ccirc\0\xC4\x88\0C\
conint\0\xE2\x88\xB0\0Cdot\0\xC4\x8A\0Cedilla\0\xC2\xB8\0Ce\
nterDot\0\xC2\xB7\0Cfr\0\xE2\x84\xAD\0Chi\0\xCE\xA7\0Circle\
Dot\0\xE2\x8A\x99\0CircleMinus\0\xE2\x8A\x96\0CirclePl\
us\0\xE2\x8A\x95\0CircleTimes\0\xE2\x8A\x97\0Clockwise\
ContourIntegral\0\xE2\x88\xB2\0CloseCurlyDo\
ubleQuote\0\xE2\x80\x9D\0CloseCurlyQuote\0\xE2\x80\
\x99\0Colon\0\xE2\x88\xB7\0Colone\0\xE2\xA9\xB4\0Congruent\
\0\xE2\x89\xA1\0Conint\0\xE2\x88\xAF\0ContourIntegral\0\
\xE2\x88\xAE\0Copf\0\xE2\x84\x82\0Coproduct\0\xE2\x88\x90\0Count\
erClockwiseContourIntegral\0\xE2\x88\xB3\0C\
ross\0\xE2\xA8\xAF\0Cscr\0\xF0\x9D\x92\x9E\0Cup\0\xE2\x8B\x93\0CupCa\
p\0\xE2\x89\x8D\0DD\0\xE2\x85\x85\0DDotrahd\0\xE2\xA4\x91\0DJcy\0\xD0\
\x82\0DScy\0\xD0\x85\0DZcy\0\xD0\x8F\0Dagger\0\xE2\x80\xA1\0Dar\
r\0\xE2\x86\xA1\0Dashv\0\xE2\xAB\xA4\0Dcaron\0\xC4\x8E\0Dcy\0\xD0\x94\
\0Del\0\xE2\x88\x87\0Delta\0\xCE\x94\0Dfr\0\xF0\x9D\x94\x87\0Diacr\
iticalAcute\0\xC2\xB4\0DiacriticalDot\0\xCB\x99\
\0DiacriticalDoubleAcute\0\xCB\x9D\0Diacr\
iticalGrave\0`\0DiacriticalTilde\0\xCB\
\x9C\0Diamond\0\xE2\x8B\x84\0DifferentialD\0\xE2\x85\x86\0\
Dopf\0\xF0\x9D\x94\xBB\0Dot\0\xC2\xA8\0DotDot\0\xE2\x83\x9C\0DotE\
qual\0\xE2\x89\x90\0DoubleContourIntegral\0\xE2\
\x88\xAF\0DoubleDot\0\xC2\xA8\0DoubleDownArrow\0\
\xE2\x87\x93\0DoubleLeftArrow\0\xE2\x87\x90\0DoubleLe\
ftRightArrow\0\xE2\x87\x94\0DoubleLeftTee\0\xE2\
\xAB\xA4\0DoubleLongLeftArrow\0\xE2\x9F\xB8\0Doubl\
eLongLeftRightArrow\0\xE2\x9F\xBA\0DoubleLo\
ngRightArrow\0\xE2\x9F\xB9\0DoubleRightArro\
w\0\xE2\x87\x92\0DoubleRightTee\0\xE2\x8A\xA8\0DoubleU\
pArrow\0\xE2\x87\x91\0DoubleUpDownArrow\0\xE2\x87\x95\
\0DoubleVerticalBar\0\xE2\x88\xA5\0DownArrow\
\0\xE2\x86\x93\0DownArrowBar\0\xE2\xA4\x93\0DownArrowU\
pArrow\0\xE2\x87\xB5\0DownBreve\0\xCC\x91\0DownLeft\
RightVector\0\xE2\xA5\x90\0DownLeftTeeVecto\
r\0\xE2\xA5\x9E\0DownLeftVector\0\xE2\x86\xBD\0DownLef\
tVectorBar\0\xE2\xA5\x96\0DownRightTeeVecto\
r\0\xE2\xA5\x9F\0DownRightVector\0\xE2\x87\x81\0DownRi\
ghtVectorBar\0\xE2\xA5\x97\0DownTee\0\xE2\x8A\xA4\0Dow\
nTeeArrow\0\xE2\x86\xA7\0Downarrow\0\xE2\x87\x93\0Dscr\
\0\xF0\x9D\x92\x9F\0Dstrok\0\xC4\x90\0ENG\0\xC5\x8A\0\1ETH\0\xC3\x90\0\1\
Eacute\0\xC3\x89\0Ecaron\0\xC4\x9A\0\1Ecirc\0\xC3\x8A\0Ec\
y\0\xD0\xAD\0Edot\0\xC4\x96\0Efr\0\xF0\x9D\x94\x88\0\1Egrave\0\xC3\x88\
\0Element\0\xE2\x88\x88\0Emacr\0\xC4\x92\0EmptySmall\
Square\0\xE2\x97\xBB\0EmptyVerySmallSquare\0\
\xE2\x96\xAB\0Eogon\0\xC4\x98\0Eopf\0\xF0\x9D\x94\xBC\0Epsilon\0\xCE\
\x95\0Equal\0\xE2\xA9\xB5\0EqualTilde\0\xE2\x89\x82\0Equil\
ibrium\0\xE2\x87\x8C\0Escr\0\xE2\x84\xB0\0Esim\0\xE2\xA9\xB3\0Eta\
\0\xCE\x97\0\1Euml\0\xC3\x8B\0Exists\0\xE2\x88\x83\0Exponent\
ialE\0\xE2\x85\x87\0Fcy\0\xD0\xA4\0Ffr\0\xF0\x9D\x94\x89\0FilledS\
mallSquare\0\xE2\x97\xBC\0FilledVerySmallSq\
uare\0\xE2\x96\xAA\0Fopf\0\xF0\x9D\x94\xBD\0ForAll\0\xE2\x88\x80\0Fo\
uriertrf\0\xE2\x84\xB1\0Fscr\0\xE2\x84\xB1\0GJcy\0\xD0\x83\0\1G\
T\0>\0Gamma\0\xCE\x93\0Gammad\0\xCF\x9C\0Gbreve\0\xC4\x9E\
\0Gcedil\0\xC4\xA2\0Gcirc\0\xC4\x9C\0Gcy\0\xD0\x93\0Gdot\0\
\xC4\xA0\0Gfr\0\xF0\x9D\x94\x8A\0Gg\0\xE2\x8B\x99\0Gopf\0\xF0\x9D\x94\xBE\0Gre\
aterEqual\0\xE2\x89\xA5\0GreaterEqualLess\0\xE2\
\x8B\x9B\0GreaterFullEqual\0\xE2\x89\xA7\0GreaterG\
reater\0\xE2\xAA\xA2\0GreaterLess\0\xE2\x89\xB7\0Great\
erSlantEqual\0\xE2\xA9\xBE\0GreaterTilde\0\xE2\x89\
\xB3\0Gscr\0\xF0\x9D\x92\xA2\0Gt\0\xE2\x89\xAB\0HARDcy\0\xD0\xAA\0Hac\
ek\0\xCB\x87\0Hat\0^\0Hcirc\0\xC4\xA4\0Hfr\0\xE2\x84\x8C\0Hil\
bertSpace\0\xE2\x84\x8B\0Hopf\0\xE2\x84\x8D\0Horizonta\
lLine\0\xE2\x94\x80\0Hscr\0\xE2\x84\x8B\0Hstrok\0\xC4\xA6\0Hum\
pDownHump\0\xE2\x89\x8E\0HumpEqual\0\xE2\x89\x8F\0IEcy\
\0\xD0\x95\0IJlig\0\xC4\xB2\0IOcy\0\xD0\x81\0\1Iacute\0\xC3\x8D\0\
\1Icirc\0\xC3\x8E\0Icy\0\xD0\x98\0Idot\0\xC4\xB0\0Ifr\0\xE2\x84\x91\
\0\1Igrave\0\xC3\x8C\0Im\0\xE2\x84\x91\0Imacr\0\xC4\xAA\0Imag\
inaryI\0\xE2\x85\x88\0Implies\0\xE2\x87\x92\0Int\0\xE2\x88\xAC\0I\
ntegral\0\xE2\x88\xAB\0Intersection\0\xE2\x8B\x82\0Inv\
isibleComma\0\xE2\x81\xA3\0InvisibleTimes\0\xE2\
\x81\xA2\0Iogon\0\xC4\xAE\0Iopf\0\xF0\x9D\x95\x80\0Iota\0\xCE\x99\0Is\
cr\0\xE2\x84\x90\0Itilde\0\xC4\xA8\0Iukcy\0\xD0\x86\0\1Iuml\0\
\xC3\x8F\0Jcirc\0\xC4\xB4\0Jcy\0\xD0\x99\0Jfr\0\xF0\x9D\x94\x8D\0Jopf\
\0\xF0\x9D\x95\x81\0Jscr\0\xF0\x9D\x92\xA5\0Jsercy\0\xD0\x88\0Jukcy\0\
\xD0\x84\0KHcy\0\xD0\xA5\0KJcy\0\xD0\x8C\0Kappa\0\xCE\x9A\0Kced\
il\0\xC4\xB6\0Kcy\0\xD0\x9A\0Kfr\0\xF0\x9D\x94\x8E\0Kopf\0\xF0\x9D\x95\x82\0\
Kscr\0\xF0\x9D\x92\xA6\0LJcy\0\xD0\x89\0\1LT\0<\0Lacute\0\xC4\
\xB9\0Lambda\0\xCE\x9B\0Lang\0\xE2\x9F\xAA\0Laplacetrf\0\
\xE2\x84\x92\0Larr\0\xE2\x86\x9E\0Lcaron\0\xC4\xBD\0Lcedil\0\xC4\xBB\
\0Lcy\0\xD0\x9B\0LeftAngleBracket\0\xE2\x9F\xA8\0Lef\
tArrow\0\xE2\x86\x90\0LeftArrowBar\0\xE2\x87\xA4\0Left\
ArrowRightArrow\0\xE2\x87\x86\0LeftCeiling\0\
\xE2\x8C\x88\0LeftDoubleBracket\0\xE2\x9F\xA6\0LeftDo\
wnTeeVector\0\xE2\xA5\xA1\0LeftDownVector\0\xE2\
\x87\x83\0LeftDownVectorBar\0\xE2\xA5\x99\0LeftFlo\
or\0\xE2\x8C\x8A\0LeftRightArrow\0\xE2\x86\x94\0LeftRi\
ghtVector\0\xE2\xA5\x8E\0LeftTee\0\xE2\x8A\xA3\0LeftTe\
eArrow\0\xE2\x86\xA4\0LeftTeeVector\0\xE2\xA5\x9A\0Lef\
tTriangle\0\xE2\x8A\xB2\0LeftTriangleBar\0\xE2\xA7\
\x8F\0LeftTriangleEqual\0\xE2\x8A\xB4\0LeftUpDo\
wnVector\0\xE2\xA5\x91\0LeftUpTeeVector\0\xE2\xA5\xA0\
\0LeftUpVector\0\xE2\x86\xBF\0LeftUpVectorBa\
r\0\xE2\xA5\x98\0LeftVector\0\xE2\x86\xBC\0LeftVectorB\
ar\0\xE2\xA5\x92\0Leftarrow\0\xE2\x87\x90\0Leftrightar\
row\0\xE2\x87\x94\0LessEqualGreater\0\xE2\x8B\x9A\0Les\
sFullEqual\0\xE2\x89\xA6\0LessGreater\0\xE2\x89\xB6\0L\
essLess\0\xE2\xAA\xA1\0LessSlantEqual\0\xE2\xA9\xBD\0L\
essTilde\0\xE2\x89\xB2\0Lfr\0\xF0\x9D\x94\x8F\0Ll\0\xE2\x8B\x98\0Lle\
ftarrow\0\xE2\x87\x9A\0Lmidot\0\xC4\xBF\0LongLeftAr\
row\0\xE2\x9F\xB5\0LongLeftRightArrow\0\xE2\x9F\xB7\0L\
ongRightArrow\0\xE2\x9F\xB6\0Longleftarrow\0\
\xE2\x9F\xB8\0Longleftrightarrow\0\xE2\x9F\xBA\0Longr\
ightarrow\0\xE2\x9F\xB9\0Lopf\0\xF0\x9D\x95\x83\0LowerLef\
tArrow\0\xE2\x86\x99\0LowerRightArrow\0\xE2\x86\x98\0L\
scr\0\xE2\x84\x92\0Lsh\0\xE2\x86\xB0\0Lstrok\0\xC5\x81\0Lt\0\xE2\x89\xAA\
\0Map\0\xE2\xA4\x85\0Mcy\0\xD0\x9C\0MediumSpace\0\xE2\x81\x9F\0\
Mellintrf\0\xE2\x84\xB3\0Mfr\0\xF0\x9D\x94\x90\0MinusPlus\
\0\xE2\x88\x93\0Mopf\0\xF0\x9D\x95\x84\0Mscr\0\xE2\x84\xB3\0Mu\0\xCE\x9C\0NJ\
cy\0\xD0\x8A\0Nacute\0\xC5\x83\0Ncaron\0\xC5\x87\0Ncedil\
\0\xC5\x85\0Ncy\0\xD0\x9D\0NegativeMediumSpace\0\xE2\
\x80\x8B\0NegativeThickSpace\0\xE2\x80\x8B\0Negati\
veThinSpace\0\xE2\x80\x8B\0NegativeVeryThin\
Space\0\xE2\x80\x8B\0NestedGreaterGreater\0\xE2\
\x89\xAB\0NestedLessLess\0\xE2\x89\xAA\0NewLine\0\n\0\
Nfr\0\xF0\x9D\x94\x91\0NoBreak\0\xE2\x81\xA0\0NonBreaking\
Space\0\xC2\xA0\0Nopf\0\xE2\x84\x95\0Not\0\xE2\xAB\xAC\0NotCon\
gruent\0\xE2\x89\xA2\0NotCupCap\0\xE2\x89\xAD\0NotDoub\
leVerticalBar\0\xE2\x88\xA6\0NotElement\0\xE2\x88\x89\
\0NotEqual\0\xE2\x89\xA0\0NotEqualTilde\0\xE2\x89\x82\0\
NotExists\0\xE2\x88\x84\0NotGreater\0\xE2\x89\xAF\0Not\
GreaterEqual\0\xE2\x89\xB1\0NotGreaterFullE\
qual\0\xE2\x89\xA7\0NotGreaterGreater\0\xE2\x89\xAB\0N\
otGreaterLess\0\xE2\x89\xB9\0NotGreaterSlan\
tEqual\0\xE2\xA9\xBE\0NotGreaterTilde\0\xE2\x89\xB5\0N\
otHumpDownHump\0\xE2\x89\x8E\0NotHumpEqual\0\
\xE2\x89\x8F\0NotLeftTriangle\0\xE2\x8B\xAA\0NotLeftT\
riangleBar\0\xE2\xA7\x8F\0NotLeftTriangleEq\
ual\0\xE2\x8B\xAC\0NotLess\0\xE2\x89\xAE\0NotLessEqual\
\0\xE2\x89\xB0\0NotLessGreater\0\xE2\x89\xB8\0NotLessL\
ess\0\xE2\x89\xAA\0NotLessSlantEqual\0\xE2\xA9\xBD\0No\
tLessTilde\0\xE2\x89\xB4\0NotNestedGreaterG\
reater\0\xE2\xAA\xA2\0NotNestedLessLess\0\xE2\xAA\xA1\
\0NotPrecedes\0\xE2\x8A\x80\0NotPrecedesEqua\
l\0\xE2\xAA\xAF\0NotPrecedesSlantEqual\0\xE2\x8B\xA0\0\
NotReverseElement\0\xE2\x88\x8C\0NotRightTr\
iangle\0\xE2\x8B\xAB\0NotRightTriangleBar\0\xE2\
\xA7\x90\0NotRightTriangleEqual\0\xE2\x8B\xAD\0Not\
SquareSubset\0\xE2\x8A\x8F\0NotSquareSubset\
Equal\0\xE2\x8B\xA2\0NotSquareSuperset\0\xE2\x8A\x90\0\
NotSquareSupersetEqual\0\xE2\x8B\xA3\0NotSu\
bset\0\xE2\x8A\x82\0NotSubsetEqual\0\xE2\x8A\x88\0NotS\
ucceeds\0\xE2\x8A\x81\0NotSucceedsEqual\0\xE2\xAA\xB0\
\0NotSucceedsSlantEqual\0\xE2\x8B\xA1\0NotSu\
cceedsTilde\0\xE2\x89\xBF\0NotSuperset\0\xE2\x8A\x83\0\
NotSupersetEqual\0\xE2\x8A\x89\0NotTilde\0\xE2\x89\
\x81\0NotTildeEqual\0\xE2\x89\x84\0NotTildeFull\
Equal\0\xE2\x89\x87\0NotTildeTilde\0\xE2\x89\x89\0NotV\
erticalBar\0\xE2\x88\xA4\0Nscr\0\xF0\x9D\x92\xA9\0\1Ntilde\
\0\xC3\x91\0Nu\0\xCE\x9D\0OElig\0\xC5\x92\0\1Oacute\0\xC3\x93\0\1O\
circ\0\xC3\x94\0Ocy\0\xD0\x9E\0Odblac\0\xC5\x90\0Ofr\0\xF0\x9D\x94\
\x92\0\1Ograve\0\xC3\x92\0Omacr\0\xC5\x8C\0Omega\0\xCE\xA9\0O\
micron\0\xCE\x9F\0Oopf\0\xF0\x9D\x95\x86\0OpenCurlyDou\
bleQuote\0\xE2\x80\x9C\0OpenCurlyQuote\0\xE2\x80\x98\0\
Or\0\xE2\xA9\x94\0Oscr\0\xF0\x9D\x92\xAA\0\1Oslash\0\xC3\x98\0\1Oti\
lde\0\xC3\x95\0Otimes\0\xE2\xA8\xB7\0\1Ouml\0\xC3\x96\0OverB\
ar\0\xE2\x80\xBE\0OverBrace\0\xE2\x8F\x9E\0OverBracket\
\0\xE2\x8E\xB4\0OverParenthesis\0\xE2\x8F\x9C\0Partial\
D\0\xE2\x88\x82\0Pcy\0\xD0\x9F\0Pfr\0\xF0\x9D\x94\x93\0Phi\0\xCE\xA6\0Pi\0\
\xCE\xA0\0PlusMinus\0\xC2\xB1\0Poincareplane\0\xE2\x84\
\x8C\0Popf\0\xE2\x84\x99\0Pr\0\xE2\xAA\xBB\0Precedes\0\xE2\x89\xBA\0P\
recedesEqual\0\xE2\xAA\xAF\0PrecedesSlantEq\
ual\0\xE2\x89\xBC\0PrecedesTilde\0\xE2\x89\xBE\0Prime\0\
\xE2\x80\xB3\0Product\0\xE2\x88\x8F\0Proportion\0\xE2\x88\xB7\0P\
roportional\0\xE2\x88\x9D\0Pscr\0\xF0\x9D\x92\xAB\0Psi\0\xCE\xA8\
\0\1QUOT\0\"\0Qfr\0\xF0\x9D\x94\x94\0Qopf\0\xE2\x84\x9A\0Qscr\0\
\xF0\x9D\x92\xAC\0RBarr\0\xE2\xA4\x90\0\1REG\0\xC2\xAE\0Racute\0\xC5\x94\
\0Rang\0\xE2\x9F\xAB\0Rarr\0\xE2\x86\xA0\0Rarrtl\0\xE2\xA4\x96\0Rc\
aron\0\xC5\x98\0Rcedil\0\xC5\x96\0Rcy\0\xD0\xA0\0Re\0\xE2\x84\x9C\0\
ReverseElement\0\xE2\x88\x8B\0ReverseEquili\
brium\0\xE2\x87\x8B\0ReverseUpEquilibrium\0\xE2\
\xA5\xAF\0Rfr\0\xE2\x84\x9C\0Rho\0\xCE\xA1\0RightAngleBrac\
ket\0\xE2\x9F\xA9\0RightArrow\0\xE2\x86\x92\0RightArro\
wBar\0\xE2\x87\xA5\0RightArrowLeftArrow\0\xE2\x87\x84\
\0RightCeiling\0\xE2\x8C\x89\0RightDoubleBra\
cket\0\xE2\x9F\xA7\0RightDownTeeVector\0\xE2\xA5\x9D\0\
RightDownVector\0\xE2\x87\x82\0RightDownVec\
torBar\0\xE2\xA5\x95\0RightFloor\0\xE2\x8C\x8B\0RightT\
ee\0\xE2\x8A\xA2\0RightTeeArrow\0\xE2\x86\xA6\0RightTe\
eVector\0\xE2\xA5\x9B\0RightTriangle\0\xE2\x8A\xB3\0Ri\
ghtTriangleBar\0\xE2\xA7\x90\0RightTriangle\
Equal\0\xE2\x8A\xB5\0RightUpDownVector\0\xE2\xA5\x8F\0\
RightUpTeeVector\0\xE2\xA5\x9C\0RightUpVect\
or\0\xE2\x86\xBE\0RightUpVectorBar\0\xE2\xA5\x94\0Righ\
tVector\0\xE2\x87\x80\0RightVectorBar\0\xE2\xA5\x93\0R\
ightarrow\0\xE2\x87\x92\0Ropf\0\xE2\x84\x9D\0RoundImpl\
ies\0\xE2\xA5\xB0\0Rrightarrow\0\xE2\x87\x9B\0Rscr\0\xE2\x84\x9B\
\0Rsh\0\xE2\x86\xB1\0RuleDelayed\0\xE2\xA7\xB4\0SHCHcy\0\
\xD0\xA9\0SHcy\0\xD0\xA8\0SOFTcy\0\xD0\xAC\0Sacute\0\xC5\x9A\0S\
c\0\xE2\xAA\xBC\0Scaron\0\xC5\xA0\0Scedil\0\xC5\x9E\0Scirc\0\
\xC5\x9C\0Scy\0\xD0\xA1\0Sfr\0\xF0\x9D\x94\x96\0ShortDownArro\
w\0\xE2\x86\x93\0ShortLeftArrow\0\xE2\x86\x90\0ShortRi\
ghtArrow\0\xE2\x86\x92\0ShortUpArrow\0\xE2\x86\x91\0Si\
gma\0\xCE\xA3\0SmallCircle\0\xE2\x88\x98\0Sopf\0\xF0\x9D\x95\x8A\
\0Sqrt\0\xE2\x88\x9A\0Square\0\xE2\x96\xA1\0SquareInter\
section\0\xE2\x8A\x93\0SquareSubset\0\xE2\x8A\x8F\0Squ\
areSubsetEqual\0\xE2\x8A\x91\0SquareSuperse\
t\0\xE2\x8A\x90\0SquareSupersetEqual\0\xE2\x8A\x92\0Sq\
uareUnion\0\xE2\x8A\x94\0Sscr\0\xF0\x9D\x92\xAE\0Star\0\xE2\x8B\x86\
\0Sub\0\xE2\x8B\x90\0Subset\0\xE2\x8B\x90\0SubsetEqual\0\
\xE2\x8A\x86\0Succeeds\0\xE2\x89\xBB\0SucceedsEqual\0\xE2\
\xAA\xB0\0SucceedsSlantEqual\0\xE2\x89\xBD\0Succee\
dsTilde\0\xE2\x89\xBF\0SuchThat\0\xE2\x88\x8B\0Sum\0\xE2\x88\x91\
\0Sup\0\xE2\x8B\x91\0Superset\0\xE2\x8A\x83\0SupersetEq\
ual\0\xE2\x8A\x87\0Supset\0\xE2\x8B\x91\0\1THORN\0\xC3\x9E\0TRA\
DE\0\xE2\x84\xA2\0TSHcy\0\xD0\x8B\0TScy\0\xD0\xA6\0Tab\0\t\0Ta\
u\0\xCE\xA4\0Tcaron\0\xC5\xA4\0Tcedil\0\xC5\xA2\0Tcy\0\xD0\xA2\0\
Tfr\0\xF0\x9D\x94\x97\0Therefore\0\xE2\x88\xB4\0Theta\0\xCE\x98\0\
ThickSpace\0\xE2\x81\x9F\0ThinSpace\0\xE2\x80\x89\0Til\
de\0\xE2\x88\xBC\0TildeEqual\0\xE2\x89\x83\0TildeFullE\
qual\0\xE2\x89\x85\0TildeTilde\0\xE2\x89\x88\0Topf\0\xF0\x9D\x95\
\x8B\0TripleDot\0\xE2\x83\x9B\0Tscr\0\xF0\x9D\x92\xAF\0Tstrok\
\0\xC5\xA6\0\1Uacute\0\xC3\x9A\0Uarr\0\xE2\x86\x9F\0Uarrocir\
\0\xE2\xA5\x89\0Ubrcy\0\xD0\x8E\0Ubreve\0\xC5\xAC\0\1Ucirc\0\xC3\
\x9B\0Ucy\0\xD0\xA3\0Udblac\0\xC5\xB0\0Ufr\0\xF0\x9D\x94\x98\0\1Ugr\
ave\0\xC3\x99\0Umacr\0\xC5\xAA\0UnderBar\0_\0Under\
Brace\0\xE2\x8F\x9F\0UnderBracket\0\xE2\x8E\xB5\0Under\
Parenthesis\0\xE2\x8F\x9D\0Union\0\xE2\x8B\x83\0UnionP\
lus\0\xE2\x8A\x8E\0Uogon\0\xC5\xB2\0Uopf\0\xF0\x9D\x95\x8C\0UpArr\
ow\0\xE2\x86\x91\0UpArrowBar\0\xE2\xA4\x92\0UpArrowDow\
nArrow\0\xE2\x87\x85\0UpDownArrow\0\xE2\x86\x95\0UpEqu\
ilibrium\0\xE2\xA5\xAE\0UpTee\0\xE2\x8A\xA5\0UpTeeArro\
w\0\xE2\x86\xA5\0Uparrow\0\xE2\x87\x91\0Updownarrow\0\xE2\x87\
\x95\0UpperLeftArrow\0\xE2\x86\x96\0UpperRightA\
rrow\0\xE2\x86\x97\0Upsi\0\xCF\x92\0Upsilon\0\xCE\xA5\0Urin\
g\0\xC5\xAE\0Uscr\0\xF0\x9D\x92\xB0\0Utilde\0\xC5\xA8\0\1Uuml\0\xC3\
\x9C\0VDash\0\xE2\x8A\xAB\0Vbar\0\xE2\xAB\xAB\0Vcy\0\xD0\x92\0Vdas\
h\0\xE2\x8A\xA9\0Vdashl\0\xE2\xAB\xA6\0Vee\0\xE2\x8B\x81\0Verbar\0\
\xE2\x80\x96\0Vert\0\xE2\x80\x96\0VerticalBar\0\xE2\x88\xA3\0Ver\
ticalLine\0|\0VerticalSeparator\0\xE2\x9D\
\x98\0VerticalTilde\0\xE2\x89\x80\0VeryThinSpac\
e\0\xE2\x80\x8A\0Vfr\0\xF0\x9D\x94\x99\0Vopf\0\xF0\x9D\x95\x8D\0Vscr\0\xF0\x9D\
\x92\xB1\0Vvdash\0\xE2\x8A\xAA\0Wcirc\0\xC5\xB4\0Wedge\0\xE2\x8B\x80\
\0Wfr\0\xF0\x9D\x94\x9A\0Wopf\0\xF0\x9D\x95\x8E\0Wscr\0\xF0\x9D\x92\xB2\0Xf\
r\0\xF0\x9D\x94\x9B\0Xi\0\xCE\x9E\0Xopf\0\xF0\x9D\x95\x8F\0Xscr\0\xF0\x9D\x92\xB3\
\0YAcy\0\xD0\xAF\0YIcy\0\xD0\x87\0YUcy\0\xD0\xAE\0\1Yacute\
\0\xC3\x9D\0Ycirc\0\xC5\xB6\0Ycy\0\xD0\xAB\0Yfr\0\xF0\x9D\x94\x9C\0Yop\
f\0\xF0\x9D\x95\x90\0Yscr\0\xF0\x9D\x92\xB4\0Yuml\0\xC5\xB8\0ZHcy\0\xD0\x96\
\0Zacute\0\xC5\xB9\0Zcaron\0\xC5\xBD\0Zcy\0\xD0\x97\0Zdot\
\0\xC5\xBB\0ZeroWidthSpace\0\xE2\x80\x8B\0Zeta\0\xCE\x96\0Z\
fr\0\xE2\x84\xA8\0Zopf\0\xE2\x84\xA4\0Zscr\0\xF0\x9D\x92\xB5\0\1aacut\
e\0\xC3\xA1\0abreve\0\xC4\x83\0ac\0\xE2\x88\xBE\0acE\0\xE2\x88\xBE\0ac\
d\0\xE2\x88\xBF\0\1acirc\0\xC3\xA2\0\1acute\0\xC2\xB4\0acy\0\xD0\xB0\
\0\1aelig\0\xC3\xA6\0af\0\xE2\x81\xA1\0afr\0\xF0\x9D\x94\x9E\0\1agra\
ve\0\xC3\xA0\0alefsym\0\xE2\x84\xB5\0aleph\0\xE2\x84\xB5\0alph\
a\0\xCE\xB1\0amacr\0\xC4\x81\0amalg\0\xE2\xA8\xBF\0\1amp\0&\0a\
nd\0\xE2\x88\xA7\0andand\0\xE2\xA9\x95\0andd\0\xE2\xA9\x9C\0andsl\
ope\0\xE2\xA9\x98\0andv\0\xE2\xA9\x9A\0ang\0\xE2\x88\xA0\0ange\0\xE2\xA6\
\xA4\0angle\0\xE2\x88\xA0\0angmsd\0\xE2\x88\xA1\0angmsdaa\0\
\xE2\xA6\xA8\0angmsdab\0\xE2\xA6\xA9\0angmsdac\0\xE2\xA6\xAA\0an\
gmsdad\0\xE2\xA6\xAB\0angmsdae\0\xE2\xA6\xAC\0angmsdaf\
\0\xE2\xA6\xAD\0angmsdag\0\xE2\xA6\xAE\0angmsdah\0\xE2\xA6\xAF\0a\
ngrt\0\xE2\x88\x9F\0angrtvb\0\xE2\x8A\xBE\0angrtvbd\0\xE2\xA6\
\x9D\0angsph\0\xE2\x88\xA2\0angst\0\xC3\x85\0angzarr\0\xE2\x8D\
\xBC\0aogon\0\xC4\x85\0aopf\0\xF0\x9D\x95\x92\0ap\0\xE2\x89\x88\0apE\0\
\xE2\xA9\xB0\0apacir\0\xE2\xA9\xAF\0ape\0\xE2\x89\x8A\0apid\0\xE2\x89\x8B\0\
apos\0\'\0approx\0\xE2\x89\x88\0approxeq\0\xE2\x89\x8A\0\1\
aring\0\xC3\xA5\0ascr\0\xF0\x9D\x92\xB6\0ast\0*\0asymp\0\xE2\
\x89\x88\0asympeq\0\xE2\x89\x8D\0\1atilde\0\xC3\xA3\0\1auml\0\
\xC3\xA4\0awconint\0\xE2\x88\xB3\0awint\0\xE2\xA8\x91\0bNot\0\xE2\
\xAB\xAD\0backcong\0\xE2\x89\x8C\0backepsilon\0\xCF\xB6\0b\
ackprime\0\xE2\x80\xB5\0backsim\0\xE2\x88\xBD\0backsim\
eq\0\xE2\x8B\x8D\0barvee\0\xE2\x8A\xBD\0barwed\0\xE2\x8C\x85\0bar\
wedge\0\xE2\x8C\x85\0bbrk\0\xE2\x8E\xB5\0bbrktbrk\0\xE2\x8E\xB6\0\
bcong\0\xE2\x89\x8C\0bcy\0\xD0\xB1\0bdquo\0\xE2\x80\x9E\0becau\
s\0\xE2\x88\xB5\0because\0\xE2\x88\xB5\0bemptyv\0\xE2\xA6\xB0\0be\
psi\0\xCF\xB6\0bernou\0\xE2\x84\xAC\0beta\0\xCE\xB2\0beth\0\xE2\
\x84\xB6\0between\0\xE2\x89\xAC\0bfr\0\xF0\x9D\x94\x9F\0bigcap\0\xE2\
\x8B\x82\0bigcirc\0\xE2\x97\xAF\0bigcup\0\xE2\x8B\x83\0bigodo\
t\0\xE2\xA8\x80\0bigoplus\0\xE2\xA8\x81\0bigotimes\0\xE2\xA8\x82\
\0bigsqcup\0\xE2\xA8\x86\0bigstar\0\xE2\x98\x85\0bigtri\
angledown\0\xE2\x96\xBD\0bigtriangleup\0\xE2\x96\xB3\0\
biguplus\0\xE2\xA8\x84\0bigvee\0\xE2\x8B\x81\0bigwedge\
\0\xE2\x8B\x80\0bkarow\0\xE2\xA4\x8D\0blacklozenge\0\xE2\xA7\xAB\
\0blacksquare\0\xE2\x96\xAA\0blacktriangle\0\xE2\
\x96\xB4\0blacktriangledown\0\xE2\x96\xBE\0blacktr\
iangleleft\0\xE2\x97\x82\0blacktrianglerigh\
t\0\xE2\x96\xB8\0blank\0\xE2\x90\xA3\0blk12\0\xE2\x96\x92\0blk14\0\
\xE2\x96\x91\0blk34\0\xE2\x96\x93\0block\0\xE2\x96\x88\0bne\0=\0bn\
equiv\0\xE2\x89\xA1\0bnot\0\xE2\x8C\x90\0bopf\0\xF0\x9D\x95\x93\0bot\
\0\xE2\x8A\xA5\0bottom\0\xE2\x8A\xA5\0bowtie\0\xE2\x8B\x88\0boxDL\
\0\xE2\x95\x97\0boxDR\0\xE2\x95\x94\0boxDl\0\xE2\x95\x96\0boxDr\0\xE2\
\x95\x93\0boxH\0\xE2\x95\x90\0boxHD\0\xE2\x95\xA6\0boxHU\0\xE2\x95\xA9\0\
boxHd\0\xE2\x95\xA4\0boxHu\0\xE2\x95\xA7\0boxUL\0\xE2\x95\x9D\0bo\
xUR\0\xE2\x95\x9A\0boxUl\0\xE2\x95\x9C\0boxUr\0\xE2\x95\x99\0boxV\
\0\xE2\x95\x91\0boxVH\0\xE2\x95\xAC\0boxVL\0\xE2\x95\xA3\0boxVR\0\xE2\
\x95\xA0\0boxVh\0\xE2\x95\xAB\0boxVl\0\xE2\x95\xA2\0boxVr\0\xE2\x95\x9F\
\0boxbox\0\xE2\xA7\x89\0boxdL\0\xE2\x95\x95\0boxdR\0\xE2\x95\x92\0\
boxdl\0\xE2\x94\x90\0boxdr\0\xE2\x94\x8C\0boxh\0\xE2\x94\x80\0box\
hD\0\xE2\x95\xA5\0boxhU\0\xE2\x95\xA8\0boxhd\0\xE2\x94\xAC\0boxhu\
\0\xE2\x94\xB4\0boxminus\0\xE2\x8A\x9F\0boxplus\0\xE2\x8A\x9E\0bo\
xtimes\0\xE2\x8A\xA0\0boxuL\0\xE2\x95\x9B\0boxuR\0\xE2\x95\x98\0b\
oxul\0\xE2\x94\x98\0boxur\0\xE2\x94\x94\0boxv\0\xE2\x94\x82\0boxv\
H\0\xE2\x95\xAA\0boxvL\0\xE2\x95\xA1\0boxvR\0\xE2\x95\x9E\0boxvh\0\
\xE2\x94\xBC\0boxvl\0\xE2\x94\xA4\0boxvr\0\xE2\x94\x9C\0bprime\0\xE2\
\x80\xB5\0breve\0\xCB\x98\0\1brvbar\0\xC2\xA6\0bscr\0\xF0\x9D\x92\xB7\
\0bsemi\0\xE2\x81\x8F\0bsim\0\xE2\x88\xBD\0bsime\0\xE2\x8B\x8D\0bs\
ol\0\\\0bsolb\0\xE2\xA7\x85\0bsolhsub\0\xE2\x9F\x88\0bull\
\0\xE2\x80\xA2\0bullet\0\xE2\x80\xA2\0bump\0\xE2\x89\x8E\0bumpE\0\xE2\
\xAA\xAE\0bumpe\0\xE2\x89\x8F\0bumpeq\0\xE2\x89\x8F\0cacute\0\xC4\
\x87\0cap\0\xE2\x88\xA9\0capand\0\xE2\xA9\x84\0capbrcup\0\xE2\xA9\
\x89\0capcap\0\xE2\xA9\x8B\0capcup\0\xE2\xA9\x87\0capdot\0\xE2\
\xA9\x80\0caps\0\xE2\x88\xA9\0caret\0\xE2\x81\x81\0caron\0\xCB\x87\0c\
caps\0\xE2\xA9\x8D\0ccaron\0\xC4\x8D\0\1ccedil\0\xC3\xA7\0cc\
irc\0\xC4\x89\0ccups\0\xE2\xA9\x8C\0ccupssm\0\xE2\xA9\x90\0cdo\
t\0\xC4\x8B\0\1cedil\0\xC2\xB8\0cemptyv\0\xE2\xA6\xB2\0\1cent\
\0\xC2\xA2\0centerdot\0\xC2\xB7\0cfr\0\xF0\x9D\x94\xA0\0chcy\0\xD1\
\x87\0check\0\xE2\x9C\x93\0checkmark\0\xE2\x9C\x93\0chi\0\xCF\x87\
\0cir\0\xE2\x97\x8B\0cirE\0\xE2\xA7\x83\0circ\0\xCB\x86\0circeq\
\0\xE2\x89\x97\0circlearrowleft\0\xE2\x86\xBA\0circlea\
rrowright\0\xE2\x86\xBB\0circledR\0\xC2\xAE\0circle\
dS\0\xE2\x93\x88\0circledast\0\xE2\x8A\x9B\0circledcir\
c\0\xE2\x8A\x9A\0circleddash\0\xE2\x8A\x9D\0cire\0\xE2\x89\x97\0c\
irfnint\0\xE2\xA8\x90\0cirmid\0\xE2\xAB\xAF\0cirscir\0\xE2\
\xA7\x82\0clubs\0\xE2\x99\xA3\0clubsuit\0\xE2\x99\xA3\0colon\0\
:\0colone\0\xE2\x89\x94\0coloneq\0\xE2\x89\x94\0comma\0,\
\0commat\0@\0comp\0\xE2\x88\x81\0compfn\0\xE2\x88\x98\0co\
mplement\0\xE2\x88\x81\0complexes\0\xE2\x84\x82\0cong\0\
\xE2\x89\x85\0congdot\0\xE2\xA9\xAD\0conint\0\xE2\x88\xAE\0copf\0\
\xF0\x9D\x95\x94\0coprod\0\xE2\x88\x90\0\1copy\0\xC2\xA9\0copysr\0\
\xE2\x84\x97\0crarr\0\xE2\x86\xB5\0cross\0\xE2\x9C\x97\0cscr\0\xF0\x9D\x92\
\xB8\0csub\0\xE2\xAB\x8F\0csube\0\xE2\xAB\x91\0csup\0\xE2\xAB\x90\0cs\
upe\0\xE2\xAB\x92\0ctdot\0\xE2\x8B\xAF\0cudarrl\0\xE2\xA4\xB8\0cu\
darrr\0\xE2\xA4\xB5\0cuepr\0\xE2\x8B\x9E\0cuesc\0\xE2\x8B\x9F\0cu\
larr\0\xE2\x86\xB6\0cularrp\0\xE2\xA4\xBD\0cup\0\xE2\x88\xAA\0cup\
brcap\0\xE2\xA9\x88\0cupcap\0\xE2\xA9\x86\0cupcup\0\xE2\xA9\x8A\0\
cupdot\0\xE2\x8A\x8D\0cupor\0\xE2\xA9\x85\0cups\0\xE2\x88\xAA\0cu\
rarr\0\xE2\x86\xB7\0curarrm\0\xE2\xA4\xBC\0curlyeqprec\
\0\xE2\x8B\x9E\0curlyeqsucc\0\xE2\x8B\x9F\0curlyvee\0\xE2\x8B\
\x8E\0curlywedge\0\xE2\x8B\x8F\0\1curren\0\xC2\xA4\0curv\
earrowleft\0\xE2\x86\xB6\0curvearrowright\0\xE2\
\x86\xB7\0cuvee\0\xE2\x8B\x8E\0cuwed\0\xE2\x8B\x8F\0cwconint\0\
\xE2\x88\xB2\0cwint\0\xE2\x88\xB1\0cylcty\0\xE2\x8C\xAD\0dArr\0\xE2\x87\
\x93\0dHar\0\xE2\xA5\xA5\0dagger\0\xE2\x80\xA0\0daleth\0\xE2\x84\xB8\
\0darr\0\xE2\x86\x93\0dash\0\xE2\x80\x90\0dashv\0\xE2\x8A\xA3\0dbk\
arow\0\xE2\xA4\x8F\0dblac\0\xCB\x9D\0dcaron\0\xC4\x8F\0dcy\0\
\xD0\xB4\0dd\0\xE2\x85\x86\0ddagger\0\xE2\x80\xA1\0ddarr\0\xE2\x87\x8A\0\
ddotseq\0\xE2\xA9\xB7\0\1deg\0\xC2\xB0\0delta\0\xCE\xB4\0dem\
ptyv\0\xE2\xA6\xB1\0dfisht\0\xE2\xA5\xBF\0dfr\0\xF0\x9D\x94\xA1\0dha\
rl\0\xE2\x87\x83\0dharr\0\xE2\x87\x82\0diam\0\xE2\x8B\x84\0diamon\
d\0\xE2\x8B\x84\0diamondsuit\0\xE2\x99\xA6\0diams\0\xE2\x99\xA6\0\
die\0\xC2\xA8\0digamma\0\xCF\x9D\0disin\0\xE2\x8B\xB2\0div\0\
\xC3\xB7\0\1divide\0\xC3\xB7\0divideontimes\0\xE2\x8B\x87\0\
divonx\0\xE2\x8B\x87\0djcy\0\xD1\x92\0dlcorn\0\xE2\x8C\x9E\0dl\
crop\0\xE2\x8C\x8D\0dollar\0$\0dopf\0\xF0\x9D\x95\x95\0dot\0\
\xCB\x99\0doteq\0\xE2\x89\x90\0doteqdot\0\xE2\x89\x91\0dotmin\
us\0\xE2\x88\xB8\0dotplus\0\xE2\x88\x94\0dotsquare\0\xE2\x8A\xA1\
\0doublebarwedge\0\xE2\x8C\x86\0downarrow\0\xE2\x86\
\x93\0downdownarrows\0\xE2\x87\x8A\0downharpoon\
left\0\xE2\x87\x83\0downharpoonright\0\xE2\x87\x82\0dr\
bkarow\0\xE2\xA4\x90\0drcorn\0\xE2\x8C\x9F\0drcrop\0\xE2\x8C\x8C\
\0dscr\0\xF0\x9D\x92\xB9\0dscy\0\xD1\x95\0dsol\0\xE2\xA7\xB6\0dstr\
ok\0\xC4\x91\0dtdot\0\xE2\x8B\xB1\0dtri\0\xE2\x96\xBF\0dtrif\0\xE2\
\x96\xBE\0duarr\0\xE2\x87\xB5\0duhar\0\xE2\xA5\xAF\0dwangle\0\xE2\
\xA6\xA6\0dzcy\0\xD1\x9F\0dzigrarr\0\xE2\x9F\xBF\0eDDot\0\xE2\xA9\
\xB7\0eDot\0\xE2\x89\x91\0\1eacute\0\xC3\xA9\0easter\0\xE2\xA9\xAE\
\0ecaron\0\xC4\x9B\0ecir\0\xE2\x89\x96\0\1ecirc\0\xC3\xAA\0ec\
olon\0\xE2\x89\x95\0ecy\0\xD1\x8D\0edot\0\xC4\x97\0ee\0\xE2\x85\x87\0e\
fDot\0\xE2\x89\x92\0efr\0\xF0\x9D\x94\xA2\0eg\0\xE2\xAA\x9A\0\1egrave\
\0\xC3\xA8\0egs\0\xE2\xAA\x96\0egsdot\0\xE2\xAA\x98\0el\0\xE2\xAA\x99\0el\
inters\0\xE2\x8F\xA7\0ell\0\xE2\x84\x93\0els\0\xE2\xAA\x95\0elsdo\
t\0\xE2\xAA\x97\0emacr\0\xC4\x93\0empty\0\xE2\x88\x85\0emptyse\
t\0\xE2\x88\x85\0emptyv\0\xE2\x88\x85\0emsp\0\xE2\x80\x83\0emsp13\
\0\xE2\x80\x84\0emsp14\0\xE2\x80\x85\0eng\0\xC5\x8B\0ensp\0\xE2\x80\x82\0\
eogon\0\xC4\x99\0eopf\0\xF0\x9D\x95\x96\0epar\0\xE2\x8B\x95\0epar\
sl\0\xE2\xA7\xA3\0eplus\0\xE2\xA9\xB1\0epsi\0\xCE\xB5\0epsilon\
\0\xCE\xB5\0epsiv\0\xCF\xB5\0eqcirc\0\xE2\x89\x96\0eqcolon\0\
\xE2\x89\x95\0eqsim\0\xE2\x89\x82\0eqslantgtr\0\xE2\xAA\x96\0eqs\
lantless\0\xE2\xAA\x95\0equals\0=\0equest\0\xE2\x89\x9F\
\0equiv\0\xE2\x89\xA1\0equivDD\0\xE2\xA9\xB8\0eqvparsl\0\
\xE2\xA7\xA5\0erDot\0\xE2\x89\x93\0erarr\0\xE2\xA5\xB1\0escr\0\xE2\x84\xAF\
\0esdot\0\xE2\x89\x90\0esim\0\xE2\x89\x82\0eta\0\xCE\xB7\0\1eth\0\
\xC3\xB0\0\1euml\0\xC3\xAB\0euro\0\xE2\x82\xAC\0excl\0!\0exis\
t\0\xE2\x88\x83\0expectation\0\xE2\x84\xB0\0exponentia\
le\0\xE2\x85\x87\0fallingdotseq\0\xE2\x89\x92\0fcy\0\xD1\x84\0\
female\0\xE2\x99\x80\0ffilig\0\xEF\xAC\x83\0fflig\0\xEF\xAC\x80\0\
ffllig\0\xEF\xAC\x84\0ffr\0\xF0\x9D\x94\xA3\0filig\0\xEF\xAC\x81\0fj\
lig\0f\0flat\0\xE2\x99\xAD\0fllig\0\xEF\xAC\x82\0fltns\0\xE2\
\x96\xB1\0fnof\0\xC6\x92\0fopf\0\xF0\x9D\x95\x97\0forall\0\xE2\x88\x80\0\
fork\0\xE2\x8B\x94\0forkv\0\xE2\xAB\x99\0fpartint\0\xE2\xA8\x8D\0\
\1frac12\0\xC2\xBD\0frac13\0\xE2\x85\x93\0\1frac14\0\xC2\xBC\
\0frac15\0\xE2\x85\x95\0frac16\0\xE2\x85\x99\0frac18\0\xE2\x85\
\x9B\0frac23\0\xE2\x85\x94\0frac25\0\xE2\x85\x96\0\1frac34\0\
\xC2\xBE\0frac35\0\xE2\x85\x97\0frac38\0\xE2\x85\x9C\0frac45\0\
\xE2\x85\x98\0frac56\0\xE2\x85\x9A\0frac58\0\xE2\x85\x9D\0frac78\
\0\xE2\x85\x9E\0frasl\0\xE2\x81\x84\0frown\0\xE2\x8C\xA2\0fscr\0\xF0\x9D\
\x92\xBB\0gE\0\xE2\x89\xA7\0gEl\0\xE2\xAA\x8C\0gacute\0\xC7\xB5\0gamm\
a\0\xCE\xB3\0gammad\0\xCF\x9D\0gap\0\xE2\xAA\x86\0gbreve\0\xC4\x9F\
\0gcirc\0\xC4\x9D\0gcy\0\xD0\xB3\0gdot\0\xC4\xA1\0ge\0\xE2\x89\xA5\0\
gel\0\xE2\x8B\x9B\0geq\0\xE2\x89\xA5\0geqq\0\xE2\x89\xA7\0geqslan\
t\0\xE2\xA9\xBE\0ges\0\xE2\xA9\xBE\0gescc\0\xE2\xAA\xA9\0gesdot\0\xE2\
\xAA\x80\0gesdoto\0\xE2\xAA\x82\0gesdotol\0\xE2\xAA\x84\0gesl\
\0\xE2\x8B\x9B\0gesles\0\xE2\xAA\x94\0gfr\0\xF0\x9D\x94\xA4\0gg\0\xE2\x89\xAB\0\
ggg\0\xE2\x8B\x99\0gimel\0\xE2\x84\xB7\0gjcy\0\xD1\x93\0gl\0\xE2\x89\xB7\
\0glE\0\xE2\xAA\x92\0gla\0\xE2\xAA\xA5\0glj\0\xE2\xAA\xA4\0gnE\0\xE2\x89\xA9\
\0gnap\0\xE2\xAA\x8A\0gnapprox\0\xE2\xAA\x8A\0gne\0\xE2\xAA\x88\0g\
neq\0\xE2\xAA\x88\0gneqq\0\xE2\x89\xA9\0gnsim\0\xE2\x8B\xA7\0gopf\
\0\xF0\x9D\x95\x98\0grave\0`\0gscr\0\xE2\x84\x8A\0gsim\0\xE2\x89\xB3\0\
gsime\0\xE2\xAA\x8E\0gsiml\0\xE2\xAA\x90\0\1gt\0>\0gtcc\0\xE2\
\xAA\xA7\0gtcir\0\xE2\xA9\xBA\0gtdot\0\xE2\x8B\x97\0gtlPar\0\xE2\xA6\
\x95\0gtquest\0\xE2\xA9\xBC\0gtrapprox\0\xE2\xAA\x86\0gtra\
rr\0\xE2\xA5\xB8\0gtrdot\0\xE2\x8B\x97\0gtreqless\0\xE2\x8B\x9B\0\
gtreqqless\0\xE2\xAA\x8C\0gtrless\0\xE2\x89\xB7\0gtrsi\
m\0\xE2\x89\xB3\0gvertneqq\0\xE2\x89\xA9\0gvnE\0\xE2\x89\xA9\0hAr\
r\0\xE2\x87\x94\0hairsp\0\xE2\x80\x8A\0half\0\xC2\xBD\0hamilt\0\
\xE2\x84\x8B\0hardcy\0\xD1\x8A\0harr\0\xE2\x86\x94\0harrcir\0\xE2\
\xA5\x88\0harrw\0\xE2\x86\xAD\0hbar\0\xE2\x84\x8F\0hcirc\0\xC4\xA5\0h\
earts\0\xE2\x99\xA5\0heartsuit\0\xE2\x99\xA5\0hellip\0\xE2\
\x80\xA6\0hercon\0\xE2\x8A\xB9\0hfr\0\xF0\x9D\x94\xA5\0hksearow\0\
\xE2\xA4\xA5\0hkswarow\0\xE2\xA4\xA6\0hoarr\0\xE2\x87\xBF\0homth\
t\0\xE2\x88\xBB\0hookleftarrow\0\xE2\x86\xA9\0hookrigh\
tarrow\0\xE2\x86\xAA\0hopf\0\xF0\x9D\x95\x99\0horbar\0\xE2\x80\x95\0\
hscr\0\xF0\x9D\x92\xBD\0hslash\0\xE2\x84\x8F\0hstrok\0\xC4\xA7\0h\
ybull\0\xE2\x81\x83\0hyphen\0\xE2\x80\x90\0\1iacute\0\xC3\xAD\0\
ic\0\xE2\x81\xA3\0\1icirc\0\xC3\xAE\0icy\0\xD0\xB8\0iecy\0\xD0\xB5\0\
\1iexcl\0\xC2\xA1\0iff\0\xE2\x87\x94\0ifr\0\xF0\x9D\x94\xA6\0\1igra\
ve\0\xC3\xAC\0ii\0\xE2\x85\x88\0iiiint\0\xE2\xA8\x8C\0iiint\0\xE2\x88\
\xAD\0iinfin\0\xE2\xA7\x9C\0iiota\0\xE2\x84\xA9\0ijlig\0\xC4\xB3\0\
imacr\0\xC4\xAB\0image\0\xE2\x84\x91\0imagline\0\xE2\x84\x90\0\
imagpart\0\xE2\x84\x91\0imath\0\xC4\xB1\0imof\0\xE2\x8A\xB7\0i\
mped\0\xC6\xB5\0in\0\xE2\x88\x88\0incare\0\xE2\x84\x85\0infin\0\
\xE2\x88\x9E\0infintie\0\xE2\xA7\x9D\0inodot\0\xC4\xB1\0int\0\xE2\
\x88\xAB\0intcal\0\xE2\x8A\xBA\0integers\0\xE2\x84\xA4\0inter\
cal\0\xE2\x8A\xBA\0intlarhk\0\xE2\xA8\x97\0intprod\0\xE2\xA8\xBC\
\0iocy\0\xD1\x91\0iogon\0\xC4\xAF\0iopf\0\xF0\x9D\x95\x9A\0iota\
\0\xCE\xB9\0iprod\0\xE2\xA8\xBC\0\1iquest\0\xC2\xBF\0iscr\0\xF0\x9D\
\x92\xBE\0isin\0\xE2\x88\x88\0isinE\0\xE2\x8B\xB9\0isindot\0\xE2\x8B\
\xB5\0isins\0\xE2\x8B\xB4\0isinsv\0\xE2\x8B\xB3\0isinv\0\xE2\x88\x88\
\0it\0\xE2\x81\xA2\0itilde\0\xC4\xA9\0iukcy\0\xD1\x96\0\1iuml\
\0\xC3\xAF\0jcirc\0\xC4\xB5\0jcy\0\xD0\xB9\0jfr\0\xF0\x9D\x94\xA7\0jma\
th\0\xC8\xB7\0jopf\0\xF0\x9D\x95\x9B\0jscr\0\xF0\x9D\x92\xBF\0jsercy\
\0\xD1\x98\0jukcy\0\xD1\x94\0kappa\0\xCE\xBA\0kappav\0\xCF\xB0\0\
kcedil\0\xC4\xB7\0kcy\0\xD0\xBA\0kfr\0\xF0\x9D\x94\xA8\0kgreen\
\0\xC4\xB8\0khcy\0\xD1\x85\0kjcy\0\xD1\x9C\0kopf\0\xF0\x9D\x95\x9C\0ks\
cr\0\xF0\x9D\x93\x80\0lAarr\0\xE2\x87\x9A\0lArr\0\xE2\x87\x90\0lAtai\
l\0\xE2\xA4\x9B\0lBarr\0\xE2\xA4\x8E\0lE\0\xE2\x89\xA6\0lEg\0\xE2\xAA\x8B\0l\
Har\0\xE2\xA5\xA2\0lacute\0\xC4\xBA\0laemptyv\0\xE2\xA6\xB4\0l\
agran\0\xE2\x84\x92\0lambda\0\xCE\xBB\0lang\0\xE2\x9F\xA8\0lan\
gd\0\xE2\xA6\x91\0langle\0\xE2\x9F\xA8\0lap\0\xE2\xAA\x85\0\1laquo\
\0\xC2\xAB\0larr\0\xE2\x86\x90\0larrb\0\xE2\x87\xA4\0larrbfs\0\xE2\
\xA4\x9F\0larrfs\0\xE2\xA4\x9D\0larrhk\0\xE2\x86\xA9\0larrlp\0\
\xE2\x86\xAB\0larrpl\0\xE2\xA4\xB9\0larrsim\0\xE2\xA5\xB3\0larrt\
l\0\xE2\x86\xA2\0lat\0\xE2\xAA\xAB\0latail\0\xE2\xA4\x99\0late\0\xE2\xAA\
\xAD\0lates\0\xE2\xAA\xAD\0lbarr\0\xE2\xA4\x8C\0lbbrk\0\xE2\x9D\xB2\0\
lbrace\0{\0lbrack\0[\0lbrke\0\xE2\xA6\x8B\0lbrk\
sld\0\xE2\xA6\x8F\0lbrkslu\0\xE2\xA6\x8D\0lcaron\0\xC4\xBE\0lc\
edil\0\xC4\xBC\0lceil\0\xE2\x8C\x88\0lcub\0{\0lcy\0\xD0\xBB\0\
ldca\0\xE2\xA4\xB6\0ldquo\0\xE2\x80\x9C\0ldquor\0\xE2\x80\x9E\0ld\
rdhar\0\xE2\xA5\xA7\0ldrushar\0\xE2\xA5\x8B\0ldsh\0\xE2\x86\xB2\0\
le\0\xE2\x89\xA4\0leftarrow\0\xE2\x86\x90\0leftarrowta\
il\0\xE2\x86\xA2\0leftharpoondown\0\xE2\x86\xBD\0lefth\
arpoonup\0\xE2\x86\xBC\0leftleftarrows\0\xE2\x87\x87\0\
leftrightarrow\0\xE2\x86\x94\0leftrightarro\
ws\0\xE2\x87\x86\0leftrightharpoons\0\xE2\x87\x8B\0lef\
trightsquigarrow\0\xE2\x86\xAD\0leftthreeti\
mes\0\xE2\x8B\x8B\0leg\0\xE2\x8B\x9A\0leq\0\xE2\x89\xA4\0leqq\0\xE2\x89\xA6\
\0leqslant\0\xE2\xA9\xBD\0les\0\xE2\xA9\xBD\0lescc\0\xE2\xAA\xA8\0\
lesdot\0\xE2\xA9\xBF\0lesdoto\0\xE2\xAA\x81\0lesdotor\0\
\xE2\xAA\x83\0lesg\0\xE2\x8B\x9A\0lesges\0\xE2\xAA\x93\0lessappr\
ox\0\xE2\xAA\x85\0lessdot\0\xE2\x8B\x96\0lesseqgtr\0\xE2\x8B\x9A\
\0lesseqqgtr\0\xE2\xAA\x8B\0lessgtr\0\xE2\x89\xB6\0less\
sim\0\xE2\x89\xB2\0lfisht\0\xE2\xA5\xBC\0lfloor\0\xE2\x8C\x8A\0lf\
r\0\xF0\x9D\x94\xA9\0lg\0\xE2\x89\xB6\0lgE\0\xE2\xAA\x91\0lhard\0\xE2\x86\xBD\0\
lharu\0\xE2\x86\xBC\0lharul\0\xE2\xA5\xAA\0lhblk\0\xE2\x96\x84\0l\
jcy\0\xD1\x99\0ll\0\xE2\x89\xAA\0llarr\0\xE2\x87\x87\0llcorner\
\0\xE2\x8C\x9E\0llhard\0\xE2\xA5\xAB\0lltri\0\xE2\x97\xBA\0lmidot\
\0\xC5\x80\0lmoust\0\xE2\x8E\xB0\0lmoustache\0\xE2\x8E\xB0\0ln\
E\0\xE2\x89\xA8\0lnap\0\xE2\xAA\x89\0lnapprox\0\xE2\xAA\x89\0lne\0\
\xE2\xAA\x87\0lneq\0\xE2\xAA\x87\0lneqq\0\xE2\x89\xA8\0lnsim\0\xE2\x8B\xA6\
\0loang\0\xE2\x9F\xAC\0loarr\0\xE2\x87\xBD\0lobrk\0\xE2\x9F\xA6\0l\
ongleftarrow\0\xE2\x9F\xB5\0longleftrightar\
row\0\xE2\x9F\xB7\0longmapsto\0\xE2\x9F\xBC\0longright\
arrow\0\xE2\x9F\xB6\0looparrowleft\0\xE2\x86\xAB\0loop\
arrowright\0\xE2\x86\xAC\0lopar\0\xE2\xA6\x85\0lopf\0\xF0\x9D\
\x95\x9D\0loplus\0\xE2\xA8\xAD\0lotimes\0\xE2\xA8\xB4\0lowast\
\0\xE2\x88\x97\0lowbar\0_\0loz\0\xE2\x97\x8A\0lozenge\0\xE2\x97\
\x8A\0lozf\0\xE2\xA7\xAB\0lpar\0(\0lparlt\0\xE2\xA6\x93\0lra\
rr\0\xE2\x87\x86\0lrcorner\0\xE2\x8C\x9F\0lrhar\0\xE2\x87\x8B\0lr\
hard\0\xE2\xA5\xAD\0lrm\0\xE2\x80\x8E\0lrtri\0\xE2\x8A\xBF\0lsaqu\
o\0\xE2\x80\xB9\0lscr\0\xF0\x9D\x93\x81\0lsh\0\xE2\x86\xB0\0lsim\0\xE2\x89\xB2\
\0lsime\0\xE2\xAA\x8D\0lsimg\0\xE2\xAA\x8F\0lsqb\0[\0lsqu\
o\0\xE2\x80\x98\0lsquor\0\xE2\x80\x9A\0lstrok\0\xC5\x82\0\1lt\0<\
\0ltcc\0\xE2\xAA\xA6\0ltcir\0\xE2\xA9\xB9\0ltdot\0\xE2\x8B\x96\0lt\
hree\0\xE2\x8B\x8B\0ltimes\0\xE2\x8B\x89\0ltlarr\0\xE2\xA5\xB6\0l\
tquest\0\xE2\xA9\xBB\0ltrPar\0\xE2\xA6\x96\0ltri\0\xE2\x97\x83\0l\
trie\0\xE2\x8A\xB4\0ltrif\0\xE2\x97\x82\0lurdshar\0\xE2\xA5\x8A\0\
luruhar\0\xE2\xA5\xA6\0lvertneqq\0\xE2\x89\xA8\0lvnE\0\xE2\
\x89\xA8\0mDDot\0\xE2\x88\xBA\0\1macr\0\xC2\xAF\0male\0\xE2\x99\x82\0m\
alt\0\xE2\x9C\xA0\0maltese\0\xE2\x9C\xA0\0map\0\xE2\x86\xA6\0maps\
to\0\xE2\x86\xA6\0mapstodown\0\xE2\x86\xA7\0mapstoleft\
\0\xE2\x86\xA4\0mapstoup\0\xE2\x86\xA5\0marker\0\xE2\x96\xAE\0mco\
mma\0\xE2\xA8\xA9\0mcy\0\xD0\xBC\0mdash\0\xE2\x80\x94\0measure\
dangle\0\xE2\x88\xA1\0mfr\0\xF0\x9D\x94\xAA\0mho\0\xE2\x84\xA7\0\1mic\
ro\0\xC2\xB5\0mid\0\xE2\x88\xA3\0midast\0*\0midcir\0\xE2\xAB\
\xB0\0\1middot\0\xC2\xB7\0minus\0\xE2\x88\x92\0minusb\0\xE2\x8A\
\x9F\0minusd\0\xE2\x88\xB8\0minusdu\0\xE2\xA8\xAA\0mlcp\0\xE2\xAB\
\x9B\0mldr\0\xE2\x80\xA6\0mnplus\0\xE2\x88\x93\0models\0\xE2\x8A\xA7\
\0mopf\0\xF0\x9D\x95\x9E\0mp\0\xE2\x88\x93\0mscr\0\xF0\x9D\x93\x82\0mstp\
os\0\xE2\x88\xBE\0mu\0\xCE\xBC\0multimap\0\xE2\x8A\xB8\0mumap\0\
\xE2\x8A\xB8\0nGg\0\xE2\x8B\x99\0nGt\0\xE2\x89\xAB\0nGtv\0\xE2\x89\xAB\0nLe\
ftarrow\0\xE2\x87\x8D\0nLeftrightarrow\0\xE2\x87\x8E\0\
nLl\0\xE2\x8B\x98\0nLt\0\xE2\x89\xAA\0nLtv\0\xE2\x89\xAA\0nRighta\
rrow\0\xE2\x87\x8F\0nVDash\0\xE2\x8A\xAF\0nVdash\0\xE2\x8A\xAE\0n\
abla\0\xE2\x88\x87\0nacute\0\xC5\x84\0nang\0\xE2\x88\xA0\0nap\0\
\xE2\x89\x89\0napE\0\xE2\xA9\xB0\0napid\0\xE2\x89\x8B\0napos\0\xC5\x89\0\
napprox\0\xE2\x89\x89\0natur\0\xE2\x99\xAE\0natural\0\xE2\x99\
\xAE\0naturals\0\xE2\x84\x95\0\1nbsp\0\xC2\xA0\0nbump\0\xE2\x89\
\x8E\0nbumpe\0\xE2\x89\x8F\0ncap\0\xE2\xA9\x83\0ncaron\0\xC5\x88\0\
ncedil\0\xC5\x86\0ncong\0\xE2\x89\x87\0ncongdot\0\xE2\xA9\xAD\
\0ncup\0\xE2\xA9\x82\0ncy\0\xD0\xBD\0ndash\0\xE2\x80\x93\0ne\0\xE2\x89\
\xA0\0neArr\0\xE2\x87\x97\0nearhk\0\xE2\xA4\xA4\0nearr\0\xE2\x86\x97\
\0nearrow\0\xE2\x86\x97\0nedot\0\xE2\x89\x90\0nequiv\0\xE2\x89\
\xA2\0nesear\0\xE2\xA4\xA8\0nesim\0\xE2\x89\x82\0nexist\0\xE2\x88\
\x84\0nexists\0\xE2\x88\x84\0nfr\0\xF0\x9D\x94\xAB\0ngE\0\xE2\x89\xA7\0n\
ge\0\xE2\x89\xB1\0ngeq\0\xE2\x89\xB1\0ngeqq\0\xE2\x89\xA7\0ngeqsl\
ant\0\xE2\xA9\xBE\0nges\0\xE2\xA9\xBE\0ngsim\0\xE2\x89\xB5\0ngt\0\xE2\
\x89\xAF\0ngtr\0\xE2\x89\xAF\0nhArr\0\xE2\x87\x8E\0nharr\0\xE2\x86\xAE\0\
nhpar\0\xE2\xAB\xB2\0ni\0\xE2\x88\x8B\0nis\0\xE2\x8B\xBC\0nisd\0\xE2\x8B\
\xBA\0niv\0\xE2\x88\x8B\0njcy\0\xD1\x9A\0nlArr\0\xE2\x87\x8D\0nlE\0\
\xE2\x89\xA6\0nlarr\0\xE2\x86\x9A\0nldr\0\xE2\x80\xA5\0nle\0\xE2\x89\xB0\0n\
leftarrow\0\xE2\x86\x9A\0nleftrightarrow\0\xE2\x86\
\xAE\0nleq\0\xE2\x89\xB0\0nleqq\0\xE2\x89\xA6\0nleqslant\0\xE2\
\xA9\xBD\0nles\0\xE2\xA9\xBD\0nless\0\xE2\x89\xAE\0nlsim\0\xE2\x89\xB4\0\
nlt\0\xE2\x89\xAE\0nltri\0\xE2\x8B\xAA\0nltrie\0\xE2\x8B\xAC\0nmi\
d\0\xE2\x88\xA4\0nopf\0\xF0\x9D\x95\x9F\0\1not\0\xC2\xAC\0notin\0\xE2\x88\
\x89\0notinE\0\xE2\x8B\xB9\0notindot\0\xE2\x8B\xB5\0notinv\
a\0\xE2\x88\x89\0notinvb\0\xE2\x8B\xB7\0notinvc\0\xE2\x8B\xB6\0no\
tni\0\xE2\x88\x8C\0notniva\0\xE2\x88\x8C\0notnivb\0\xE2\x8B\xBE\0\
notnivc\0\xE2\x8B\xBD\0npar\0\xE2\x88\xA6\0nparallel\0\xE2\
\x88\xA6\0nparsl\0\xE2\xAB\xBD\0npart\0\xE2\x88\x82\0npolint\0\
\xE2\xA8\x94\0npr\0\xE2\x8A\x80\0nprcue\0\xE2\x8B\xA0\0npre\0\xE2\xAA\xAF\0\
nprec\0\xE2\x8A\x80\0npreceq\0\xE2\xAA\xAF\0nrArr\0\xE2\x87\x8F\0\
nrarr\0\xE2\x86\x9B\0nrarrc\0\xE2\xA4\xB3\0nrarrw\0\xE2\x86\x9D\0\
nrightarrow\0\xE2\x86\x9B\0nrtri\0\xE2\x8B\xAB\0nrtrie\
\0\xE2\x8B\xAD\0nsc\0\xE2\x8A\x81\0nsccue\0\xE2\x8B\xA1\0nsce\0\xE2\xAA\xB0\
\0nscr\0\xF0\x9D\x93\x83\0nshortmid\0\xE2\x88\xA4\0nshortp\
arallel\0\xE2\x88\xA6\0nsim\0\xE2\x89\x81\0nsime\0\xE2\x89\x84\0n\
simeq\0\xE2\x89\x84\0nsmid\0\xE2\x88\xA4\0nspar\0\xE2\x88\xA6\0ns\
qsube\0\xE2\x8B\xA2\0nsqsupe\0\xE2\x8B\xA3\0nsub\0\xE2\x8A\x84\0n\
subE\0\xE2\xAB\x85\0nsube\0\xE2\x8A\x88\0nsubset\0\xE2\x8A\x82\0n\
subseteq\0\xE2\x8A\x88\0nsubseteqq\0\xE2\xAB\x85\0nsuc\
c\0\xE2\x8A\x81\0nsucceq\0\xE2\xAA\xB0\0nsup\0\xE2\x8A\x85\0nsupE\
\0\xE2\xAB\x86\0nsupe\0\xE2\x8A\x89\0nsupset\0\xE2\x8A\x83\0nsups\
eteq\0\xE2\x8A\x89\0nsupseteqq\0\xE2\xAB\x86\0ntgl\0\xE2\x89\xB9\
\0\1ntilde\0\xC3\xB1\0ntlg\0\xE2\x89\xB8\0ntrianglele\
ft\0\xE2\x8B\xAA\0ntrianglelefteq\0\xE2\x8B\xAC\0ntria\
ngleright\0\xE2\x8B\xAB\0ntrianglerighteq\0\xE2\
\x8B\xAD\0nu\0\xCE\xBD\0num\0#\0numero\0\xE2\x84\x96\0numsp\0\
\xE2\x80\x87\0nvDash\0\xE2\x8A\xAD\0nvHarr\0\xE2\xA4\x84\0nvap\0\xE2\
\x89\x8D\0nvdash\0\xE2\x8A\xAC\0nvge\0\xE2\x89\xA5\0nvgt\0>\0nv\
infin\0\xE2\xA7\x9E\0nvlArr\0\xE2\xA4\x82\0nvle\0\xE2\x89\xA4\0nv\
lt\0<\0nvltrie\0\xE2\x8A\xB4\0nvrArr\0\xE2\xA4\x83\0nvrt\
rie\0\xE2\x8A\xB5\0nvsim\0\xE2\x88\xBC\0nwArr\0\xE2\x87\x96\0nwar\
hk\0\xE2\xA4\xA3\0nwarr\0\xE2\x86\x96\0nwarrow\0\xE2\x86\x96\0nwn\
ear\0\xE2\xA4\xA7\0oS\0\xE2\x93\x88\0\1oacute\0\xC3\xB3\0oast\0\xE2\
\x8A\x9B\0ocir\0\xE2\x8A\x9A\0\1ocirc\0\xC3\xB4\0ocy\0\xD0\xBE\0oda\
sh\0\xE2\x8A\x9D\0odblac\0\xC5\x91\0odiv\0\xE2\xA8\xB8\0odot\0\xE2\
\x8A\x99\0odsold\0\xE2\xA6\xBC\0oelig\0\xC5\x93\0ofcir\0\xE2\xA6\xBF\
\0ofr\0\xF0\x9D\x94\xAC\0ogon\0\xCB\x9B\0\1ograve\0\xC3\xB2\0ogt\
\0\xE2\xA7\x81\0ohbar\0\xE2\xA6\xB5\0ohm\0\xCE\xA9\0oint\0\xE2\x88\xAE\0o\
larr\0\xE2\x86\xBA\0olcir\0\xE2\xA6\xBE\0olcross\0\xE2\xA6\xBB\0o\
line\0\xE2\x80\xBE\0olt\0\xE2\xA7\x80\0omacr\0\xC5\x8D\0omega\0\
\xCF\x89\0omicron\0\xCE\xBF\0omid\0\xE2\xA6\xB6\0ominus\0\xE2\x8A\
\x96\0oopf\0\xF0\x9D\x95\xA0\0opar\0\xE2\xA6\xB7\0operp\0\xE2\xA6\xB9\0o\
plus\0\xE2\x8A\x95\0or\0\xE2\x88\xA8\0orarr\0\xE2\x86\xBB\0ord\0\xE2\xA9\
\x9D\0order\0\xE2\x84\xB4\0orderof\0\xE2\x84\xB4\0\1ordf\0\xC2\xAA\
\0\1ordm\0\xC2\xBA\0origof\0\xE2\x8A\xB6\0oror\0\xE2\xA9\x96\0or\
slope\0\xE2\xA9\x97\0orv\0\xE2\xA9\x9B\0oscr\0\xE2\x84\xB4\0\1osla\
sh\0\xC3\xB8\0osol\0\xE2\x8A\x98\0\1otilde\0\xC3\xB5\0otimes\
\0\xE2\x8A\x97\0otimesas\0\xE2\xA8\xB6\0\1ouml\0\xC3\xB6\0ovbar\
\0\xE2\x8C\xBD\0par\0\xE2\x88\xA5\0\1para\0\xC2\xB6\0parallel\0\xE2\
\x88\xA5\0parsim\0\xE2\xAB\xB3\0parsl\0\xE2\xAB\xBD\0part\0\xE2\x88\x82\
\0pcy\0\xD0\xBF\0percnt\0%\0period\0.\0permil\
\0\xE2\x80\xB0\0perp\0\xE2\x8A\xA5\0pertenk\0\xE2\x80\xB1\0pfr\0\xF0\x9D\
\x94\xAD\0phi\0\xCF\x86\0phiv\0\xCF\x95\0phmmat\0\xE2\x84\xB3\0pho\
ne\0\xE2\x98\x8E\0pi\0\xCF\x80\0pitchfork\0\xE2\x8B\x94\0piv\0\xCF\
\x96\0planck\0\xE2\x84\x8F\0planckh\0\xE2\x84\x8E\0plankv\0\
\xE2\x84\x8F\0plus\0+\0plusacir\0\xE2\xA8\xA3\0plusb\0\xE2\x8A\
\x9E\0pluscir\0\xE2\xA8\xA2\0plusdo\0\xE2\x88\x94\0plusdu\0\
\xE2\xA8\xA5\0pluse\0\xE2\xA9\xB2\0\1plusmn\0\xC2\xB1\0plussim\
\0\xE2\xA8\xA6\0plustwo\0\xE2\xA8\xA7\0pm\0\xC2\xB1\0pointint\0\
\xE2\xA8\x95\0popf\0\xF0\x9D\x95\xA1\0\1pound\0\xC2\xA3\0pr\0\xE2\x89\xBA\0p\
rE\0\xE2\xAA\xB3\0prap\0\xE2\xAA\xB7\0prcue\0\xE2\x89\xBC\0pre\0\xE2\xAA\
\xAF\0prec\0\xE2\x89\xBA\0precapprox\0\xE2\xAA\xB7\0preccu\
rlyeq\0\xE2\x89\xBC\0preceq\0\xE2\xAA\xAF\0precnapprox\
\0\xE2\xAA\xB9\0precneqq\0\xE2\xAA\xB5\0precnsim\0\xE2\x8B\xA8\0p\
recsim\0\xE2\x89\xBE\0prime\0\xE2\x80\xB2\0primes\0\xE2\x84\x99\0\
prnE\0\xE2\xAA\xB5\0prnap\0\xE2\xAA\xB9\0prnsim\0\xE2\x8B\xA8\0pr\
od\0\xE2\x88\x8F\0profalar\0\xE2\x8C\xAE\0profline\0\xE2\x8C\x92\
\0profsurf\0\xE2\x8C\x93\0prop\0\xE2\x88\x9D\0propto\0\xE2\x88\
\x9D\0prsim\0\xE2\x89\xBE\0prurel\0\xE2\x8A\xB0\0pscr\0\xF0\x9D\x93\x85\
\0psi\0\xCF\x88\0puncsp\0\xE2\x80\x88\0qfr\0\xF0\x9D\x94\xAE\0qint\
\0\xE2\xA8\x8C\0qopf\0\xF0\x9D\x95\xA2\0qprime\0\xE2\x81\x97\0qscr\0\xF0\
\x9D\x93\x86\0quaternions\0\xE2\x84\x8D\0quatint\0\xE2\xA8\x96\0\
quest\0?\0questeq\0\xE2\x89\x9F\0\1quot\0\"\0rAar\
r\0\xE2\x87\x9B\0rArr\0\xE2\x87\x92\0rAtail\0\xE2\xA4\x9C\0rBarr\0\
\xE2\xA4\x8F\0rHar\0\xE2\xA5\xA4\0race\0\xE2\x88\xBD\0racute\0\xC5\x95\0\
radic\0\xE2\x88\x9A\0raemptyv\0\xE2\xA6\xB3\0rang\0\xE2\x9F\xA9\0\
rangd\0\xE2\xA6\x92\0range\0\xE2\xA6\xA5\0rangle\0\xE2\x9F\xA9\0\1\
raquo\0\xC2\xBB\0rarr\0\xE2\x86\x92\0rarrap\0\xE2\xA5\xB5\0rar\
rb\0\xE2\x87\xA5\0rarrbfs\0\xE2\xA4\xA0\0rarrc\0\xE2\xA4\xB3\0rar\
rfs\0\xE2\xA4\x9E\0rarrhk\0\xE2\x86\xAA\0rarrlp\0\xE2\x86\xAC\0ra\
rrpl\0\xE2\xA5\x85\0rarrsim\0\xE2\xA5\xB4\0rarrtl\0\xE2\x86\xA3\0\
rarrw\0\xE2\x86\x9D\0ratail\0\xE2\xA4\x9A\0ratio\0\xE2\x88\xB6\0r\
ationals\0\xE2\x84\x9A\0rbarr\0\xE2\xA4\x8D\0rbbrk\0\xE2\x9D\xB3\
\0rbrace\0}\0rbrack\0]\0rbrke\0\xE2\xA6\x8C\0rbr\
ksld\0\xE2\xA6\x8E\0rbrkslu\0\xE2\xA6\x90\0rcaron\0\xC5\x99\0r\
cedil\0\xC5\x97\0rceil\0\xE2\x8C\x89\0rcub\0}\0rcy\0\xD1\x80\
\0rdca\0\xE2\xA4\xB7\0rdldhar\0\xE2\xA5\xA9\0rdquo\0\xE2\x80\x9D\0\
rdquor\0\xE2\x80\x9D\0rdsh\0\xE2\x86\xB3\0real\0\xE2\x84\x9C\0rea\
line\0\xE2\x84\x9B\0realpart\0\xE2\x84\x9C\0reals\0\xE2\x84\x9D\0\
rect\0\xE2\x96\xAD\0\1reg\0\xC2\xAE\0rfisht\0\xE2\xA5\xBD\0rflo\
or\0\xE2\x8C\x8B\0rfr\0\xF0\x9D\x94\xAF\0rhard\0\xE2\x87\x81\0rharu\0\
\xE2\x87\x80\0rharul\0\xE2\xA5\xAC\0rho\0\xCF\x81\0rhov\0\xCF\xB1\0ri\
ghtarrow\0\xE2\x86\x92\0rightarrowtail\0\xE2\x86\xA3\0\
rightharpoondown\0\xE2\x87\x81\0rightharpoo\
nup\0\xE2\x87\x80\0rightleftarrows\0\xE2\x87\x84\0righ\
tleftharpoons\0\xE2\x87\x8C\0rightrightarro\
ws\0\xE2\x87\x89\0rightsquigarrow\0\xE2\x86\x9D\0right\
threetimes\0\xE2\x8B\x8C\0ring\0\xCB\x9A\0risingdot\
seq\0\xE2\x89\x93\0rlarr\0\xE2\x87\x84\0rlhar\0\xE2\x87\x8C\0rlm\0\
\xE2\x80\x8F\0rmoust\0\xE2\x8E\xB1\0rmoustache\0\xE2\x8E\xB1\0rn\
mid\0\xE2\xAB\xAE\0roang\0\xE2\x9F\xAD\0roarr\0\xE2\x87\xBE\0robr\
k\0\xE2\x9F\xA7\0ropar\0\xE2\xA6\x86\0ropf\0\xF0\x9D\x95\xA3\0roplus\
\0\xE2\xA8\xAE\0rotimes\0\xE2\xA8\xB5\0rpar\0)\0rpargt\0\xE2\
\xA6\x94\0rppolint\0\xE2\xA8\x92\0rrarr\0\xE2\x87\x89\0rsaquo\
\0\xE2\x80\xBA\0rscr\0\xF0\x9D\x93\x87\0rsh\0\xE2\x86\xB1\0rsqb\0]\0rs\
quo\0\xE2\x80\x99\0rsquor\0\xE2\x80\x99\0rthree\0\xE2\x8B\x8C\0rt\
imes\0\xE2\x8B\x8A\0rtri\0\xE2\x96\xB9\0rtrie\0\xE2\x8A\xB5\0rtri\
f\0\xE2\x96\xB8\0rtriltri\0\xE2\xA7\x8E\0ruluhar\0\xE2\xA5\xA8\0r\
x\0\xE2\x84\x9E\0sacute\0\xC5\x9B\0sbquo\0\xE2\x80\x9A\0sc\0\xE2\x89\xBB\
\0scE\0\xE2\xAA\xB4\0scap\0\xE2\xAA\xB8\0scaron\0\xC5\xA1\0sccu\
e\0\xE2\x89\xBD\0sce\0\xE2\xAA\xB0\0scedil\0\xC5\x9F\0scirc\0\xC5\x9D\
\0scnE\0\xE2\xAA\xB6\0scnap\0\xE2\xAA\xBA\0scnsim\0\xE2\x8B\xA9\0s\
cpolint\0\xE2\xA8\x93\0scsim\0\xE2\x89\xBF\0scy\0\xD1\x81\0sdo\
t\0\xE2\x8B\x85\0sdotb\0\xE2\x8A\xA1\0sdote\0\xE2\xA9\xA6\0seArr\0\
\xE2\x87\x98\0searhk\0\xE2\xA4\xA5\0searr\0\xE2\x86\x98\0searrow\
\0\xE2\x86\x98\0\1sect\0\xC2\xA7\0semi\0;\0seswar\0\xE2\xA4\xA9\0\
setminus\0\xE2\x88\x96\0setmn\0\xE2\x88\x96\0sext\0\xE2\x9C\xB6\0\
sfr\0\xF0\x9D\x94\xB0\0sfrown\0\xE2\x8C\xA2\0sharp\0\xE2\x99\xAF\0sh\
chcy\0\xD1\x89\0shcy\0\xD1\x88\0shortmid\0\xE2\x88\xA3\0sho\
rtparallel\0\xE2\x88\xA5\0\1shy\0\xC2\xAD\0sigma\0\xCF\x83\0\
sigmaf\0\xCF\x82\0sigmav\0\xCF\x82\0sim\0\xE2\x88\xBC\0simd\
ot\0\xE2\xA9\xAA\0sime\0\xE2\x89\x83\0simeq\0\xE2\x89\x83\0simg\0\xE2\
\xAA\x9E\0simgE\0\xE2\xAA\xA0\0siml\0\xE2\xAA\x9D\0simlE\0\xE2\xAA\x9F\0\
simne\0\xE2\x89\x86\0simplus\0\xE2\xA8\xA4\0simrarr\0\xE2\xA5\
\xB2\0slarr\0\xE2\x86\x90\0smallsetminus\0\xE2\x88\x96\0sm\
ashp\0\xE2\xA8\xB3\0smeparsl\0\xE2\xA7\xA4\0smid\0\xE2\x88\xA3\0s\
mile\0\xE2\x8C\xA3\0smt\0\xE2\xAA\xAA\0smte\0\xE2\xAA\xAC\0smtes\0\
\xE2\xAA\xAC\0softcy\0\xD1\x8C\0sol\0/\0solb\0\xE2\xA7\x84\0sol\
bar\0\xE2\x8C\xBF\0sopf\0\xF0\x9D\x95\xA4\0spades\0\xE2\x99\xA0\0spa\
desuit\0\xE2\x99\xA0\0spar\0\xE2\x88\xA5\0sqcap\0\xE2\x8A\x93\0sq\
caps\0\xE2\x8A\x93\0sqcup\0\xE2\x8A\x94\0sqcups\0\xE2\x8A\x94\0sq\
sub\0\xE2\x8A\x8F\0sqsube\0\xE2\x8A\x91\0sqsubset\0\xE2\x8A\x8F\0\
sqsubseteq\0\xE2\x8A\x91\0sqsup\0\xE2\x8A\x90\0sqsupe\0\
\xE2\x8A\x92\0sqsupset\0\xE2\x8A\x90\0sqsupseteq\0\xE2\x8A\x92\0\
squ\0\xE2\x96\xA1\0square\0\xE2\x96\xA1\0squarf\0\xE2\x96\xAA\0sq\
uf\0\xE2\x96\xAA\0srarr\0\xE2\x86\x92\0sscr\0\xF0\x9D\x93\x88\0ssetm\
n\0\xE2\x88\x96\0ssmile\0\xE2\x8C\xA3\0sstarf\0\xE2\x8B\x86\0star\
\0\xE2\x98\x86\0starf\0\xE2\x98\x85\0straightepsilon\0\xCF\
\xB5\0straightphi\0\xCF\x95\0strns\0\xC2\xAF\0sub\0\xE2\x8A\
\x82\0subE\0\xE2\xAB\x85\0subdot\0\xE2\xAA\xBD\0sube\0\xE2\x8A\x86\0s\
ubedot\0\xE2\xAB\x83\0submult\0\xE2\xAB\x81\0subnE\0\xE2\xAB\x8B\
\0subne\0\xE2\x8A\x8A\0subplus\0\xE2\xAA\xBF\0subrarr\0\xE2\
\xA5\xB9\0subset\0\xE2\x8A\x82\0subseteq\0\xE2\x8A\x86\0subse\
teqq\0\xE2\xAB\x85\0subsetneq\0\xE2\x8A\x8A\0subsetneq\
q\0\xE2\xAB\x8B\0subsim\0\xE2\xAB\x87\0subsub\0\xE2\xAB\x95\0subs\
up\0\xE2\xAB\x93\0succ\0\xE2\x89\xBB\0succapprox\0\xE2\xAA\xB8\0s\
ucccurlyeq\0\xE2\x89\xBD\0succeq\0\xE2\xAA\xB0\0succna\
pprox\0\xE2\xAA\xBA\0succneqq\0\xE2\xAA\xB6\0succnsim\0\
\xE2\x8B\xA9\0succsim\0\xE2\x89\xBF\0sum\0\xE2\x88\x91\0sung\0\xE2\x99\xAA\
\0sup\0\xE2\x8A\x83\0\1sup1\0\xC2\xB9\0\1sup2\0\xC2\xB2\0\1sup3\
\0\xC2\xB3\0supE\0\xE2\xAB\x86\0supdot\0\xE2\xAA\xBE\0supdsub\0\
\xE2\xAB\x98\0supe\0\xE2\x8A\x87\0supedot\0\xE2\xAB\x84\0suphsol\
\0\xE2\x9F\x89\0suphsub\0\xE2\xAB\x97\0suplarr\0\xE2\xA5\xBB\0sup\
mult\0\xE2\xAB\x82\0supnE\0\xE2\xAB\x8C\0supne\0\xE2\x8A\x8B\0sup\
plus\0\xE2\xAB\x80\0supset\0\xE2\x8A\x83\0supseteq\0\xE2\x8A\x87\
\0supseteqq\0\xE2\xAB\x86\0supsetneq\0\xE2\x8A\x8B\0sup\
setneqq\0\xE2\xAB\x8C\0supsim\0\xE2\xAB\x88\0supsub\0\xE2\xAB\
\x94\0supsup\0\xE2\xAB\x96\0swArr\0\xE2\x87\x99\0swarhk\0\xE2\xA4\
\xA6\0swarr\0\xE2\x86\x99\0swarrow\0\xE2\x86\x99\0swnwar\0\xE2\
\xA4\xAA\0\1szlig\0\xC3\x9F\0target\0\xE2\x8C\x96\0tau\0\xCF\x84\0t\
brk\0\xE2\x8E\xB4\0tcaron\0\xC5\xA5\0tcedil\0\xC5\xA3\0tcy\0\
\xD1\x82\0tdot\0\xE2\x83\x9B\0telrec\0\xE2\x8C\x95\0tfr\0\xF0\x9D\x94\xB1\0\
there4\0\xE2\x88\xB4\0therefore\0\xE2\x88\xB4\0theta\0\xCE\
\xB8\0thetasym\0\xCF\x91\0thetav\0\xCF\x91\0thickapp\
rox\0\xE2\x89\x88\0thicksim\0\xE2\x88\xBC\0thinsp\0\xE2\x80\x89\0\
thkap\0\xE2\x89\x88\0thksim\0\xE2\x88\xBC\0\1thorn\0\xC3\xBE\0t\
ilde\0\xCB\x9C\0\1times\0\xC3\x97\0timesb\0\xE2\x8A\xA0\0tim\
esbar\0\xE2\xA8\xB1\0timesd\0\xE2\xA8\xB0\0tint\0\xE2\x88\xAD\0to\
ea\0\xE2\xA4\xA8\0top\0\xE2\x8A\xA4\0topbot\0\xE2\x8C\xB6\0topcir\
\0\xE2\xAB\xB1\0topf\0\xF0\x9D\x95\xA5\0topfork\0\xE2\xAB\x9A\0tosa\0\
\xE2\xA4\xA9\0tprime\0\xE2\x80\xB4\0trade\0\xE2\x84\xA2\0triangl\
e\0\xE2\x96\xB5\0triangledown\0\xE2\x96\xBF\0trianglel\
eft\0\xE2\x97\x83\0trianglelefteq\0\xE2\x8A\xB4\0trian\
gleq\0\xE2\x89\x9C\0triangleright\0\xE2\x96\xB9\0trian\
glerighteq\0\xE2\x8A\xB5\0tridot\0\xE2\x97\xAC\0trie\0\xE2\
\x89\x9C\0triminus\0\xE2\xA8\xBA\0triplus\0\xE2\xA8\xB9\0tris\
b\0\xE2\xA7\x8D\0tritime\0\xE2\xA8\xBB\0trpezium\0\xE2\x8F\xA2\0t\
scr\0\xF0\x9D\x93\x89\0tscy\0\xD1\x86\0tshcy\0\xD1\x9B\0tstrok\
\0\xC5\xA7\0twixt\0\xE2\x89\xAC\0twoheadleftarrow\0\xE2\
\x86\x9E\0twoheadrightarrow\0\xE2\x86\xA0\0uArr\0\xE2\x87\
\x91\0uHar\0\xE2\xA5\xA3\0\1uacute\0\xC3\xBA\0uarr\0\xE2\x86\x91\0u\
brcy\0\xD1\x9E\0ubreve\0\xC5\xAD\0\1ucirc\0\xC3\xBB\0ucy\0\
\xD1\x83\0udarr\0\xE2\x87\x85\0udblac\0\xC5\xB1\0udhar\0\xE2\xA5\xAE\
\0ufisht\0\xE2\xA5\xBE\0ufr\0\xF0\x9D\x94\xB2\0\1ugrave\0\xC3\xB9\0\
uharl\0\xE2\x86\xBF\0uharr\0\xE2\x86\xBE\0uhblk\0\xE2\x96\x80\0ul\
corn\0\xE2\x8C\x9C\0ulcorner\0\xE2\x8C\x9C\0ulcrop\0\xE2\x8C\x8F\
\0ultri\0\xE2\x97\xB8\0umacr\0\xC5\xAB\0\1uml\0\xC2\xA8\0uogo\
n\0\xC5\xB3\0uopf\0\xF0\x9D\x95\xA6\0uparrow\0\xE2\x86\x91\0updow\
narrow\0\xE2\x86\x95\0upharpoonleft\0\xE2\x86\xBF\0uph\
arpoonright\0\xE2\x86\xBE\0uplus\0\xE2\x8A\x8E\0upsi\0\xCF\
\x85\0upsih\0\xCF\x92\0upsilon\0\xCF\x85\0upuparrows\
\0\xE2\x87\x88\0urcorn\0\xE2\x8C\x9D\0urcorner\0\xE2\x8C\x9D\0urc\
rop\0\xE2\x8C\x8E\0uring\0\xC5\xAF\0urtri\0\xE2\x97\xB9\0uscr\0\
\xF0\x9D\x93\x8A\0utdot\0\xE2\x8B\xB0\0utilde\0\xC5\xA9\0utri\0\xE2\x96\
\xB5\0utrif\0\xE2\x96\xB4\0uuarr\0\xE2\x87\x88\0\1uuml\0\xC3\xBC\0u\
wangle\0\xE2\xA6\xA7\0vArr\0\xE2\x87\x95\0vBar\0\xE2\xAB\xA8\0vBa\
rv\0\xE2\xAB\xA9\0vDash\0\xE2\x8A\xA8\0vangrt\0\xE2\xA6\x9C\0vare\
psilon\0\xCF\xB5\0varkappa\0\xCF\xB0\0varnothing\
\0\xE2\x88\x85\0varphi\0\xCF\x95\0varpi\0\xCF\x96\0varpropt\
o\0\xE2\x88\x9D\0varr\0\xE2\x86\x95\0varrho\0\xCF\xB1\0varsigm\
a\0\xCF\x82\0varsubsetneq\0\xE2\x8A\x8A\0varsubsetn\
eqq\0\xE2\xAB\x8B\0varsupsetneq\0\xE2\x8A\x8B\0varsups\
etneqq\0\xE2\xAB\x8C\0vartheta\0\xCF\x91\0vartriang\
leleft\0\xE2\x8A\xB2\0vartriangleright\0\xE2\x8A\xB3\0\
vcy\0\xD0\xB2\0vdash\0\xE2\x8A\xA2\0vee\0\xE2\x88\xA8\0veebar\0\
\xE2\x8A\xBB\0veeeq\0\xE2\x89\x9A\0vellip\0\xE2\x8B\xAE\0verbar\0\
|\0vert\0|\0vfr\0\xF0\x9D\x94\xB3\0vltri\0\xE2\x8A\xB2\0vnsu\
b\0\xE2\x8A\x82\0vnsup\0\xE2\x8A\x83\0vopf\0\xF0\x9D\x95\xA7\0vprop\0\
\xE2\x88\x9D\0vrtri\0\xE2\x8A\xB3\0vscr\0\xF0\x9D\x93\x8B\0vsubnE\0\xE2\
\xAB\x8B\0vsubne\0\xE2\x8A\x8A\0vsupnE\0\xE2\xAB\x8C\0vsupne\0\
\xE2\x8A\x8B\0vzigzag\0\xE2\xA6\x9A\0wcirc\0\xC5\xB5\0wedbar\0\
\xE2\xA9\x9F\0wedge\0\xE2\x88\xA7\0wedgeq\0\xE2\x89\x99\0weierp\0\
\xE2\x84\x98\0wfr\0\xF0\x9D\x94\xB4\0wopf\0\xF0\x9D\x95\xA8\0wp\0\xE2\x84\x98\0wr\
\0\xE2\x89\x80\0wreath\0\xE2\x89\x80\0wscr\0\xF0\x9D\x93\x8C\0xcap\0\xE2\
\x8B\x82\0xcirc\0\xE2\x97\xAF\0xcup\0\xE2\x8B\x83\0xdtri\0\xE2\x96\xBD\0\
xfr\0\xF0\x9D\x94\xB5\0xhArr\0\xE2\x9F\xBA\0xharr\0\xE2\x9F\xB7\0xi\0\
\xCE\xBE\0xlArr\0\xE2\x9F\xB8\0xlarr\0\xE2\x9F\xB5\0xmap\0\xE2\x9F\xBC\0\
xnis\0\xE2\x8B\xBB\0xodot\0\xE2\xA8\x80\0xopf\0\xF0\x9D\x95\xA9\0xop\
lus\0\xE2\xA8\x81\0xotime\0\xE2\xA8\x82\0xrArr\0\xE2\x9F\xB9\0xra\
rr\0\xE2\x9F\xB6\0xscr\0\xF0\x9D\x93\x8D\0xsqcup\0\xE2\xA8\x86\0xupl\
us\0\xE2\xA8\x84\0xutri\0\xE2\x96\xB3\0xvee\0\xE2\x8B\x81\0xwedge\
\0\xE2\x8B\x80\0\1yacute\0\xC3\xBD\0yacy\0\xD1\x8F\0ycirc\0\xC5\xB7\
\0ycy\0\xD1\x8B\0\1yen\0\xC2\xA5\0yfr\0\xF0\x9D\x94\xB6\0yicy\0\xD1\x97\
\0yopf\0\xF0\x9D\x95\xAA\0yscr\0\xF0\x9D\x93\x8E\0yucy\0\xD1\x8E\0\1yu\
ml\0\xC3\xBF\0zacute\0\xC5\xBA\0zcaron\0\xC5\xBE\0zcy\0\xD0\xB7\
\0zdot\0\xC5\xBC\0zeetrf\0\xE2\x84\xA8\0zeta\0\xCE\xB6\0zfr\0\
\xF0\x9D\x94\xB7\0zhcy\0\xD0\xB6\0zigrarr\0\xE2\x87\x9D\0zopf\0\xF0\x9D\
\x95\xAB\0zscr\0\xF0\x9D\x93\x8F\0zwj\0\xE2\x80\x8D\0zwnj\0\xE2\x80\x8C\0..\
\0.\
";
/*[[[end]]]*/


/* Helpers for working XML Entity database entries. */
typedef char xml_entity_t;
#define xml_entity_semiopt(self) ((self)[-1] == 1)  /* non-zero iff the trailing ';' is optional */
#define xml_entity_name(self)    (self)             /* String that must appear in '&<str>;' */
#define xml_entity_utf8(self)    (strend(self) + 1) /* utf-8 sequence used for replacement. */


/* Find the start of the given entry. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) xml_entity_t const *
NOTHROW_NCX(CC xml_entity_fromptr)(char const *__restrict ptr) {
	char const *iter;
	iter = strend(ptr) + 1;
	unicode_readutf8(&iter);
	if (*iter == '\0') {
		/* `ptr' points into the first string! */
		if (*ptr == 0x01)
			return ptr + 1;
		while ((unsigned char)ptr[-1] >= 0x02)
			--ptr;
		return ptr;
	}
	/* `ptr' points into the second string! */
	while (ptr[-1] != '\0')
		--ptr;
	/* `ptr' now points at the start of the second string. */
	--ptr;
	/* `ptr' now points at the nul after the first string. */
	while ((unsigned char)ptr[-1] >= 0x02)
		--ptr;
	/* `ptr' now points at the start of the first string. */
	return ptr;
}


/* Lookup an XML entity from the database, given its name as it  would
 * appear within escaped text. If  not found, return NULL.  Otherwise,
 * return a pointer to the database entry which is associated with the
 * given name.
 * Upon success, you may use the `xml_entity_*' functions to
 * interact with the database entry. */
PRIVATE ATTR_PURE WUNUSED NONNULL((1)) xml_entity_t const *
NOTHROW_NCX(CC xml_entity_lookup)(char const *__restrict name, size_t namelen) {
	size_t lo, hi;
	lo = 1;
	hi = sizeof(xml_entity_db) - 5;
	while (lo < hi) {
		xml_entity_t const *entry;
		size_t i = (lo + hi) / 2;
		int cmp;
		entry = xml_entity_fromptr(xml_entity_db + i);
		cmp   = strcmpz(entry, name, namelen);
		if (cmp > 0) {
			hi = i;
		} else if (cmp < 0) {
			lo = i + 1;
		} else {
			/* Found it! */
			return entry;
		}
	}
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1)) xml_entity_t const *
NOTHROW_NCX(CC xml_entity_lookup_startswith)(char const *__restrict name, size_t namelen) {
	size_t lo, hi;
	lo = 1;
	hi = sizeof(xml_entity_db) - 5;
	while (lo < hi) {
		xml_entity_t const *entry;
		size_t i = (lo + hi) / 2;
		int cmp;
		entry = xml_entity_fromptr(xml_entity_db + i);
		cmp   = memcmp(entry, name, namelen * sizeof(char));
		if (cmp > 0) {
			hi = i;
		} else if (cmp < 0) {
			lo = i + 1;
		} else {
			/* Found it! */
			return entry;
		}
	}
	return NULL;
}

PRIVATE ATTR_NOINLINE ATTR_PURE WUNUSED NONNULL((1)) xml_entity_t const *
NOTHROW_NCX(CC xml_entity_lookup_startswith_plus1)(char const *__restrict name,
                                                   size_t namelen, char next_ch) {
	char *buf;
	buf = (char *)alloca((namelen + 1) * sizeof(char));
	*(char *)mempcpy(buf, name, namelen, sizeof(char)) = next_ch;
	return xml_entity_lookup_startswith(buf, namelen + 1);
}



//TODO:INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
//TODO:libiconv_xml_escape_encode(struct iconv_encode *__restrict self,
//TODO:                           /*utf-8*/ char const *__restrict data, size_t size) {
//TODO:}


INTERN NONNULL((1, 2)) ssize_t FORMATPRINTER_CC
libiconv_xml_escape_decode(struct iconv_decode *__restrict self,
                           /*utf-8*/ char const *__restrict data, size_t size) {
	ssize_t temp, result = 0;
	char const *ch_start;
	char const *end, *flush_start;
	flush_start = data;
	end         = data + size;
	if unlikely(self->icd_flags & ICONV_HASERR)
		goto err_ilseq;
	if (!mbstate_isempty(&self->icd_data.idd_xml.xe_utf8)) {
		ch_start = data;
		goto parse_unicode;
	}
	while (data < end) {
		char32_t ch;
		ch_start = data;
		ch       = (char32_t)(unsigned char)*data;
		if (ch >= 0x80) {
			size_t status;
parse_unicode:
			status = unicode_c8toc32(&ch, data, (size_t)(end - data),
			                         &self->icd_data.idd_xml.xe_utf8);
			if ((ssize_t)status < 0) {
				if (status == (size_t)-1) {
					if (IS_ICONV_ERR_ERROR_OR_ERRNO(self->icd_flags))
						goto err_ilseq;
					if (IS_ICONV_ERR_DISCARD(self->icd_flags)) {
						mbstate_init(&self->icd_data.idd_xml.xe_utf8);
						if (self->icd_data.idd_xml.xe_mode == _ICONV_DECODE_XML_TXT) {
							DO_decode_output(flush_start, (size_t)(end - flush_start));
							flush_start = data + 1;
						}
						++data;
						continue;
					}
					if (!IS_ICONV_ERR_IGNORE(self->icd_flags))
						ch = '?';
					status = 1;
				} else if (status == (size_t)-2) {
					goto done; /* Everything parsed! */
				}
			}
			data += status;
		} else {
			++data;
		}

		/* Process `ch' */
switch_on_state:
		switch (__builtin_expect(self->icd_data.idd_xml.xe_mode, _ICONV_DECODE_XML_TXT)) {

		case _ICONV_DECODE_XML_TXT:
			if unlikely(ch == 0) {
err_ilseq_ch_start:
				/* XML doesn't allow NUL characters. _ever_ */
				data = ch_start;
				goto err_ilseq;
			}
			if (ch == '&') {
				/* Enter escape mode. */
				DO_decode_output(flush_start, (size_t)(ch_start - flush_start));
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_AND;
			}
			break;

		case _ICONV_DECODE_XML_AND:
			if (ch == '#') {
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_PND;
			} else if (ch <= 0x7f) { /* Only ASCII allowed in entity names! */
				xml_entity_t const *ent;
				char firstchar[1];
				/* Technically, we'd need to check that `isalnum(ch)', but if that's
				 * not the case,  then `xml_entity_lookup_startswith()' will  return
				 * NULL so it would be a redundant check! */
				firstchar[0] = (char)(unsigned char)ch;
				ent = xml_entity_lookup_startswith(firstchar, 1);
				if unlikely(!ent)
					goto err_ilseq_ch_start;
				self->icd_data.idd_xml.xe_ent.e_str = ent;
				self->icd_data.idd_xml.xe_ent.e_len = 1;
				self->icd_data.idd_xml.xe_mode      = _ICONV_DECODE_XML_ENT;
			} else {
				/* Anything else isn't allowed after '&' */
				goto err_ilseq_ch_start;
			}
			break;

		case _ICONV_DECODE_XML_PND:
			if (ch == 'x') { /* Only lowercase 'x' is allowed here! */
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_HEX;
				self->icd_data.idd_xml.xe_chr  = 0;
			} else if (ch >= '0' && ch <= '9') {
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_DEC;
				self->icd_data.idd_xml.xe_chr  = ch - '0';
			} else {
				/* Anything else isn't allowed after '&#' */
				goto err_ilseq_ch_start;
			}
			break;

		case _ICONV_DECODE_XML_DEC:
			if (ch == ';') {
				char utf[UNICODE_UTF8_CURLEN];
				size_t len;
				/* Escape character sequence end. */
output_escape_character:
				if unlikely(self->icd_data.idd_xml.xe_chr == 0)
					goto err_ilseq_ch_start; /* Nope: you're not even allowed to encode NUL here! */
				len = (size_t)(unicode_writeutf8(utf, self->icd_data.idd_xml.xe_chr) - utf);
				DO_decode_output(utf, len);
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_TXT;
				flush_start                    = data;
				break;
			} else if (ch >= '0' && ch <= '9') {
				if (OVERFLOW_UMUL((uint32_t)self->icd_data.idd_xml.xe_chr, 10,
				                  (uint32_t *)&self->icd_data.idd_xml.xe_chr))
					goto err_ilseq_ch_start; /* Overflow */
				if (OVERFLOW_UADD((uint32_t)self->icd_data.idd_xml.xe_chr,
				                  (uint8_t)(ch - '0'),
				                  (uint32_t *)&self->icd_data.idd_xml.xe_chr))
					goto err_ilseq_ch_start; /* Overflow */
			} else {
				goto err_ilseq_ch_start;
			}
			break;

		case _ICONV_DECODE_XML_HEX: {
			uint8_t digit;
			if (ch == ';') {
				goto output_escape_character;
			} else if (ch >= '0' && ch <= '9') {
				digit = ch - '0';
			} else if (ch >= 'A' && ch <= 'F') {
				digit = 10 + ch - 'A';
			} else if (ch >= 'a' && ch <= 'f') {
				digit = 10 + ch - 'a';
			} else {
				goto err_ilseq_ch_start;
			}
			if (OVERFLOW_UMUL((uint32_t)self->icd_data.idd_xml.xe_chr, 16,
			                  (uint32_t *)&self->icd_data.idd_xml.xe_chr))
				goto err_ilseq_ch_start; /* Overflow */
			if (OVERFLOW_UADD((uint32_t)self->icd_data.idd_xml.xe_chr, digit,
			                  (uint32_t *)&self->icd_data.idd_xml.xe_chr))
				goto err_ilseq_ch_start; /* Overflow */
		}	break;

		case _ICONV_DECODE_XML_ENT: {
			if (ch == ';') {
				xml_entity_t const *ent;
				char const *utf;
				/* End of sequence. Check that our entry also ends. */
				ent = self->icd_data.idd_xml.xe_ent.e_str;
				if (xml_entity_name(ent)[self->icd_data.idd_xml.xe_ent.e_len] != 0) {
					/* If our entity doesn't end at the specified position, then there may
					 * be another one that does. To handle this case, search the  database
					 * once again but look for the exact string (instead of starts-with) */
					ent = xml_entity_lookup(xml_entity_name(ent), self->icd_data.idd_xml.xe_ent.e_len);
					if unlikely(!ent)
						goto err_ilseq_ch_start;
				}
				/* Load the utf replacement for this entry. */
				utf = xml_entity_utf8(ent);
				/* Output the utf replacement. */
				DO_decode_output(utf, strlen(utf));
				self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_TXT;
				flush_start                    = data;
				break;
			} else if (isalnum(ch) && ch <= 0x7f) {
				/* Another character apart of the entity name. */
				xml_entity_t const *ent;
				ent = self->icd_data.idd_xml.xe_ent.e_str;
				if (xml_entity_name(ent)[self->icd_data.idd_xml.xe_ent.e_len] != (char)(unsigned char)ch) {
					ent = xml_entity_lookup_startswith_plus1(xml_entity_name(ent),
					                                         self->icd_data.idd_xml.xe_ent.e_len,
					                                         (char)(unsigned char)ch);
					if unlikely(!ent)
						goto check_maybe_semi_opt;
					self->icd_data.idd_xml.xe_ent.e_str = ent;
				}
				++self->icd_data.idd_xml.xe_ent.e_len;
			} else {
				xml_entity_t const *ent;
				/* If the trailing ';' of  the current entity is  optional,
				 * and that entity was named fully, then all is still well! */
check_maybe_semi_opt:
				ent = self->icd_data.idd_xml.xe_ent.e_str;
				if (xml_entity_name(ent)[self->icd_data.idd_xml.xe_ent.e_len] == 0 &&
				    xml_entity_semiopt(ent)) {
					char const *utf = xml_entity_utf8(ent);
					/* All right: We _do_ have a completed sequence with optional (and missing) ';' */
					DO_decode_output(utf, strlen(utf));
					self->icd_data.idd_xml.xe_mode = _ICONV_DECODE_XML_TXT;
					flush_start                    = ch_start; /* Include the already loaded next character. */
					goto switch_on_state;
				}
				goto err_ilseq_ch_start; /* No sequence starts with this prefix. */
			}
		}	break;

		default:
			break;
		}
	}
done:
	if (self->icd_data.idd_xml.xe_mode == _ICONV_DECODE_XML_TXT)
		DO_decode_output(flush_start, (size_t)(end - flush_start));
	return result;
err:
	return temp;
err_ilseq:
	self->icd_flags |= ICONV_HASERR;
	if (IS_ICONV_ERR_ERRNO(self->icd_flags))
		errno = EILSEQ;
	return -(ssize_t)(size_t)(end - data);
}



DECL_END

#endif /* !GUARD_LIBICONV_CONVERT_XML_C */
