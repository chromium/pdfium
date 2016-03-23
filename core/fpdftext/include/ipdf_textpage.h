// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGE_H_
#define CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGE_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_TextObject;
class CPDF_Page;

struct FPDF_CHAR_INFO {
  FX_WCHAR m_Unicode;
  FX_WCHAR m_Charcode;
  int32_t m_Flag;
  FX_FLOAT m_FontSize;
  FX_FLOAT m_OriginX;
  FX_FLOAT m_OriginY;
  CFX_FloatRect m_CharBox;
  CPDF_TextObject* m_pTextObj;
  CFX_Matrix m_Matrix;
};

class IPDF_TextPage {
 public:
  static IPDF_TextPage* CreateTextPage(const CPDF_Page* pPage, int flags = 0);
  virtual ~IPDF_TextPage() {}

  virtual void ParseTextPage() = 0;
  virtual bool IsParsed() const = 0;
  virtual int CharIndexFromTextIndex(int TextIndex) const = 0;
  virtual int TextIndexFromCharIndex(int CharIndex) const = 0;
  virtual int CountChars() const = 0;
  virtual void GetCharInfo(int index, FPDF_CHAR_INFO* info) const = 0;
  virtual void GetRectArray(int start,
                            int nCount,
                            CFX_RectArray& rectArray) const = 0;
  virtual int GetIndexAtPos(CFX_FloatPoint point,
                            FX_FLOAT xTolerance,
                            FX_FLOAT yTolerance) const = 0;
  virtual int GetIndexAtPos(FX_FLOAT x,
                            FX_FLOAT y,
                            FX_FLOAT xTolerance,
                            FX_FLOAT yTolerance) const = 0;
  virtual CFX_WideString GetTextByRect(const CFX_FloatRect& rect) const = 0;
  virtual void GetRectsArrayByRect(const CFX_FloatRect& rect,
                                   CFX_RectArray& resRectArray) const = 0;
  virtual int CountRects(int start, int nCount) = 0;
  virtual void GetRect(int rectIndex,
                       FX_FLOAT& left,
                       FX_FLOAT& top,
                       FX_FLOAT& right,
                       FX_FLOAT& bottom) const = 0;
  virtual FX_BOOL GetBaselineRotate(int rectIndex, int& Rotate) = 0;
  virtual FX_BOOL GetBaselineRotate(const CFX_FloatRect& rect, int& Rotate) = 0;
  virtual int CountBoundedSegments(FX_FLOAT left,
                                   FX_FLOAT top,
                                   FX_FLOAT right,
                                   FX_FLOAT bottom,
                                   FX_BOOL bContains = FALSE) = 0;
  virtual void GetBoundedSegment(int index, int& start, int& count) const = 0;
  virtual int GetWordBreak(int index, int direction) const = 0;
  virtual CFX_WideString GetPageText(int start = 0, int nCount = -1) const = 0;
};

#endif  // CORE_FPDFTEXT_INCLUDE_IPDF_TEXTPAGE_H_
