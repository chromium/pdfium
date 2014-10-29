// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/paint_aggregator.h"

#include <algorithm>

#include "base/logging.h"

// ----------------------------------------------------------------------------
// ALGORITHM NOTES
//
// We attempt to maintain a scroll rect in the presence of invalidations that
// are contained within the scroll rect.  If an invalidation crosses a scroll
// rect, then we just treat the scroll rect as an invalidation rect.
//
// For invalidations performed prior to scrolling and contained within the
// scroll rect, we offset the invalidation rects to account for the fact that
// the consumer will perform scrolling before painting.
//
// We only support scrolling along one axis at a time.  A diagonal scroll will
// therefore be treated as an invalidation.
// ----------------------------------------------------------------------------

PaintAggregator::PaintUpdate::PaintUpdate() {
}

PaintAggregator::PaintUpdate::~PaintUpdate() {
}

PaintAggregator::InternalPaintUpdate::InternalPaintUpdate() :
    synthesized_scroll_damage_rect_(false) {
}

PaintAggregator::InternalPaintUpdate::~InternalPaintUpdate() {
}

pp::Rect PaintAggregator::InternalPaintUpdate::GetScrollDamage() const {
  // Should only be scrolling in one direction at a time.
  DCHECK(!(scroll_delta.x() && scroll_delta.y()));

  pp::Rect damaged_rect;

  // Compute the region we will expose by scrolling, and paint that into a
  // shared memory section.
  if (scroll_delta.x()) {
    int32_t dx = scroll_delta.x();
    damaged_rect.set_y(scroll_rect.y());
    damaged_rect.set_height(scroll_rect.height());
    if (dx > 0) {
      damaged_rect.set_x(scroll_rect.x());
      damaged_rect.set_width(dx);
    } else {
      damaged_rect.set_x(scroll_rect.right() + dx);
      damaged_rect.set_width(-dx);
    }
  } else {
    int32_t dy = scroll_delta.y();
    damaged_rect.set_x(scroll_rect.x());
    damaged_rect.set_width(scroll_rect.width());
    if (dy > 0) {
      damaged_rect.set_y(scroll_rect.y());
      damaged_rect.set_height(dy);
    } else {
      damaged_rect.set_y(scroll_rect.bottom() + dy);
      damaged_rect.set_height(-dy);
    }
  }

  // In case the scroll offset exceeds the width/height of the scroll rect
  return scroll_rect.Intersect(damaged_rect);
}

PaintAggregator::PaintAggregator() {
}

bool PaintAggregator::HasPendingUpdate() const {
  return !update_.scroll_rect.IsEmpty() || !update_.paint_rects.empty();
}

void PaintAggregator::ClearPendingUpdate() {
  update_ = InternalPaintUpdate();
}

PaintAggregator::PaintUpdate PaintAggregator::GetPendingUpdate() {
  // Convert the internal paint update to the external one, which includes a
  // bit more precomputed info for the caller.
  PaintUpdate ret;
  ret.scroll_delta = update_.scroll_delta;
  ret.scroll_rect = update_.scroll_rect;
  ret.has_scroll = ret.scroll_delta.x() != 0 || ret.scroll_delta.y() != 0;

  // Include the scroll damage (if any) in the paint rects.
  // Code invalidates damaged rect here, it pick it up from the list of paint
  // rects in the next block.
  if (ret.has_scroll  && !update_.synthesized_scroll_damage_rect_) {
    update_.synthesized_scroll_damage_rect_ = true;
    pp::Rect scroll_damage = update_.GetScrollDamage();
    InvalidateRectInternal(scroll_damage, false);
  }

  ret.paint_rects.reserve(update_.paint_rects.size() + 1);
  for (size_t i = 0; i < update_.paint_rects.size(); i++)
    ret.paint_rects.push_back(update_.paint_rects[i]);

  return ret;
}

void PaintAggregator::SetIntermediateResults(
    const std::vector<ReadyRect>& ready,
    const std::vector<pp::Rect>& pending) {
  update_.ready_rects.insert(
      update_.ready_rects.end(), ready.begin(), ready.end());
  update_.paint_rects = pending;
}

std::vector<PaintAggregator::ReadyRect> PaintAggregator::GetReadyRects() const {
  return update_.ready_rects;
}

void PaintAggregator::InvalidateRect(const pp::Rect& rect) {
  InvalidateRectInternal(rect, true);
}

void PaintAggregator::ScrollRect(const pp::Rect& clip_rect,
                                 const pp::Point& amount) {
  // We only support scrolling along one axis at a time.
  if (amount.x() != 0 && amount.y() != 0) {
    InvalidateRect(clip_rect);
    return;
  }

  // We can only scroll one rect at a time.
  if (!update_.scroll_rect.IsEmpty() && update_.scroll_rect != clip_rect) {
    InvalidateRect(clip_rect);
    return;
  }

  // Again, we only support scrolling along one axis at a time.  Make sure this
  // update doesn't scroll on a different axis than any existing one.
  if ((amount.x() && update_.scroll_delta.y()) ||
      (amount.y() && update_.scroll_delta.x())) {
    InvalidateRect(clip_rect);
    return;
  }

  // The scroll rect is new or isn't changing (though the scroll amount may
  // be changing).
  update_.scroll_rect = clip_rect;
  update_.scroll_delta += amount;

  // We might have just wiped out a pre-existing scroll.
  if (update_.scroll_delta == pp::Point()) {
    update_.scroll_rect = pp::Rect();
    return;
  }

  // Adjust any paint rects that intersect the scroll. For the portion of the
  // paint that is inside the scroll area, move it by the scroll amount and
  // replace the existing paint with it. For the portion (if any) that is
  // outside the scroll, just invalidate it.
  std::vector<pp::Rect> leftover_rects;
  for (size_t i = 0; i < update_.paint_rects.size(); ++i) {
    if (!update_.scroll_rect.Intersects(update_.paint_rects[i]))
      continue;

    pp::Rect intersection =
        update_.paint_rects[i].Intersect(update_.scroll_rect);
    pp::Rect rect = update_.paint_rects[i];
    while (!rect.IsEmpty()) {
      pp::Rect leftover = rect.Subtract(intersection);
      if (leftover.IsEmpty())
        break;
      // Don't want to call InvalidateRectInternal now since it'll modify
      // update_.paint_rects, so keep track of this and do it below.
      leftover_rects.push_back(leftover);
      rect = rect.Subtract(leftover);
    }

    update_.paint_rects[i] = ScrollPaintRect(intersection, amount);

    // The rect may have been scrolled out of view.
    if (update_.paint_rects[i].IsEmpty()) {
      update_.paint_rects.erase(update_.paint_rects.begin() + i);
      i--;
    }
  }

  for (size_t i = 0; i < leftover_rects.size(); ++i)
    InvalidateRectInternal(leftover_rects[i], false);

  for (size_t i = 0; i < update_.ready_rects.size(); ++i) {
    if (update_.scroll_rect.Contains(update_.ready_rects[i].rect)) {
      update_.ready_rects[i].rect =
          ScrollPaintRect(update_.ready_rects[i].rect, amount);
    }
  }

  if (update_.synthesized_scroll_damage_rect_) {
    pp::Rect damage = update_.GetScrollDamage();
    InvalidateRect(damage);
  }
}

pp::Rect PaintAggregator::ScrollPaintRect(const pp::Rect& paint_rect,
                                          const pp::Point& amount) const {
  pp::Rect result = paint_rect;
  result.Offset(amount);
  result = update_.scroll_rect.Intersect(result);
  return result;
}

void PaintAggregator::InvalidateScrollRect() {
  pp::Rect scroll_rect = update_.scroll_rect;
  update_.scroll_rect = pp::Rect();
  update_.scroll_delta = pp::Point();
  InvalidateRect(scroll_rect);
}

void PaintAggregator::InvalidateRectInternal(const pp::Rect& rect_old,
                                             bool check_scroll) {
  pp::Rect rect = rect_old;
  // Check if any rects that are ready to be painted overlap.
  for (size_t i = 0; i < update_.ready_rects.size(); ++i) {
    const pp::Rect& existing_rect = update_.ready_rects[i].rect;
    if (rect.Intersects(existing_rect)) {
      // Re-invalidate in case the union intersects other paint rects.
      rect = existing_rect.Union(rect);
      update_.ready_rects.erase(update_.ready_rects.begin() + i);
      break;
    }
  }

  bool add_paint = true;

  // Combine overlapping paints using smallest bounding box.
  for (size_t i = 0; i < update_.paint_rects.size(); ++i) {
    const pp::Rect& existing_rect = update_.paint_rects[i];
    if (existing_rect.Contains(rect))  // Optimize out redundancy.
      add_paint = false;
    if (rect.Intersects(existing_rect) || rect.SharesEdgeWith(existing_rect)) {
      // Re-invalidate in case the union intersects other paint rects.
      pp::Rect combined_rect = existing_rect.Union(rect);
      update_.paint_rects.erase(update_.paint_rects.begin() + i);
      InvalidateRectInternal(combined_rect, check_scroll);
      add_paint = false;
    }
  }

  if (add_paint) {
    // Add a non-overlapping paint.
    update_.paint_rects.push_back(rect);
  }

  // If the new paint overlaps with a scroll, then also invalidate the rect in
  // its new position.
  if (check_scroll &&
      !update_.scroll_rect.IsEmpty() &&
      update_.scroll_rect.Intersects(rect)) {
    InvalidateRectInternal(ScrollPaintRect(rect, update_.scroll_delta), false);
  }
}
