// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_IFX_CHARITER_H_
#define CORE_FXCRT_IFX_CHARITER_H_

#include <memory>

#include "core/fxcrt/fx_system.h"

class IFX_CharIter {
 public:
  virtual ~IFX_CharIter() {}

  virtual bool Next(bool bPrev = false) = 0;
  virtual wchar_t GetChar() = 0;
  virtual void SetAt(int32_t nIndex) = 0;
  virtual int32_t GetAt() const = 0;
  virtual bool IsEOF(bool bTail = true) const = 0;
  virtual std::unique_ptr<IFX_CharIter> Clone() = 0;
};

#endif  // CORE_FXCRT_IFX_CHARITER_H_
