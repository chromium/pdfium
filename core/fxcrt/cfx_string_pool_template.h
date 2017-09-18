// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_STRING_POOL_TEMPLATE_H_
#define CORE_FXCRT_CFX_STRING_POOL_TEMPLATE_H_

#include <unordered_set>

#include "core/fxcrt/fx_string.h"

template <typename StringType>
class CFX_StringPoolTemplate {
 public:
  StringType Intern(const StringType& str) { return *m_Pool.insert(str).first; }
  void Clear() { m_Pool.clear(); }

 private:
  std::unordered_set<StringType> m_Pool;
};

using ByteStringPool = CFX_StringPoolTemplate<ByteString>;
using WideStringPool = CFX_StringPoolTemplate<WideString>;

extern template class CFX_StringPoolTemplate<ByteString>;
extern template class CFX_StringPoolTemplate<WideString>;

#endif  // CORE_FXCRT_CFX_STRING_POOL_TEMPLATE_H_
