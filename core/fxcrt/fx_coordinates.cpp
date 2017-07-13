// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_coordinates.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/fx_extension.h"

namespace {

void MatchFloatRange(float f1, float f2, int* i1, int* i2) {
  int length = static_cast<int>(ceil(f2 - f1));
  int i1_1 = static_cast<int>(floor(f1));
  int i1_2 = static_cast<int>(ceil(f1));
  float error1 = f1 - i1_1 + (float)fabs(f2 - i1_1 - length);
  float error2 = i1_2 - f1 + (float)fabs(f2 - i1_2 - length);

  *i1 = (error1 > error2) ? i1_2 : i1_1;
  *i2 = *i1 + length;
}

}  // namespace

void FX_RECT::Normalize() {
  if (left > right)
    std::swap(left, right);
  if (top > bottom)
    std::swap(top, bottom);
}

void FX_RECT::Intersect(const FX_RECT& src) {
  FX_RECT src_n = src;
  src_n.Normalize();
  Normalize();
  left = std::max(left, src_n.left);
  top = std::max(top, src_n.top);
  right = std::min(right, src_n.right);
  bottom = std::min(bottom, src_n.bottom);
  if (left > right || top > bottom) {
    left = top = right = bottom = 0;
  }
}

CFX_FloatRect::CFX_FloatRect(const FX_RECT& rect) {
  left = static_cast<float>(rect.left);
  top = static_cast<float>(rect.bottom);
  right = static_cast<float>(rect.right);
  bottom = static_cast<float>(rect.top);
}

void CFX_FloatRect::Normalize() {
  if (left > right)
    std::swap(left, right);
  if (bottom > top)
    std::swap(top, bottom);
}

void CFX_FloatRect::Intersect(const CFX_FloatRect& other_rect) {
  Normalize();
  CFX_FloatRect other = other_rect;
  other.Normalize();
  left = std::max(left, other.left);
  bottom = std::max(bottom, other.bottom);
  right = std::min(right, other.right);
  top = std::min(top, other.top);
  if (left > right || bottom > top) {
    left = bottom = right = top = 0;
  }
}

void CFX_FloatRect::Union(const CFX_FloatRect& other_rect) {
  Normalize();
  CFX_FloatRect other = other_rect;
  other.Normalize();
  left = std::min(left, other.left);
  bottom = std::min(bottom, other.bottom);
  right = std::max(right, other.right);
  top = std::max(top, other.top);
}

int CFX_FloatRect::Substract4(CFX_FloatRect& s, CFX_FloatRect* pRects) {
  Normalize();
  s.Normalize();
  int nRects = 0;
  CFX_FloatRect rects[4];
  if (left < s.left) {
    rects[nRects].left = left;
    rects[nRects].bottom = bottom;
    rects[nRects].right = s.left;
    rects[nRects].top = top;
    nRects++;
  }
  if (s.left < right && s.top < top) {
    rects[nRects].left = s.left;
    rects[nRects].bottom = s.top;
    rects[nRects].right = right;
    rects[nRects].top = top;
    nRects++;
  }
  if (s.top > bottom && s.right < right) {
    rects[nRects].left = s.right;
    rects[nRects].bottom = bottom;
    rects[nRects].right = right;
    rects[nRects].top = s.top;
    nRects++;
  }
  if (s.bottom > bottom) {
    rects[nRects].left = s.left;
    rects[nRects].bottom = bottom;
    rects[nRects].right = s.right;
    rects[nRects].top = s.bottom;
    nRects++;
  }
  if (nRects == 0)
    return 0;

  for (int i = 0; i < nRects; i++) {
    pRects[i] = rects[i];
    pRects[i].Intersect(*this);
  }
  return nRects;
}

FX_RECT CFX_FloatRect::GetOuterRect() const {
  CFX_FloatRect rect1 = *this;
  FX_RECT rect;
  rect.left = static_cast<int>(floor(rect1.left));
  rect.bottom = static_cast<int>(ceil(rect1.top));
  rect.right = static_cast<int>(ceil(rect1.right));
  rect.top = static_cast<int>(floor(rect1.bottom));
  rect.Normalize();
  return rect;
}

FX_RECT CFX_FloatRect::GetInnerRect() const {
  CFX_FloatRect rect1 = *this;
  FX_RECT rect;
  rect.left = static_cast<int>(ceil(rect1.left));
  rect.bottom = static_cast<int>(floor(rect1.top));
  rect.right = static_cast<int>(floor(rect1.right));
  rect.top = static_cast<int>(ceil(rect1.bottom));
  rect.Normalize();
  return rect;
}

FX_RECT CFX_FloatRect::GetClosestRect() const {
  CFX_FloatRect rect1 = *this;
  FX_RECT rect;
  MatchFloatRange(rect1.left, rect1.right, &rect.left, &rect.right);
  MatchFloatRange(rect1.bottom, rect1.top, &rect.top, &rect.bottom);
  rect.Normalize();
  return rect;
}

CFX_FloatRect CFX_FloatRect::GetCenterSquare() const {
  float fWidth = right - left;
  float fHeight = top - bottom;
  float fHalfWidth = (fWidth > fHeight) ? fHeight / 2 : fWidth / 2;

  float fCenterX = (left + right) / 2.0f;
  float fCenterY = (top + bottom) / 2.0f;
  return CFX_FloatRect(fCenterX - fHalfWidth, fCenterY - fHalfWidth,
                       fCenterX + fHalfWidth, fCenterY + fHalfWidth);
}

bool CFX_FloatRect::Contains(const CFX_PointF& point) const {
  CFX_FloatRect n1(*this);
  n1.Normalize();
  return point.x <= n1.right && point.x >= n1.left && point.y <= n1.top &&
         point.y >= n1.bottom;
}

bool CFX_FloatRect::Contains(const CFX_FloatRect& other_rect) const {
  CFX_FloatRect n1(*this);
  CFX_FloatRect n2(other_rect);
  n1.Normalize();
  n2.Normalize();
  return n2.left >= n1.left && n2.right <= n1.right && n2.bottom >= n1.bottom &&
         n2.top <= n1.top;
}

void CFX_FloatRect::UpdateRect(float x, float y) {
  left = std::min(left, x);
  bottom = std::min(bottom, y);
  right = std::max(right, x);
  top = std::max(top, y);
}

CFX_FloatRect CFX_FloatRect::GetBBox(const CFX_PointF* pPoints, int nPoints) {
  if (nPoints == 0)
    return CFX_FloatRect();

  float min_x = pPoints->x;
  float max_x = pPoints->x;
  float min_y = pPoints->y;
  float max_y = pPoints->y;
  for (int i = 1; i < nPoints; i++) {
    min_x = std::min(min_x, pPoints[i].x);
    max_x = std::max(max_x, pPoints[i].x);
    min_y = std::min(min_y, pPoints[i].y);
    max_y = std::max(max_y, pPoints[i].y);
  }
  return CFX_FloatRect(min_x, min_y, max_x, max_y);
}

CFX_Matrix CFX_Matrix::GetInverse() const {
  CFX_Matrix inverse;
  float i = a * d - b * c;
  if (fabs(i) == 0)
    return inverse;

  float j = -i;
  inverse.a = d / i;
  inverse.b = b / j;
  inverse.c = c / j;
  inverse.d = a / i;
  inverse.e = (c * f - d * e) / i;
  inverse.f = (a * f - b * e) / j;
  return inverse;
}

void CFX_Matrix::Concat(const CFX_Matrix& m, bool bPrepended) {
  ConcatInternal(m, bPrepended);
}

void CFX_Matrix::ConcatInverse(const CFX_Matrix& src, bool bPrepended) {
  Concat(src.GetInverse(), bPrepended);
}

bool CFX_Matrix::Is90Rotated() const {
  return fabs(a * 1000) < fabs(b) && fabs(d * 1000) < fabs(c);
}

bool CFX_Matrix::IsScaled() const {
  return fabs(b * 1000) < fabs(a) && fabs(c * 1000) < fabs(d);
}

void CFX_Matrix::Translate(float x, float y, bool bPrepended) {
  if (bPrepended) {
    e += x * a + y * c;
    f += y * d + x * b;
    return;
  }
  e += x;
  f += y;
}

void CFX_Matrix::Scale(float sx, float sy, bool bPrepended) {
  a *= sx;
  d *= sy;
  if (bPrepended) {
    b *= sx;
    c *= sy;
    return;
  }

  b *= sy;
  c *= sx;
  e *= sx;
  f *= sy;
}

void CFX_Matrix::Rotate(float fRadian, bool bPrepended) {
  float cosValue = cos(fRadian);
  float sinValue = sin(fRadian);
  ConcatInternal(CFX_Matrix(cosValue, sinValue, -sinValue, cosValue, 0, 0),
                 bPrepended);
}

void CFX_Matrix::RotateAt(float fRadian, float dx, float dy, bool bPrepended) {
  Translate(dx, dy, bPrepended);
  Rotate(fRadian, bPrepended);
  Translate(-dx, -dy, bPrepended);
}

void CFX_Matrix::Shear(float fAlphaRadian, float fBetaRadian, bool bPrepended) {
  ConcatInternal(CFX_Matrix(1, tan(fAlphaRadian), tan(fBetaRadian), 1, 0, 0),
                 bPrepended);
}

void CFX_Matrix::MatchRect(const CFX_FloatRect& dest,
                           const CFX_FloatRect& src) {
  float fDiff = src.left - src.right;
  a = fabs(fDiff) < 0.001f ? 1 : (dest.left - dest.right) / fDiff;

  fDiff = src.bottom - src.top;
  d = fabs(fDiff) < 0.001f ? 1 : (dest.bottom - dest.top) / fDiff;
  e = dest.left - src.left * a;
  f = dest.bottom - src.bottom * d;
  b = 0;
  c = 0;
}

float CFX_Matrix::GetXUnit() const {
  if (b == 0)
    return (a > 0 ? a : -a);
  if (a == 0)
    return (b > 0 ? b : -b);
  return sqrt(a * a + b * b);
}

float CFX_Matrix::GetYUnit() const {
  if (c == 0)
    return (d > 0 ? d : -d);
  if (d == 0)
    return (c > 0 ? c : -c);
  return sqrt(c * c + d * d);
}

CFX_FloatRect CFX_Matrix::GetUnitRect() const {
  CFX_FloatRect rect(0, 0, 1, 1);
  TransformRect(rect);
  return rect;
}

float CFX_Matrix::TransformXDistance(float dx) const {
  float fx = a * dx;
  float fy = b * dx;
  return sqrt(fx * fx + fy * fy);
}

float CFX_Matrix::TransformDistance(float dx, float dy) const {
  float fx = a * dx + c * dy;
  float fy = b * dx + d * dy;
  return sqrt(fx * fx + fy * fy);
}

float CFX_Matrix::TransformDistance(float distance) const {
  return distance * (GetXUnit() + GetYUnit()) / 2;
}

CFX_PointF CFX_Matrix::Transform(const CFX_PointF& point) const {
  return CFX_PointF(a * point.x + c * point.y + e,
                    b * point.x + d * point.y + f);
}

void CFX_Matrix::TransformRect(CFX_RectF& rect) const {
  float right = rect.right(), bottom = rect.bottom();
  TransformRect(rect.left, right, bottom, rect.top);
  rect.width = right - rect.left;
  rect.height = bottom - rect.top;
}

void CFX_Matrix::TransformRect(float& left,
                               float& right,
                               float& top,
                               float& bottom) const {
  CFX_PointF points[] = {
      {left, top}, {left, bottom}, {right, top}, {right, bottom}};
  for (int i = 0; i < 4; i++)
    points[i] = Transform(points[i]);

  right = points[0].x;
  left = points[0].x;
  top = points[0].y;
  bottom = points[0].y;
  for (int i = 1; i < 4; i++) {
    right = std::max(right, points[i].x);
    left = std::min(left, points[i].x);
    top = std::max(top, points[i].y);
    bottom = std::min(bottom, points[i].y);
  }
}

void CFX_Matrix::ConcatInternal(const CFX_Matrix& other, bool prepend) {
  CFX_Matrix left;
  CFX_Matrix right;
  if (prepend) {
    left = other;
    right = *this;
  } else {
    left = *this;
    right = other;
  }

  a = left.a * right.a + left.b * right.c;
  b = left.a * right.b + left.b * right.d;
  c = left.c * right.a + left.d * right.c;
  d = left.c * right.b + left.d * right.d;
  e = left.e * right.a + left.f * right.c + right.e;
  f = left.e * right.b + left.f * right.d + right.f;
}
