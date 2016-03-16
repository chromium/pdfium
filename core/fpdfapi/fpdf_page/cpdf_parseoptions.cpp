// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
#include "core/fpdfapi/fpdf_page/cpdf_parseoptions.h"

CPDF_ParseOptions::CPDF_ParseOptions()
    : m_bTextOnly(FALSE),
      m_bMarkedContent(TRUE),
      m_bSeparateForm(TRUE),
      m_bDecodeInlineImage(FALSE) {}
