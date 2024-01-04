/*[[[magic
options["COMPILE.language"] = "c";
local gcc_opt = options.setdefault("GCC.options", []);
gcc_opt.remove("-g"); // Disable debug informations for this file!
]]]*/
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
#ifndef GUARD_LIBICONV_MBCS_CPDB_C
#define GUARD_LIBICONV_MBCS_CPDB_C 1

#ifndef __INTELLISENSE__
#include "../api.h"
/**/

#include <hybrid/byteorder.h>

#include <stddef.h>

#include "../codecs.h"
#include "cp-mbcs.h"

#if CODEC_MBCS_COUNT != 0
DECL_BEGIN

/*[[[deemon
import File from deemon;
import fs;
// Put in a separate file so this one doesn't bloat too much
local dataFilename = "cpdb.dat";
local savedStdout = File.stdout;
File.stdout = File.open(dataFilename, "w");
File.stdout.write(File.open("../api.h").read().decode("utf-8").partition("#ifndef")[0].unifylines());
(printCpMbcsDatabase from ..iconvdata.iconvdata)();
File.stdout.close();
File.stdout = savedStdout;
print "#include", repr dataFilename;
]]]*/
#include "cpdb.dat"
/*[[[end]]]*/

DECL_END
#endif /* CODEC_MBCS_COUNT != 0 */
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_LIBICONV_MBCS_CPDB_C */
