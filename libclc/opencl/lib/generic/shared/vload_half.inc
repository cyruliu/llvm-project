//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#if __CLC_FPSIZE == 32

#ifndef __CLC_SCALAR

#if __CLC_VECSIZE == 3
#define __CLC_OFFSET 4
#else
#define __CLC_OFFSET __CLC_VECSIZE
#endif

FUNC(__CLC_VECSIZE, __CLC_VECSIZE, __CLC_OFFSET, __CLC_GENTYPE, __private);
FUNC(__CLC_VECSIZE, __CLC_VECSIZE, __CLC_OFFSET, __CLC_GENTYPE, __local);
FUNC(__CLC_VECSIZE, __CLC_VECSIZE, __CLC_OFFSET, __CLC_GENTYPE, __global);
FUNC(__CLC_VECSIZE, __CLC_VECSIZE, __CLC_OFFSET, __CLC_GENTYPE, __constant);
#if _CLC_DISTINCT_GENERIC_AS_SUPPORTED
FUNC(__CLC_VECSIZE, __CLC_VECSIZE, __CLC_OFFSET, __CLC_GENTYPE, __generic);
#endif

#undef __CLC_OFFSET
#else
FUNC(, 1, 1, __CLC_GENTYPE, __private);
FUNC(, 1, 1, __CLC_GENTYPE, __local);
FUNC(, 1, 1, __CLC_GENTYPE, __global);
FUNC(, 1, 1, __CLC_GENTYPE, __constant);
#if _CLC_DISTINCT_GENERIC_AS_SUPPORTED
FUNC(, 1, 1, __CLC_GENTYPE, __generic);
#endif
#endif
#endif
