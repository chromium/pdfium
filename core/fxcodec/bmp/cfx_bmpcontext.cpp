// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/bmp/cfx_bmpcontext.h"

namespace fxcodec {

CFX_BmpContext::CFX_BmpContext(BmpDecoder::Delegate* pDelegate)
    : m_Bmp(this), m_pDelegate(pDelegate) {}

CFX_BmpContext::~CFX_BmpContext() = default;

}  // namespace fxcodec
