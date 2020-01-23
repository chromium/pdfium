// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_CPDF_TEXTPAGE_H_
#define CORE_FPDFTEXT_CPDF_TEXTPAGE_H_

#include <deque>
#include <functional>
#include <vector>

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"

class CPDF_Font;
class CPDF_FormObject;
class CPDF_Page;
class CPDF_TextObject;

#define FPDFTEXT_CHAR_NORMAL 0
#define FPDFTEXT_CHAR_GENERATED 1
#define FPDFTEXT_CHAR_UNUNICODE 2
#define FPDFTEXT_CHAR_HYPHEN 3
#define FPDFTEXT_CHAR_PIECE 4

#define TEXT_SPACE_CHAR L' '
#define TEXT_LINEFEED_CHAR L'\n'
#define TEXT_RETURN_CHAR L'\r'
#define TEXT_HYPHEN_CHAR L'-'
#define TEXT_HYPHEN L"-"
#define TEXT_CHARRATIO_GAPDELTA 0.070

class PAGECHAR_INFO {
 public:
  PAGECHAR_INFO();
  PAGECHAR_INFO(const PAGECHAR_INFO&);
  ~PAGECHAR_INFO();

  int m_Index = 0;
  int m_CharCode = 0;
  wchar_t m_Unicode = 0;
  int32_t m_Flag = 0;
  CFX_PointF m_Origin;
  CFX_FloatRect m_CharBox;
  UnownedPtr<CPDF_TextObject> m_pTextObj;
  CFX_Matrix m_Matrix;
};

struct PDFTEXT_Obj {
  PDFTEXT_Obj();
  PDFTEXT_Obj(const PDFTEXT_Obj& that);
  ~PDFTEXT_Obj();

  UnownedPtr<CPDF_TextObject> m_pTextObj;
  CFX_Matrix m_formMatrix;
};

class CPDF_TextPage {
 public:
  class CharInfo {
   public:
    CharInfo();
    ~CharInfo();

    wchar_t m_Unicode = 0;
    wchar_t m_Charcode = 0;
    int32_t m_Flag = 0;
    float m_FontSize = 0;
    CFX_PointF m_Origin;
    CFX_FloatRect m_CharBox;
    UnownedPtr<CPDF_TextObject> m_pTextObj;
    CFX_Matrix m_Matrix;
  };

  CPDF_TextPage(const CPDF_Page* pPage, bool rtl);
  ~CPDF_TextPage();

  int CharIndexFromTextIndex(int text_index) const;
  int TextIndexFromCharIndex(int char_index) const;
  size_t size() const { return m_CharList.size(); }
  int CountChars() const;
  void GetCharInfo(size_t index, CharInfo* info) const;
  std::vector<CFX_FloatRect> GetRectArray(int start, int nCount) const;
  int GetIndexAtPos(const CFX_PointF& point, const CFX_SizeF& tolerance) const;
  WideString GetTextByRect(const CFX_FloatRect& rect) const;
  WideString GetTextByObject(const CPDF_TextObject* pTextObj) const;

  // Returns string with the text from |m_TextBuf| that are covered by the input
  // range. |start| and |count| are in terms of the |m_CharIndices|, so the
  // range will be converted into appropriate indices.
  WideString GetPageText(int start, int count) const;
  WideString GetAllPageText() const { return GetPageText(0, CountChars()); }

  int CountRects(int start, int nCount);
  bool GetRect(int rectIndex, CFX_FloatRect* pRect) const;

 private:
  enum class TextOrientation {
    kUnknown,
    kHorizontal,
    kVertical,
  };

  enum class GenerateCharacter {
    kNone,
    kSpace,
    kLineBreak,
    kHyphen,
  };

  enum class MarkedContentState { kPass = 0, kDone, kDelay };

  void Init();
  bool IsHyphen(wchar_t curChar) const;
  void ProcessObject();
  void ProcessFormObject(CPDF_FormObject* pFormObj,
                         const CFX_Matrix& formMatrix);
  void ProcessTextObject(PDFTEXT_Obj pObj);
  void ProcessTextObject(CPDF_TextObject* pTextObj,
                         const CFX_Matrix& formMatrix,
                         const CPDF_PageObjectHolder* pObjList,
                         CPDF_PageObjectHolder::const_iterator ObjPos);
  GenerateCharacter ProcessInsertObject(const CPDF_TextObject* pObj,
                                        const CFX_Matrix& formMatrix);
  const PAGECHAR_INFO* GetPrevCharInfo() const;
  Optional<PAGECHAR_INFO> GenerateCharInfo(wchar_t unicode);
  bool IsSameAsPreTextObject(CPDF_TextObject* pTextObj,
                             const CPDF_PageObjectHolder* pObjList,
                             CPDF_PageObjectHolder::const_iterator iter) const;
  bool IsSameTextObject(CPDF_TextObject* pTextObj1,
                        CPDF_TextObject* pTextObj2) const;
  void CloseTempLine();
  MarkedContentState PreMarkedContent(PDFTEXT_Obj pObj);
  void ProcessMarkedContent(PDFTEXT_Obj pObj);
  void FindPreviousTextObject();
  void AddCharInfoByLRDirection(wchar_t wChar, const PAGECHAR_INFO& info);
  void AddCharInfoByRLDirection(wchar_t wChar, const PAGECHAR_INFO& info);
  TextOrientation GetTextObjectWritingMode(
      const CPDF_TextObject* pTextObj) const;
  TextOrientation FindTextlineFlowOrientation() const;
  void AppendGeneratedCharacter(wchar_t unicode, const CFX_Matrix& formMatrix);
  void SwapTempTextBuf(int32_t iCharListStartAppend, int32_t iBufStartAppend);
  WideString GetTextByPredicate(
      const std::function<bool(const PAGECHAR_INFO&)>& predicate) const;

  UnownedPtr<const CPDF_Page> const m_pPage;
  std::vector<uint16_t> m_CharIndices;
  std::deque<PAGECHAR_INFO> m_CharList;
  std::deque<PAGECHAR_INFO> m_TempCharList;
  CFX_WideTextBuf m_TextBuf;
  CFX_WideTextBuf m_TempTextBuf;
  UnownedPtr<CPDF_TextObject> m_pPreTextObj;
  CFX_Matrix m_perMatrix;
  const bool m_rtl;
  const CFX_Matrix m_DisplayMatrix;
  std::vector<CFX_FloatRect> m_SelRects;
  std::vector<PDFTEXT_Obj> m_LineObj;
  TextOrientation m_TextlineDir = TextOrientation::kUnknown;
  CFX_FloatRect m_CurlineRect;
};

#endif  // CORE_FPDFTEXT_CPDF_TEXTPAGE_H_
