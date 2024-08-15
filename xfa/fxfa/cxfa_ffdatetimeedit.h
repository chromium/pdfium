// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDATETIMEEDIT_H_
#define XFA_FXFA_CXFA_FFDATETIMEEDIT_H_

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fxfa/cxfa_fftextedit.h"

namespace pdfium {
class CFWL_DateTimePicker;
class CFWL_Widget;
}  // namespace pdfium

class CXFA_FFDateTimeEdit final : public CXFA_FFTextEdit {
 public:
  explicit CXFA_FFDateTimeEdit(CXFA_Node* pNode);
  ~CXFA_FFDateTimeEdit() override;

  // CXFA_FFTextEdit
  CFX_RectF GetBBox(FocusOption focus) override;
  bool LoadWidget() override;
  void UpdateWidgetProperty() override;
  void OnProcessEvent(pdfium::CFWL_Event* pEvent) override;

  void OnSelectChanged(pdfium::CFWL_Widget* pWidget,
                       int32_t iYear,
                       int32_t iMonth,
                       int32_t iDay);

  // CXFA_FFWidget
  bool CanUndo() override;
  bool CanRedo() override;
  bool CanCopy() override;
  bool CanCut() override;
  bool CanPaste() override;
  bool CanSelectAll() override;
  bool Undo() override;
  bool Redo() override;
  std::optional<WideString> Copy() override;
  std::optional<WideString> Cut() override;
  bool Paste(const WideString& wsPaste) override;
  void SelectAll() override;
  void Delete() override;
  void DeSelect() override;
  WideString GetText() override;

 private:
  bool PtInActiveRect(const CFX_PointF& point) override;
  bool CommitData() override;
  bool UpdateFWLData() override;
  bool IsDataChanged() override;

  pdfium::CFWL_DateTimePicker* GetPickerWidget();

  uint32_t GetAlignment();
};

#endif  // XFA_FXFA_CXFA_FFDATETIMEEDIT_H_
