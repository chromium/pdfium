// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_path.h"

#include <math.h>

#include <algorithm>
#include <array>
#include <iterator>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_system.h"

namespace {

bool IsRectPreTransform(const std::vector<CFX_Path::Point>& points) {
  if (points.size() != 5 && points.size() != 4) {
    return false;
  }

  if (points.size() == 5 && points[0].point_ != points[4].point_) {
    return false;
  }

  if (points[0].point_ == points[2].point_ ||
      points[1].point_ == points[3].point_) {
    return false;
  }

  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i].type_ != CFX_Path::Point::Type::kLine) {
      return false;
    }
  }
  return true;
}

bool XYBothNotEqual(const CFX_PointF& p1, const CFX_PointF& p2) {
  return p1.x != p2.x && p1.y != p2.y;
}

bool IsRectImpl(const std::vector<CFX_Path::Point>& points) {
  if (!IsRectPreTransform(points)) {
    return false;
  }

  for (int i = 1; i < 4; i++) {
    if (XYBothNotEqual(points[i].point_, points[i - 1].point_)) {
      return false;
    }
  }

  if (XYBothNotEqual(points[0].point_, points[3].point_)) {
    return false;
  }

  return true;
}

CFX_FloatRect CreateRectFromPoints(const CFX_PointF& p1, const CFX_PointF& p2) {
  CFX_FloatRect rect(p1.x, p1.y, p2.x, p2.y);
  rect.Normalize();
  return rect;
}

bool PathPointsNeedNormalization(const std::vector<CFX_Path::Point>& points) {
  return points.size() > 5;
}

std::vector<CFX_Path::Point> GetNormalizedPoints(
    const std::vector<CFX_Path::Point>& points) {
  DCHECK(PathPointsNeedNormalization(points));

  if (points[0].point_ != points.back().point_) {
    return {};
  }

  std::vector<CFX_Path::Point> normalized;
  normalized.reserve(6);
  normalized.push_back(points[0]);
  for (auto it = points.begin() + 1; it != points.end(); ++it) {
    // Exactly 5 points left. Stop normalizing and take what is left.
    if (normalized.size() + std::distance(it, points.end()) == 5) {
      std::copy(it, points.end(), std::back_inserter(normalized));
      break;
    }

    // If the line does not move, skip this point.
    const auto& point = *it;
    if (point.type_ == CFX_Path::Point::Type::kLine && !point.close_figure_ &&
        !normalized.back().close_figure_ &&
        point.point_ == normalized.back().point_) {
      continue;
    }

    normalized.push_back(point);

    // Too many points. Not considered as a rectangle.
    if (normalized.size() > 5) {
      return {};
    }
  }

  DCHECK_EQ(5u, normalized.size());
  return normalized;
}

void UpdateLineEndPoints(CFX_FloatRect* rect,
                         const CFX_PointF& start_pos,
                         const CFX_PointF& end_pos,
                         float hw) {
  if (start_pos.x == end_pos.x) {
    if (start_pos.y == end_pos.y) {
      rect->UpdateRect(end_pos + CFX_PointF(hw, hw));
      rect->UpdateRect(end_pos - CFX_PointF(hw, hw));
      return;
    }

    float point_y;
    if (end_pos.y < start_pos.y) {
      point_y = end_pos.y - hw;
    } else {
      point_y = end_pos.y + hw;
    }

    rect->UpdateRect(CFX_PointF(end_pos.x + hw, point_y));
    rect->UpdateRect(CFX_PointF(end_pos.x - hw, point_y));
    return;
  }

  if (start_pos.y == end_pos.y) {
    float point_x;
    if (end_pos.x < start_pos.x) {
      point_x = end_pos.x - hw;
    } else {
      point_x = end_pos.x + hw;
    }

    rect->UpdateRect(CFX_PointF(point_x, end_pos.y + hw));
    rect->UpdateRect(CFX_PointF(point_x, end_pos.y - hw));
    return;
  }

  CFX_PointF diff = end_pos - start_pos;
  float ll = hypotf(diff.x, diff.y);
  float mx = end_pos.x + hw * diff.x / ll;
  float my = end_pos.y + hw * diff.y / ll;
  float dx1 = hw * diff.y / ll;
  float dy1 = hw * diff.x / ll;
  rect->UpdateRect(CFX_PointF(mx - dx1, my + dy1));
  rect->UpdateRect(CFX_PointF(mx + dx1, my - dy1));
}

void UpdateLineJoinPoints(CFX_FloatRect* rect,
                          const CFX_PointF& start_pos,
                          const CFX_PointF& mid_pos,
                          const CFX_PointF& end_pos,
                          float half_width,
                          float miter_limit) {
  float start_k = 0;
  float start_c = 0;
  float end_k = 0;
  float end_c = 0;
  float start_len = 0;
  float start_dc = 0;
  float end_len = 0;
  float end_dc = 0;
  float one_twentieth = 1.0f / 20;

  bool bStartVert = fabs(start_pos.x - mid_pos.x) < one_twentieth;
  bool bEndVert = fabs(mid_pos.x - end_pos.x) < one_twentieth;
  if (bStartVert && bEndVert) {
    int start_dir = mid_pos.y > start_pos.y ? 1 : -1;
    float point_y = mid_pos.y + half_width * start_dir;
    rect->UpdateRect(CFX_PointF(mid_pos.x + half_width, point_y));
    rect->UpdateRect(CFX_PointF(mid_pos.x - half_width, point_y));
    return;
  }

  if (!bStartVert) {
    CFX_PointF start_to_mid = start_pos - mid_pos;
    start_k = (mid_pos.y - start_pos.y) / (mid_pos.x - start_pos.x);
    start_c = mid_pos.y - (start_k * mid_pos.x);
    start_len = hypotf(start_to_mid.x, start_to_mid.y);
    start_dc = fabsf(half_width * start_len / start_to_mid.x);
  }
  if (!bEndVert) {
    CFX_PointF end_to_mid = end_pos - mid_pos;
    end_k = end_to_mid.y / end_to_mid.x;
    end_c = mid_pos.y - (end_k * mid_pos.x);
    end_len = hypotf(end_to_mid.x, end_to_mid.y);
    end_dc = fabs(half_width * end_len / end_to_mid.x);
  }
  if (bStartVert) {
    CFX_PointF outside(start_pos.x, 0);
    if (end_pos.x < start_pos.x) {
      outside.x += half_width;
    } else {
      outside.x -= half_width;
    }

    if (start_pos.y < (end_k * start_pos.x) + end_c) {
      outside.y = (end_k * outside.x) + end_c + end_dc;
    } else {
      outside.y = (end_k * outside.x) + end_c - end_dc;
    }

    rect->UpdateRect(outside);
    return;
  }

  if (bEndVert) {
    CFX_PointF outside(end_pos.x, 0);
    if (start_pos.x < end_pos.x) {
      outside.x += half_width;
    } else {
      outside.x -= half_width;
    }

    if (end_pos.y < (start_k * end_pos.x) + start_c) {
      outside.y = (start_k * outside.x) + start_c + start_dc;
    } else {
      outside.y = (start_k * outside.x) + start_c - start_dc;
    }

    rect->UpdateRect(outside);
    return;
  }

  if (fabs(start_k - end_k) < one_twentieth) {
    int start_dir = mid_pos.x > start_pos.x ? 1 : -1;
    int end_dir = end_pos.x > mid_pos.x ? 1 : -1;
    if (start_dir == end_dir) {
      UpdateLineEndPoints(rect, mid_pos, end_pos, half_width);
    } else {
      UpdateLineEndPoints(rect, start_pos, mid_pos, half_width);
    }
    return;
  }

  float start_outside_c = start_c;
  if (end_pos.y < (start_k * end_pos.x) + start_c) {
    start_outside_c += start_dc;
  } else {
    start_outside_c -= start_dc;
  }

  float end_outside_c = end_c;
  if (start_pos.y < (end_k * start_pos.x) + end_c) {
    end_outside_c += end_dc;
  } else {
    end_outside_c -= end_dc;
  }

  float join_x = (end_outside_c - start_outside_c) / (start_k - end_k);
  float join_y = start_k * join_x + start_outside_c;
  rect->UpdateRect(CFX_PointF(join_x, join_y));
}

}  // namespace

CFX_Path::Point::Point() = default;

CFX_Path::Point::Point(const CFX_PointF& point, Type type, bool close)
    : point_(point), type_(type), close_figure_(close) {}

CFX_Path::Point::Point(const Point& other) = default;

CFX_Path::Point::~Point() = default;

CFX_Path::CFX_Path() = default;

CFX_Path::CFX_Path(const CFX_Path& src) = default;

CFX_Path::CFX_Path(CFX_Path&& src) noexcept = default;

CFX_Path::~CFX_Path() = default;

void CFX_Path::Clear() {
  points_.clear();
}

void CFX_Path::ClosePath() {
  if (points_.empty()) {
    return;
  }
  points_.back().close_figure_ = true;
}

void CFX_Path::Append(const CFX_Path& src, const CFX_Matrix* matrix) {
  if (src.points_.empty()) {
    return;
  }

  size_t cur_size = points_.size();
  points_.insert(points_.end(), src.points_.begin(), src.points_.end());

  if (!matrix) {
    return;
  }

  for (size_t i = cur_size; i < points_.size(); i++) {
    points_[i].point_ = matrix->Transform(points_[i].point_);
  }
}

void CFX_Path::AppendPoint(const CFX_PointF& point, Point::Type type) {
  points_.emplace_back(point, type, /*close=*/false);
}

void CFX_Path::AppendPointAndClose(const CFX_PointF& point, Point::Type type) {
  points_.emplace_back(point, type, /*close=*/true);
}

void CFX_Path::AppendLine(const CFX_PointF& pt1, const CFX_PointF& pt2) {
  if (points_.empty() || fabs(points_.back().point_.x - pt1.x) > 0.001 ||
      fabs(points_.back().point_.y - pt1.y) > 0.001) {
    AppendPoint(pt1, CFX_Path::Point::Type::kMove);
  }
  AppendPoint(pt2, CFX_Path::Point::Type::kLine);
}

void CFX_Path::AppendFloatRect(const CFX_FloatRect& rect) {
  return AppendRect(rect.left, rect.bottom, rect.right, rect.top);
}

void CFX_Path::AppendRect(float left, float bottom, float right, float top) {
  CFX_PointF left_bottom(left, bottom);
  CFX_PointF left_top(left, top);
  CFX_PointF right_top(right, top);
  CFX_PointF right_bottom(right, bottom);

  AppendLine(left_bottom, left_top);
  AppendLine(left_top, right_top);
  AppendLine(right_top, right_bottom);
  AppendLine(right_bottom, left_bottom);
  ClosePath();
}

CFX_FloatRect CFX_Path::GetBoundingBox() const {
  if (points_.empty()) {
    return CFX_FloatRect();
  }

  CFX_FloatRect rect(points_[0].point_);
  for (size_t i = 1; i < points_.size(); ++i) {
    rect.UpdateRect(points_[i].point_);
  }
  return rect;
}

CFX_FloatRect CFX_Path::GetBoundingBoxForStrokePath(float line_width,
                                                    float miter_limit) const {
  CFX_FloatRect rect(100000.0f, 100000.0f, -100000.0f, -100000.0f);
  size_t iPoint = 0;
  float half_width = line_width;
  size_t iStartPoint = 0;
  size_t iEndPoint = 0;
  size_t iMiddlePoint = 0;
  bool bJoin;
  while (iPoint < points_.size()) {
    if (points_[iPoint].type_ == CFX_Path::Point::Type::kMove) {
      if (iPoint + 1 == points_.size()) {
        if (points_[iPoint].close_figure_) {
          // Update `rect` right away since this is the final point to be drawn.
          rect.UpdateRect(points_[iPoint].point_);
        }
        break;
      }

      iStartPoint = iPoint + 1;
      iEndPoint = iPoint;
      bJoin = false;
    } else {
      if (points_[iPoint].IsTypeAndOpen(CFX_Path::Point::Type::kBezier)) {
        // Callers are responsible for adding Beziers in sets of 3.
        CHECK_LT(iPoint + 2, points_.size());
        DCHECK_EQ(points_[iPoint + 1].type_, CFX_Path::Point::Type::kBezier);
        DCHECK_EQ(points_[iPoint + 2].type_, CFX_Path::Point::Type::kBezier);
        rect.UpdateRect(points_[iPoint].point_);
        rect.UpdateRect(points_[iPoint + 1].point_);
        iPoint += 2;
      }
      if (iPoint + 1 == points_.size() ||
          points_[iPoint + 1].type_ == CFX_Path::Point::Type::kMove) {
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
    CHECK_LT(iStartPoint, points_.size());
    CHECK_LT(iEndPoint, points_.size());
    if (bJoin) {
      CHECK_LT(iMiddlePoint, points_.size());
      UpdateLineJoinPoints(&rect, points_[iStartPoint].point_,
                           points_[iMiddlePoint].point_,
                           points_[iEndPoint].point_, half_width, miter_limit);
    } else {
      UpdateLineEndPoints(&rect, points_[iStartPoint].point_,
                          points_[iEndPoint].point_, half_width);
    }
    ++iPoint;
  }
  return rect;
}

void CFX_Path::Transform(const CFX_Matrix& matrix) {
  for (auto& point : points_) {
    point.point_ = matrix.Transform(point.point_);
  }
}

bool CFX_Path::IsRect() const {
  if (PathPointsNeedNormalization(points_)) {
    return IsRectImpl(GetNormalizedPoints(points_));
  }
  return IsRectImpl(points_);
}

std::optional<CFX_FloatRect> CFX_Path::GetRect(const CFX_Matrix* matrix) const {
  bool do_normalize = PathPointsNeedNormalization(points_);
  std::vector<Point> normalized;
  if (do_normalize) {
    normalized = GetNormalizedPoints(points_);
  }
  const std::vector<Point>& path_points = do_normalize ? normalized : points_;

  if (!matrix) {
    if (!IsRectImpl(path_points)) {
      return std::nullopt;
    }

    return CreateRectFromPoints(path_points[0].point_, path_points[2].point_);
  }

  if (!IsRectPreTransform(path_points)) {
    return std::nullopt;
  }

  std::array<CFX_PointF, 5> points;
  for (size_t i = 0; i < path_points.size(); ++i) {
    points[i] = matrix->Transform(path_points[i].point_);

    if (i == 0) {
      continue;
    }
    if (XYBothNotEqual(points[i], points[i - 1])) {
      return std::nullopt;
    }
  }

  if (XYBothNotEqual(points[0], points[3])) {
    return std::nullopt;
  }

  return CreateRectFromPoints(points[0], points[2]);
}

CFX_RetainablePath::CFX_RetainablePath() = default;

// Note: can't default the copy constructor since Retainable<> has a deleted
// copy constructor (as it should). Instead, we want the default Retainable<>
// constructor to be invoked so as to create a copy with a ref-count of 1 as
// of the time it is created, then populate the remainder of the members from
// the |src| object.
CFX_RetainablePath::CFX_RetainablePath(const CFX_RetainablePath& src)
    : CFX_Path(src) {}

CFX_RetainablePath::~CFX_RetainablePath() = default;

RetainPtr<CFX_RetainablePath> CFX_RetainablePath::Clone() const {
  return pdfium::MakeRetain<CFX_RetainablePath>(*this);
}
