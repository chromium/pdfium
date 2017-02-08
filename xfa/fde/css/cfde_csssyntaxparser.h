// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSYNTAXPARSER_H_
#define XFA_FDE_CSS_CFDE_CSSSYNTAXPARSER_H_

#include <stack>

#include "xfa/fde/css/cfde_csstextbuf.h"

#define FDE_CSSSYNTAXCHECK_AllowCharset 1
#define FDE_CSSSYNTAXCHECK_AllowImport 2

enum class FDE_CSSSyntaxMode {
  RuleSet,
  Comment,
  UnknownRule,
  Selector,
  PropertyName,
  PropertyValue,
};

enum class FDE_CSSSyntaxStatus : uint8_t {
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

class CFDE_CSSSyntaxParser {
 public:
  CFDE_CSSSyntaxParser();
  ~CFDE_CSSSyntaxParser();

  bool Init(const FX_WCHAR* pBuffer,
            int32_t iBufferSize,
            int32_t iTextDatSize = 32,
            bool bOnlyDeclaration = false);
  FDE_CSSSyntaxStatus DoSyntaxParse();
  CFX_WideStringC GetCurrentString() const;

 protected:
  void Reset(bool bOnlyDeclaration);
  void SwitchMode(FDE_CSSSyntaxMode eMode);
  int32_t SwitchToComment();

  bool RestoreMode();
  bool AppendChar(FX_WCHAR wch);
  int32_t SaveTextData();
  bool IsCharsetEnabled() const {
    return (m_dwCheck & FDE_CSSSYNTAXCHECK_AllowCharset) != 0;
  }
  void DisableCharset() { m_dwCheck = FDE_CSSSYNTAXCHECK_AllowImport; }
  bool IsImportEnabled() const;
  void DisableImport() { m_dwCheck = 0; }

  CFDE_CSSTextBuf m_TextData;
  CFDE_CSSTextBuf m_TextPlane;
  int32_t m_iTextDataLen;
  uint32_t m_dwCheck;
  FDE_CSSSyntaxMode m_eMode;
  FDE_CSSSyntaxStatus m_eStatus;
  std::stack<FDE_CSSSyntaxMode> m_ModeStack;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSYNTAXPARSER_H_
