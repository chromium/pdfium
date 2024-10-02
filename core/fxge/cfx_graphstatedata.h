// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_GRAPHSTATEDATA_H_
#define CORE_FXGE_CFX_GRAPHSTATEDATA_H_

#include <stdint.h>

#include <vector>

#include "core/fxcrt/retain_ptr.h"

class CFX_GraphStateData {
 public:
  enum class LineCap : uint8_t { kButt = 0, kRound = 1, kSquare = 2 };

  enum class LineJoin : uint8_t { kMiter = 0, kRound = 1, kBevel = 2 };

  CFX_GraphStateData();
  CFX_GraphStateData(const CFX_GraphStateData& src);
  CFX_GraphStateData(CFX_GraphStateData&& src) noexcept;
  ~CFX_GraphStateData();

  CFX_GraphStateData& operator=(const CFX_GraphStateData& that);
  CFX_GraphStateData& operator=(CFX_GraphStateData&& that) noexcept;

  LineCap line_cap() const { return line_cap_; }
  void set_line_cap(LineCap line_cap) { line_cap_ = line_cap; }

  LineJoin line_join() const { return line_join_; }
  void set_line_join(LineJoin line_join) { line_join_ = line_join; }

  float miter_limit() const { return miter_limit_; }
  void set_miter_limit(float miter_limit) { miter_limit_ = miter_limit; }

  float line_width() const { return line_width_; }
  void set_line_width(float line_width) { line_width_ = line_width; }

  float dash_phase() const { return dash_phase_; }
  void set_dash_phase(float dash_phase) { dash_phase_ = dash_phase; }

  const std::vector<float>& dash_array() const { return dash_array_; }
  void set_dash_array(std::vector<float> dash_array);

 private:
  LineCap line_cap_ = LineCap::kButt;
  LineJoin line_join_ = LineJoin::kMiter;
  float miter_limit_ = 10.0f;
  float line_width_ = 1.0f;

  float dash_phase_ = 0.0f;
  std::vector<float> dash_array_;
};

class CFX_RetainableGraphStateData final : public Retainable,
                                           public CFX_GraphStateData {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  RetainPtr<CFX_RetainableGraphStateData> Clone() const;

 private:
  CFX_RetainableGraphStateData();
  CFX_RetainableGraphStateData(const CFX_RetainableGraphStateData& src);
  ~CFX_RetainableGraphStateData() override;
};

#endif  // CORE_FXGE_CFX_GRAPHSTATEDATA_H_
