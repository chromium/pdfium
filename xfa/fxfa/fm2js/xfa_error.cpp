// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_error.h"

const wchar_t kFMErrUnsupportedChar[] = L"unsupported char '%c'";
const wchar_t kFMErrBadSuffixNumber[] = L"bad suffix on number";
const wchar_t kFMErrExpectedIdentifier[] =
    L"expected identifier instead of '%s'";
const wchar_t kFMErrExpectedToken[] = L"expected '%s' instead of '%s'";
const wchar_t kFMErrExpectedEndIf[] = L"expected 'endif' instead of '%s'";
const wchar_t kFMErrUnexpectedExpression[] = L"unexpected expression '%s'";
const wchar_t kFMErrExpectedNonEmptyExpression[] =
    L"expected non-empty expression";
const wchar_t kFMErrLongAssignmentChain[] =
    L"long assignment chains are unsupported";
