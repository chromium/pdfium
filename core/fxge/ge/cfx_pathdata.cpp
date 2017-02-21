// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_pathdata.h"

#include "core/fxcrt/fx_system.h"
#include "third_party/base/numerics/safe_math.h"

CFX_PathData::CFX_PathData() {}

CFX_PathData::~CFX_PathData() {}

CFX_PathData::CFX_PathData(const CFX_PathData& src) : m_Points(src.m_Points) {}

void CFX_PathData::Clear() {
  m_Points.clear();
}

void CFX_PathData::ClosePath() {
  if (m_Points.empty())
    return;
  m_Points.back().m_CloseFigure = true;
}

void CFX_PathData::Append(const CFX_PathData* pSrc, const CFX_Matrix* pMatrix) {
  if (pSrc->m_Points.empty())
    return;

  size_t cur_size = m_Points.size();
  m_Points.insert(m_Points.end(), pSrc->m_Points.begin(), pSrc->m_Points.end());

  if (!pMatrix)
    return;

  for (size_t i = cur_size; i < m_Points.size(); i++)
    pMatrix->TransformPoint(m_Points[i].m_PointX, m_Points[i].m_PointY);
}

void CFX_PathData::AppendPoint(FX_FLOAT x,
                               FX_FLOAT y,
                               FXPT_TYPE type,
                               bool closeFigure) {
  m_Points.push_back({x, y, type, closeFigure});
}

void CFX_PathData::AppendRect(FX_FLOAT left,
                              FX_FLOAT bottom,
                              FX_FLOAT right,
                              FX_FLOAT top) {
  m_Points.push_back({left, bottom, FXPT_TYPE::MoveTo, false});
  m_Points.push_back({left, top, FXPT_TYPE::LineTo, false});
  m_Points.push_back({right, top, FXPT_TYPE::LineTo, false});
  m_Points.push_back({right, bottom, FXPT_TYPE::LineTo, false});
  m_Points.push_back({left, bottom, FXPT_TYPE::LineTo, true});
}

CFX_FloatRect CFX_PathData::GetBoundingBox() const {
  if (m_Points.empty())
    return CFX_FloatRect();

  CFX_FloatRect rect;
  rect.InitRect(m_Points[0].m_PointX, m_Points[0].m_PointY);
  for (size_t i = 1; i < m_Points.size(); i++)
    rect.UpdateRect(m_Points[i].m_PointX, m_Points[i].m_PointY);
  return rect;
}

static void _UpdateLineEndPoints(CFX_FloatRect& rect,
                                 FX_FLOAT start_x,
                                 FX_FLOAT start_y,
                                 FX_FLOAT end_x,
                                 FX_FLOAT end_y,
                                 FX_FLOAT hw) {
  if (start_x == end_x) {
    if (start_y == end_y) {
      rect.UpdateRect(end_x + hw, end_y + hw);
      rect.UpdateRect(end_x - hw, end_y - hw);
      return;
    }
    FX_FLOAT point_y;
    if (end_y < start_y) {
      point_y = end_y - hw;
    } else {
      point_y = end_y + hw;
    }
    rect.UpdateRect(end_x + hw, point_y);
    rect.UpdateRect(end_x - hw, point_y);
    return;
  }
  if (start_y == end_y) {
    FX_FLOAT point_x;
    if (end_x < start_x) {
      point_x = end_x - hw;
    } else {
      point_x = end_x + hw;
    }
    rect.UpdateRect(point_x, end_y + hw);
    rect.UpdateRect(point_x, end_y - hw);
    return;
  }
  FX_FLOAT dx = end_x - start_x;
  FX_FLOAT dy = end_y - start_y;
  FX_FLOAT ll = FXSYS_sqrt2(dx, dy);
  FX_FLOAT mx = end_x + hw * dx / ll;
  FX_FLOAT my = end_y + hw * dy / ll;
  FX_FLOAT dx1 = hw * dy / ll;
  FX_FLOAT dy1 = hw * dx / ll;
  rect.UpdateRect(mx - dx1, my + dy1);
  rect.UpdateRect(mx + dx1, my - dy1);
}

static void _UpdateLineJoinPoints(CFX_FloatRect& rect,
                                  FX_FLOAT start_x,
                                  FX_FLOAT start_y,
                                  FX_FLOAT middle_x,
                                  FX_FLOAT middle_y,
                                  FX_FLOAT end_x,
                                  FX_FLOAT end_y,
                                  FX_FLOAT half_width,
                                  FX_FLOAT miter_limit) {
  FX_FLOAT start_k = 0, start_c = 0, end_k = 0, end_c = 0, start_len = 0,
           start_dc = 0, end_len = 0, end_dc = 0;
  bool bStartVert = FXSYS_fabs(start_x - middle_x) < 1.0f / 20;
  bool bEndVert = FXSYS_fabs(middle_x - end_x) < 1.0f / 20;
  if (bStartVert && bEndVert) {
    int start_dir = middle_y > start_y ? 1 : -1;
    FX_FLOAT point_y = middle_y + half_width * start_dir;
    rect.UpdateRect(middle_x + half_width, point_y);
    rect.UpdateRect(middle_x - half_width, point_y);
    return;
  }
  if (!bStartVert) {
    start_k = (middle_y - start_y) / (middle_x - start_x);
    start_c = middle_y - (start_k * middle_x);
    start_len = FXSYS_sqrt2(start_x - middle_x, start_y - middle_y);
    start_dc =
        (FX_FLOAT)FXSYS_fabs(half_width * start_len / (start_x - middle_x));
  }
  if (!bEndVert) {
    end_k = (end_y - middle_y) / (end_x - middle_x);
    end_c = middle_y - (end_k * middle_x);
    end_len = FXSYS_sqrt2(end_x - middle_x, end_y - middle_y);
    end_dc = (FX_FLOAT)FXSYS_fabs(half_width * end_len / (end_x - middle_x));
  }
  if (bStartVert) {
    FX_FLOAT outside_x = start_x;
    if (end_x < start_x) {
      outside_x += half_width;
    } else {
      outside_x -= half_width;
    }
    FX_FLOAT outside_y;
    if (start_y < (end_k * start_x) + end_c) {
      outside_y = (end_k * outside_x) + end_c + end_dc;
    } else {
      outside_y = (end_k * outside_x) + end_c - end_dc;
    }
    rect.UpdateRect(outside_x, outside_y);
    return;
  }
  if (bEndVert) {
    FX_FLOAT outside_x = end_x;
    if (start_x < end_x) {
      outside_x += half_width;
    } else {
      outside_x -= half_width;
    }
    FX_FLOAT outside_y;
    if (end_y < (start_k * end_x) + start_c) {
      outside_y = (start_k * outside_x) + start_c + start_dc;
    } else {
      outside_y = (start_k * outside_x) + start_c - start_dc;
    }
    rect.UpdateRect(outside_x, outside_y);
    return;
  }
  if (FXSYS_fabs(start_k - end_k) < 1.0f / 20) {
    int start_dir = middle_x > start_x ? 1 : -1;
    int end_dir = end_x > middle_x ? 1 : -1;
    if (start_dir == end_dir) {
      _UpdateLineEndPoints(rect, middle_x, middle_y, end_x, end_y, half_width);
    } else {
      _UpdateLineEndPoints(rect, start_x, start_y, middle_x, middle_y,
                           half_width);
    }
    return;
  }
  FX_FLOAT start_outside_c = start_c;
  if (end_y < (start_k * end_x) + start_c) {
    start_outside_c += start_dc;
  } else {
    start_outside_c -= start_dc;
  }
  FX_FLOAT end_outside_c = end_c;
  if (start_y < (end_k * start_x) + end_c) {
    end_outside_c += end_dc;
  } else {
    end_outside_c -= end_dc;
  }
  FX_FLOAT join_x = (end_outside_c - start_outside_c) / (start_k - end_k);
  FX_FLOAT join_y = (start_k * join_x) + start_outside_c;
  rect.UpdateRect(join_x, join_y);
}

CFX_FloatRect CFX_PathData::GetBoundingBox(FX_FLOAT line_width,
                                           FX_FLOAT miter_limit) const {
  CFX_FloatRect rect(100000 * 1.0f, 100000 * 1.0f, -100000 * 1.0f,
                     -100000 * 1.0f);
  size_t iPoint = 0;
  FX_FLOAT half_width = line_width;
  int iStartPoint = 0;
  int iEndPoint = 0;
  int iMiddlePoint = 0;
  bool bJoin;
  while (iPoint < m_Points.size()) {
    if (m_Points[iPoint].IsTypeAndOpen(FXPT_TYPE::MoveTo)) {
      iStartPoint = iPoint + 1;
      iEndPoint = iPoint;
      bJoin = false;
    } else {
      if (m_Points[iPoint].IsTypeAndOpen(FXPT_TYPE::BezierTo)) {
        rect.UpdateRect(m_Points[iPoint].m_PointX, m_Points[iPoint].m_PointY);
        rect.UpdateRect(m_Points[iPoint + 1].m_PointX,
                        m_Points[iPoint + 1].m_PointY);
        iPoint += 2;
      }
      if (iPoint == m_Points.size() - 1 ||
          m_Points[iPoint + 1].IsTypeAndOpen(FXPT_TYPE::MoveTo)) {
        iStartPoint = iPoint - 1;
        iEndPoint = iPoint;
        bJoin = false;
      } else {
        iStartPoint = iPoint - 1;
        iMiddlePoint = iPoint;
        iEndPoint = iPoint + 1;
        bJoin = true;
      }
    }
    FX_FLOAT start_x = m_Points[iStartPoint].m_PointX;
    FX_FLOAT start_y = m_Points[iStartPoint].m_PointY;
    FX_FLOAT end_x = m_Points[iEndPoint].m_PointX;
    FX_FLOAT end_y = m_Points[iEndPoint].m_PointY;
    if (bJoin) {
      FX_FLOAT middle_x = m_Points[iMiddlePoint].m_PointX;
      FX_FLOAT middle_y = m_Points[iMiddlePoint].m_PointY;
      _UpdateLineJoinPoints(rect, start_x, start_y, middle_x, middle_y, end_x,
                            end_y, half_width, miter_limit);
    } else {
      _UpdateLineEndPoints(rect, start_x, start_y, end_x, end_y, half_width);
    }
    iPoint++;
  }
  return rect;
}

void CFX_PathData::Transform(const CFX_Matrix* pMatrix) {
  if (!pMatrix)
    return;
  for (auto& point : m_Points)
    pMatrix->TransformPoint(point.m_PointX, point.m_PointY);
}

bool CFX_PathData::GetZeroAreaPath(const CFX_Matrix* pMatrix,
                                   bool bAdjust,
                                   CFX_PathData* NewPath,
                                   bool* bThin,
                                   bool* setIdentity) const {
  *setIdentity = false;
  if (m_Points.size() < 3)
    return false;

  if (m_Points.size() == 3 && m_Points[0].m_Type == FXPT_TYPE::MoveTo &&
      m_Points[1].m_Type == FXPT_TYPE::LineTo &&
      m_Points[2].m_Type == FXPT_TYPE::LineTo &&
      m_Points[0].m_PointX == m_Points[2].m_PointX &&
      m_Points[0].m_PointY == m_Points[2].m_PointY) {
    for (size_t i = 0; i < 2; i++) {
      FX_FLOAT x = m_Points[i].m_PointX;
      FX_FLOAT y = m_Points[i].m_PointY;
      if (bAdjust) {
        if (pMatrix)
          pMatrix->TransformPoint(x, y);

        x = static_cast<int>(x) + 0.5f;
        y = static_cast<int>(y) + 0.5f;
      }
      NewPath->AppendPoint(x, y, i == 0 ? FXPT_TYPE::MoveTo : FXPT_TYPE::LineTo,
                           false);
    }
    if (bAdjust && pMatrix)
      *setIdentity = true;

    if (m_Points[0].m_PointX != m_Points[1].m_PointX &&
        m_Points[0].m_PointY != m_Points[1].m_PointY) {
      *bThin = true;
    }
    return true;
  }

  if (((m_Points.size() > 3) && (m_Points.size() % 2))) {
    int mid = m_Points.size() / 2;
    bool bZeroArea = false;
    CFX_PathData t_path;
    for (int i = 0; i < mid; i++) {
      if (!(m_Points[mid - i - 1].m_PointX == m_Points[mid + i + 1].m_PointX &&
            m_Points[mid - i - 1].m_PointY == m_Points[mid + i + 1].m_PointY &&
            m_Points[mid - i - 1].m_Type != FXPT_TYPE::BezierTo &&
            m_Points[mid + i + 1].m_Type != FXPT_TYPE::BezierTo)) {
        bZeroArea = true;
        break;
      }

      t_path.AppendPoint(m_Points[mid - i].m_PointX, m_Points[mid - i].m_PointY,
                         FXPT_TYPE::MoveTo, false);
      t_path.AppendPoint(m_Points[mid - i - 1].m_PointX,
                         m_Points[mid - i - 1].m_PointY, FXPT_TYPE::LineTo,
                         false);
    }
    if (!bZeroArea) {
      NewPath->Append(&t_path, nullptr);
      *bThin = true;
      return true;
    }
  }

  int stratPoint = 0;
  int next = 0;
  for (size_t i = 0; i < m_Points.size(); i++) {
    FXPT_TYPE point_type = m_Points[i].m_Type;
    if (point_type == FXPT_TYPE::MoveTo) {
      stratPoint = i;
    } else if (point_type == FXPT_TYPE::LineTo) {
      next = (i + 1 - stratPoint) % (m_Points.size() - stratPoint) + stratPoint;
      if (m_Points[next].m_Type != FXPT_TYPE::BezierTo &&
          m_Points[next].m_Type != FXPT_TYPE::MoveTo) {
        if ((m_Points[i - 1].m_PointX == m_Points[i].m_PointX &&
             m_Points[i].m_PointX == m_Points[next].m_PointX) &&
            ((m_Points[i].m_PointY - m_Points[i - 1].m_PointY) *
                 (m_Points[i].m_PointY - m_Points[next].m_PointY) >
             0)) {
          int pre = i;
          if (FXSYS_fabs(m_Points[i].m_PointY - m_Points[i - 1].m_PointY) <
              FXSYS_fabs(m_Points[i].m_PointY - m_Points[next].m_PointY)) {
            pre--;
            next--;
          }

          NewPath->AppendPoint(m_Points[pre].m_PointX, m_Points[pre].m_PointY,
                               FXPT_TYPE::MoveTo, false);
          NewPath->AppendPoint(m_Points[next].m_PointX, m_Points[next].m_PointY,
                               FXPT_TYPE::LineTo, false);
        } else if ((m_Points[i - 1].m_PointY == m_Points[i].m_PointY &&
                    m_Points[i].m_PointY == m_Points[next].m_PointY) &&
                   ((m_Points[i].m_PointX - m_Points[i - 1].m_PointX) *
                        (m_Points[i].m_PointX - m_Points[next].m_PointX) >
                    0)) {
          int pre = i;
          if (FXSYS_fabs(m_Points[i].m_PointX - m_Points[i - 1].m_PointX) <
              FXSYS_fabs(m_Points[i].m_PointX - m_Points[next].m_PointX)) {
            pre--;
            next--;
          }

          NewPath->AppendPoint(m_Points[pre].m_PointX, m_Points[pre].m_PointY,
                               FXPT_TYPE::MoveTo, false);
          NewPath->AppendPoint(m_Points[next].m_PointX, m_Points[next].m_PointY,
                               FXPT_TYPE::LineTo, false);
        } else if (m_Points[i - 1].m_Type == FXPT_TYPE::MoveTo &&
                   m_Points[next].m_Type == FXPT_TYPE::LineTo &&
                   m_Points[i - 1].m_PointX == m_Points[next].m_PointX &&
                   m_Points[i - 1].m_PointY == m_Points[next].m_PointY &&
                   m_Points[next].m_CloseFigure) {
          NewPath->AppendPoint(m_Points[i - 1].m_PointX,
                               m_Points[i - 1].m_PointY, FXPT_TYPE::MoveTo,
                               false);
          NewPath->AppendPoint(m_Points[i].m_PointX, m_Points[i].m_PointY,
                               FXPT_TYPE::LineTo, false);
          *bThin = true;
        }
      }
    } else if (point_type == FXPT_TYPE::BezierTo) {
      i += 2;
      continue;
    }
  }

  size_t new_path_size = NewPath->GetPoints().size();
  if (m_Points.size() > 3 && new_path_size > 0)
    *bThin = true;
  return new_path_size != 0;
}

bool CFX_PathData::IsRect() const {
  if (m_Points.size() != 5 && m_Points.size() != 4)
    return false;

  if ((m_Points.size() == 5 &&
       (m_Points[0].m_PointX != m_Points[4].m_PointX ||
        m_Points[0].m_PointY != m_Points[4].m_PointY)) ||
      (m_Points[0].m_PointX == m_Points[2].m_PointX &&
       m_Points[0].m_PointY == m_Points[2].m_PointY) ||
      (m_Points[1].m_PointX == m_Points[3].m_PointX &&
       m_Points[1].m_PointY == m_Points[3].m_PointY)) {
    return false;
  }
  if (m_Points[0].m_PointX != m_Points[3].m_PointX &&
      m_Points[0].m_PointY != m_Points[3].m_PointY) {
    return false;
  }
  for (int i = 1; i < 4; i++) {
    if (m_Points[i].m_Type != FXPT_TYPE::LineTo)
      return false;
    if (m_Points[i].m_PointX != m_Points[i - 1].m_PointX &&
        m_Points[i].m_PointY != m_Points[i - 1].m_PointY) {
      return false;
    }
  }
  return m_Points.size() == 5 || m_Points[3].m_CloseFigure;
}

bool CFX_PathData::IsRect(const CFX_Matrix* pMatrix,
                          CFX_FloatRect* pRect) const {
  if (!pMatrix) {
    if (!IsRect())
      return false;

    if (pRect) {
      pRect->left = m_Points[0].m_PointX;
      pRect->right = m_Points[2].m_PointX;
      pRect->bottom = m_Points[0].m_PointY;
      pRect->top = m_Points[2].m_PointY;
      pRect->Normalize();
    }
    return true;
  }

  if (m_Points.size() != 5 && m_Points.size() != 4)
    return false;

  if ((m_Points.size() == 5 &&
       (m_Points[0].m_PointX != m_Points[4].m_PointX ||
        m_Points[0].m_PointY != m_Points[4].m_PointY)) ||
      (m_Points[1].m_PointX == m_Points[3].m_PointX &&
       m_Points[1].m_PointY == m_Points[3].m_PointY)) {
    return false;
  }
  if (m_Points.size() == 4 && m_Points[0].m_PointX != m_Points[3].m_PointX &&
      m_Points[0].m_PointY != m_Points[3].m_PointY) {
    return false;
  }

  CFX_PointF points[5];
  for (size_t i = 0; i < m_Points.size(); i++) {
    points[i] = pMatrix->Transform(
        CFX_PointF(m_Points[i].m_PointX, m_Points[i].m_PointY));

    if (i == 0)
      continue;
    if (m_Points[i].m_Type != FXPT_TYPE::LineTo)
      return false;
    if (points[i].x != points[i - 1].x && points[i].y != points[i - 1].y)
      return false;
  }

  if (pRect) {
    pRect->left = points[0].x;
    pRect->right = points[2].x;
    pRect->bottom = points[0].y;
    pRect->top = points[2].y;
    pRect->Normalize();
  }
  return true;
}
