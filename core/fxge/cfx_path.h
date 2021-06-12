// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_PATH_H_
#define CORE_FXGE_CFX_PATH_H_

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

class CFX_Path {
 public:
  CFX_Path();
  CFX_Path(const CFX_Path& src);
  CFX_Path(CFX_Path&& src) noexcept;
  ~CFX_Path();

  void Clear();

  FXPT_TYPE GetType(int index) const { return m_Points[index].m_Type; }
  bool IsClosingFigure(int index) const {
    return m_Points[index].m_CloseFigure;
  }

  CFX_PointF GetPoint(int index) const { return m_Points[index].m_Point; }
  const std::vector<FX_PATHPOINT>& GetPoints() const { return m_Points; }
  std::vector<FX_PATHPOINT>& GetPoints() { return m_Points; }

  CFX_FloatRect GetBoundingBox() const;
  CFX_FloatRect GetBoundingBoxForStrokePath(float line_width,
                                            float miter_limit) const;

  void Transform(const CFX_Matrix& matrix);
  bool IsRect() const;
  Optional<CFX_FloatRect> GetRect(const CFX_Matrix* matrix) const;

  void Append(const CFX_Path& src, const CFX_Matrix* matrix);
  void AppendFloatRect(const CFX_FloatRect& rect);
  void AppendRect(float left, float bottom, float right, float top);
  void AppendLine(const CFX_PointF& pt1, const CFX_PointF& pt2);
  void AppendPoint(const CFX_PointF& point, FXPT_TYPE type);
  void AppendPointAndClose(const CFX_PointF& point, FXPT_TYPE type);
  void ClosePath();

 private:
  std::vector<FX_PATHPOINT> m_Points;
};

class CFX_RetainablePath final : public Retainable, public CFX_Path {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  RetainPtr<CFX_RetainablePath> Clone() const;

 private:
  CFX_RetainablePath();
  CFX_RetainablePath(const CFX_RetainablePath& src);
  ~CFX_RetainablePath() override;
};

#endif  // CORE_FXGE_CFX_PATH_H_
