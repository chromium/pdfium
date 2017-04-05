// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_
#define XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlattributenode.h"

class CFDE_XMLInstruction : public CFDE_XMLAttributeNode {
 public:
  explicit CFDE_XMLInstruction(const CFX_WideString& wsTarget);
  ~CFDE_XMLInstruction() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFDE_XMLNode> Clone() override;

  const std::vector<CFX_WideString>& GetTargetData() const {
    return m_TargetData;
  }
  void AppendData(const CFX_WideString& wsData);
  void RemoveData(int32_t index);

 private:
  std::vector<CFX_WideString> m_TargetData;
};

#endif  // XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_
