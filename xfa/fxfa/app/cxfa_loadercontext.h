// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_LOADERCONTEXT_H_
#define XFA_FXFA_APP_CXFA_LOADERCONTEXT_H_

#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/cfde_csscomputedstyle.h"

class CFDE_XMLNode;
class CXFA_Node;

class CXFA_LoaderContext {
 public:
  CXFA_LoaderContext();
  ~CXFA_LoaderContext();

  bool m_bSaveLineHeight;
  FX_FLOAT m_fWidth;
  FX_FLOAT m_fHeight;
  FX_FLOAT m_fLastPos;
  FX_FLOAT m_fStartLineOffset;
  int32_t m_iChar;
  int32_t m_iLines;
  int32_t m_iTotalLines;
  CFDE_XMLNode* m_pXMLNode;
  CXFA_Node* m_pNode;
  CFX_RetainPtr<CFDE_CSSComputedStyle> m_pParentStyle;
  CFX_ArrayTemplate<FX_FLOAT> m_lineHeights;
  uint32_t m_dwFlags;
  std::vector<FX_FLOAT> m_BlocksHeight;
};

#endif  // XFA_FXFA_APP_CXFA_LOADERCONTEXT_H_
