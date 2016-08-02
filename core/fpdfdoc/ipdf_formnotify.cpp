// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/include/ipdf_formnotify.h"

IPDF_FormNotify::~IPDF_FormNotify() {}

int IPDF_FormNotify::BeforeValueChange(CPDF_FormField* pField,
                                       const CFX_WideString& csValue) {
  return 0;
}

void IPDF_FormNotify::AfterValueChange(CPDF_FormField* pField) {}

int IPDF_FormNotify::BeforeSelectionChange(CPDF_FormField* pField,
                                           const CFX_WideString& csValue) {
  return 0;
}

void IPDF_FormNotify::AfterSelectionChange(CPDF_FormField* pField) {}

void IPDF_FormNotify::AfterCheckedStatusChange(CPDF_FormField* pField) {}

int IPDF_FormNotify::BeforeFormReset(CPDF_InterForm* pForm) {
  return 0;
}

void IPDF_FormNotify::AfterFormReset(CPDF_InterForm* pForm) {}

int IPDF_FormNotify::BeforeFormImportData(CPDF_InterForm* pForm) {
  return 0;
}

void IPDF_FormNotify::AfterFormImportData(CPDF_InterForm* pForm) {}
