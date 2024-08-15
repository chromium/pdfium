// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_DATETIMEPICKERTP_H_
#define XFA_FWL_THEME_CFWL_DATETIMEPICKERTP_H_

#include "fxjs/gc/heap.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

namespace pdfium {

class CFWL_DateTimePickerTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_DateTimePickerTP() override;

  // CFWL_WidgetTP:
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  CFWL_DateTimePickerTP();
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_DateTimePickerTP;

#endif  // XFA_FWL_THEME_CFWL_DATETIMEPICKERTP_H_
