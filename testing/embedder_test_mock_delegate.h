// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_
#define TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_

#include "testing/embedder_test.h"
#include "testing/gmock/include/gmock/gmock.h"

class EmbedderTestMockDelegate : public EmbedderTest::Delegate {
 public:
  MOCK_METHOD1(UnsupportedHandler, void(int type));
  MOCK_METHOD4(
      Alert,
      int(FPDF_WIDESTRING message, FPDF_WIDESTRING title, int type, int icon));
  MOCK_METHOD2(SetTimer, int(int msecs, TimerCallback fn));
  MOCK_METHOD1(KillTimer, void(int msecs));
  MOCK_METHOD1(DoURIAction, void(FPDF_BYTESTRING uri));
  MOCK_METHOD5(DoGoToAction,
               void(FPDF_FORMFILLINFO* info,
                    int page_index,
                    int zoom_mode,
                    float* pos_array,
                    int array_size));
  MOCK_METHOD3(OnFocusChange,
               void(FPDF_FORMFILLINFO* info,
                    FPDF_ANNOTATION annot,
                    int page_index));
  MOCK_METHOD3(DoURIActionWithKeyboardModifier,
               void(FPDF_FORMFILLINFO* info,
                    FPDF_BYTESTRING uri,
                    int modifiers));
};

#endif  // TESTING_EMBEDDER_TEST_MOCK_DELEGATE_H_
