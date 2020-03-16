// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssexttextbuf.h"

CFX_CSSExtTextBuf::CFX_CSSExtTextBuf(WideStringView str) : m_Buffer(str) {}

CFX_CSSExtTextBuf::~CFX_CSSExtTextBuf() = default;
