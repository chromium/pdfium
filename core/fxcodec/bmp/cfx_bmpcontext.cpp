// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/cfx_bmpcontext.h"

namespace fxcodec {

CFX_BmpContext::CFX_BmpContext(BmpModule* pModule,
                               BmpModule::Delegate* pDelegate)
    : m_Bmp(this), m_pModule(pModule), m_pDelegate(pDelegate) {}

CFX_BmpContext::~CFX_BmpContext() = default;

}  // namespace fxcodec
