// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/cxfa_textparsecontext.h"

#include "xfa/fde/css/cfde_csscomputedstyle.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssstyleselector.h"

CXFA_TextParseContext::CXFA_TextParseContext()
    : m_pParentStyle(nullptr),
      m_eDisplay(FDE_CSSDisplay::None) {}

CXFA_TextParseContext::~CXFA_TextParseContext() {}
