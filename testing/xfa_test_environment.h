// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_XFA_UNIT_TEST_SUPPORT_H_
#define TESTING_XFA_UNIT_TEST_SUPPORT_H_

#include <memory>

#include "testing/gtest/include/gtest/gtest.h"

#ifndef PDF_ENABLE_XFA
#error "XFA must be enabled"
#endif

class CFGAS_FontMgr;

// XFATestEnvironement loads a CFGAS_FontMgr, whose time is linear in the
// number of times it is loaded. So, if a test suite has a lot of tests that
// need a font manager they can end up executing very, very slowly.

class XFATestEnvironment : public testing::Environment {
 public:
  XFATestEnvironment();
  ~XFATestEnvironment();

  // testing::TestEnvironment:
  void SetUp() override;
  void TearDown() override;

  // Does not create if not present.
  static CFGAS_FontMgr* GetGlobalFontManager();

 private:
  std::unique_ptr<CFGAS_FontMgr> font_mgr_;
};

#endif  // TESTING_XFA_UNIT_TEST_SUPPORT_H_
