// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_H_
#define XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fee/ifde_txtedtengine.h"

class CFX_CharIter : public IFX_CharIter {
 public:
  CFX_CharIter(const CFX_WideString& wsText);
  virtual void Release();
  virtual FX_BOOL Next(FX_BOOL bPrev = FALSE);
  virtual FX_WCHAR GetChar();
  virtual void SetAt(int32_t nIndex);
  virtual int32_t GetAt() const;
  virtual FX_BOOL IsEOF(FX_BOOL bTail = TRUE) const;
  virtual IFX_CharIter* Clone();

 protected:
  ~CFX_CharIter();

 private:
  const CFX_WideString& m_wsText;
  int32_t m_nIndex;
};
class CFX_WordBreak {
 public:
  CFX_WordBreak();

  void Release();
  void Attach(IFX_CharIter* pIter);
  void Attach(const CFX_WideString& wsText);
  FX_BOOL Next(FX_BOOL bPrev);
  void SetAt(int32_t nIndex);
  int32_t GetWordPos() const;
  int32_t GetWordLength() const;
  void GetWord(CFX_WideString& wsWord) const;
  FX_BOOL IsEOF(FX_BOOL bTail) const;

 protected:
  ~CFX_WordBreak();
  FX_BOOL FindNextBreakPos(IFX_CharIter* pIter,
                           FX_BOOL bPrev,
                           FX_BOOL bFromNext = TRUE);

 private:
  IFX_CharIter* m_pPreIter;
  IFX_CharIter* m_pCurIter;
};

#endif  // XFA_FEE_FX_WORDBREAK_FX_WORDBREAK_H_
