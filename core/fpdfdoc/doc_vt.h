// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_DOC_VT_H_
#define CORE_FPDFDOC_DOC_VT_H_

#include "core/fpdfdoc/cpvt_floatrect.h"
#include "core/fpdfdoc/cpvt_lineinfo.h"
#include "core/fpdfdoc/include/cpvt_wordrange.h"

class CPDF_VariableText;

struct CPVT_WordInfo;

#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa, fb) IsFloatZero((fa) - (fb))

class CLine {
 public:
  CLine();
  ~CLine();

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
  ~CLines() { RemoveAll(); }

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
  CFX_ArrayTemplate<CLine*> m_Lines;
  int32_t m_nTotal;
};

class CPDF_EditContainer {
 public:
  CPDF_EditContainer();
  virtual ~CPDF_EditContainer();

  virtual const CFX_FloatRect& GetPlateRect() const { return m_rcPlate; }
  virtual void SetPlateRect(const CFX_FloatRect& rect) { m_rcPlate = rect; }
  virtual CFX_FloatRect GetContentRect() const { return m_rcContent; }
  virtual void SetContentRect(const CPVT_FloatRect& rect) {
    m_rcContent = rect;
  }

  FX_FLOAT GetPlateWidth() const { return m_rcPlate.right - m_rcPlate.left; }
  FX_FLOAT GetPlateHeight() const { return m_rcPlate.top - m_rcPlate.bottom; }
  CFX_FloatPoint GetBTPoint() const {
    return CFX_FloatPoint(m_rcPlate.left, m_rcPlate.top);
  }
  CFX_FloatPoint InToOut(const CFX_FloatPoint& point) const {
    return CFX_FloatPoint(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CFX_FloatPoint OutToIn(const CFX_FloatPoint& point) const {
    return CFX_FloatPoint(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
  }
  CFX_FloatRect InToOut(const CPVT_FloatRect& rect) const;

 private:
  CFX_FloatRect m_rcPlate;
  CPVT_FloatRect m_rcContent;
};

#endif  // CORE_FPDFDOC_DOC_VT_H_
