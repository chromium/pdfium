// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFDOC_PDF_VT_H_
#define CORE_SRC_FPDFDOC_PDF_VT_H_

class CPVT_Size;
class CPVT_FloatRect;
struct CPVT_SectionInfo;
struct CPVT_LineInfo;
struct CPVT_WordInfo;
class CLine;
class CLines;
class CSection;
class CTypeset;
class CPDF_EditContainer;
class CPDF_VariableText;
class CPDF_VariableText_Iterator;
#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))

class CPVT_Size {
 public:
  CPVT_Size() : x(0.0f), y(0.0f) {}
  CPVT_Size(FX_FLOAT other_x, FX_FLOAT other_y) {
    x = other_x;
    y = other_y;
  }
  FX_FLOAT x, y;
};
class CPVT_FloatRect : public CFX_FloatRect {
 public:
  CPVT_FloatRect() { left = top = right = bottom = 0.0f; }
  CPVT_FloatRect(FX_FLOAT other_left,
                 FX_FLOAT other_top,
                 FX_FLOAT other_right,
                 FX_FLOAT other_bottom) {
    left = other_left;
    top = other_top;
    right = other_right;
    bottom = other_bottom;
  }
  explicit CPVT_FloatRect(const CPDF_Rect& rect) {
    left = rect.left;
    top = rect.top;
    right = rect.right;
    bottom = rect.bottom;
  }
  void Default() { left = top = right = bottom = 0.0f; }
  FX_FLOAT Height() const {
    if (top > bottom)
      return top - bottom;
    return bottom - top;
  }
};
struct CPVT_SectionInfo {
  CPVT_SectionInfo()
      : rcSection(), nTotalLine(0), pSecProps(NULL), pWordProps(NULL) {}
  virtual ~CPVT_SectionInfo() {
    delete pSecProps;
    delete pWordProps;
  }
  CPVT_SectionInfo(const CPVT_SectionInfo& other)
      : rcSection(), nTotalLine(0), pSecProps(NULL), pWordProps(NULL) {
    operator=(other);
  }
  void operator=(const CPVT_SectionInfo& other) {
    if (this == &other) {
      return;
    }
    rcSection = other.rcSection;
    nTotalLine = other.nTotalLine;
    if (other.pSecProps) {
      if (pSecProps) {
        *pSecProps = *other.pSecProps;
      } else {
        pSecProps = new CPVT_SecProps(*other.pSecProps);
      }
    }
    if (other.pWordProps) {
      if (pWordProps) {
        *pWordProps = *other.pWordProps;
      } else {
        pWordProps = new CPVT_WordProps(*other.pWordProps);
      }
    }
  }
  CPVT_FloatRect rcSection;
  int32_t nTotalLine;
  CPVT_SecProps* pSecProps;
  CPVT_WordProps* pWordProps;
};
struct CPVT_LineInfo {
  CPVT_LineInfo()
      : nTotalWord(0),
        nBeginWordIndex(-1),
        nEndWordIndex(-1),
        fLineX(0.0f),
        fLineY(0.0f),
        fLineWidth(0.0f),
        fLineAscent(0.0f),
        fLineDescent(0.0f) {}
  int32_t nTotalWord;
  int32_t nBeginWordIndex;
  int32_t nEndWordIndex;
  FX_FLOAT fLineX;
  FX_FLOAT fLineY;
  FX_FLOAT fLineWidth;
  FX_FLOAT fLineAscent;
  FX_FLOAT fLineDescent;
};
struct CPVT_WordInfo {
  CPVT_WordInfo()
      : Word(0),
        nCharset(0),
        fWordX(0.0f),
        fWordY(0.0f),
        fWordTail(0.0f),
        nFontIndex(-1),
        pWordProps(NULL) {}
  CPVT_WordInfo(FX_WORD word,
                int32_t charset,
                int32_t fontIndex,
                CPVT_WordProps* pProps)
      : Word(word),
        nCharset(charset),
        fWordX(0.0f),
        fWordY(0.0f),
        fWordTail(0.0f),
        nFontIndex(fontIndex),
        pWordProps(pProps) {}
  virtual ~CPVT_WordInfo() { delete pWordProps; }
  CPVT_WordInfo(const CPVT_WordInfo& word)
      : Word(0),
        nCharset(0),
        fWordX(0.0f),
        fWordY(0.0f),
        fWordTail(0.0f),
        nFontIndex(-1),
        pWordProps(NULL) {
    operator=(word);
  }
  void operator=(const CPVT_WordInfo& word) {
    if (this == &word) {
      return;
    }
    Word = word.Word;
    nCharset = word.nCharset;
    nFontIndex = word.nFontIndex;
    if (word.pWordProps) {
      if (pWordProps) {
        *pWordProps = *word.pWordProps;
      } else {
        pWordProps = new CPVT_WordProps(*word.pWordProps);
      }
    }
  }
  FX_WORD Word;
  int32_t nCharset;
  FX_FLOAT fWordX;
  FX_FLOAT fWordY;
  FX_FLOAT fWordTail;
  int32_t nFontIndex;
  CPVT_WordProps* pWordProps;
};
struct CPVT_FloatRange {
  CPVT_FloatRange() : fMin(0.0f), fMax(0.0f) {}
  CPVT_FloatRange(FX_FLOAT min, FX_FLOAT max) : fMin(min), fMax(max) {}
  FX_FLOAT Range() const { return fMax - fMin; }
  FX_FLOAT fMin, fMax;
};
template <class TYPE>
class CPVT_ArrayTemplate : public CFX_ArrayTemplate<TYPE> {
 public:
  FX_BOOL IsEmpty() { return CFX_ArrayTemplate<TYPE>::GetSize() <= 0; }
  TYPE GetAt(int nIndex) const {
    if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) {
      return CFX_ArrayTemplate<TYPE>::GetAt(nIndex);
    }
    return NULL;
  }
  void RemoveAt(int nIndex) {
    if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) {
      CFX_ArrayTemplate<TYPE>::RemoveAt(nIndex);
    }
  }
};
class CLine {
 public:
  CLine();
  virtual ~CLine();
  CPVT_WordPlace GetBeginWordPlace() const;
  CPVT_WordPlace GetEndWordPlace() const;
  CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace LinePlace;
  CPVT_LineInfo m_LineInfo;
};
class CLines {
 public:
  CLines() : m_nTotal(0) {}
  virtual ~CLines() { RemoveAll(); }
  int32_t GetSize() const { return m_Lines.GetSize(); }
  CLine* GetAt(int32_t nIndex) const { return m_Lines.GetAt(nIndex); }
  void Empty() { m_nTotal = 0; }
  void RemoveAll() {
    for (int32_t i = 0, sz = GetSize(); i < sz; i++) {
      delete GetAt(i);
    }
    m_Lines.RemoveAll();
    m_nTotal = 0;
  }
  int32_t Add(const CPVT_LineInfo& lineinfo) {
    if (m_nTotal >= GetSize()) {
      CLine* pLine = new CLine;
      pLine->m_LineInfo = lineinfo;
      m_Lines.Add(pLine);
    } else if (CLine* pLine = GetAt(m_nTotal)) {
      pLine->m_LineInfo = lineinfo;
    }
    return m_nTotal++;
  }
  void Clear() {
    for (int32_t i = GetSize() - 1; i >= m_nTotal; i--) {
      delete GetAt(i);
      m_Lines.RemoveAt(i);
    }
  }

 private:
  CPVT_ArrayTemplate<CLine*> m_Lines;
  int32_t m_nTotal;
};
class CSection {
  friend class CTypeset;

 public:
  explicit CSection(CPDF_VariableText* pVT);
  virtual ~CSection();
  void ResetAll();
  void ResetLineArray();
  void ResetWordArray();
  void ResetLinePlace();
  CPVT_WordPlace AddWord(const CPVT_WordPlace& place,
                         const CPVT_WordInfo& wordinfo);
  CPVT_WordPlace AddLine(const CPVT_LineInfo& lineinfo);
  void ClearWords(const CPVT_WordRange& PlaceRange);
  void ClearWord(const CPVT_WordPlace& place);
  CPVT_FloatRect Rearrange();
  CPVT_Size GetSectionSize(FX_FLOAT fFontSize);
  CPVT_WordPlace GetBeginWordPlace() const;
  CPVT_WordPlace GetEndWordPlace() const;
  CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const;
  CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const;
  void UpdateWordPlace(CPVT_WordPlace& place) const;
  CPVT_WordPlace SearchWordPlace(const CPDF_Point& point) const;
  CPVT_WordPlace SearchWordPlace(FX_FLOAT fx,
                                 const CPVT_WordPlace& lineplace) const;
  CPVT_WordPlace SearchWordPlace(FX_FLOAT fx,
                                 const CPVT_WordRange& range) const;

 public:
  CPVT_WordPlace SecPlace;
  CPVT_SectionInfo m_SecInfo;
  CLines m_LineArray;
  CPVT_ArrayTemplate<CPVT_WordInfo*> m_WordArray;

 private:
  void ClearLeftWords(int32_t nWordIndex);
  void ClearRightWords(int32_t nWordIndex);
  void ClearMidWords(int32_t nBeginIndex, int32_t nEndIndex);

  CPDF_VariableText* m_pVT;
};
class CTypeset {
 public:
  explicit CTypeset(CSection* pSection);
  virtual ~CTypeset();
  CPVT_Size GetEditSize(FX_FLOAT fFontSize);
  CPVT_FloatRect Typeset();
  CPVT_FloatRect CharArray();

 private:
  void SplitLines(FX_BOOL bTypeset, FX_FLOAT fFontSize);
  void OutputLines();

  CPVT_FloatRect m_rcRet;
  CPDF_VariableText* m_pVT;
  CSection* m_pSection;
};
class CPDF_EditContainer {
 public:
  CPDF_EditContainer() : m_rcPlate(0, 0, 0, 0), m_rcContent(0, 0, 0, 0) {}
  virtual ~CPDF_EditContainer() {}
  virtual void SetPlateRect(const CPDF_Rect& rect) { m_rcPlate = rect; }
  virtual const CPDF_Rect& GetPlateRect() const { return m_rcPlate; }
  virtual void SetContentRect(const CPVT_FloatRect& rect) {
    m_rcContent = rect;
  }
  virtual CPDF_Rect GetContentRect() const { return m_rcContent; }
  FX_FLOAT GetPlateWidth() const { return m_rcPlate.right - m_rcPlate.left; }
  FX_FLOAT GetPlateHeight() const { return m_rcPlate.top - m_rcPlate.bottom; }
  CPVT_Size GetPlateSize() const {
    return CPVT_Size(GetPlateWidth(), GetPlateHeight());
  }
  CPDF_Point GetBTPoint() const {
    return CPDF_Point(m_rcPlate.left, m_rcPlate.top);
  }
  CPDF_Point GetETPoint() const {
    return CPDF_Point(m_rcPlate.right, m_rcPlate.bottom);
  }
  inline CPDF_Point InToOut(const CPDF_Point& point) const {
    return CPDF_Point(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
  }
  inline CPDF_Point OutToIn(const CPDF_Point& point) const {
    return CPDF_Point(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
  }
  inline CPDF_Rect InToOut(const CPVT_FloatRect& rect) const {
    CPDF_Point ptLeftTop = InToOut(CPDF_Point(rect.left, rect.top));
    CPDF_Point ptRightBottom = InToOut(CPDF_Point(rect.right, rect.bottom));
    return CPDF_Rect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x,
                     ptLeftTop.y);
  }
  inline CPVT_FloatRect OutToIn(const CPDF_Rect& rect) const {
    CPDF_Point ptLeftTop = OutToIn(CPDF_Point(rect.left, rect.top));
    CPDF_Point ptRightBottom = OutToIn(CPDF_Point(rect.right, rect.bottom));
    return CPVT_FloatRect(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x,
                          ptRightBottom.y);
  }

 private:
  CPDF_Rect m_rcPlate;
  CPVT_FloatRect m_rcContent;
};

class CPDF_VariableText : public IPDF_VariableText, private CPDF_EditContainer {
  friend class CTypeset;
  friend class CSection;
  friend class CPDF_VariableText_Iterator;

 public:
  CPDF_VariableText();
  ~CPDF_VariableText() override;

  // IPDF_VariableText
  IPDF_VariableText_Provider* SetProvider(
      IPDF_VariableText_Provider* pProvider) override;
  IPDF_VariableText_Iterator* GetIterator() override;
  void SetPlateRect(const CPDF_Rect& rect) override {
    CPDF_EditContainer::SetPlateRect(rect);
  }
  void SetAlignment(int32_t nFormat = 0) override { m_nAlignment = nFormat; }
  void SetPasswordChar(FX_WORD wSubWord = '*') override {
    m_wSubWord = wSubWord;
  }
  void SetLimitChar(int32_t nLimitChar = 0) override {
    m_nLimitChar = nLimitChar;
  }
  void SetCharSpace(FX_FLOAT fCharSpace = 0.0f) override {
    m_fCharSpace = fCharSpace;
  }
  void SetHorzScale(int32_t nHorzScale = 100) override {
    m_nHorzScale = nHorzScale;
  }
  void SetMultiLine(FX_BOOL bMultiLine = TRUE) override {
    m_bMultiLine = bMultiLine;
  }
  void SetAutoReturn(FX_BOOL bAuto = TRUE) override { m_bLimitWidth = bAuto; }
  void SetFontSize(FX_FLOAT fFontSize) override { m_fFontSize = fFontSize; }
  void SetCharArray(int32_t nCharArray = 0) override {
    m_nCharArray = nCharArray;
  }
  void SetAutoFontSize(FX_BOOL bAuto = TRUE) override {
    m_bAutoFontSize = bAuto;
  }
  void SetRichText(FX_BOOL bRichText) override { m_bRichText = bRichText; }
  void SetLineLeading(FX_FLOAT fLineLeading) override {
    m_fLineLeading = fLineLeading;
  }
  void Initialize() override;
  FX_BOOL IsValid() const override { return m_bInitial; }
  FX_BOOL IsRichText() const override { return m_bRichText; }
  void RearrangeAll() override;
  void RearrangePart(const CPVT_WordRange& PlaceRange) override;
  void ResetAll() override;
  void SetText(const FX_WCHAR* text,
               int32_t charset = 1,
               const CPVT_SecProps* pSecProps = NULL,
               const CPVT_WordProps* pWordProps = NULL) override;
  CPVT_WordPlace InsertWord(const CPVT_WordPlace& place,
                            FX_WORD word,
                            int32_t charset = 1,
                            const CPVT_WordProps* pWordProps = NULL) override;
  CPVT_WordPlace InsertSection(
      const CPVT_WordPlace& place,
      const CPVT_SecProps* pSecProps = NULL,
      const CPVT_WordProps* pWordProps = NULL) override;
  CPVT_WordPlace InsertText(const CPVT_WordPlace& place,
                            const FX_WCHAR* text,
                            int32_t charset = 1,
                            const CPVT_SecProps* pSecProps = NULL,
                            const CPVT_WordProps* pWordProps = NULL) override;
  CPVT_WordPlace DeleteWords(const CPVT_WordRange& PlaceRange) override;
  CPVT_WordPlace DeleteWord(const CPVT_WordPlace& place) override;
  CPVT_WordPlace BackSpaceWord(const CPVT_WordPlace& place) override;
  const CPDF_Rect& GetPlateRect() const override {
    return CPDF_EditContainer::GetPlateRect();
  }
  CPDF_Rect GetContentRect() const override;
  int32_t GetTotalWords() const override;
  FX_FLOAT GetFontSize() const override { return m_fFontSize; }
  int32_t GetAlignment() const override { return m_nAlignment; }
  FX_WORD GetPasswordChar() const override { return GetSubWord(); }
  int32_t GetCharArray() const override { return m_nCharArray; }
  int32_t GetLimitChar() const override { return m_nLimitChar; }
  FX_BOOL IsMultiLine() const override { return m_bMultiLine; }
  int32_t GetHorzScale() const override { return m_nHorzScale; }
  FX_FLOAT GetCharSpace() const override { return m_fCharSpace; }
  CPVT_WordPlace GetBeginWordPlace() const override;
  CPVT_WordPlace GetEndWordPlace() const override;
  CPVT_WordPlace GetPrevWordPlace(const CPVT_WordPlace& place) const override;
  CPVT_WordPlace GetNextWordPlace(const CPVT_WordPlace& place) const override;
  CPVT_WordPlace SearchWordPlace(const CPDF_Point& point) const override;
  CPVT_WordPlace GetUpWordPlace(const CPVT_WordPlace& place,
                                const CPDF_Point& point) const override;
  CPVT_WordPlace GetDownWordPlace(const CPVT_WordPlace& place,
                                  const CPDF_Point& point) const override;
  CPVT_WordPlace GetLineBeginPlace(const CPVT_WordPlace& place) const override;
  CPVT_WordPlace GetLineEndPlace(const CPVT_WordPlace& place) const override;
  CPVT_WordPlace GetSectionBeginPlace(
      const CPVT_WordPlace& place) const override;
  CPVT_WordPlace GetSectionEndPlace(const CPVT_WordPlace& place) const override;
  void UpdateWordPlace(CPVT_WordPlace& place) const override;
  CPVT_WordPlace AdjustLineHeader(const CPVT_WordPlace& place,
                                  FX_BOOL bPrevOrNext) const override;
  int32_t WordPlaceToWordIndex(const CPVT_WordPlace& place) const override;
  CPVT_WordPlace WordIndexToWordPlace(int32_t index) const override;

  FX_WORD GetSubWord() const { return m_wSubWord; }

 private:
  int32_t GetCharWidth(int32_t nFontIndex,
                       FX_WORD Word,
                       FX_WORD SubWord,
                       int32_t nWordStyle);
  int32_t GetTypeAscent(int32_t nFontIndex);
  int32_t GetTypeDescent(int32_t nFontIndex);
  int32_t GetWordFontIndex(FX_WORD word, int32_t charset, int32_t nFontIndex);
  int32_t GetDefaultFontIndex();
  FX_BOOL IsLatinWord(FX_WORD word);

  CPVT_WordPlace AddSection(const CPVT_WordPlace& place,
                            const CPVT_SectionInfo& secinfo);
  CPVT_WordPlace AddLine(const CPVT_WordPlace& place,
                         const CPVT_LineInfo& lineinfo);
  CPVT_WordPlace AddWord(const CPVT_WordPlace& place,
                         const CPVT_WordInfo& wordinfo);
  FX_BOOL GetWordInfo(const CPVT_WordPlace& place, CPVT_WordInfo& wordinfo);
  FX_BOOL SetWordInfo(const CPVT_WordPlace& place,
                      const CPVT_WordInfo& wordinfo);
  FX_BOOL GetLineInfo(const CPVT_WordPlace& place, CPVT_LineInfo& lineinfo);
  FX_BOOL GetSectionInfo(const CPVT_WordPlace& place,
                         CPVT_SectionInfo& secinfo);
  FX_FLOAT GetWordFontSize(const CPVT_WordInfo& WordInfo,
                           FX_BOOL bFactFontSize = FALSE);
  FX_FLOAT GetWordWidth(int32_t nFontIndex,
                        FX_WORD Word,
                        FX_WORD SubWord,
                        FX_FLOAT fCharSpace,
                        int32_t nHorzScale,
                        FX_FLOAT fFontSize,
                        FX_FLOAT fWordTail,
                        int32_t nWordStyle);
  FX_FLOAT GetWordWidth(const CPVT_WordInfo& WordInfo);
  FX_FLOAT GetWordAscent(const CPVT_WordInfo& WordInfo, FX_FLOAT fFontSize);
  FX_FLOAT GetWordDescent(const CPVT_WordInfo& WordInfo, FX_FLOAT fFontSize);
  FX_FLOAT GetWordAscent(const CPVT_WordInfo& WordInfo,
                         FX_BOOL bFactFontSize = FALSE);
  FX_FLOAT GetWordDescent(const CPVT_WordInfo& WordInfo,
                          FX_BOOL bFactFontSize = FALSE);
  FX_FLOAT GetLineAscent(const CPVT_SectionInfo& SecInfo);
  FX_FLOAT GetLineDescent(const CPVT_SectionInfo& SecInfo);
  FX_FLOAT GetFontAscent(int32_t nFontIndex, FX_FLOAT fFontSize);
  FX_FLOAT GetFontDescent(int32_t nFontIndex, FX_FLOAT fFontSize);
  int32_t GetWordFontIndex(const CPVT_WordInfo& WordInfo);
  FX_FLOAT GetCharSpace(const CPVT_WordInfo& WordInfo);
  int32_t GetHorzScale(const CPVT_WordInfo& WordInfo);
  FX_FLOAT GetLineLeading(const CPVT_SectionInfo& SecInfo);
  FX_FLOAT GetLineIndent(const CPVT_SectionInfo& SecInfo);
  int32_t GetAlignment(const CPVT_SectionInfo& SecInfo);

  void ClearSectionRightWords(const CPVT_WordPlace& place);

  FX_BOOL ClearEmptySection(const CPVT_WordPlace& place);
  void ClearEmptySections(const CPVT_WordRange& PlaceRange);
  void LinkLatterSection(const CPVT_WordPlace& place);
  void ClearWords(const CPVT_WordRange& PlaceRange);
  CPVT_WordPlace ClearLeftWord(const CPVT_WordPlace& place);
  CPVT_WordPlace ClearRightWord(const CPVT_WordPlace& place);

 private:
  CPVT_FloatRect Rearrange(const CPVT_WordRange& PlaceRange);
  FX_FLOAT GetAutoFontSize();
  FX_BOOL IsBigger(FX_FLOAT fFontSize);
  CPVT_FloatRect RearrangeSections(const CPVT_WordRange& PlaceRange);

  void ResetSectionArray();

  CPVT_ArrayTemplate<CSection*> m_SectionArray;
  int32_t m_nLimitChar;
  int32_t m_nCharArray;
  FX_BOOL m_bMultiLine;
  FX_BOOL m_bLimitWidth;
  FX_BOOL m_bAutoFontSize;
  int32_t m_nAlignment;
  FX_FLOAT m_fLineLeading;
  FX_FLOAT m_fCharSpace;
  int32_t m_nHorzScale;
  FX_WORD m_wSubWord;
  FX_FLOAT m_fFontSize;

 private:
  FX_BOOL m_bInitial;
  FX_BOOL m_bRichText;
  IPDF_VariableText_Provider* m_pVTProvider;
  CPDF_VariableText_Iterator* m_pVTIterator;
};

class CPDF_VariableText_Iterator : public IPDF_VariableText_Iterator {
 public:
  explicit CPDF_VariableText_Iterator(CPDF_VariableText* pVT);
  ~CPDF_VariableText_Iterator() override;

  // IPDF_VariableText_Iterator
  FX_BOOL NextWord() override;
  FX_BOOL PrevWord() override;
  FX_BOOL NextLine() override;
  FX_BOOL PrevLine() override;
  FX_BOOL NextSection() override;
  FX_BOOL PrevSection() override;
  FX_BOOL SetWord(const CPVT_Word& word) override;
  FX_BOOL GetWord(CPVT_Word& word) const override;
  FX_BOOL GetLine(CPVT_Line& line) const override;
  FX_BOOL GetSection(CPVT_Section& section) const override;
  FX_BOOL SetSection(const CPVT_Section& section) override;
  void SetAt(int32_t nWordIndex) override;
  void SetAt(const CPVT_WordPlace& place) override;
  const CPVT_WordPlace& GetAt() const override { return m_CurPos; }

 private:
  CPVT_WordPlace m_CurPos;
  CPDF_VariableText* m_pVT;
};

#endif  // CORE_SRC_FPDFDOC_PDF_VT_H_
