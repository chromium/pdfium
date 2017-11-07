// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_value.h"

#include "xfa/fxfa/parser/cxfa_node.h"

XFA_Element CXFA_Value::GetChildValueClassID() {
  if (!m_pNode)
    return XFA_Element::Unknown;
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
    return pNode->GetElementType();
  return XFA_Element::Unknown;
}

bool CXFA_Value::GetChildValueContent(WideString& wsContent) {
  if (!m_pNode)
    return false;
  if (CXFA_Node* pNode = m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild))
    return pNode->JSNode()->TryContent(wsContent, false, true);
  return false;
}

CXFA_ArcData CXFA_Value::GetArcData() {
  return CXFA_ArcData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                              : nullptr);
}

CXFA_LineData CXFA_Value::GetLineData() {
  return CXFA_LineData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                               : nullptr);
}

CXFA_Rectangle CXFA_Value::GetRectangle() {
  return CXFA_Rectangle(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                                : nullptr);
}

CXFA_Text CXFA_Value::GetText() {
  return CXFA_Text(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                           : nullptr);
}

CXFA_ExData CXFA_Value::GetExData() {
  return CXFA_ExData(m_pNode ? m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)
                             : nullptr);
}

CXFA_ImageData CXFA_Value::GetImageData() {
  return CXFA_ImageData(
      m_pNode ? (m_pNode->GetNodeItem(XFA_NODEITEM_FirstChild)) : nullptr,
      true);
}
