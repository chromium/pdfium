// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_WORDBREAK_H
#define _FX_WORDBREAK_H
class IFX_CharIter;

class IFX_WordBreak {
 public:
  virtual ~IFX_WordBreak() {}
  virtual void Release() = 0;
  virtual void Attach(IFX_CharIter* pIter) = 0;
  virtual void Attach(const CFX_WideString& wsText) = 0;
  virtual FX_BOOL Next(FX_BOOL bPrev) = 0;
  virtual void SetAt(int32_t nIndex) = 0;
  virtual int32_t GetWordPos() const = 0;
  virtual int32_t GetWordLength() const = 0;
  virtual void GetWord(CFX_WideString& wsWord) const = 0;
  virtual FX_BOOL IsEOF(FX_BOOL bTail = TRUE) const = 0;
};
IFX_WordBreak* FX_WordBreak_Create();
#endif
