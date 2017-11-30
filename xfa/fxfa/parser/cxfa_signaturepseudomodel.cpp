// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_signaturepseudomodel.h"

namespace {

constexpr wchar_t kName[] = L"signaturePseudoModel";

}  // namespace

CXFA_SignaturePseudoModel::CXFA_SignaturePseudoModel(CXFA_Document* doc,
                                                     XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_XDP,
                XFA_ObjectType::Object,
                XFA_Element::SignaturePseudoModel,
                nullptr,
                nullptr,
                kName) {}

CXFA_SignaturePseudoModel::~CXFA_SignaturePseudoModel() {}
