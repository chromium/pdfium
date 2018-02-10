// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_FX_FALLTHROUGH_H_
#define CORE_FXCRT_FX_FALLTHROUGH_H_

// When clang suggests inserting [[clang::fallthrough]], it first checks if
// it knows of a macro expanding to it, and if so suggests inserting the
// macro.  This means that this macro must be used only in code internal
// to PDFium, so that PDFium's user code doesn't end up getting suggestions
// for FX_FALLTHROUGH instead of the user-specific fallthrough macro.
// So do not include this header in any of PDFium's public headers.
#if defined(__clang__)
#define FX_FALLTHROUGH [[clang::fallthrough]]
#else
#define FX_FALLTHROUGH
#endif

#endif  // CORE_FXCRT_FX_FALLTHROUGH_H_
