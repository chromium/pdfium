// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_textprovider.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffcheckbutton.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_Node* CXFA_TextProvider::GetTextNode(bool& bRichText) {
  bRichText = false;

  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    CXFA_Node* pElementNode = m_pWidgetAcc->GetNode();
    CXFA_Node* pValueNode =
        pElementNode->GetChild(0, XFA_Element::Value, false);
    if (!pValueNode)
      return nullptr;

    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetElementType() == XFA_Element::ExData) {
      pdfium::Optional<WideString> contentType =
          pChildNode->JSNode()->TryAttribute(XFA_Attribute::ContentType, false);
      if (contentType && *contentType == L"text/html")
        bRichText = true;
    }
    return pChildNode;
  }

  if (m_eType == XFA_TEXTPROVIDERTYPE_Datasets) {
    CXFA_Node* pBind = m_pWidgetAcc->GetDatasets();
    CFX_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    ASSERT(pXMLNode);
    for (CFX_XMLNode* pXMLChild =
             pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
         pXMLChild;
         pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
      if (pXMLChild->GetType() == FX_XMLNODE_Element) {
        CFX_XMLElement* pElement = static_cast<CFX_XMLElement*>(pXMLChild);
        if (XFA_RecognizeRichText(pElement))
          bRichText = true;
      }
    }
    return pBind;
  }

  if (m_eType == XFA_TEXTPROVIDERTYPE_Caption) {
    CXFA_Node* pCaptionNode =
        m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption, false);
    if (!pCaptionNode)
      return nullptr;

    CXFA_Node* pValueNode =
        pCaptionNode->GetChild(0, XFA_Element::Value, false);
    if (!pValueNode)
      return nullptr;

    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetElementType() == XFA_Element::ExData) {
      pdfium::Optional<WideString> contentType =
          pChildNode->JSNode()->TryAttribute(XFA_Attribute::ContentType, false);
      if (contentType && *contentType == L"text/html")
        bRichText = true;
    }
    return pChildNode;
  }

  CXFA_Node* pItemNode =
      m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Items, false);
  if (!pItemNode)
    return nullptr;

  CXFA_Node* pNode = pItemNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pNode) {
    WideString wsName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
    if (m_eType == XFA_TEXTPROVIDERTYPE_Rollover && wsName == L"rollover")
      return pNode;
    if (m_eType == XFA_TEXTPROVIDERTYPE_Down && wsName == L"down")
      return pNode;

    pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return nullptr;
}

CXFA_ParaData CXFA_TextProvider::GetParaData() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text)
    return m_pWidgetAcc->GetParaData();

  CXFA_Node* pNode =
      m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption, false);
  return CXFA_ParaData(pNode->GetChild(0, XFA_Element::Para, false));
}

CXFA_FontData CXFA_TextProvider::GetFontData() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text)
    return m_pWidgetAcc->GetFontData(false);

  CXFA_Node* pNode =
      m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption, false);
  pNode = pNode->GetChild(0, XFA_Element::Font, false);
  return pNode ? CXFA_FontData(pNode) : m_pWidgetAcc->GetFontData(false);
}

bool CXFA_TextProvider::IsCheckButtonAndAutoWidth() {
  XFA_Element eType = m_pWidgetAcc->GetUIType();
  if (eType != XFA_Element::CheckButton)
    return false;
  return !m_pWidgetAcc->TryWidth();
}

bool CXFA_TextProvider::GetEmbbedObj(bool bURI,
                                     bool bRaw,
                                     const WideString& wsAttr,
                                     WideString& wsValue) {
  if (m_eType != XFA_TEXTPROVIDERTYPE_Text)
    return false;

  if (!bURI)
    return false;

  CXFA_Node* pWidgetNode = m_pWidgetAcc->GetNode();
  CXFA_Node* pParent = pWidgetNode->GetNodeItem(XFA_NODEITEM_Parent);
  CXFA_Document* pDocument = pWidgetNode->GetDocument();
  CXFA_Node* pIDNode = nullptr;
  CXFA_WidgetAcc* pEmbAcc = nullptr;
  if (pParent)
    pIDNode = pDocument->GetNodeByID(pParent, wsAttr.AsStringView());

  if (!pIDNode) {
    pIDNode = pDocument->GetNodeByID(
        ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Form)),
        wsAttr.AsStringView());
  }
  if (pIDNode)
    pEmbAcc = static_cast<CXFA_WidgetAcc*>(pIDNode->GetWidgetData());

  if (!pEmbAcc)
    return false;

  wsValue = pEmbAcc->GetValue(XFA_VALUEPICTURE_Display);
  return true;
}
