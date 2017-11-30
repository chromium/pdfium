// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datawindow.h"

namespace {

constexpr wchar_t kName[] = L"dataWindow";

}  // namespace

CXFA_DataWindow::CXFA_DataWindow(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Datasets,
                XFA_ObjectType::Object,
                XFA_Element::DataWindow,
                nullptr,
                nullptr,
                kName) {}

CXFA_DataWindow::~CXFA_DataWindow() {}
