// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/xfa_ffwidgetacc.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fde/xml/cfde_xmlelement.h"
#include "xfa/fde/xml/cfde_xmlnode.h"
#include "xfa/fxfa/app/xfa_ffcheckbutton.h"
#include "xfa/fxfa/app/xfa_ffchoicelist.h"
#include "xfa/fxfa/app/xfa_fffield.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_scriptcontext.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

CXFA_Node* CXFA_TextProvider::GetTextNode(bool& bRichText) {
  bRichText = false;
  if (m_pTextNode) {
    if (m_pTextNode->GetElementType() == XFA_Element::ExData) {
      CFX_WideString wsContentType;
      m_pTextNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType,
                                false);
      if (wsContentType == L"text/html") {
        bRichText = true;
      }
    }
    return m_pTextNode;
  }
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    CXFA_Node* pElementNode = m_pWidgetAcc->GetNode();
    CXFA_Node* pValueNode = pElementNode->GetChild(0, XFA_Element::Value);
    if (!pValueNode) {
      return nullptr;
    }
    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetElementType() == XFA_Element::ExData) {
      CFX_WideString wsContentType;
      pChildNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, false);
      if (wsContentType == L"text/html") {
        bRichText = true;
      }
    }
    return pChildNode;
  } else if (m_eType == XFA_TEXTPROVIDERTYPE_Datasets) {
    CXFA_Node* pBind = m_pWidgetAcc->GetDatasets();
    CFDE_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    ASSERT(pXMLNode);
    for (CFDE_XMLNode* pXMLChild =
             pXMLNode->GetNodeItem(CFDE_XMLNode::FirstChild);
         pXMLChild;
         pXMLChild = pXMLChild->GetNodeItem(CFDE_XMLNode::NextSibling)) {
      if (pXMLChild->GetType() == FDE_XMLNODE_Element) {
        CFDE_XMLElement* pElement = static_cast<CFDE_XMLElement*>(pXMLChild);
        if (XFA_RecognizeRichText(pElement)) {
          bRichText = true;
        }
      }
    }
    return pBind;
  } else if (m_eType == XFA_TEXTPROVIDERTYPE_Caption) {
    CXFA_Node* pCaptionNode =
        m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption);
    if (!pCaptionNode) {
      return nullptr;
    }
    CXFA_Node* pValueNode = pCaptionNode->GetChild(0, XFA_Element::Value);
    if (!pValueNode) {
      return nullptr;
    }
    CXFA_Node* pChildNode = pValueNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    if (pChildNode && pChildNode->GetElementType() == XFA_Element::ExData) {
      CFX_WideString wsContentType;
      pChildNode->GetAttribute(XFA_ATTRIBUTE_ContentType, wsContentType, false);
      if (wsContentType == L"text/html") {
        bRichText = true;
      }
    }
    return pChildNode;
  }
  CXFA_Node* pItemNode =
      m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Items);
  if (!pItemNode) {
    return nullptr;
  }
  CXFA_Node* pNode = pItemNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pNode) {
    CFX_WideStringC wsName;
    pNode->TryCData(XFA_ATTRIBUTE_Name, wsName);
    if (m_eType == XFA_TEXTPROVIDERTYPE_Rollover && wsName == L"rollover") {
      return pNode;
    }
    if (m_eType == XFA_TEXTPROVIDERTYPE_Down && wsName == L"down") {
      return pNode;
    }
    pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  return nullptr;
}
CXFA_Para CXFA_TextProvider::GetParaNode() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    return m_pWidgetAcc->GetPara();
  }
  CXFA_Node* pNode = m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption);
  return CXFA_Para(pNode->GetChild(0, XFA_Element::Para));
}
CXFA_Font CXFA_TextProvider::GetFontNode() {
  if (m_eType == XFA_TEXTPROVIDERTYPE_Text) {
    return m_pWidgetAcc->GetFont();
  }
  CXFA_Node* pNode = m_pWidgetAcc->GetNode()->GetChild(0, XFA_Element::Caption);
  pNode = pNode->GetChild(0, XFA_Element::Font);
  if (pNode) {
    return CXFA_Font(pNode);
  }
  return m_pWidgetAcc->GetFont();
}
bool CXFA_TextProvider::IsCheckButtonAndAutoWidth() {
  XFA_Element eType = m_pWidgetAcc->GetUIType();
  if (eType == XFA_Element::CheckButton) {
    float fWidth = 0;
    return !m_pWidgetAcc->GetWidth(fWidth);
  }
  return false;
}
bool CXFA_TextProvider::GetEmbbedObj(bool bURI,
                                     bool bRaw,
                                     const CFX_WideString& wsAttr,
                                     CFX_WideString& wsValue) {
  if (m_eType != XFA_TEXTPROVIDERTYPE_Text) {
    return false;
  }
  if (bURI) {
    CXFA_Node* pWidgetNode = m_pWidgetAcc->GetNode();
    CXFA_Node* pParent = pWidgetNode->GetNodeItem(XFA_NODEITEM_Parent);
    CXFA_Document* pDocument = pWidgetNode->GetDocument();
    CXFA_Node* pIDNode = nullptr;
    CXFA_WidgetAcc* pEmbAcc = nullptr;
    if (pParent) {
      pIDNode = pDocument->GetNodeByID(pParent, wsAttr.AsStringC());
    }
    if (!pIDNode) {
      pIDNode = pDocument->GetNodeByID(
          ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Form)),
          wsAttr.AsStringC());
    }
    if (pIDNode) {
      pEmbAcc = static_cast<CXFA_WidgetAcc*>(pIDNode->GetWidgetData());
    }
    if (pEmbAcc) {
      pEmbAcc->GetValue(wsValue, XFA_VALUEPICTURE_Display);
      return true;
    }
  }
  return false;
}
