// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dataexporter.h"

#include <vector>

#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

constexpr const wchar_t kFormNS[] = L"http://www.xfa.org/schema/xfa-form/";

WideString ExportEncodeAttribute(const WideString& str) {
  CFX_WideTextBuf textBuf;
  int32_t iLen = str.GetLength();
  for (int32_t i = 0; i < iLen; i++) {
    switch (str[i]) {
      case '&':
        textBuf << L"&amp;";
        break;
      case '<':
        textBuf << L"&lt;";
        break;
      case '>':
        textBuf << L"&gt;";
        break;
      case '\'':
        textBuf << L"&apos;";
        break;
      case '\"':
        textBuf << L"&quot;";
        break;
      default:
        textBuf.AppendChar(str[i]);
    }
  }
  return textBuf.MakeString();
}

bool IsXMLValidChar(wchar_t ch) {
  return ch == 0x09 || ch == 0x0A || ch == 0x0D ||
         (ch >= 0x20 && ch <= 0xD7FF) || (ch >= 0xE000 && ch <= 0xFFFD);
}

WideString ExportEncodeContent(const WideString& str) {
  CFX_WideTextBuf textBuf;
  int32_t iLen = str.GetLength();
  for (int32_t i = 0; i < iLen; i++) {
    wchar_t ch = str[i];
    if (!IsXMLValidChar(ch))
      continue;

    if (ch == '&') {
      textBuf << L"&amp;";
    } else if (ch == '<') {
      textBuf << L"&lt;";
    } else if (ch == '>') {
      textBuf << L"&gt;";
    } else if (ch == '\'') {
      textBuf << L"&apos;";
    } else if (ch == '\"') {
      textBuf << L"&quot;";
    } else if (ch == ' ') {
      if (i && str[i - 1] != ' ') {
        textBuf.AppendChar(' ');
      } else {
        textBuf << L"&#x20;";
      }
    } else {
      textBuf.AppendChar(str[i]);
    }
  }
  return textBuf.MakeString();
}

void SaveAttribute(CXFA_Node* pNode,
                   XFA_Attribute eName,
                   const WideString& wsName,
                   bool bProto,
                   WideString& wsOutput) {
  if (!bProto && !pNode->JSNode()->HasAttribute(eName))
    return;

  pdfium::Optional<WideString> value =
      pNode->JSNode()->TryAttribute(eName, false);
  if (!value)
    return;

  wsOutput += L" ";
  wsOutput += wsName;
  wsOutput += L"=\"";
  wsOutput += ExportEncodeAttribute(*value);
  wsOutput += L"\"";
}

bool AttributeSaveInDataModel(CXFA_Node* pNode, XFA_Attribute eAttribute) {
  bool bSaveInDataModel = false;
  if (pNode->GetElementType() != XFA_Element::Image)
    return bSaveInDataModel;

  CXFA_Node* pValueNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pValueNode || pValueNode->GetElementType() != XFA_Element::Value)
    return bSaveInDataModel;

  CXFA_Node* pFieldNode = pValueNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (pFieldNode && pFieldNode->GetBindData() &&
      eAttribute == XFA_Attribute::Href) {
    bSaveInDataModel = true;
  }
  return bSaveInDataModel;
}

bool ContentNodeNeedtoExport(CXFA_Node* pContentNode) {
  pdfium::Optional<WideString> wsContent =
      pContentNode->JSNode()->TryContent(false, false);
  if (!wsContent)
    return false;

  ASSERT(pContentNode->IsContentNode());
  CXFA_Node* pParentNode = pContentNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParentNode || pParentNode->GetElementType() != XFA_Element::Value)
    return true;

  CXFA_Node* pGrandParentNode = pParentNode->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pGrandParentNode || !pGrandParentNode->IsContainerNode())
    return true;
  if (pGrandParentNode->GetBindData())
    return false;

  CXFA_WidgetData* pWidgetData = pGrandParentNode->GetWidgetData();
  XFA_Element eUIType = pWidgetData->GetUIType();
  if (eUIType == XFA_Element::PasswordEdit)
    return false;
  return true;
}

void RecognizeXFAVersionNumber(CXFA_Node* pTemplateRoot,
                               WideString& wsVersionNumber) {
  wsVersionNumber.clear();
  if (!pTemplateRoot)
    return;

  pdfium::Optional<WideString> templateNS =
      pTemplateRoot->JSNode()->TryNamespace();
  if (!templateNS)
    return;

  XFA_VERSION eVersion =
      pTemplateRoot->GetDocument()->RecognizeXFAVersionNumber(*templateNS);
  if (eVersion == XFA_VERSION_UNKNOWN)
    eVersion = XFA_VERSION_DEFAULT;

  wsVersionNumber =
      WideString::Format(L"%i.%i", eVersion / 100, eVersion % 100);
}

void RegenerateFormFile_Changed(CXFA_Node* pNode,
                                CFX_WideTextBuf& buf,
                                bool bSaveXML) {
  WideString wsAttrs;
  for (size_t i = 0;; ++i) {
    XFA_Attribute attr = pNode->GetAttribute(i);
    if (attr == XFA_Attribute::Unknown)
      break;

    if (attr == XFA_Attribute::Name ||
        (AttributeSaveInDataModel(pNode, attr) && !bSaveXML)) {
      continue;
    }
    WideString wsAttr;
    SaveAttribute(pNode, attr, CXFA_Node::AttributeToName(attr), bSaveXML,
                  wsAttr);
    wsAttrs += wsAttr;
  }

  WideString wsChildren;
  switch (pNode->GetObjectType()) {
    case XFA_ObjectType::ContentNode: {
      if (!bSaveXML && !ContentNodeNeedtoExport(pNode))
        break;

      CXFA_Node* pRawValueNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
      while (pRawValueNode &&
             pRawValueNode->GetElementType() != XFA_Element::SharpxHTML &&
             pRawValueNode->GetElementType() != XFA_Element::Sharptext &&
             pRawValueNode->GetElementType() != XFA_Element::Sharpxml) {
        pRawValueNode = pRawValueNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      }
      if (!pRawValueNode)
        break;

      pdfium::Optional<WideString> contentType =
          pNode->JSNode()->TryAttribute(XFA_Attribute::ContentType, false);
      if (pRawValueNode->GetElementType() == XFA_Element::SharpxHTML &&
          (contentType && *contentType == L"text/html")) {
        CFX_XMLNode* pExDataXML = pNode->GetXMLMappingNode();
        if (!pExDataXML)
          break;

        CFX_XMLNode* pRichTextXML =
            pExDataXML->GetNodeItem(CFX_XMLNode::FirstChild);
        if (!pRichTextXML)
          break;

        auto pMemStream = pdfium::MakeRetain<CFX_MemoryStream>(true);
        auto pTempStream =
            pdfium::MakeRetain<CFX_SeekableStreamProxy>(pMemStream, true);

        pTempStream->SetCodePage(FX_CODEPAGE_UTF8);
        pRichTextXML->SaveXMLNode(pTempStream);
        wsChildren += WideString::FromUTF8(
            ByteStringView(pMemStream->GetBuffer(), pMemStream->GetSize()));
      } else if (pRawValueNode->GetElementType() == XFA_Element::Sharpxml &&
                 (contentType && *contentType == L"text/xml")) {
        pdfium::Optional<WideString> rawValue =
            pRawValueNode->JSNode()->TryAttribute(XFA_Attribute::Value, false);
        if (!rawValue || rawValue->IsEmpty())
          break;

        std::vector<WideString> wsSelTextArray;
        size_t iStart = 0;
        auto iEnd = rawValue->Find(L'\n', iStart);
        iEnd = !iEnd.has_value() ? rawValue->GetLength() : iEnd;
        while (iEnd.has_value() && iEnd >= iStart) {
          wsSelTextArray.push_back(
              rawValue->Mid(iStart, iEnd.value() - iStart));
          iStart = iEnd.value() + 1;
          if (iStart >= rawValue->GetLength())
            break;
          iEnd = rawValue->Find(L'\n', iStart);
        }

        CXFA_Node* pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
        ASSERT(pParentNode);
        CXFA_Node* pGrandparentNode =
            pParentNode->GetNodeItem(XFA_NODEITEM_Parent);
        ASSERT(pGrandparentNode);
        WideString bodyTagName;
        bodyTagName = pGrandparentNode->JSNode()->GetCData(XFA_Attribute::Name);
        if (bodyTagName.IsEmpty())
          bodyTagName = L"ListBox1";

        buf << L"<";
        buf << bodyTagName;
        buf << L" xmlns=\"\"\n>";
        for (int32_t i = 0; i < pdfium::CollectionSize<int32_t>(wsSelTextArray);
             i++) {
          buf << L"<value\n>";
          buf << ExportEncodeContent(wsSelTextArray[i]);
          buf << L"</value\n>";
        }
        buf << L"</";
        buf << bodyTagName;
        buf << L"\n>";
        wsChildren += buf.AsStringView();
        buf.Clear();
      } else {
        WideString wsValue =
            pRawValueNode->JSNode()->GetCData(XFA_Attribute::Value);
        wsChildren += ExportEncodeContent(wsValue);
      }
      break;
    }
    case XFA_ObjectType::TextNode:
    case XFA_ObjectType::NodeC:
    case XFA_ObjectType::NodeV: {
      WideString wsValue = pNode->JSNode()->GetCData(XFA_Attribute::Value);
      wsChildren += ExportEncodeContent(wsValue);
      break;
    }
    default:
      if (pNode->GetElementType() == XFA_Element::Items) {
        CXFA_Node* pTemplateNode = pNode->GetTemplateNode();
        if (!pTemplateNode ||
            pTemplateNode->CountChildren(XFA_Element::Unknown, false) !=
                pNode->CountChildren(XFA_Element::Unknown, false)) {
          bSaveXML = true;
        }
      }
      CFX_WideTextBuf newBuf;
      CXFA_Node* pChildNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
      while (pChildNode) {
        RegenerateFormFile_Changed(pChildNode, newBuf, bSaveXML);
        wsChildren += newBuf.AsStringView();
        newBuf.Clear();
        pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling);
      }
      if (!bSaveXML && !wsChildren.IsEmpty() &&
          pNode->GetElementType() == XFA_Element::Items) {
        wsChildren.clear();
        bSaveXML = true;
        CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        while (pChild) {
          RegenerateFormFile_Changed(pChild, newBuf, bSaveXML);
          wsChildren += newBuf.AsStringView();
          newBuf.Clear();
          pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
        }
      }
      break;
  }

  if (!wsChildren.IsEmpty() || !wsAttrs.IsEmpty() ||
      pNode->JSNode()->HasAttribute(XFA_Attribute::Name)) {
    WideStringView wsElement = pNode->GetClassName();
    WideString wsName;
    SaveAttribute(pNode, XFA_Attribute::Name, L"name", true, wsName);
    buf << L"<";
    buf << wsElement;
    buf << wsName;
    buf << wsAttrs;
    if (wsChildren.IsEmpty()) {
      buf << L"\n/>";
    } else {
      buf << L"\n>";
      buf << wsChildren;
      buf << L"</";
      buf << wsElement;
      buf << L"\n>";
    }
  }
}

void RegenerateFormFile_Container(
    CXFA_Node* pNode,
    const RetainPtr<CFX_SeekableStreamProxy>& pStream,
    bool bSaveXML) {
  XFA_Element eType = pNode->GetElementType();
  if (eType == XFA_Element::Field || eType == XFA_Element::Draw ||
      !pNode->IsContainerNode()) {
    CFX_WideTextBuf buf;
    RegenerateFormFile_Changed(pNode, buf, bSaveXML);
    size_t nLen = buf.GetLength();
    if (nLen > 0)
      pStream->WriteString(buf.AsStringView());
    return;
  }

  WideStringView wsElement(pNode->GetClassName());
  pStream->WriteString(L"<");
  pStream->WriteString(wsElement);

  WideString wsOutput;
  SaveAttribute(pNode, XFA_Attribute::Name, L"name", true, wsOutput);

  WideString wsAttrs;
  for (size_t i = 0;; ++i) {
    XFA_Attribute attr = pNode->GetAttribute(i);
    if (attr == XFA_Attribute::Unknown)
      break;
    if (attr == XFA_Attribute::Name)
      continue;

    WideString wsAttr;
    SaveAttribute(pNode, attr, CXFA_Node::AttributeToName(attr), false, wsAttr);
    wsOutput += wsAttr;
  }

  if (!wsOutput.IsEmpty())
    pStream->WriteString(wsOutput.AsStringView());

  CXFA_Node* pChildNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (pChildNode) {
    pStream->WriteString(L"\n>");
    while (pChildNode) {
      RegenerateFormFile_Container(pChildNode, pStream, bSaveXML);
      pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    pStream->WriteString(L"</");
    pStream->WriteString(wsElement);
    pStream->WriteString(L"\n>");
  } else {
    pStream->WriteString(L"\n/>");
  }
}

}  // namespace

void XFA_DataExporter_RegenerateFormFile(
    CXFA_Node* pNode,
    const RetainPtr<CFX_SeekableStreamProxy>& pStream,
    const char* pChecksum,
    bool bSaveXML) {
  if (pNode->IsModelNode()) {
    pStream->WriteString(L"<form");
    if (pChecksum) {
      WideString wsChecksum = WideString::FromUTF8(pChecksum);
      pStream->WriteString(L" checksum=\"");
      pStream->WriteString(wsChecksum.AsStringView());
      pStream->WriteString(L"\"");
    }
    pStream->WriteString(L" xmlns=\"");
    pStream->WriteString(WideStringView(kFormNS));

    WideString wsVersionNumber;
    RecognizeXFAVersionNumber(
        ToNode(pNode->GetDocument()->GetXFAObject(XFA_HASHCODE_Template)),
        wsVersionNumber);
    if (wsVersionNumber.IsEmpty())
      wsVersionNumber = L"2.8";

    wsVersionNumber += L"/\"\n>";
    pStream->WriteString(wsVersionNumber.AsStringView());

    CXFA_Node* pChildNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
    while (pChildNode) {
      RegenerateFormFile_Container(pChildNode, pStream, false);
      pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling);
    }
    pStream->WriteString(L"</form\n>");
  } else {
    RegenerateFormFile_Container(pNode, pStream, bSaveXML);
  }
}

void XFA_DataExporter_DealWithDataGroupNode(CXFA_Node* pDataNode) {
  if (!pDataNode || pDataNode->GetElementType() == XFA_Element::DataValue)
    return;

  int32_t iChildNum = 0;
  for (CXFA_Node* pChildNode = pDataNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildNode;
       pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    iChildNum++;
    XFA_DataExporter_DealWithDataGroupNode(pChildNode);
  }

  if (pDataNode->GetElementType() != XFA_Element::DataGroup)
    return;

  if (iChildNum > 0) {
    CFX_XMLNode* pXMLNode = pDataNode->GetXMLMappingNode();
    ASSERT(pXMLNode->GetType() == FX_XMLNODE_Element);
    CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
    if (pXMLElement->HasAttribute(L"xfa:dataNode"))
      pXMLElement->RemoveAttribute(L"xfa:dataNode");

    return;
  }

  CFX_XMLNode* pXMLNode = pDataNode->GetXMLMappingNode();
  ASSERT(pXMLNode->GetType() == FX_XMLNODE_Element);
  static_cast<CFX_XMLElement*>(pXMLNode)->SetString(L"xfa:dataNode",
                                                    L"dataGroup");
}

CXFA_DataExporter::CXFA_DataExporter(CXFA_Document* pDocument)
    : m_pDocument(pDocument) {
  ASSERT(m_pDocument);
}

CXFA_DataExporter::~CXFA_DataExporter() {}

bool CXFA_DataExporter::Export(const RetainPtr<IFX_SeekableStream>& pWrite) {
  return Export(pWrite, m_pDocument->GetRoot(), 0, nullptr);
}

bool CXFA_DataExporter::Export(const RetainPtr<IFX_SeekableStream>& pWrite,
                               CXFA_Node* pNode,
                               uint32_t dwFlag,
                               const char* pChecksum) {
  ASSERT(pWrite);
  if (!pWrite)
    return false;

  auto pStream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pWrite, true);
  pStream->SetCodePage(FX_CODEPAGE_UTF8);
  return Export(pStream, pNode, dwFlag, pChecksum);
}

bool CXFA_DataExporter::Export(
    const RetainPtr<CFX_SeekableStreamProxy>& pStream,
    CXFA_Node* pNode,
    uint32_t dwFlag,
    const char* pChecksum) {
  CFX_XMLDoc* pXMLDoc = m_pDocument->GetXMLDoc();
  if (pNode->IsModelNode()) {
    switch (pNode->GetPacketType()) {
      case XFA_PacketType::Xdp: {
        pStream->WriteString(
            L"<xdp:xdp xmlns:xdp=\"http://ns.adobe.com/xdp/\">");
        for (CXFA_Node* pChild = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
             pChild; pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
          Export(pStream, pChild, dwFlag, pChecksum);
        }
        pStream->WriteString(L"</xdp:xdp\n>");
        break;
      }
      case XFA_PacketType::Datasets: {
        CFX_XMLElement* pElement =
            static_cast<CFX_XMLElement*>(pNode->GetXMLMappingNode());
        if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
          return false;

        CXFA_Node* pDataNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
        ASSERT(pDataNode);
        XFA_DataExporter_DealWithDataGroupNode(pDataNode);
        pXMLDoc->SaveXMLNode(pStream, pElement);
        break;
      }
      case XFA_PacketType::Form: {
        XFA_DataExporter_RegenerateFormFile(pNode, pStream, pChecksum);
        break;
      }
      case XFA_PacketType::Template:
      default: {
        CFX_XMLElement* pElement =
            static_cast<CFX_XMLElement*>(pNode->GetXMLMappingNode());
        if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
          return false;

        pXMLDoc->SaveXMLNode(pStream, pElement);
        break;
      }
    }
    return true;
  }

  CXFA_Node* pDataNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  CXFA_Node* pExportNode = pNode;
  for (CXFA_Node* pChildNode = pDataNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pChildNode;
       pChildNode = pChildNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pChildNode != pNode) {
      pExportNode = pDataNode;
      break;
    }
  }
  CFX_XMLElement* pElement =
      static_cast<CFX_XMLElement*>(pExportNode->GetXMLMappingNode());
  if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
    return false;

  XFA_DataExporter_DealWithDataGroupNode(pExportNode);
  pElement->SetString(L"xmlns:xfa", L"http://www.xfa.org/schema/xfa-data/1.0/");
  pXMLDoc->SaveXMLNode(pStream, pElement);
  pElement->RemoveAttribute(L"xmlns:xfa");

  return true;
}
