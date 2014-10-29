// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/thumbnail_control.h"

#include <algorithm>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "pdf/draw_utils.h"
#include "pdf/number_image_generator.h"

namespace chrome_pdf {

const int kLeftBorderSize = 52;
const int kBorderSize = 12;
const int kHighlightBorderSize = 2;

const uint32 kLeftColor = 0x003F537B;
const uint32 kRightColor = 0x990D1626;

const uint32 kTopHighlightColor = 0xFF426DC9;
const uint32 kBottomHighlightColor = 0xFF6391DE;
const uint32 kThumbnailBackgroundColor = 0xFF000000;

const uint32 kSlidingTimeoutMs = 50;
const int32 kSlidingShift = 50;

const double kNonSelectedThumbnailAlpha = 0.91;

ThumbnailControl::ThumbnailControl()
    : engine_(NULL), sliding_width_(0), sliding_shift_(kSlidingShift),
      sliding_timeout_(kSlidingTimeoutMs), sliding_timer_id_(0) {
}

ThumbnailControl::~ThumbnailControl() {
  ClearCache();
}

bool ThumbnailControl::CreateThumbnailControl(
    uint32 id, const pp::Rect& rc,
    bool visible, Owner* owner, PDFEngine* engine,
    NumberImageGenerator* number_image_generator) {
  engine_ = engine;
  number_image_generator_ = number_image_generator;
  sliding_width_ = rc.width();

  return Control::Create(id, rc, visible, owner);
}

void ThumbnailControl::SetPosition(int position, int total, bool invalidate) {
  visible_rect_ = pp::Rect();
  visible_pages_.clear();

  if (rect().width() < kLeftBorderSize + kBorderSize) {
    return;  // control is too narrow to show thumbnails.
  }

  int num_pages = engine_->GetNumberOfPages();

  int max_doc_width = 0, total_doc_height = 0;
  std::vector<pp::Rect> page_sizes(num_pages);
  for (int i = 0; i < num_pages; ++i) {
    page_sizes[i] = engine_->GetPageRect(i);
    max_doc_width = std::max(max_doc_width, page_sizes[i].width());
    total_doc_height += page_sizes[i].height();
  }

  if (!max_doc_width)
    return;

  int max_thumbnail_width = rect().width() - kLeftBorderSize - kBorderSize;
  double thumbnail_ratio =
      max_thumbnail_width / static_cast<double>(max_doc_width);

  int total_thumbnail_height = 0;
  for (int i = 0; i < num_pages; ++i) {
    total_thumbnail_height += kBorderSize;
    int thumbnail_width =
        static_cast<int>(page_sizes[i].width() * thumbnail_ratio);
    int thumbnail_height =
        static_cast<int>(page_sizes[i].height() * thumbnail_ratio);
    int x = (max_thumbnail_width - thumbnail_width) / 2;
    page_sizes[i] =
        pp::Rect(x, total_thumbnail_height, thumbnail_width, thumbnail_height);
    total_thumbnail_height += thumbnail_height;
  }
  total_thumbnail_height += kBorderSize;

  int visible_y = 0;
  if (total > 0) {
    double range = total_thumbnail_height - rect().height();
    if (range < 0)
      range = 0;
    visible_y = static_cast<int>(range * position / total);
  }
  visible_rect_ = pp::Rect(0, visible_y, max_thumbnail_width, rect().height());

  for (int i = 0; i < num_pages; ++i) {
    if (page_sizes[i].Intersects(visible_rect_)) {
      PageInfo page_info;
      page_info.index = i;
      page_info.rect = page_sizes[i];
      page_info.rect.Offset(kLeftBorderSize, -visible_rect_.y());
      visible_pages_.push_back(page_info);
    }
  }

  if (invalidate)
    owner()->Invalidate(id(), rect());
}

void ThumbnailControl::Show(bool visible, bool invalidate) {
  if (!visible || invalidate)
    ClearCache();
  sliding_width_ = rect().width();
  Control::Show(visible, invalidate);
}

void ThumbnailControl::SlideIn() {
  if (visible())
    return;

  Show(true, false);
  sliding_width_ = 0;
  sliding_shift_ = kSlidingShift;

  sliding_timer_id_ = owner()->ScheduleTimer(id(), sliding_timeout_);
  owner()->Invalidate(id(), rect());
}

void ThumbnailControl::SlideOut() {
  if (!visible())
    return;
  sliding_shift_ = -kSlidingShift;
  sliding_timer_id_ = owner()->ScheduleTimer(id(), sliding_timeout_);
}

void ThumbnailControl::Paint(pp::ImageData* image_data, const pp::Rect& rc) {
  if (!visible())
    return;

  pp::Rect control_rc(rect());
  control_rc.Offset(control_rc.width() - sliding_width_, 0);
  control_rc.set_width(sliding_width_);

  pp::Rect draw_rc = rc.Intersect(control_rc);
  if (draw_rc.IsEmpty())
    return;

  pp::Rect gradient_rc(control_rc.x(), draw_rc.y(),
                       control_rc.width(), draw_rc.height());
  GradientFill(owner()->GetInstance(),
               image_data,
               draw_rc,
               gradient_rc,
               kLeftColor,
               kRightColor,
               true,
               transparency());

  int selected_page = engine_->GetMostVisiblePage();
  for (size_t i = 0; i < visible_pages_.size(); ++i) {
    pp::Rect page_rc = visible_pages_[i].rect;
    page_rc.Offset(control_rc.point());

    if (visible_pages_[i].index == selected_page) {
      pp::Rect highlight_rc = page_rc;
      highlight_rc.Inset(-kHighlightBorderSize, -kHighlightBorderSize);
      GradientFill(owner()->GetInstance(),
                   image_data,
                   draw_rc,
                   highlight_rc,
                   kTopHighlightColor,
                   kBottomHighlightColor,
                   false,
                   transparency());
    }

    pp::Rect draw_page_rc = page_rc.Intersect(draw_rc);
    if (draw_page_rc.IsEmpty())
      continue;

    // First search page image in the cache.
    pp::ImageData* thumbnail = NULL;
    std::map<int, pp::ImageData*>::iterator it =
        image_cache_.find(visible_pages_[i].index);
    if (it != image_cache_.end()) {
      if (it->second->size() == page_rc.size())
        thumbnail = image_cache_[visible_pages_[i].index];
      else
        image_cache_.erase(it);
    }

    // If page is not found in the cache, create new one.
    if (thumbnail == NULL) {
      thumbnail = new pp::ImageData(owner()->GetInstance(),
                                    PP_IMAGEDATAFORMAT_BGRA_PREMUL,
                                    page_rc.size(),
                                    false);
      engine_->PaintThumbnail(thumbnail, visible_pages_[i].index);

      pp::ImageData page_number;
      number_image_generator_->GenerateImage(
          visible_pages_[i].index + 1, &page_number);
      pp::Point origin(
          (thumbnail->size().width() - page_number.size().width()) / 2,
          (thumbnail->size().height() - page_number.size().height()) / 2);

      if (origin.x() > 0 && origin.y() > 0) {
        AlphaBlend(page_number, pp::Rect(pp::Point(), page_number.size()),
                   thumbnail, origin, kOpaqueAlpha);
      }

      image_cache_[visible_pages_[i].index] = thumbnail;
    }

    uint8 alpha = transparency();
    if (visible_pages_[i].index != selected_page)
      alpha = static_cast<uint8>(alpha * kNonSelectedThumbnailAlpha);
    FillRect(image_data, draw_page_rc, kThumbnailBackgroundColor);
    draw_page_rc.Offset(-page_rc.x(), -page_rc.y());
    AlphaBlend(*thumbnail, draw_page_rc, image_data,
        draw_page_rc.point() + page_rc.point(), alpha);
  }
}

bool ThumbnailControl::HandleEvent(const pp::InputEvent& event) {
  if (!visible())
    return false;

  pp::MouseInputEvent mouse_event(event);
  if (mouse_event.is_null())
    return false;
  pp::Point pt = mouse_event.GetPosition();
  if (!rect().Contains(pt))
    return false;

  int over_page = -1;
  for (size_t i = 0; i < visible_pages_.size(); ++i) {
    pp::Rect page_rc = visible_pages_[i].rect;
    page_rc.Offset(rect().point());
    if (page_rc.Contains(pt)) {
      over_page = i;
      break;
    }
  }

  bool handled = false;
  switch (event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
      owner()->SetCursor(id(),
          over_page == -1 ? PP_CURSORTYPE_POINTER : PP_CURSORTYPE_HAND);
      break;
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
      if (over_page != -1) {
        owner()->Invalidate(id(), rect());
        owner()->OnEvent(id(), EVENT_ID_THUMBNAIL_SELECTED,
                         &visible_pages_[over_page].index);
      }
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

void ThumbnailControl::OnTimerFired(uint32 timer_id) {
  if (timer_id == sliding_timer_id_) {
    sliding_width_ += sliding_shift_;
    if (sliding_width_ <= 0) {
      // We completely slided out. Make control invisible now.
      Show(false, false);
    } else if (sliding_width_ >= rect().width()) {
      // We completely slided in. Make sliding width to full control width.
      sliding_width_ = rect().width();
    } else {
      // We have not completed sliding yet. Keep sliding.
      sliding_timer_id_ = owner()->ScheduleTimer(id(), sliding_timeout_);
    }
    owner()->Invalidate(id(), rect());
  }
}

void ThumbnailControl::ResetEngine(PDFEngine* engine) {
  engine_ = engine;
  ClearCache();
}

void ThumbnailControl::ClearCache() {
  std::map<int, pp::ImageData*>::iterator it;
  for (it = image_cache_.begin(); it != image_cache_.end(); ++it) {
    delete it->second;
  }
  image_cache_.clear();
}

}  // namespace chrome_pdf
