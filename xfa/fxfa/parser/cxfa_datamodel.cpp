// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datamodel.h"

#include "fxjs/xfa/cjx_model.h"
#include "xfa/fxfa/parser/cxfa_document.h"

CXFA_DataModel::CXFA_DataModel(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET::kDatasets,
                XFA_ObjectType::ModelNode,
                XFA_Element::DataModel,
                {},
                {},
                cppgc::MakeGarbageCollected<CJX_Model>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_DataModel::~CXFA_DataModel() = default;
