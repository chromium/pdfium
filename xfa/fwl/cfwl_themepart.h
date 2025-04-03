// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_THEMEPART_H_
#define XFA_FWL_CFWL_THEMEPART_H_

#include <stdint.h>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/mask.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fwl/theme/cfwl_utils.h"

namespace pdfium {

class CFWL_Widget;

enum class CFWL_PartState : uint16_t {
  kNormal = 0,
  kChecked = 1 << 1,
  kDefault = 1 << 2,
  kDisabled = 1 << 3,
  kFlagged = 1 << 4,
  kFocused = 1 << 5,
  kHightLight = 1 << 6,
  kHovered = 1 << 7,
  kNeutral = 1 << 9,
  kPressed = 1 << 10,
  kReadOnly = 1 << 11,
  kLSelected = 1 << 12,
  kRSelected = 1 << 13,
  kSelected = 1 << 14
};

class CFWL_ThemePart {
 public:
  enum class Part : uint8_t {
    kNone = 0,

    kBackArrow,
    kBackground,
    kBorder,
    kCaption,
    kCheck,
    kCheckBox,
    kCloseBox,
    kCombTextLine,
    kDateInBK,
    kDateInCircle,
    kDatesIn,
    kDatesOut,
    kDownButton,
    kDropDownButton,
    kForeArrow,
    kHSeparator,
    kHeadText,
    kHeader,
    kIcon,
    kImage,
    kLBtn,
    kListItem,
    kLowerTrack,
    kMinimizeBox,
    kMaximizeBox,
    kNarrowCaption,
    kRBtn,
    kThumb,
    kThumbBackArrow,
    kThumbForeArrow,
    kThumbLowerTrack,
    kThumbThumb,
    kThumbUpperTrack,
    kToday,
    kTodayCircle,
    kUpButton,
    kUpperTrack,
    kVSeparator,
    kWeek,
    kWeekNum,
    kWeekNumSep
  };

  FX_STACK_ALLOCATED();

  CFWL_ThemePart(Part iPart, CFWL_Widget* pWidget);
  ~CFWL_ThemePart();

  Part GetPart() const { return part_; }
  CFWL_Widget* GetWidget() const { return widget_; }
  FWLTHEME_STATE GetThemeState() const;

  CFX_Matrix matrix_;
  CFX_RectF part_rect_;
  UnownedPtr<const CFX_RectF> data_rect_;
  Mask<CFWL_PartState> states_ = CFWL_PartState::kNormal;
  bool maximize_ = false;
  bool static_background_ = false;

 private:
  const Part part_;
  UnownedPtr<CFWL_Widget> const widget_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_PartState;
using pdfium::CFWL_ThemePart;

#endif  // XFA_FWL_CFWL_THEMEPART_H_
