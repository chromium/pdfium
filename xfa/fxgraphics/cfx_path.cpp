// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cfx_path.h"

#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxgraphics/cfx_path_generator.h"

CFX_Path::CFX_Path() {}

FWL_Error CFX_Path::Create() {
  if (m_generator)
    return FWL_Error::PropertyInvalid;

  m_generator = pdfium::MakeUnique<CFX_PathGenerator>();
  return FWL_Error::Succeeded;
}

CFX_Path::~CFX_Path() {}

FWL_Error CFX_Path::MoveTo(const CFX_PointF& point) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->MoveTo(point);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::LineTo(const CFX_PointF& point) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->LineTo(point);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::BezierTo(const CFX_PointF& c1,
                             const CFX_PointF& c2,
                             const CFX_PointF& to) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->BezierTo(c1, c2, to);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::ArcTo(const CFX_PointF& pos,
                          const CFX_SizeF& size,
                          FX_FLOAT startAngle,
                          FX_FLOAT sweepAngle) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  CFX_SizeF newSize = size / 2.0f;
  m_generator->ArcTo(CFX_PointF(pos.x + newSize.width, pos.y + newSize.height),
                     newSize, startAngle, sweepAngle);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::Close() {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->Close();
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddLine(const CFX_PointF& p1, const CFX_PointF& p2) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->AddLine(p1, p2);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddBezier(const CFX_PointF& p1,
                              const CFX_PointF& c1,
                              const CFX_PointF& c2,
                              const CFX_PointF& p2) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->AddBezier(p1, c1, c2, p2);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddRectangle(FX_FLOAT left,
                                 FX_FLOAT top,
                                 FX_FLOAT width,
                                 FX_FLOAT height) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->AddRectangle(left, top, left + width, top + height);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddEllipse(const CFX_PointF& pos, const CFX_SizeF& size) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  CFX_SizeF newSize = size / 2.0f;
  m_generator->AddEllipse(
      CFX_PointF(pos.x + newSize.width, pos.y + newSize.height), newSize);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddEllipse(const CFX_RectF& rect) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->AddEllipse(
      CFX_PointF(rect.left + rect.Width() / 2, rect.top + rect.Height() / 2),
      CFX_SizeF(rect.Width() / 2, rect.Height() / 2));
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddArc(const CFX_PointF& pos,
                           const CFX_SizeF& size,
                           FX_FLOAT startAngle,
                           FX_FLOAT sweepAngle) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  CFX_SizeF newSize = size / 2;
  m_generator->AddArc(CFX_PointF(pos.x + newSize.width, pos.y + newSize.height),
                      newSize, startAngle, sweepAngle);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddPie(const CFX_PointF& pos,
                           const CFX_SizeF& size,
                           FX_FLOAT startAngle,
                           FX_FLOAT sweepAngle) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  CFX_SizeF newSize = size / 2;
  m_generator->AddPie(CFX_PointF(pos.x + newSize.width, pos.y + newSize.height),
                      newSize, startAngle, sweepAngle);
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::AddSubpath(CFX_Path* path) {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->AddPathData(path->GetPathData());
  return FWL_Error::Succeeded;
}

FWL_Error CFX_Path::Clear() {
  if (!m_generator)
    return FWL_Error::PropertyInvalid;
  m_generator->GetPathData()->Clear();
  return FWL_Error::Succeeded;
}

bool CFX_Path::IsEmpty() const {
  if (!m_generator)
    return false;
  if (m_generator->GetPathData()->GetPoints().empty())
    return true;
  return false;
}

CFX_PathData* CFX_Path::GetPathData() const {
  if (!m_generator)
    return nullptr;
  return m_generator->GetPathData();
}
