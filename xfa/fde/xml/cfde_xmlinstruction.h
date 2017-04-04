// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_
#define XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlnode.h"

class CFDE_XMLInstruction : public CFDE_XMLNode {
 public:
  explicit CFDE_XMLInstruction(const CFX_WideString& wsTarget);
  ~CFDE_XMLInstruction() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFDE_XMLNode> Clone() override;

  void GetTargetName(CFX_WideString& wsTarget) const { wsTarget = m_wsTarget; }
  int32_t CountAttributes() const;
  bool GetAttribute(int32_t index,
                    CFX_WideString& wsAttriName,
                    CFX_WideString& wsAttriValue) const;
  bool HasAttribute(const wchar_t* pwsAttriName) const;
  void GetString(const wchar_t* pwsAttriName,
                 CFX_WideString& wsAttriValue,
                 const wchar_t* pwsDefValue = nullptr) const;
  void SetString(const CFX_WideString& wsAttriName,
                 const CFX_WideString& wsAttriValue);
  int32_t GetInteger(const wchar_t* pwsAttriName, int32_t iDefValue = 0) const;
  void SetInteger(const wchar_t* pwsAttriName, int32_t iAttriValue);
  float GetFloat(const wchar_t* pwsAttriName, float fDefValue = 0) const;
  void SetFloat(const wchar_t* pwsAttriName, float fAttriValue);
  void RemoveAttribute(const wchar_t* pwsAttriName);
  int32_t CountData() const;
  bool GetData(int32_t index, CFX_WideString& wsData) const;
  void AppendData(const CFX_WideString& wsData);
  void RemoveData(int32_t index);

  CFX_WideString m_wsTarget;
  std::vector<CFX_WideString> m_Attributes;
  std::vector<CFX_WideString> m_TargetData;
};

#endif  // XFA_FDE_XML_CFDE_XMLINSTRUCTION_H_
