// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSSYNTAXPARSER_H_
#define CORE_FXCRT_CSS_CFX_CSSSYNTAXPARSER_H_

#include <stack>

#include "core/fxcrt/css/cfx_cssexttextbuf.h"
#include "core/fxcrt/css/cfx_csstextbuf.h"
#include "core/fxcrt/fx_string.h"

#define CFX_CSSSYNTAXCHECK_AllowCharset 1
#define CFX_CSSSYNTAXCHECK_AllowImport 2

enum class CFX_CSSSyntaxMode {
  RuleSet,
  Comment,
  UnknownRule,
  Selector,
  PropertyName,
  PropertyValue,
};

enum class CFX_CSSSyntaxStatus : uint8_t {
  Error,
  EOS,
  None,
  StyleRule,
  Selector,
  DeclOpen,
  DeclClose,
  PropertyName,
  PropertyValue,
};

class CFX_CSSSyntaxParser {
 public:
  CFX_CSSSyntaxParser(const wchar_t* pBuffer, int32_t iBufferSize);
  CFX_CSSSyntaxParser(const wchar_t* pBuffer,
                      int32_t iBufferSize,
                      int32_t iTextDatSize,
                      bool bOnlyDeclaration);
  ~CFX_CSSSyntaxParser();

  CFX_CSSSyntaxStatus DoSyntaxParse();
  WideStringView GetCurrentString() const;

 private:
  void SwitchMode(CFX_CSSSyntaxMode eMode);
  int32_t SwitchToComment();

  bool RestoreMode();
  void AppendCharIfNotLeadingBlank(wchar_t wch);
  void SaveTextData();

  CFX_CSSTextBuf m_TextData;
  CFX_CSSExtTextBuf m_TextPlane;
  int32_t m_iTextDataLen;
  CFX_CSSSyntaxMode m_eMode;
  CFX_CSSSyntaxStatus m_eStatus;
  std::stack<CFX_CSSSyntaxMode> m_ModeStack;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSSYNTAXPARSER_H_
