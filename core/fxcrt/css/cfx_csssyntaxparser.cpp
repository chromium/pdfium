// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_csssyntaxparser.h"

#include <algorithm>

#include "core/fxcrt/css/cfx_cssdata.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/logging.h"

namespace {

bool IsSelectorStart(wchar_t wch) {
  return wch == '.' || wch == '#' || wch == '*' ||
         (isascii(wch) && isalpha(wch));
}

}  // namespace

CFX_CSSSyntaxParser::CFX_CSSSyntaxParser(const wchar_t* pBuffer,
                                         int32_t iBufferSize) {
  ASSERT(pBuffer);
  m_Input.AttachBuffer(pBuffer, iBufferSize);
}

CFX_CSSSyntaxParser::~CFX_CSSSyntaxParser() = default;

void CFX_CSSSyntaxParser::SetParseOnlyDeclarations() {
  m_eMode = SyntaxMode::kPropertyName;
}

CFX_CSSSyntaxStatus CFX_CSSSyntaxParser::DoSyntaxParse() {
  m_Output.Clear();
  if (m_bError)
    return CFX_CSSSyntaxStatus::kError;

  while (!m_Input.IsEOF()) {
    wchar_t wch = m_Input.GetChar();
    switch (m_eMode) {
      case SyntaxMode::kRuleSet:
        switch (wch) {
          case '}':
            m_bError = true;
            return CFX_CSSSyntaxStatus::kError;
          case '/':
            if (m_Input.GetNextChar() == '*') {
              m_ModeStack.push(m_eMode);
              SwitchMode(SyntaxMode::kComment);
              break;
            }
            FALLTHROUGH;
          default:
            if (wch <= ' ') {
              m_Input.MoveNext();
            } else if (IsSelectorStart(wch)) {
              SwitchMode(SyntaxMode::kSelector);
              return CFX_CSSSyntaxStatus::kStyleRule;
            } else {
              m_bError = true;
              return CFX_CSSSyntaxStatus::kError;
            }
            break;
        }
        break;
      case SyntaxMode::kSelector:
        switch (wch) {
          case ',':
            m_Input.MoveNext();
            SwitchMode(SyntaxMode::kSelector);
            if (!m_Output.IsEmpty())
              return CFX_CSSSyntaxStatus::kSelector;
            break;
          case '{':
            if (!m_Output.IsEmpty())
              return CFX_CSSSyntaxStatus::kSelector;
            m_Input.MoveNext();
            m_ModeStack.push(SyntaxMode::kRuleSet);
            SwitchMode(SyntaxMode::kPropertyName);
            return CFX_CSSSyntaxStatus::kDeclOpen;
          case '/':
            if (m_Input.GetNextChar() == '*') {
              if (SwitchToComment())
                return CFX_CSSSyntaxStatus::kSelector;
              break;
            }
            FALLTHROUGH;
          default:
            m_Output.AppendCharIfNotLeadingBlank(wch);
            m_Input.MoveNext();
            break;
        }
        break;
      case SyntaxMode::kPropertyName:
        switch (wch) {
          case ':':
            m_Input.MoveNext();
            SwitchMode(SyntaxMode::kPropertyValue);
            return CFX_CSSSyntaxStatus::kPropertyName;
          case '}':
            m_Input.MoveNext();
            if (RestoreMode())
              return CFX_CSSSyntaxStatus::kDeclClose;

            m_bError = true;
            return CFX_CSSSyntaxStatus::kError;
          case '/':
            if (m_Input.GetNextChar() == '*') {
              if (SwitchToComment())
                return CFX_CSSSyntaxStatus::kPropertyName;
              break;
            }
            FALLTHROUGH;
          default:
            m_Output.AppendCharIfNotLeadingBlank(wch);
            m_Input.MoveNext();
            break;
        }
        break;
      case SyntaxMode::kPropertyValue:
        switch (wch) {
          case ';':
            m_Input.MoveNext();
            FALLTHROUGH;
          case '}':
            SwitchMode(SyntaxMode::kPropertyName);
            return CFX_CSSSyntaxStatus::kPropertyValue;
          case '/':
            if (m_Input.GetNextChar() == '*') {
              if (SwitchToComment())
                return CFX_CSSSyntaxStatus::kPropertyValue;
              break;
            }
            FALLTHROUGH;
          default:
            m_Output.AppendCharIfNotLeadingBlank(wch);
            m_Input.MoveNext();
            break;
        }
        break;
      case SyntaxMode::kComment:
        if (wch == '*' && m_Input.GetNextChar() == '/') {
          RestoreMode();
          m_Input.MoveNext();
        }
        m_Input.MoveNext();
        break;
      default:
        NOTREACHED();
        break;
    }
  }
  if (m_eMode == SyntaxMode::kPropertyValue && !m_Output.IsEmpty())
    return CFX_CSSSyntaxStatus::kPropertyValue;

  return CFX_CSSSyntaxStatus::kEOS;
}

void CFX_CSSSyntaxParser::SwitchMode(SyntaxMode eMode) {
  m_eMode = eMode;
}

bool CFX_CSSSyntaxParser::SwitchToComment() {
  const bool bEmpty = m_Output.IsEmpty();
  m_ModeStack.push(m_eMode);
  SwitchMode(SyntaxMode::kComment);
  return !bEmpty;
}

bool CFX_CSSSyntaxParser::RestoreMode() {
  if (m_ModeStack.empty())
    return false;

  SwitchMode(m_ModeStack.top());
  m_ModeStack.pop();
  return true;
}

WideStringView CFX_CSSSyntaxParser::GetCurrentString() const {
  return m_Output.GetTrailingBlankTrimmedString();
}
