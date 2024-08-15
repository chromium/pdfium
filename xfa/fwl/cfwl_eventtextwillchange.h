// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTTEXTWILLCHANGE_H_
#define XFA_FWL_CFWL_EVENTTEXTWILLCHANGE_H_

#include "core/fxcrt/widestring.h"
#include "xfa/fwl/cfwl_event.h"

namespace pdfium {

class CFWL_EventTextWillChange final : public CFWL_Event {
 public:
  CFWL_EventTextWillChange(CFWL_Widget* pSrcTarget,
                           const WideString& change_text,
                           const WideString& previous_text,
                           size_t selection_start,
                           size_t selection_end);
  ~CFWL_EventTextWillChange() override;

  WideString GetChangeText() const { return change_text_; }
  WideString GetPreviousText() const { return previous_text_; }
  size_t GetSelectionStart() const { return selection_start_; }
  size_t GetSelectionEnd() const { return selection_end_; }
  bool GetCancelled() const { return cancelled_; }

  void SetChangeText(const WideString& change_text) {
    change_text_ = change_text;
  }
  void SetPreviousText(const WideString& previous_text) {
    previous_text_ = previous_text;
  }
  void SetSelectionStart(size_t selection_start) {
    selection_start_ = selection_start;
  }
  void SetSelectionEnd(size_t selection_end) { selection_end_ = selection_end; }
  void SetCancelled(bool cancelled) { cancelled_ = cancelled; }

 protected:
  WideString change_text_;
  WideString previous_text_;
  size_t selection_start_;
  size_t selection_end_;
  bool cancelled_ = false;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_EventTextWillChange;

#endif  // XFA_FWL_CFWL_EVENTTEXTWILLCHANGE_H_
