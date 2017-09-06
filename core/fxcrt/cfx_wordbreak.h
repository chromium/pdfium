// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WORDBREAK_H_
#define CORE_FXCRT_CFX_WORDBREAK_H_

#include <memory>
#include <utility>

class IFX_CharIter;

class CFX_WordBreak {
 public:
  explicit CFX_WordBreak(std::unique_ptr<IFX_CharIter> pIter);
  ~CFX_WordBreak();

  // <start_idx, end_idx>
  std::pair<size_t, size_t> BoundsAt(int32_t nIndex);

 private:
  bool FindNextBreakPos(IFX_CharIter* pIter, bool bPrev);

  std::unique_ptr<IFX_CharIter> m_pPreIter;
  std::unique_ptr<IFX_CharIter> m_pCurIter;
};

#endif  // CORE_FXCRT_CFX_WORDBREAK_H_
