// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCUMENT_DATADESCRIPTION_IMP_H_
#define _XFA_DOCUMENT_DATADESCRIPTION_IMP_H_
void XFA_DataDescription_UpdateDataRelation(CXFA_Node* pDataNode,
                                            CXFA_Node* pDataDescriptionNode);
CXFA_Node* XFA_DataDescription_MaybeCreateDataNode(
    CXFA_Document* pDocument,
    CXFA_Node* pDataParent,
    XFA_ELEMENT eNodeType,
    const CFX_WideStringC& wsName);
#endif
