// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_PROGRESS_CONTROL_H_
#define PDF_PROGRESS_CONTROL_H_

#include <string>
#include <vector>

#include "pdf/control.h"
#include "pdf/fading_control.h"
#include "ppapi/cpp/image_data.h"

namespace chrome_pdf {

class ProgressControl : public FadingControl {
 public:
  static const double kCompleted;

  enum ProgressEventIds {
    EVENT_ID_PROGRESS_COMPLETED,
  };

  ProgressControl();
  virtual ~ProgressControl();
  virtual bool CreateProgressControl(uint32 id,
                                     bool visible,
                                     Control::Owner* delegate,
                                     double progress,
                                     float device_scale,
                                     const std::vector<pp::ImageData>& images,
                                     const pp::ImageData& background,
                                     const std::string& text);
  void Reconfigure(const pp::ImageData& background,
                   const std::vector<pp::ImageData>& images,
                   float device_scale);

  static void CalculateLayout(pp::Instance* instance,
                              const std::vector<pp::ImageData>& images,
                              const pp::ImageData& background,
                              const std::string& text,
                              float device_scale,
                              pp::Size* ctrl_size,
                              pp::Rect* image_rc,
                              pp::Rect* text_rc);

  // Control interface.
  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc);

  // ProgressControl interface
  // Set progress indicator in percents from 0% to 100%.
  virtual void SetProgress(double progress);

 private:
  void PrepareBackground();
  void AdjustBackground();
  size_t GetImageIngex() const;

  double progress_;
  float device_scale_;
  std::vector<pp::ImageData> images_;
  pp::ImageData background_;
  pp::ImageData ctrl_background_;
  std::string text_;
  pp::Rect image_rc_;
  pp::Rect text_rc_;
};

}  // namespace chrome_pdf

#endif  // PDF_PROGRESS_CONTROL_H_
