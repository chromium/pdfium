// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_UNIT_TEST_SUPPORT_H_
#define TESTING_XFA_UNIT_TEST_SUPPORT_H_

#ifndef PDF_ENABLE_XFA
#error "XFA must be enabled"
#endif

class CFGAS_FontMgr;

void InitializeXFATestEnvironment();

CFGAS_FontMgr* GetGlobalFontManager();

#endif  // TESTING_XFA_UNIT_TEST_SUPPORT_H_
