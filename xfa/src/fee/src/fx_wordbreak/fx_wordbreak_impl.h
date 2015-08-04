// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_WORDBREAK_IMPL_H
#define _FX_WORDBREAK_IMPL_H
extern const FX_WORD gs_FX_WordBreak_Table[16];
extern const uint8_t gs_FX_WordBreak_CodePointProperties[(0xFFFF - 1) / 2 + 1];
enum FX_WordBreakProp {
  FX_WordBreakProp_None = 0,
  FX_WordBreakProp_CR,
  FX_WordBreakProp_LF,
  FX_WordBreakProp_NewLine,
  FX_WordBreakProp_Extend,
  FX_WordBreakProp_Format,
  FX_WordBreakProp_KataKana,
  FX_WordBreakProp_ALetter,
  FX_WordBreakProp_MidLetter,
  FX_WordBreakProp_MidNum,
  FX_WordBreakProp_MidNumLet,
  FX_WordBreakProp_Numberic,
  FX_WordBreakProp_ExtendNumLet,
};
FX_WordBreakProp FX_GetWordBreakProperty(FX_WCHAR wcCodePoint);
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
class CFX_WordBreak : public IFX_WordBreak {
 public:
  CFX_WordBreak();
  virtual void Release();
  virtual void Attach(IFX_CharIter* pIter);
  virtual void Attach(const CFX_WideString& wsText);
  virtual FX_BOOL Next(FX_BOOL bPrev);
  virtual void SetAt(int32_t nIndex);
  virtual int32_t GetWordPos() const;
  virtual int32_t GetWordLength() const;
  virtual void GetWord(CFX_WideString& wsWord) const;
  virtual FX_BOOL IsEOF(FX_BOOL bTail) const;

 protected:
  ~CFX_WordBreak();
  FX_BOOL FindNextBreakPos(IFX_CharIter* pIter,
                           FX_BOOL bPrev,
                           FX_BOOL bFromNext = TRUE);

 private:
  IFX_CharIter* m_pPreIter;
  IFX_CharIter* m_pCurIter;
};
#endif
