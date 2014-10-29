// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_THUMBNAIL_CONTROL_H_
#define PDF_THUMBNAIL_CONTROL_H_

#include <map>
#include <vector>

#include "pdf/control.h"
#include "pdf/pdf_engine.h"
#include "ppapi/cpp/input_event.h"

namespace chrome_pdf {

class NumberImageGenerator;

class ThumbnailControl : public Control {
 public:
  enum ThumbnailEventIds {
    EVENT_ID_THUMBNAIL_SELECTED = 100,
  };

  explicit ThumbnailControl();
  virtual ~ThumbnailControl();

  // Sets current position of the thumnail control.
  void SetPosition(int position, int total, bool invalidate);
  void SlideIn();
  void SlideOut();

  virtual bool CreateThumbnailControl(
      uint32 id, const pp::Rect& rc,
      bool visible, Owner* owner, PDFEngine* engine,
      NumberImageGenerator* number_image_generator);

  // Control interface.
  virtual void Show(bool visible, bool invalidate);
  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc);
  virtual bool HandleEvent(const pp::InputEvent& event);
  virtual void OnTimerFired(uint32 timer_id);

  virtual void ResetEngine(PDFEngine* engine);

 private:
  void ClearCache();

  struct PageInfo {
    int index;
    pp::Rect rect;
  };

  PDFEngine* engine_;
  pp::Rect visible_rect_;
  std::vector<PageInfo> visible_pages_;
  std::map<int, pp::ImageData*> image_cache_;
  int sliding_width_;
  int sliding_shift_;
  int sliding_timeout_;
  uint32 sliding_timer_id_;
  NumberImageGenerator* number_image_generator_;
};

}  // namespace chrome_pdf

#endif  // PDF_THUMBNAIL_CONTROL_H_
