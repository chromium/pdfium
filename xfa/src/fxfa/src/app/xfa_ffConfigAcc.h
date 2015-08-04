// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLERCONFIGACC_IMP_H
#define _FXFA_FORMFILLERCONFIGACC_IMP_H
class CXFA_FFConfigAcc {
 public:
  CXFA_FFConfigAcc(CXFA_Node* pNode);
  ~CXFA_FFConfigAcc();
  int32_t CountChildren();
  FX_BOOL GetFontInfo(int32_t index,
                      CFX_WideString& wsFontFamily,
                      CFX_WideString& wsPsName,
                      FX_BOOL bBold,
                      FX_BOOL bItalic);

 private:
  void GetPsMapNode();
  CXFA_Node* m_pNode;
  CXFA_Node* m_pPsMapNode;
};
#endif
