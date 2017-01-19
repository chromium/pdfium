// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csssyntaxparser.h"

#include <algorithm>

#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/fde_cssdatatable.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

bool IsSelectorStart(FX_WCHAR wch) {
  return wch == '.' || wch == '#' || wch == '*' || (wch >= 'a' && wch <= 'z') ||
         (wch >= 'A' && wch <= 'Z');
}

bool ParseCSSURI(const FX_WCHAR* pszValue, int32_t* iOffset, int32_t* iLength) {
  ASSERT(pszValue && *iLength > 0);
  if (*iLength < 6 || pszValue[*iLength - 1] != ')' ||
      FXSYS_wcsnicmp(L"url(", pszValue, 4)) {
    return false;
  }
  if (CFDE_CSSDeclaration::ParseCSSString(pszValue + 4, *iLength - 5, iOffset,
                                          iLength)) {
    *iOffset += 4;
    return true;
  }
  return false;
}

}  // namespace

CFDE_CSSSyntaxParser::CFDE_CSSSyntaxParser()
    : m_iTextDatLen(0),
      m_dwCheck((uint32_t)-1),
      m_eMode(FDE_CSSSyntaxMode::RuleSet),
      m_eStatus(FDE_CSSSyntaxStatus::None) {}

CFDE_CSSSyntaxParser::~CFDE_CSSSyntaxParser() {
  m_TextData.Reset();
  m_TextPlane.Reset();
}

bool CFDE_CSSSyntaxParser::Init(const FX_WCHAR* pBuffer,
                                int32_t iBufferSize,
                                int32_t iTextDatSize,
                                bool bOnlyDeclaration) {
  ASSERT(pBuffer && iBufferSize > 0 && iTextDatSize > 0);
  Reset(bOnlyDeclaration);
  if (!m_TextData.EstimateSize(iTextDatSize))
    return false;
  return m_TextPlane.AttachBuffer(pBuffer, iBufferSize);
}

void CFDE_CSSSyntaxParser::Reset(bool bOnlyDeclaration) {
  m_TextPlane.Reset();
  m_TextData.Reset();
  m_iTextDatLen = 0;
  m_dwCheck = (uint32_t)-1;
  m_eStatus = FDE_CSSSyntaxStatus::None;
  m_eMode = bOnlyDeclaration ? FDE_CSSSyntaxMode::PropertyName
                             : FDE_CSSSyntaxMode::RuleSet;
}

FDE_CSSSyntaxStatus CFDE_CSSSyntaxParser::DoSyntaxParse() {
  while (m_eStatus >= FDE_CSSSyntaxStatus::None) {
    if (m_TextPlane.IsEOF()) {
      if (m_eMode == FDE_CSSSyntaxMode::PropertyValue &&
          m_TextData.GetLength() > 0) {
        SaveTextData();
        m_eStatus = FDE_CSSSyntaxStatus::PropertyValue;
        return m_eStatus;
      }
      m_eStatus = FDE_CSSSyntaxStatus::EOS;
      return m_eStatus;
    }
    FX_WCHAR wch;
    while (!m_TextPlane.IsEOF()) {
      wch = m_TextPlane.GetChar();
      switch (m_eMode) {
        case FDE_CSSSyntaxMode::RuleSet:
          switch (wch) {
            case '@':
              m_TextPlane.MoveNext();
              SwitchMode(FDE_CSSSyntaxMode::AtRule);
              break;
            case '}':
              m_TextPlane.MoveNext();
              if (RestoreMode())
                return FDE_CSSSyntaxStatus::DeclClose;

              m_eStatus = FDE_CSSSyntaxStatus::Error;
              return m_eStatus;
            case '/':
              if (m_TextPlane.GetNextChar() == '*') {
                m_ModeStack.push(m_eMode);
                SwitchMode(FDE_CSSSyntaxMode::Comment);
                break;
              }
            default:
              if (wch <= ' ') {
                m_TextPlane.MoveNext();
              } else if (IsSelectorStart(wch)) {
                SwitchMode(FDE_CSSSyntaxMode::Selector);
                return FDE_CSSSyntaxStatus::StyleRule;
              } else {
                m_eStatus = FDE_CSSSyntaxStatus::Error;
                return m_eStatus;
              }
              break;
          }
          break;
        case FDE_CSSSyntaxMode::Selector:
          switch (wch) {
            case ',':
              m_TextPlane.MoveNext();
              SwitchMode(FDE_CSSSyntaxMode::Selector);
              if (m_iTextDatLen > 0)
                return FDE_CSSSyntaxStatus::Selector;
              break;
            case '{':
              if (m_TextData.GetLength() > 0) {
                SaveTextData();
                return FDE_CSSSyntaxStatus::Selector;
              }
              m_TextPlane.MoveNext();
              m_ModeStack.push(FDE_CSSSyntaxMode::RuleSet);
              SwitchMode(FDE_CSSSyntaxMode::PropertyName);
              return FDE_CSSSyntaxStatus::DeclOpen;
            case '/':
              if (m_TextPlane.GetNextChar() == '*') {
                if (SwitchToComment() > 0)
                  return FDE_CSSSyntaxStatus::Selector;
                break;
              }
            default:
              AppendChar(wch);
              break;
          }
          break;
        case FDE_CSSSyntaxMode::PropertyName:
          switch (wch) {
            case ':':
              m_TextPlane.MoveNext();
              SwitchMode(FDE_CSSSyntaxMode::PropertyValue);
              return FDE_CSSSyntaxStatus::PropertyName;
            case '}':
              m_TextPlane.MoveNext();
              if (RestoreMode())
                return FDE_CSSSyntaxStatus::DeclClose;

              m_eStatus = FDE_CSSSyntaxStatus::Error;
              return m_eStatus;
            case '/':
              if (m_TextPlane.GetNextChar() == '*') {
                if (SwitchToComment() > 0)
                  return FDE_CSSSyntaxStatus::PropertyName;
                break;
              }
            default:
              AppendChar(wch);
              break;
          }
          break;
        case FDE_CSSSyntaxMode::PropertyValue:
          switch (wch) {
            case ';':
              m_TextPlane.MoveNext();
            case '}':
              SwitchMode(FDE_CSSSyntaxMode::PropertyName);
              return FDE_CSSSyntaxStatus::PropertyValue;
            case '/':
              if (m_TextPlane.GetNextChar() == '*') {
                if (SwitchToComment() > 0)
                  return FDE_CSSSyntaxStatus::PropertyValue;
                break;
              }
            default:
              AppendChar(wch);
              break;
          }
          break;
        case FDE_CSSSyntaxMode::Comment:
          if (wch == '/' && m_TextData.GetLength() > 0 &&
              m_TextData.GetAt(m_TextData.GetLength() - 1) == '*') {
            RestoreMode();
          } else {
            m_TextData.AppendChar(wch);
          }
          m_TextPlane.MoveNext();
          break;
        case FDE_CSSSyntaxMode::MediaType:
          switch (wch) {
            case ',':
              m_TextPlane.MoveNext();
              SwitchMode(FDE_CSSSyntaxMode::MediaType);
              if (m_iTextDatLen > 0)
                return FDE_CSSSyntaxStatus::MediaType;
              break;
            case '{': {
              if (m_ModeStack.empty() ||
                  m_ModeStack.top() != FDE_CSSSyntaxMode::MediaRule) {
                m_eStatus = FDE_CSSSyntaxStatus::Error;
                return m_eStatus;
              }

              if (m_TextData.GetLength() > 0) {
                SaveTextData();
                return FDE_CSSSyntaxStatus::MediaType;
              }
              m_TextPlane.MoveNext();

              // Replace the MediaRule with a RuleSet rule.
              m_ModeStack.top() = FDE_CSSSyntaxMode::RuleSet;

              SwitchMode(FDE_CSSSyntaxMode::RuleSet);
              return FDE_CSSSyntaxStatus::DeclOpen;
            }
            case ';': {
              if (m_ModeStack.empty() ||
                  m_ModeStack.top() != FDE_CSSSyntaxMode::Import) {
                m_eStatus = FDE_CSSSyntaxStatus::Error;
                return m_eStatus;
              }

              if (m_TextData.GetLength() > 0) {
                SaveTextData();
                if (IsImportEnabled())
                  return FDE_CSSSyntaxStatus::MediaType;
              } else {
                bool bEnabled = IsImportEnabled();
                m_TextPlane.MoveNext();
                m_ModeStack.pop();
                SwitchMode(FDE_CSSSyntaxMode::RuleSet);
                if (bEnabled) {
                  DisableImport();
                  return FDE_CSSSyntaxStatus::ImportClose;
                }
              }
            } break;
            case '/':
              if (m_TextPlane.GetNextChar() == '*') {
                if (SwitchToComment() > 0)
                  return FDE_CSSSyntaxStatus::MediaType;
                break;
              }
            default:
              AppendChar(wch);
              break;
          }
          break;
        case FDE_CSSSyntaxMode::URI: {
          if (m_ModeStack.empty() ||
              m_ModeStack.top() != FDE_CSSSyntaxMode::Import) {
            m_eStatus = FDE_CSSSyntaxStatus::Error;
            return m_eStatus;
          }

          if (wch <= ' ' || wch == ';') {
            int32_t iURIStart, iURILength = m_TextData.GetLength();
            if (iURILength > 0 &&
                ParseCSSURI(m_TextData.GetBuffer(), &iURIStart, &iURILength)) {
              m_TextData.Subtract(iURIStart, iURILength);
              SwitchMode(FDE_CSSSyntaxMode::MediaType);
              if (IsImportEnabled())
                return FDE_CSSSyntaxStatus::URI;
              break;
            }
          }
          AppendChar(wch);
        } break;
        case FDE_CSSSyntaxMode::AtRule:
          if (wch > ' ') {
            AppendChar(wch);
          } else {
            int32_t iLen = m_TextData.GetLength();
            const FX_WCHAR* psz = m_TextData.GetBuffer();
            if (FXSYS_wcsncmp(L"charset", psz, iLen) == 0) {
              SwitchMode(FDE_CSSSyntaxMode::Charset);
            } else if (FXSYS_wcsncmp(L"import", psz, iLen) == 0) {
              m_ModeStack.push(FDE_CSSSyntaxMode::Import);
              SwitchMode(FDE_CSSSyntaxMode::URI);
              if (IsImportEnabled())
                return FDE_CSSSyntaxStatus::ImportRule;
              break;
            } else if (FXSYS_wcsncmp(L"media", psz, iLen) == 0) {
              m_ModeStack.push(FDE_CSSSyntaxMode::MediaRule);
              SwitchMode(FDE_CSSSyntaxMode::MediaType);
              return FDE_CSSSyntaxStatus::MediaRule;
            } else if (FXSYS_wcsncmp(L"font-face", psz, iLen) == 0) {
              SwitchMode(FDE_CSSSyntaxMode::Selector);
              return FDE_CSSSyntaxStatus::FontFaceRule;
            } else if (FXSYS_wcsncmp(L"page", psz, iLen) == 0) {
              SwitchMode(FDE_CSSSyntaxMode::Selector);
              return FDE_CSSSyntaxStatus::PageRule;
            } else {
              SwitchMode(FDE_CSSSyntaxMode::UnknownRule);
            }
          }
          break;
        case FDE_CSSSyntaxMode::Charset:
          if (wch == ';') {
            m_TextPlane.MoveNext();
            SwitchMode(FDE_CSSSyntaxMode::RuleSet);
            if (IsCharsetEnabled()) {
              DisableCharset();
              if (m_iTextDatLen > 0)
                return FDE_CSSSyntaxStatus::Charset;
            }
          } else {
            AppendChar(wch);
          }
          break;
        case FDE_CSSSyntaxMode::UnknownRule:
          if (wch == ';')
            SwitchMode(FDE_CSSSyntaxMode::RuleSet);
          m_TextPlane.MoveNext();
          break;
        default:
          ASSERT(false);
          break;
      }
    }
  }
  return m_eStatus;
}

bool CFDE_CSSSyntaxParser::IsImportEnabled() const {
  if ((m_dwCheck & FDE_CSSSYNTAXCHECK_AllowImport) == 0)
    return false;
  if (m_ModeStack.size() > 1)
    return false;
  return true;
}

bool CFDE_CSSSyntaxParser::AppendChar(FX_WCHAR wch) {
  m_TextPlane.MoveNext();
  if (m_TextData.GetLength() > 0 || wch > ' ') {
    m_TextData.AppendChar(wch);
    return true;
  }
  return false;
}

int32_t CFDE_CSSSyntaxParser::SaveTextData() {
  m_iTextDatLen = m_TextData.TrimEnd();
  m_TextData.Clear();
  return m_iTextDatLen;
}

void CFDE_CSSSyntaxParser::SwitchMode(FDE_CSSSyntaxMode eMode) {
  m_eMode = eMode;
  SaveTextData();
}

int32_t CFDE_CSSSyntaxParser::SwitchToComment() {
  int32_t iLength = m_TextData.GetLength();
  m_ModeStack.push(m_eMode);
  SwitchMode(FDE_CSSSyntaxMode::Comment);
  return iLength;
}

bool CFDE_CSSSyntaxParser::RestoreMode() {
  if (m_ModeStack.empty())
    return false;

  SwitchMode(m_ModeStack.top());
  m_ModeStack.pop();
  return true;
}

const FX_WCHAR* CFDE_CSSSyntaxParser::GetCurrentString(int32_t& iLength) const {
  iLength = m_iTextDatLen;
  return m_TextData.GetBuffer();
}
