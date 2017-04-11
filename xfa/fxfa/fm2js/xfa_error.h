// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_XFA_ERROR_H_
#define XFA_FXFA_FM2JS_XFA_ERROR_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

extern const wchar_t kFMErrUnsupportedChar[];
extern const wchar_t kFMErrBadSuffixNumber[];
extern const wchar_t kFMErrExpectedIdentifier[];
extern const wchar_t kFMErrExpectedToken[];
extern const wchar_t kFMErrExpectedEndIf[];
extern const wchar_t kFMErrUnexpectedExpression[];
extern const wchar_t kFMErrExpectedNonEmptyExpression[];
extern const wchar_t kFMErrLongAssignmentChain[];

class CXFA_FMErrorInfo {
 public:
  CXFA_FMErrorInfo() : linenum(0) {}
  ~CXFA_FMErrorInfo() {}
  uint32_t linenum;
  CFX_WideString message;
};

#endif  // XFA_FXFA_FM2JS_XFA_ERROR_H_
