// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FXFA_H_
#define XFA_FXFA_FXFA_H_

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

enum class XFA_WidgetStatus : uint16_t {
  kNone = 0,
  kAccess = 1 << 0,
  kButtonDown = 1 << 1,
  kDisabled = 1 << 2,
  kFocused = 1 << 3,
  kPrintable = 1 << 4,
  kRectCached = 1 << 5,
  kTextEditValueChanged = 1 << 6,
  kViewable = 1 << 7,
  kVisible = 1 << 8
};

#endif  // XFA_FXFA_FXFA_H_
