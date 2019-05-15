// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_WIN_WIN_UTIL_H_
#define THIRD_PARTY_BASE_WIN_WIN_UTIL_H_

namespace pdfium {
namespace base {
namespace win {

// Returns true if the current process can make USER32 or GDI32 calls such as
// CreateWindow and CreateDC. Windows 8 and above allow the kernel component
// of these calls to be disabled which can cause undefined behaviour such as
// crashes. This function can be used to guard areas of code using these calls
// and provide a fallback path if necessary.
bool IsUser32AndGdi32Available();

}  // namespace win
}  // namespace base
}  // namespace pdfium

#endif  // THIRD_PARTY_BASE_WIN_WIN_UTIL_H_
