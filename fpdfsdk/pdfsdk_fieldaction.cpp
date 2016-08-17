// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfsdk_fieldaction.h"

PDFSDK_FieldAction::PDFSDK_FieldAction()
    : bModifier(FALSE),
      bShift(FALSE),
      nCommitKey(0),
      bKeyDown(FALSE),
      nSelEnd(0),
      nSelStart(0),
      bWillCommit(FALSE),
      bFieldFull(FALSE),
      bRC(TRUE) {}
