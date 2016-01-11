// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFTEXT_TEXT_INT_H_
#define CORE_SRC_FPDFTEXT_TEXT_INT_H_

#include "core/include/fpdftext/fpdf_text.h"
#include "core/include/fxcrt/fx_basic.h"

class CFX_BidiChar;
class CPDF_DocProgressiveSearch;
class CPDF_FormObject;
class CPDF_LinkExtract;
class CPDF_TextPageFind;

#define FPDFTEXT_CHAR_ERROR -1
#define FPDFTEXT_CHAR_NORMAL 0
#define FPDFTEXT_CHAR_GENERATED 1
#define FPDFTEXT_CHAR_UNUNICODE 2
#define FPDFTEXT_CHAR_HYPHEN 3
#define FPDFTEXT_CHAR_PIECE 4
#define FPDFTEXT_MC_PASS 0
#define FPDFTEXT_MC_DONE 1
#define FPDFTEXT_MC_DELAY 2

typedef struct _PAGECHAR_INFO {
  int m_CharCode;
  FX_WCHAR m_Unicode;
  FX_FLOAT m_OriginX;
  FX_FLOAT m_OriginY;
  int32_t m_Flag;
  CFX_FloatRect m_CharBox;
  CPDF_TextObject* m_pTextObj;
  CFX_Matrix m_Matrix;
  int m_Index;
} PAGECHAR_INFO;
typedef CFX_SegmentedArray<PAGECHAR_INFO> PAGECHAR_InfoArray;
typedef struct {
  int m_Start;
  int m_nCount;
} FPDF_SEGMENT;
typedef CFX_ArrayTemplate<FPDF_SEGMENT> SEGMENT_Array;
typedef struct {
  CPDF_TextObject* m_pTextObj;
  CFX_Matrix m_formMatrix;
} PDFTEXT_Obj;
typedef CFX_ArrayTemplate<PDFTEXT_Obj> LINEOBJ;

class CPDF_TextPage : public IPDF_TextPage {
 public:
  CPDF_TextPage(const CPDF_Page* pPage, int flags);
  ~CPDF_TextPage() override {}

  // IPDF_TextPage
  FX_BOOL ParseTextPage() override;
  void NormalizeObjects(FX_BOOL bNormalize) override;
  bool IsParsed() const override { return m_bIsParsed; }
  int CharIndexFromTextIndex(int TextIndex) const override;
  int TextIndexFromCharIndex(int CharIndex) const override;
  int CountChars() const override;
  void GetCharInfo(int index, FPDF_CHAR_INFO* info) const override;
  void GetRectArray(int start,
                    int nCount,
                    CFX_RectArray& rectArray) const override;
  int GetIndexAtPos(CPDF_Point point,
                    FX_FLOAT xTolerance,
                    FX_FLOAT yTolerance) const override;
  int GetIndexAtPos(FX_FLOAT x,
                    FX_FLOAT y,
                    FX_FLOAT xTolerance,
                    FX_FLOAT yTolerance) const override;
  CFX_WideString GetTextByRect(const CFX_FloatRect& rect) const override;
  void GetRectsArrayByRect(const CFX_FloatRect& rect,
                           CFX_RectArray& resRectArray) const override;
  CFX_WideString GetPageText(int start = 0, int nCount = -1) const override;
  int CountRects(int start, int nCount) override;
  void GetRect(int rectIndex,
               FX_FLOAT& left,
               FX_FLOAT& top,
               FX_FLOAT& right,
               FX_FLOAT& bottom) const override;
  FX_BOOL GetBaselineRotate(int rectIndex, int& Rotate) override;
  FX_BOOL GetBaselineRotate(const CFX_FloatRect& rect, int& Rotate) override;
  int CountBoundedSegments(FX_FLOAT left,
                           FX_FLOAT top,
                           FX_FLOAT right,
                           FX_FLOAT bottom,
                           FX_BOOL bContains = FALSE) override;
  void GetBoundedSegment(int index, int& start, int& count) const override;
  int GetWordBreak(int index, int direction) const override;

  const PAGECHAR_InfoArray* GetCharList() const { return &m_charList; }
  static FX_BOOL IsRectIntersect(const CFX_FloatRect& rect1,
                                 const CFX_FloatRect& rect2);
  static FX_BOOL IsLetter(FX_WCHAR unicode);

 private:
  FX_BOOL IsHyphen(FX_WCHAR curChar);
  bool IsControlChar(const PAGECHAR_INFO& charInfo);
  FX_BOOL GetBaselineRotate(int start, int end, int& Rotate);
  void ProcessObject();
  void ProcessFormObject(CPDF_FormObject* pFormObj,
                         const CFX_Matrix& formMatrix);
  void ProcessTextObject(PDFTEXT_Obj pObj);
  void ProcessTextObject(CPDF_TextObject* pTextObj,
                         const CFX_Matrix& formMatrix,
                         FX_POSITION ObjPos);
  int ProcessInsertObject(const CPDF_TextObject* pObj,
                          const CFX_Matrix& formMatrix);
  FX_BOOL GenerateCharInfo(FX_WCHAR unicode, PAGECHAR_INFO& info);
  FX_BOOL IsSameAsPreTextObject(CPDF_TextObject* pTextObj, FX_POSITION ObjPos);
  FX_BOOL IsSameTextObject(CPDF_TextObject* pTextObj1,
                           CPDF_TextObject* pTextObj2);
  int GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont) const;
  void CloseTempLine();
  void OnPiece(CFX_BidiChar* pBidi, CFX_WideString& str);
  int32_t PreMarkedContent(PDFTEXT_Obj pObj);
  void ProcessMarkedContent(PDFTEXT_Obj pObj);
  void CheckMarkedContentObject(int32_t& start, int32_t& nCount) const;
  void FindPreviousTextObject(void);
  void AddCharInfoByLRDirection(CFX_WideString& str, int i);
  void AddCharInfoByRLDirection(CFX_WideString& str, int i);
  int32_t GetTextObjectWritingMode(const CPDF_TextObject* pTextObj);
  int32_t FindTextlineFlowDirection();

  void SwapTempTextBuf(int32_t iCharListStartAppend, int32_t iBufStartAppend);
  FX_BOOL IsRightToLeft(const CPDF_TextObject* pTextObj,
                        const CPDF_Font* pFont,
                        int nItems) const;

  CPDFText_ParseOptions m_ParseOptions;
  CFX_WordArray m_CharIndex;
  const CPDF_PageObjects* const m_pPage;
  PAGECHAR_InfoArray m_charList;
  CFX_WideTextBuf m_TextBuf;
  PAGECHAR_InfoArray m_TempCharList;
  CFX_WideTextBuf m_TempTextBuf;
  const int m_parserflag;
  CPDF_TextObject* m_pPreTextObj;
  CFX_Matrix m_perMatrix;
  bool m_bIsParsed;
  CFX_Matrix m_DisplayMatrix;
  SEGMENT_Array m_Segment;
  CFX_RectArray m_SelRects;
  LINEOBJ m_LineObj;
  int32_t m_TextlineDir;
  CFX_FloatRect m_CurlineRect;
};

class CPDF_TextPageFind : public IPDF_TextPageFind {
 public:
  explicit CPDF_TextPageFind(const IPDF_TextPage* pTextPage);
  ~CPDF_TextPageFind() override {}

  // IPDF_TextPageFind
  FX_BOOL FindFirst(const CFX_WideString& findwhat,
                    int flags,
                    int startPos = 0) override;
  FX_BOOL FindNext() override;
  FX_BOOL FindPrev() override;
  void GetRectArray(CFX_RectArray& rects) const override;
  int GetCurOrder() const override;
  int GetMatchedCount() const override;

 protected:
  void ExtractFindWhat(const CFX_WideString& findwhat);
  FX_BOOL IsMatchWholeWord(const CFX_WideString& csPageText,
                           int startPos,
                           int endPos);
  FX_BOOL ExtractSubString(CFX_WideString& rString,
                           const FX_WCHAR* lpszFullString,
                           int iSubString,
                           FX_WCHAR chSep);
  CFX_WideString MakeReverse(const CFX_WideString& str);
  int ReverseFind(const CFX_WideString& csPageText,
                  const CFX_WideString& csWord,
                  int nStartPos,
                  int& WordLength);
  int GetCharIndex(int index) const;

 private:
  CFX_WordArray m_CharIndex;
  const IPDF_TextPage* m_pTextPage;
  CFX_WideString m_strText;
  CFX_WideString m_findWhat;
  int m_flags;
  CFX_WideStringArray m_csFindWhatArray;
  int m_findNextStart;
  int m_findPreStart;
  FX_BOOL m_bMatchCase;
  FX_BOOL m_bMatchWholeWord;
  int m_resStart;
  int m_resEnd;
  CFX_RectArray m_resArray;
  FX_BOOL m_IsFind;
};

class CPDF_LinkExt {
 public:
  CPDF_LinkExt() {}
  int m_Start;
  int m_Count;
  CFX_WideString m_strUrl;
  virtual ~CPDF_LinkExt() {}
};

typedef CFX_ArrayTemplate<CPDF_LinkExt*> LINK_InfoArray;

class CPDF_LinkExtract : public IPDF_LinkExtract {
 public:
  CPDF_LinkExtract();
  ~CPDF_LinkExtract() override;

  // IPDF_LinkExtract
  FX_BOOL ExtractLinks(const IPDF_TextPage* pTextPage) override;
  int CountLinks() const override;
  CFX_WideString GetURL(int index) const override;
  void GetBoundedSegment(int index, int& start, int& count) const override;
  void GetRects(int index, CFX_RectArray& rects) const override;

  FX_BOOL IsExtract() const { return m_bIsParsed; }

 protected:
  void ParseLink();
  void DeleteLinkList();
  FX_BOOL CheckWebLink(CFX_WideString& strBeCheck);
  bool CheckMailLink(CFX_WideString& str);
  void AppendToLinkList(int start, int count, const CFX_WideString& strUrl);

 private:
  LINK_InfoArray m_LinkList;
  const CPDF_TextPage* m_pTextPage;
  CFX_WideString m_strPageText;
  bool m_bIsParsed;
};

FX_STRSIZE FX_Unicode_GetNormalization(FX_WCHAR wch, FX_WCHAR* pDst);
void NormalizeString(CFX_WideString& str);
void NormalizeCompositeChar(FX_WCHAR wChar, CFX_WideString& sDest);
void GetTextStream_Unicode(CFX_WideTextBuf& buffer,
                           CPDF_PageObjects* pPage,
                           FX_BOOL bUseLF,
                           CFX_PtrArray* pObjArray);

#endif  // CORE_SRC_FPDFTEXT_TEXT_INT_H_
