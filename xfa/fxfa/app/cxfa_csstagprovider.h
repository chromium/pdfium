// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_CXFA_CSSTAGPROVIDER_H_
#define XFA_FXFA_APP_CXFA_CSSTAGPROVIDER_H_

#include <map>

#include "core/fxcrt/fx_string.h"

class CXFA_CSSTagProvider {
 public:
  using AttributeMap = std::map<CFX_WideString, CFX_WideString>;

  CXFA_CSSTagProvider();
  ~CXFA_CSSTagProvider();

  CFX_WideString GetTagName() { return m_wsTagName; }

  AttributeMap::iterator begin() { return m_Attributes.begin(); }
  AttributeMap::iterator end() { return m_Attributes.end(); }

  bool empty() const { return m_Attributes.empty(); }

  void SetTagNameObj(const CFX_WideString& wsName) { m_wsTagName = wsName; }
  void SetAttribute(const CFX_WideString& wsAttr,
                    const CFX_WideString& wsValue) {
    m_Attributes.insert({wsAttr, wsValue});
  }

  bool m_bTagAvailable;
  bool m_bContent;

 protected:
  CFX_WideString m_wsTagName;
  AttributeMap m_Attributes;
};

#endif  // XFA_FXFA_APP_CXFA_CSSTAGPROVIDER_H_
