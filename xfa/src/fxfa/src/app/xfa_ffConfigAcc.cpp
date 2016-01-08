// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffConfigAcc.h"
CXFA_FFConfigAcc::CXFA_FFConfigAcc(CXFA_Node* pNode)
    : m_pNode(pNode), m_pPsMapNode(NULL) {}
CXFA_FFConfigAcc::~CXFA_FFConfigAcc() {}
int32_t CXFA_FFConfigAcc::CountChildren() {
  GetPsMapNode();
  if (m_pPsMapNode == NULL) {
    return 0;
  }
  int32_t iCount = 0;
  CXFA_Node* pNode = m_pPsMapNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    iCount++;
  }
  return iCount;
}
FX_BOOL CXFA_FFConfigAcc::GetFontInfo(int32_t index,
                                      CFX_WideString& wsFontFamily,
                                      CFX_WideString& wsPsName,
                                      FX_BOOL bBold,
                                      FX_BOOL bItalic) {
  if (index < 0 || index >= CountChildren()) {
    return FALSE;
  }
  CXFA_Node* pFontNode = m_pPsMapNode->GetChild(index, XFA_ELEMENT_Font);
  if (pFontNode == NULL) {
    return FALSE;
  }
  wsFontFamily.Empty();
  wsPsName.Empty();
  bBold = FALSE;
  bItalic = FALSE;
  pFontNode->GetAttribute(XFA_ATTRIBUTE_Typeface, wsFontFamily);
  pFontNode->GetAttribute(XFA_ATTRIBUTE_PsName, wsPsName);
  CFX_WideString wsValue;
  pFontNode->GetAttribute(XFA_ATTRIBUTE_Weight, wsValue);
  wsValue.MakeLower();
  if (wsValue == FX_WSTRC(L"bold")) {
    bBold = TRUE;
  }
  pFontNode->GetAttribute(XFA_ATTRIBUTE_Posture, wsValue);
  wsValue.MakeLower();
  if (wsValue == FX_WSTRC(L"italic")) {
    bItalic = TRUE;
  }
  return wsFontFamily.GetLength() > 0;
}
void CXFA_FFConfigAcc::GetPsMapNode() {
  if (m_pNode == NULL) {
    return;
  }
  m_pPsMapNode = m_pNode->GetChild(0, XFA_ELEMENT_PsMap);
}
