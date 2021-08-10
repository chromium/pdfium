// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FXFA_H_
#define XFA_FXFA_FXFA_H_

#include <type_traits>

// Note, values must match fpdf_formfill.h JSPLATFORM_ALERT_BUTTON_* flags.
enum class AlertButton {
  kDefault = 0,
  kOK = 0,
  kOKCancel = 1,
  kYesNo = 2,
  kYesNoCancel = 3,
};

// Note, values must match fpdf_formfill.h JSPLATFORM_ALERT_ICON_* flags.
enum class AlertIcon {
  kDefault = 0,
  kError = 0,
  kWarning = 1,
  kQuestion = 2,
  kStatus = 3,
  kAsterisk = 4,
};

// Note, values must match fpdf_formfill.h JSPLATFORM_ALERT_RETURN_* flags.
enum class AlertReturn {
  kOK = 1,
  kCancel = 2,
  kNo = 3,
  kYes = 4,
};

// Note, values must match fpdf_formfill.h FORMTYPE_* flags.
enum class FormType {
  kNone = 0,
  kAcroForm = 1,
  kXFAFull = 2,
  kXFAForeground = 3,
};

enum class XFA_PrintOpt : uint8_t {
  kShowDialog = 1 << 0,
  kCanCancel = 1 << 1,
  kShrinkPage = 1 << 2,
  kAsImage = 1 << 3,
  kReverseOrder = 1 << 4,
  kPrintAnnot = 1 << 5,
};

enum class XFA_EventError {
  kError = -1,
  kNotExist = 0,
  kSuccess = 1,
  kDisabled = 2,
};

enum XFA_WidgetStatus : uint16_t {
  XFA_WidgetStatus_None = 0,
  XFA_WidgetStatus_Access = 1 << 0,
  XFA_WidgetStatus_ButtonDown = 1 << 1,
  XFA_WidgetStatus_Disabled = 1 << 2,
  XFA_WidgetStatus_Focused = 1 << 3,
  XFA_WidgetStatus_Printable = 1 << 4,
  XFA_WidgetStatus_RectCached = 1 << 5,
  XFA_WidgetStatus_TextEditValueChanged = 1 << 6,
  XFA_WidgetStatus_Viewable = 1 << 7,
  XFA_WidgetStatus_Visible = 1 << 8
};
using XFA_WidgetStatusMask = std::underlying_type<XFA_WidgetStatus>::type;

#endif  // XFA_FXFA_FXFA_H_
