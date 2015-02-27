// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_
#define TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_

#include "embedder_test.h"
#include "testing/gmock/include/gmock/gmock.h"

class EmbedderTestMockDelegate : public EmbedderTest::Delegate {
 public:
  MOCK_METHOD1(UnsupportedHandler, void(int type));
  MOCK_METHOD4(Alert, int(FPDF_WIDESTRING message, FPDF_WIDESTRING title,
                          int type, int icon));
};

#endif  // TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_

