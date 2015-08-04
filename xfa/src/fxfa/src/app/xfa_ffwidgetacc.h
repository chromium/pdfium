// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_WIDGETACC_IMP_H
#define _FXFA_FORMFILLER_WIDGETACC_IMP_H
#include "xfa_textlayout.h"
enum XFA_TEXTPROVIDERTYPE {
  XFA_TEXTPROVIDERTYPE_Text,
  XFA_TEXTPROVIDERTYPE_Datasets,
  XFA_TEXTPROVIDERTYPE_Caption,
  XFA_TEXTPROVIDERTYPE_Rollover,
  XFA_TEXTPROVIDERTYPE_Down,
};
class CXFA_TextProvider : public IXFA_TextProvider {
 public:
  CXFA_TextProvider(CXFA_WidgetAcc* pWidgetAcc,
                    XFA_TEXTPROVIDERTYPE eType,
                    CXFA_Node* pTextNode = NULL)
      : m_pWidgetAcc(pWidgetAcc), m_eType(eType), m_pTextNode(pTextNode) {
    FXSYS_assert(m_pWidgetAcc != NULL);
  }
  virtual ~CXFA_TextProvider() {}
  virtual CXFA_Node* GetTextNode(FX_BOOL& bRichText);
  virtual CXFA_Para GetParaNode();
  virtual CXFA_Font GetFontNode();
  virtual FX_BOOL IsCheckButtonAndAutoWidth();
  virtual CXFA_FFDoc* GetDocNode() { return m_pWidgetAcc->GetDoc(); }
  virtual FX_BOOL GetEmbbedObj(FX_BOOL bURI,
                               FX_BOOL bRaw,
                               const CFX_WideString& wsAttr,
                               CFX_WideString& wsValue);

 protected:
  CXFA_WidgetAcc* m_pWidgetAcc;
  XFA_TEXTPROVIDERTYPE m_eType;
  CXFA_Node* m_pTextNode;
};
#endif
