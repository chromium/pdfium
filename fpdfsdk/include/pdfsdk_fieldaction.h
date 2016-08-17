// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFSDK_FIELDACTION_H_
#define FPDFSDK_INCLUDE_PDFSDK_FIELDACTION_H_

#include "core/fxcrt/include/fx_string.h"

#ifdef PDF_ENABLE_XFA
typedef enum {
  PDFSDK_XFA_Click = 0,
  PDFSDK_XFA_Full,
  PDFSDK_XFA_PreOpen,
  PDFSDK_XFA_PostOpen
} PDFSDK_XFAAActionType;
#endif  // PDF_ENABLE_XFA

struct PDFSDK_FieldAction {
  PDFSDK_FieldAction();
  PDFSDK_FieldAction(const PDFSDK_FieldAction& other) = delete;

  FX_BOOL bModifier;
  FX_BOOL bShift;
  int nCommitKey;
  CFX_WideString sChange;
  CFX_WideString sChangeEx;
  FX_BOOL bKeyDown;
  int nSelEnd;
  int nSelStart;
  CFX_WideString sValue;
  FX_BOOL bWillCommit;
  FX_BOOL bFieldFull;
  FX_BOOL bRC;
};

#endif  // FPDFSDK_INCLUDE_PDFSDK_FIELDACTION_H_
