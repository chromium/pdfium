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
  m_Output.InitWithSize(32);
  m_Input.AttachBuffer(pBuffer, iBufferSize);
}

CFX_CSSSyntaxParser::~CFX_CSSSyntaxParser() = default;

void CFX_CSSSyntaxParser::SetParseOnlyDeclarations() {
  m_eMode = SyntaxMode::kPropertyName;
}

CFX_CSSSyntaxStatus CFX_CSSSyntaxParser::DoSyntaxParse() {
  if (m_bError)
    return CFX_CSSSyntaxStatus::kError;

  while (!m_Input.IsEOF()) {
    wchar_t wch = m_Input.GetChar();
    switch (m_eMode) {
      case SyntaxMode::kRuleSet:
        switch (wch) {
          case '}':
            m_Input.MoveNext();
            if (RestoreMode())
              return CFX_CSSSyntaxStatus::kDeclClose;
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
            if (m_iTextDataLen > 0)
              return CFX_CSSSyntaxStatus::kSelector;
            break;
          case '{':
            if (m_Output.GetLength() > 0) {
              SaveTextData();
              return CFX_CSSSyntaxStatus::kSelector;
            }
            m_Input.MoveNext();
            m_ModeStack.push(SyntaxMode::kRuleSet);
            SwitchMode(SyntaxMode::kPropertyName);
            return CFX_CSSSyntaxStatus::kDeclOpen;
          case '/':
            if (m_Input.GetNextChar() == '*') {
              if (SwitchToComment() > 0)
                return CFX_CSSSyntaxStatus::kSelector;
              break;
            }
            FALLTHROUGH;
          default:
            AppendCharIfNotLeadingBlank(wch);
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
              if (SwitchToComment() > 0)
                return CFX_CSSSyntaxStatus::kPropertyName;
              break;
            }
            FALLTHROUGH;
          default:
            AppendCharIfNotLeadingBlank(wch);
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
              if (SwitchToComment() > 0)
                return CFX_CSSSyntaxStatus::kPropertyValue;
              break;
            }
            FALLTHROUGH;
          default:
            AppendCharIfNotLeadingBlank(wch);
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
      case SyntaxMode::kUnknownRule:
        if (wch == ';')
          SwitchMode(SyntaxMode::kRuleSet);
        m_Input.MoveNext();
        break;
      default:
        NOTREACHED();
        break;
    }
  }
  if (m_eMode == SyntaxMode::kPropertyValue && m_Output.GetLength() > 0) {
    SaveTextData();
    return CFX_CSSSyntaxStatus::kPropertyValue;
  }
  return CFX_CSSSyntaxStatus::kEOS;
}

void CFX_CSSSyntaxParser::AppendCharIfNotLeadingBlank(wchar_t wch) {
  if (m_Output.GetLength() > 0 || wch > ' ')
    m_Output.AppendChar(wch);
}

void CFX_CSSSyntaxParser::SaveTextData() {
  m_iTextDataLen = m_Output.TrimEnd();
  m_Output.Clear();
}

void CFX_CSSSyntaxParser::SwitchMode(SyntaxMode eMode) {
  m_eMode = eMode;
  SaveTextData();
}

int32_t CFX_CSSSyntaxParser::SwitchToComment() {
  int32_t iLength = m_Output.GetLength();
  m_ModeStack.push(m_eMode);
  SwitchMode(SyntaxMode::kComment);
  return iLength;
}

bool CFX_CSSSyntaxParser::RestoreMode() {
  if (m_ModeStack.empty())
    return false;

  SwitchMode(m_ModeStack.top());
  m_ModeStack.pop();
  return true;
}

WideStringView CFX_CSSSyntaxParser::GetCurrentString() const {
  return WideStringView(m_Output.GetBuffer(), m_iTextDataLen);
}
