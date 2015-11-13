// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFDOC_FPDF_VT_H_
#define CORE_INCLUDE_FPDFDOC_FPDF_VT_H_

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fxge/fx_dib.h"

class IPDF_VariableText;
class IPDF_VariableText_Iterator;
class IPDF_VariableText_Provider;
struct CPVT_Line;
struct CPVT_Section;
struct CPVT_Word;
struct CPVT_WordPlace;
struct CPVT_WordRange;

struct CPVT_WordPlace {
  CPVT_WordPlace() : nSecIndex(-1), nLineIndex(-1), nWordIndex(-1) {}

  CPVT_WordPlace(int32_t other_nSecIndex,
                 int32_t other_nLineIndex,
                 int32_t other_nWordIndex) {
    nSecIndex = other_nSecIndex;
    nLineIndex = other_nLineIndex;
    nWordIndex = other_nWordIndex;
  }

  void Default() { nSecIndex = nLineIndex = nWordIndex = -1; }

  FX_BOOL operator==(const CPVT_WordPlace& wp) const {
    return wp.nSecIndex == nSecIndex && wp.nLineIndex == nLineIndex &&
           wp.nWordIndex == nWordIndex;
  }

  FX_BOOL operator!=(const CPVT_WordPlace& wp) const {
    return wp.nSecIndex != nSecIndex || wp.nLineIndex != nLineIndex ||
           wp.nWordIndex != nWordIndex;
  }

  inline int32_t WordCmp(const CPVT_WordPlace& wp) const {
    if (nSecIndex > wp.nSecIndex) {
      return 1;
    }
    if (nSecIndex < wp.nSecIndex) {
      return -1;
    }
    if (nLineIndex > wp.nLineIndex) {
      return 1;
    }
    if (nLineIndex < wp.nLineIndex) {
      return -1;
    }
    if (nWordIndex > wp.nWordIndex) {
      return 1;
    }
    if (nWordIndex < wp.nWordIndex) {
      return -1;
    }
    return 0;
  }

  inline int32_t LineCmp(const CPVT_WordPlace& wp) const {
    if (nSecIndex > wp.nSecIndex) {
      return 1;
    }
    if (nSecIndex < wp.nSecIndex) {
      return -1;
    }
    if (nLineIndex > wp.nLineIndex) {
      return 1;
    }
    if (nLineIndex < wp.nLineIndex) {
      return -1;
    }
    return 0;
  }

  inline int32_t SecCmp(const CPVT_WordPlace& wp) const {
    if (nSecIndex > wp.nSecIndex) {
      return 1;
    }
    if (nSecIndex < wp.nSecIndex) {
      return -1;
    }
    return 0;
  }

  int32_t nSecIndex;

  int32_t nLineIndex;

  int32_t nWordIndex;
};
struct CPVT_WordRange {
  CPVT_WordRange() {}

  CPVT_WordRange(const CPVT_WordPlace& begin, const CPVT_WordPlace& end) {
    Set(begin, end);
  }

  void Default() {
    BeginPos.Default();
    EndPos.Default();
  }

  void Set(const CPVT_WordPlace& begin, const CPVT_WordPlace& end) {
    BeginPos = begin;
    EndPos = end;
    SwapWordPlace();
  }

  void SetBeginPos(const CPVT_WordPlace& begin) {
    BeginPos = begin;
    SwapWordPlace();
  }

  void SetEndPos(const CPVT_WordPlace& end) {
    EndPos = end;
    SwapWordPlace();
  }

  FX_BOOL IsExist() const { return BeginPos != EndPos; }

  FX_BOOL operator!=(const CPVT_WordRange& wr) const {
    return wr.BeginPos != BeginPos || wr.EndPos != EndPos;
  }

  void SwapWordPlace() {
    if (BeginPos.WordCmp(EndPos) > 0) {
      CPVT_WordPlace place = EndPos;
      EndPos = BeginPos;
      BeginPos = place;
    }
  }

  CPVT_WordPlace BeginPos;

  CPVT_WordPlace EndPos;
};
struct CPVT_SecProps {
  CPVT_SecProps() : fLineLeading(0.0f), fLineIndent(0.0f), nAlignment(0) {}

  CPVT_SecProps(FX_FLOAT lineLeading, FX_FLOAT lineIndent, int32_t alignment)
      : fLineLeading(lineLeading),
        fLineIndent(lineIndent),
        nAlignment(alignment) {}

  CPVT_SecProps(const CPVT_SecProps& other)
      : fLineLeading(other.fLineLeading),
        fLineIndent(other.fLineIndent),
        nAlignment(other.nAlignment) {}

  FX_FLOAT fLineLeading;

  FX_FLOAT fLineIndent;

  int32_t nAlignment;
};
struct CPVT_WordProps {
  CPVT_WordProps()
      : nFontIndex(-1),
        fFontSize(0.0f),
        dwWordColor(0),
        nScriptType(0),
        nWordStyle(0),
        fCharSpace(0.0f),
        nHorzScale(0) {}

  CPVT_WordProps(int32_t fontIndex,
                 FX_FLOAT fontSize,
                 FX_COLORREF wordColor = 0,
                 int32_t scriptType = 0,
                 int32_t wordStyle = 0,
                 FX_FLOAT charSpace = 0,
                 int32_t horzScale = 100)
      : nFontIndex(fontIndex),
        fFontSize(fontSize),
        dwWordColor(wordColor),
        nScriptType(scriptType),
        nWordStyle(wordStyle),
        fCharSpace(charSpace),
        nHorzScale(horzScale) {}

  CPVT_WordProps(const CPVT_WordProps& other)
      : nFontIndex(other.nFontIndex),
        fFontSize(other.fFontSize),
        dwWordColor(other.dwWordColor),
        nScriptType(other.nScriptType),
        nWordStyle(other.nWordStyle),
        fCharSpace(other.fCharSpace),
        nHorzScale(other.nHorzScale) {}

  int32_t nFontIndex;

  FX_FLOAT fFontSize;

  FX_COLORREF dwWordColor;

  int32_t nScriptType;

  int32_t nWordStyle;

  FX_FLOAT fCharSpace;

  int32_t nHorzScale;
};
struct CPVT_Word {
  CPVT_Word()
      : Word(0),
        nCharset(0),
        ptWord(0, 0),
        fAscent(0.0f),
        fDescent(0.0f),
        fWidth(0.0f),
        fFontSize(0),
        WordProps() {}

  FX_WORD Word;

  int32_t nCharset;

  CPVT_WordPlace WordPlace;

  CPDF_Point ptWord;

  FX_FLOAT fAscent;

  FX_FLOAT fDescent;

  FX_FLOAT fWidth;

  int32_t nFontIndex;

  FX_FLOAT fFontSize;

  CPVT_WordProps WordProps;
};
struct CPVT_Line {
  CPVT_Line()
      : ptLine(0, 0), fLineWidth(0.0f), fLineAscent(0.0f), fLineDescent(0.0f) {}

  CPVT_WordPlace lineplace;

  CPVT_WordPlace lineEnd;

  CPDF_Point ptLine;

  FX_FLOAT fLineWidth;

  FX_FLOAT fLineAscent;

  FX_FLOAT fLineDescent;
};
struct CPVT_Section {
  CPVT_WordPlace secplace;

  CPDF_Rect rcSection;

  CPVT_SecProps SecProps;

  CPVT_WordProps WordProps;
};
class IPDF_VariableText_Provider {
 public:
  virtual ~IPDF_VariableText_Provider() {}

  virtual int32_t GetCharWidth(int32_t nFontIndex,
                               FX_WORD word,
                               int32_t nWordStyle) = 0;

  virtual int32_t GetTypeAscent(int32_t nFontIndex) = 0;

  virtual int32_t GetTypeDescent(int32_t nFontIndex) = 0;

  virtual int32_t GetWordFontIndex(FX_WORD word,
                                   int32_t charset,
                                   int32_t nFontIndex) = 0;

  virtual FX_BOOL IsLatinWord(FX_WORD word) = 0;

  virtual int32_t GetDefaultFontIndex() = 0;
};
class IPDF_VariableText_Iterator {
 public:
  virtual ~IPDF_VariableText_Iterator() {}

  virtual FX_BOOL NextWord() = 0;

  virtual FX_BOOL PrevWord() = 0;

  virtual FX_BOOL NextLine() = 0;

  virtual FX_BOOL PrevLine() = 0;

  virtual FX_BOOL NextSection() = 0;

  virtual FX_BOOL PrevSection() = 0;

  virtual FX_BOOL GetWord(CPVT_Word& word) const = 0;

  virtual FX_BOOL SetWord(const CPVT_Word& word) = 0;

  virtual FX_BOOL GetLine(CPVT_Line& line) const = 0;

  virtual FX_BOOL GetSection(CPVT_Section& section) const = 0;

  virtual FX_BOOL SetSection(const CPVT_Section& section) = 0;

  virtual void SetAt(int32_t nWordIndex) = 0;

  virtual void SetAt(const CPVT_WordPlace& place) = 0;

  virtual const CPVT_WordPlace& GetAt() const = 0;
};
class IPDF_VariableText {
 public:
  static IPDF_VariableText* NewVariableText();

  static void DelVariableText(IPDF_VariableText* pVT);

  virtual IPDF_VariableText_Provider* SetProvider(
      IPDF_VariableText_Provider* pProvider) = 0;

  virtual IPDF_VariableText_Iterator* GetIterator() = 0;

  virtual void SetPlateRect(const CPDF_Rect& rect) = 0;

  virtual void SetAlignment(int32_t nFormat = 0) = 0;

  virtual void SetPasswordChar(FX_WORD wSubWord = '*') = 0;

  virtual void SetLimitChar(int32_t nLimitChar = 0) = 0;

  virtual void SetCharArray(int32_t nCharArray = 0) = 0;

  virtual void SetCharSpace(FX_FLOAT fCharSpace = 0.0f) = 0;

  virtual void SetHorzScale(int32_t nHorzScale = 100) = 0;

  virtual void SetMultiLine(FX_BOOL bMultiLine = TRUE) = 0;

  virtual void SetAutoReturn(FX_BOOL bAuto = TRUE) = 0;

  virtual void SetAutoFontSize(FX_BOOL bAuto = TRUE) = 0;

  virtual void SetFontSize(FX_FLOAT fFontSize) = 0;

  virtual void SetLineLeading(FX_FLOAT fLineLeading) = 0;

  virtual void SetRichText(FX_BOOL bRichText) = 0;

  virtual void Initialize() = 0;

  virtual FX_BOOL IsValid() const = 0;

  virtual FX_BOOL IsRichText() const = 0;

  virtual void RearrangeAll() = 0;

  virtual void RearrangePart(const CPVT_WordRange& PlaceRange) = 0;

  virtual void ResetAll() = 0;

  virtual void SetText(const FX_WCHAR* text,
                       int32_t charset = 1,
                       const CPVT_SecProps* pSecProps = NULL,
                       const CPVT_WordProps* pWordProps = NULL) = 0;

  virtual CPVT_WordPlace InsertWord(
      const CPVT_WordPlace& place,
      FX_WORD word,
      int32_t charset = 1,
      const CPVT_WordProps* pWordProps = NULL) = 0;

  virtual CPVT_WordPlace InsertSection(
      const CPVT_WordPlace& place,
      const CPVT_SecProps* pSecProps = NULL,
      const CPVT_WordProps* pWordProps = NULL) = 0;

  virtual CPVT_WordPlace InsertText(
      const CPVT_WordPlace& place,
      const FX_WCHAR* text,
      int32_t charset = 1,
      const CPVT_SecProps* pSecProps = NULL,
      const CPVT_WordProps* pWordProps = NULL) = 0;

  virtual CPVT_WordPlace DeleteWords(const CPVT_WordRange& PlaceRange) = 0;

  virtual CPVT_WordPlace DeleteWord(const CPVT_WordPlace& place) = 0;

  virtual CPVT_WordPlace BackSpaceWord(const CPVT_WordPlace& place) = 0;

  virtual const CPDF_Rect& GetPlateRect() const = 0;

  virtual CPDF_Rect GetContentRect() const = 0;

  virtual int32_t GetTotalWords() const = 0;

  virtual FX_FLOAT GetFontSize() const = 0;

  virtual int32_t GetAlignment() const = 0;

  virtual FX_WORD GetPasswordChar() const = 0;

  virtual int32_t GetCharArray() const = 0;

  virtual int32_t GetLimitChar() const = 0;

  virtual FX_BOOL IsMultiLine() const = 0;

  virtual int32_t GetHorzScale() const = 0;

  virtual FX_FLOAT GetCharSpace() const = 0;

  virtual CPVT_WordPlace GetBeginWordPlace() const = 0;

  virtual CPVT_WordPlace GetEndWordPlace() const = 0;

  virtual CPVT_WordPlace GetPrevWordPlace(
      const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace GetNextWordPlace(
      const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace SearchWordPlace(const CPDF_Point& point) const = 0;

  virtual CPVT_WordPlace GetUpWordPlace(const CPVT_WordPlace& place,
                                        const CPDF_Point& point) const = 0;

  virtual CPVT_WordPlace GetDownWordPlace(const CPVT_WordPlace& place,
                                          const CPDF_Point& point) const = 0;

  virtual CPVT_WordPlace GetLineBeginPlace(
      const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace GetLineEndPlace(const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace GetSectionBeginPlace(
      const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace GetSectionEndPlace(
      const CPVT_WordPlace& place) const = 0;

  virtual void UpdateWordPlace(CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace AdjustLineHeader(const CPVT_WordPlace& place,
                                          FX_BOOL bPrevOrNext) const = 0;

  virtual int32_t WordPlaceToWordIndex(const CPVT_WordPlace& place) const = 0;

  virtual CPVT_WordPlace WordIndexToWordPlace(int32_t index) const = 0;

 protected:
  ~IPDF_VariableText() {}
};

#endif  // CORE_INCLUDE_FPDFDOC_FPDF_VT_H_
