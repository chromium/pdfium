// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_valuedata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

XFA_Element CXFA_ValueData::GetChildValueClassID() {
  if (!m_pNode)
    return XFA_Element::Unknown;
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
    return pNode->GetElementType();
  return XFA_Element::Unknown;
}

bool CXFA_ValueData::GetChildValueContent(WideString& wsContent) {
  if (!m_pNode)
    return false;
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
    return pNode->JSNode()->TryContent(wsContent, false, true);
  return false;
}

CXFA_ArcData CXFA_ValueData::GetArcData() {
  return CXFA_ArcData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                              : nullptr);
}

CXFA_LineData CXFA_ValueData::GetLineData() {
  return CXFA_LineData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                               : nullptr);
}

CXFA_RectangleData CXFA_ValueData::GetRectangleData() {
  return CXFA_RectangleData(
      m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild) : nullptr);
}

CXFA_TextData CXFA_ValueData::GetTextData() {
  return CXFA_TextData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                               : nullptr);
}

CXFA_ExDataData CXFA_ValueData::GetExData() {
  return CXFA_ExDataData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                                 : nullptr);
}

CXFA_ImageData CXFA_ValueData::GetImageData() {
  return CXFA_ImageData(
      m_pNode ? (m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) : nullptr,
      true);
}
