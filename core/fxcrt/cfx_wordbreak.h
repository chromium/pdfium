// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WORDBREAK_H_
#define CORE_FXCRT_CFX_WORDBREAK_H_

#include <memory>

class IFX_CharIter;

class CFX_WordBreak {
 public:
  CFX_WordBreak();
  ~CFX_WordBreak();

  void Attach(IFX_CharIter* pIter);
  void SetAt(int32_t nIndex);
  int32_t GetWordPos() const;
  int32_t GetWordLength() const;

 private:
  bool FindNextBreakPos(IFX_CharIter* pIter, bool bPrev);
  bool IsEOF(bool bTail) const;

  std::unique_ptr<IFX_CharIter> m_pPreIter;
  std::unique_ptr<IFX_CharIter> m_pCurIter;
};

#endif  // CORE_FXCRT_CFX_WORDBREAK_H_
