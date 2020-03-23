// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_PATHDATA_H_
#define CORE_FXGE_CFX_PATHDATA_H_

#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/optional.h"

enum class FXPT_TYPE : uint8_t { LineTo, BezierTo, MoveTo };

class FX_PATHPOINT {
 public:
  FX_PATHPOINT();
  FX_PATHPOINT(const CFX_PointF& point, FXPT_TYPE type, bool close);
  FX_PATHPOINT(const FX_PATHPOINT& other);
  ~FX_PATHPOINT();

  bool IsTypeAndOpen(FXPT_TYPE type) const {
    return m_Type == type && !m_CloseFigure;
  }

  CFX_PointF m_Point;
  FXPT_TYPE m_Type;
  bool m_CloseFigure;
};

class CFX_PathData {
 public:
  CFX_PathData();
  CFX_PathData(const CFX_PathData& src);
  CFX_PathData(CFX_PathData&& src);
  ~CFX_PathData();

  void Clear();

  FXPT_TYPE GetType(int index) const { return m_Points[index].m_Type; }
  bool IsClosingFigure(int index) const {
    return m_Points[index].m_CloseFigure;
  }

  CFX_PointF GetPoint(int index) const { return m_Points[index].m_Point; }
  const std::vector<FX_PATHPOINT>& GetPoints() const { return m_Points; }
  std::vector<FX_PATHPOINT>& GetPoints() { return m_Points; }

  CFX_FloatRect GetBoundingBox() const;
  CFX_FloatRect GetBoundingBox(float line_width, float miter_limit) const;

  void Transform(const CFX_Matrix& matrix);
  bool IsRect() const;
  bool GetZeroAreaPath(const CFX_Matrix* pMatrix,
                       bool bAdjust,
                       CFX_PathData* NewPath,
                       bool* bThin,
                       bool* setIdentity) const;
  Optional<CFX_FloatRect> GetRect(const CFX_Matrix* pMatrix) const;

  void Append(const CFX_PathData* pSrc, const CFX_Matrix* pMatrix);
  void AppendFloatRect(const CFX_FloatRect& rect);
  void AppendRect(float left, float bottom, float right, float top);
  void AppendLine(const CFX_PointF& pt1, const CFX_PointF& pt2);
  void AppendPoint(const CFX_PointF& point, FXPT_TYPE type, bool closeFigure);
  void ClosePath();

 private:
  std::vector<FX_PATHPOINT> m_Points;
};

class CFX_RetainablePathData final : public Retainable, public CFX_PathData {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  RetainPtr<CFX_RetainablePathData> Clone() const;

 private:
  CFX_RetainablePathData();
  CFX_RetainablePathData(const CFX_RetainablePathData& src);
  ~CFX_RetainablePathData() override;
};

#endif  // CORE_FXGE_CFX_PATHDATA_H_
