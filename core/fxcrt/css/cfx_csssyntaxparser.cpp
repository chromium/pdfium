// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_csssyntaxparser.h"

#include "core/fxcrt/css/cfx_cssdata.h"
#include "core/fxcrt/css/cfx_cssdeclaration.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"

namespace {

bool IsSelectorStart(wchar_t wch) {
  return wch == '.' || wch == '#' || wch == '*' ||
         (isascii(wch) && isalpha(wch));
}

}  // namespace

CFX_CSSSyntaxParser::CFX_CSSSyntaxParser(WideStringView str) : input_(str) {}

CFX_CSSSyntaxParser::~CFX_CSSSyntaxParser() = default;

void CFX_CSSSyntaxParser::SetParseOnlyDeclarations() {
  mode_ = Mode::kPropertyName;
}

CFX_CSSSyntaxParser::Status CFX_CSSSyntaxParser::DoSyntaxParse() {
  output_.Clear();
  if (has_error_) {
    return Status::kError;
  }

  while (!input_.IsEOF()) {
    wchar_t wch = input_.GetChar();
    switch (mode_) {
      case Mode::kRuleSet:
        switch (wch) {
          case '}':
            has_error_ = true;
            return Status::kError;
          case '/':
            if (input_.GetNextChar() == '*') {
              SaveMode(Mode::kRuleSet);
              mode_ = Mode::kComment;
              break;
            }
            [[fallthrough]];
          default:
            if (wch <= ' ') {
              input_.MoveNext();
            } else if (IsSelectorStart(wch)) {
              mode_ = Mode::kSelector;
              return Status::kStyleRule;
            } else {
              has_error_ = true;
              return Status::kError;
            }
            break;
        }
        break;
      case Mode::kSelector:
        switch (wch) {
          case ',':
            input_.MoveNext();
            if (!output_.IsEmpty()) {
              return Status::kSelector;
            }
            break;
          case '{':
            if (!output_.IsEmpty()) {
              return Status::kSelector;
            }
            input_.MoveNext();
            SaveMode(Mode::kRuleSet);  // Back to validate ruleset again.
            mode_ = Mode::kPropertyName;
            return Status::kDeclOpen;
          case '/':
            if (input_.GetNextChar() == '*') {
              SaveMode(Mode::kSelector);
              mode_ = Mode::kComment;
              if (!output_.IsEmpty()) {
                return Status::kSelector;
              }
              break;
            }
            [[fallthrough]];
          default:
            output_.AppendCharIfNotLeadingBlank(wch);
            input_.MoveNext();
            break;
        }
        break;
      case Mode::kPropertyName:
        switch (wch) {
          case ':':
            input_.MoveNext();
            mode_ = Mode::kPropertyValue;
            return Status::kPropertyName;
          case '}':
            input_.MoveNext();
            if (!RestoreMode()) {
              return Status::kError;
            }

            return Status::kDeclClose;
          case '/':
            if (input_.GetNextChar() == '*') {
              SaveMode(Mode::kPropertyName);
              mode_ = Mode::kComment;
              if (!output_.IsEmpty()) {
                return Status::kPropertyName;
              }
              break;
            }
            [[fallthrough]];
          default:
            output_.AppendCharIfNotLeadingBlank(wch);
            input_.MoveNext();
            break;
        }
        break;
      case Mode::kPropertyValue:
        switch (wch) {
          case ';':
            input_.MoveNext();
            [[fallthrough]];
          case '}':
            mode_ = Mode::kPropertyName;
            return Status::kPropertyValue;
          case '/':
            if (input_.GetNextChar() == '*') {
              SaveMode(Mode::kPropertyValue);
              mode_ = Mode::kComment;
              if (!output_.IsEmpty()) {
                return Status::kPropertyValue;
              }
              break;
            }
            [[fallthrough]];
          default:
            output_.AppendCharIfNotLeadingBlank(wch);
            input_.MoveNext();
            break;
        }
        break;
      case Mode::kComment:
        if (wch == '*' && input_.GetNextChar() == '/') {
          if (!RestoreMode()) {
            return Status::kError;
          }
          input_.MoveNext();
        }
        input_.MoveNext();
        break;
    }
  }
  if (mode_ == Mode::kPropertyValue && !output_.IsEmpty()) {
    return Status::kPropertyValue;
  }

  return Status::kEOS;
}

void CFX_CSSSyntaxParser::SaveMode(Mode mode) {
  mode_stack_.push(mode);
}

bool CFX_CSSSyntaxParser::RestoreMode() {
  if (mode_stack_.empty()) {
    has_error_ = true;
    return false;
  }
  mode_ = mode_stack_.top();
  mode_stack_.pop();
  return true;
}

WideStringView CFX_CSSSyntaxParser::GetCurrentString() const {
  return output_.GetTrailingBlankTrimmedString();
}
