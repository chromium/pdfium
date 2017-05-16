// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmerrorinfo.h"

const wchar_t kFMErrUnsupportedChar[] = L"unsupported char '%c'";
const wchar_t kFMErrBadSuffixNumber[] = L"bad suffix on number";
const wchar_t kFMErrExpectedIdentifier[] =
    L"expected identifier instead of '%.16s'";
const wchar_t kFMErrExpectedToken[] = L"expected '%.16s' instead of '%.16s'";
const wchar_t kFMErrExpectedEndIf[] = L"expected 'endif' instead of '%.16s'";
const wchar_t kFMErrUnexpectedExpression[] = L"unexpected expression '%.16s'";
const wchar_t kFMErrExpectedNonEmptyExpression[] =
    L"expected non-empty expression";
const wchar_t kFMErrLongAssignmentChain[] =
    L"long assignment chains are unsupported";
const wchar_t kFMErrEndOfInput[] = L"unexpected end of input";
