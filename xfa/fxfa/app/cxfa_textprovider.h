// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_TEXTPROVIDER_H_
#define XFA_FXFA_APP_CXFA_TEXTPROVIDER_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/app/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_widgetacc.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_para.h"

class CXFA_FFDoc;
class CXFA_Node;

enum XFA_TEXTPROVIDERTYPE {
  XFA_TEXTPROVIDERTYPE_Text,
  XFA_TEXTPROVIDERTYPE_Datasets,
  XFA_TEXTPROVIDERTYPE_Caption,
  XFA_TEXTPROVIDERTYPE_Rollover,
  XFA_TEXTPROVIDERTYPE_Down,
};

class CXFA_TextProvider {
 public:
  CXFA_TextProvider(CXFA_WidgetAcc* pWidgetAcc, XFA_TEXTPROVIDERTYPE eType)
      : m_pWidgetAcc(pWidgetAcc), m_eType(eType) {
    ASSERT(m_pWidgetAcc);
  }
  ~CXFA_TextProvider() {}

  CXFA_Node* GetTextNode(bool& bRichText);
  CXFA_Para GetParaNode();
  CXFA_Font GetFontNode();
  bool IsCheckButtonAndAutoWidth();
  CXFA_FFDoc* GetDocNode() { return m_pWidgetAcc->GetDoc(); }
  bool GetEmbbedObj(bool bURI,
                    bool bRaw,
                    const CFX_WideString& wsAttr,
                    CFX_WideString& wsValue);

 private:
  CXFA_WidgetAcc* m_pWidgetAcc;
  XFA_TEXTPROVIDERTYPE m_eType;
};

#endif  // XFA_FXFA_APP_CXFA_TEXTPROVIDER_H_
