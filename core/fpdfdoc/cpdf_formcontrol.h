// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_FORMCONTROL_H_
#define CORE_FPDFDOC_CPDF_FORMCONTROL_H_

#include <optional>

#include "constants/appearance.h"
#include "core/fpdfdoc/cpdf_aaction.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_annotlist.h"
#include "core/fpdfdoc/cpdf_apsettings.h"
#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_iconfit.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_color.h"
#include "core/fxge/dib/fx_dib.h"

class CFX_RenderDevice;
class CPDF_Dictionary;
class CPDF_Font;
class CPDF_FormField;
class CPDF_InteractiveForm;
class CPDF_Stream;

class CPDF_FormControl {
 public:
  enum HighlightingMode { kNone = 0, kInvert, kOutline, kPush, kToggle };

  CPDF_FormControl(CPDF_FormField* pField,
                   RetainPtr<CPDF_Dictionary> pWidgetDict,
                   CPDF_InteractiveForm* pForm);
  ~CPDF_FormControl();

  CPDF_FormField::Type GetType() const { return field_->GetType(); }
  CPDF_FormField* GetField() const { return field_; }
  RetainPtr<const CPDF_Dictionary> GetWidgetDict() const {
    return widget_dict_;
  }
  CFX_FloatRect GetRect() const;

  ByteString GetCheckedAPState() const;
  WideString GetExportValue() const;

  bool IsChecked() const;
  bool IsDefaultChecked() const;

  HighlightingMode GetHighlightingMode() const;
  bool HasMKEntry(ByteStringView entry) const;
  int GetRotation() const;

  CFX_Color::TypeAndARGB GetColorARGB(ByteStringView entry);
  float GetOriginalColorComponent(int index, ByteStringView entry);

  CFX_Color GetOriginalBorderColor() {
    return GetOriginalColor(pdfium::appearance::kBC);
  }

  CFX_Color GetOriginalBackgroundColor() {
    return GetOriginalColor(pdfium::appearance::kBG);
  }

  WideString GetNormalCaption() const {
    return GetCaption(pdfium::appearance::kCA);
  }
  WideString GetRolloverCaption() const {
    return GetCaption(pdfium::appearance::kRC);
  }
  WideString GetDownCaption() const {
    return GetCaption(pdfium::appearance::kAC);
  }

  RetainPtr<CPDF_Stream> GetNormalIcon() {
    return GetIcon(pdfium::appearance::kI);
  }
  RetainPtr<CPDF_Stream> GetRolloverIcon() {
    return GetIcon(pdfium::appearance::kRI);
  }
  RetainPtr<CPDF_Stream> GetDownIcon() {
    return GetIcon(pdfium::appearance::kIX);
  }
  CPDF_IconFit GetIconFit() const;

  int GetTextPosition() const;
  CPDF_DefaultAppearance GetDefaultAppearance() const;

  std::optional<WideString> GetDefaultControlFontName() const;
  int GetControlAlignment() const;

  ByteString GetOnStateName() const;
  void CheckControl(bool bChecked);

 private:
  RetainPtr<CPDF_Font> GetDefaultControlFont() const;
  CFX_Color GetOriginalColor(ByteStringView entry);

  WideString GetCaption(ByteStringView entry) const;
  RetainPtr<CPDF_Stream> GetIcon(ByteStringView entry);
  CPDF_ApSettings GetMK() const;

  UnownedPtr<CPDF_FormField> const field_;
  RetainPtr<CPDF_Dictionary> const widget_dict_;
  UnownedPtr<const CPDF_InteractiveForm> const form_;
};

#endif  // CORE_FPDFDOC_CPDF_FORMCONTROL_H_
