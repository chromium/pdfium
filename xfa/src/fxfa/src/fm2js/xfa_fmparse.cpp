// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
CXFA_FMParse::CXFA_FMParse() {
  m_pScript = 0;
  m_uLength = 0;
  m_pErrorInfo = 0;
  m_lexer = 0;
  m_pToken = 0;
}
CXFA_FMParse::~CXFA_FMParse() {
  if (m_lexer) {
    delete m_lexer;
  }
  m_lexer = 0;
  m_pErrorInfo = 0;
  m_pScript = 0;
  m_pToken = 0;
}
int32_t CXFA_FMParse::Init(const CFX_WideStringC& wsFormcalc,
                           CXFA_FMErrorInfo* pErrorInfo) {
  m_pScript = wsFormcalc.GetPtr();
  m_uLength = wsFormcalc.GetLength();
  m_pErrorInfo = pErrorInfo;
  m_lexer = new CXFA_FMLexer(wsFormcalc, m_pErrorInfo);
  if (m_lexer == 0) {
    return -1;
  }
  return 0;
}
void CXFA_FMParse::NextToken() {
  m_pToken = m_lexer->NextToken();
  while (m_pToken->m_type == TOKreserver) {
    if (m_lexer->HasError()) {
      break;
    }
    m_pToken = m_lexer->NextToken();
  }
}
void CXFA_FMParse::Check(XFA_FM_TOKEN op) {
  if (m_pToken->m_type != op) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
          XFA_FM_KeywordToString(op), ws_TempString.c_str());
  }
  NextToken();
}
void CXFA_FMParse::Error(FX_DWORD lineNum, XFA_FM_ERRMSG msg, ...) {
  m_pErrorInfo->linenum = lineNum;
  const FX_WCHAR* lpMessageInfo = XFA_FM_ErrorMsg(msg);
  va_list ap;
  va_start(ap, msg);
  m_pErrorInfo->message.FormatV(lpMessageInfo, ap);
  va_end(ap);
}
CFX_PtrArray* CXFA_FMParse::ParseTopExpression() {
  CXFA_FMExpression* e = 0;
  CFX_PtrArray* expression = new CFX_PtrArray();
  while (1) {
    if (m_pToken->m_type == TOKeof) {
      return expression;
    }
    if (m_pToken->m_type == TOKendfunc) {
      return expression;
    }
    if (m_pToken->m_type == TOKendif) {
      return expression;
    }
    if (m_pToken->m_type == TOKelseif) {
      return expression;
    }
    if (m_pToken->m_type == TOKelse) {
      return expression;
    }
    if (m_pToken->m_type == TOKfunc) {
      e = ParseFunction();
      if (e) {
        expression->Add(e);
      } else {
        break;
      }
    } else {
      e = ParseExpression();
      if (e) {
        expression->Add(e);
      } else {
        break;
      }
    }
  }
  return expression;
}
CXFA_FMExpression* CXFA_FMParse::ParseFunction() {
  CXFA_FMExpression* e = 0;
  CFX_WideStringC ident;
  CFX_WideStringCArray* pArguments = 0;
  CFX_PtrArray* pExpressions = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
          ws_TempString.c_str());
  } else {
    ident = m_pToken->m_wstring;
    NextToken();
  }
  Check(TOKlparen);
  if (m_pToken->m_type == TOKrparen) {
    NextToken();
  } else {
    pArguments = new CFX_WideStringCArray();
    CFX_WideStringC p;
    while (1) {
      if (m_pToken->m_type == TOKidentifier) {
        p = m_pToken->m_wstring;
        pArguments->Add(p);
        NextToken();
        if (m_pToken->m_type == TOKcomma) {
          NextToken();
          continue;
        } else if (m_pToken->m_type == TOKrparen) {
          NextToken();
          break;
        } else {
          Check(TOKrparen);
          break;
        }
      } else {
        CFX_WideString ws_TempString = m_pToken->m_wstring;
        Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
              ws_TempString.c_str());
        NextToken();
        break;
      }
    }
  }
  Check(TOKdo);
  if (m_pToken->m_type == TOKendfunc) {
    NextToken();
  } else {
    pExpressions = ParseTopExpression();
    Check(TOKendfunc);
  }
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMFunctionDefinition(line, 0, ident, pArguments, pExpressions);
  } else {
    int32_t size = 0;
    int32_t index = 0;
    if (pArguments) {
      pArguments->RemoveAll();
      delete pArguments;
      pArguments = 0;
    }
    index = 0;
    if (pExpressions) {
      CXFA_FMExpression* e1 = 0;
      size = pExpressions->GetSize();
      while (index < size) {
        e1 = (CXFA_FMExpression*)pExpressions->GetAt(index);
        delete e1;
        index++;
      }
      pExpressions->RemoveAll();
      delete pExpressions;
      pExpressions = 0;
    }
  }
  return e;
}
CXFA_FMExpression* CXFA_FMParse::ParseExpression() {
  CXFA_FMExpression* e = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKvar:
      e = ParseVarExpression();
      break;
    case TOKnull:
    case TOKnumber:
    case TOKstring:
    case TOKplus:
    case TOKminus:
    case TOKksnot:
    case TOKidentifier:
    case TOKlparen:
      e = ParseExpExpression();
      break;
    case TOKif:
      e = ParseIfExpression();
      break;
    case TOKwhile:
      e = ParseWhileExpression();
      break;
    case TOKfor:
      e = ParseForExpression();
      break;
    case TOKforeach:
      e = ParseForeachExpression();
      break;
    case TOKdo:
      e = ParseDoExpression();
      break;
    case TOKbreak:
      e = new CXFA_FMBreakExpression(line);
      NextToken();
      break;
    case TOKcontinue:
      e = new CXFA_FMContinueExpression(line);
      NextToken();
      break;
    default:
      CFX_WideString ws_TempString = m_pToken->m_wstring;
      Error(m_pToken->m_uLinenum, FMERR_UNEXPECTED_EXPRESSION,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  return e;
}
CXFA_FMExpression* CXFA_FMParse::ParseVarExpression() {
  CXFA_FMExpression* e = 0;
  CFX_WideStringC ident;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
          ws_TempString.c_str());
  } else {
    ident = m_pToken->m_wstring;
    NextToken();
  }
  if (m_pToken->m_type == TOKassign) {
    NextToken();
    e = ParseExpExpression();
  }
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMVarExpression(line, ident, e);
  } else {
    delete e;
    e = 0;
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseSimpleExpression() {
  FX_DWORD line = m_pToken->m_uLinenum;
  CXFA_FMSimpleExpression *pExp1 = 0, *pExp2 = 0;
  pExp1 = ParseLogicalOrExpression();
  while (m_pToken->m_type == TOKassign) {
    NextToken();
    pExp2 = ParseLogicalOrExpression();
    if (m_pErrorInfo->message.IsEmpty()) {
      pExp1 = new CXFA_FMAssignExpression(line, TOKassign, pExp1, pExp2);
    } else {
      delete pExp1;
      pExp1 = 0;
    }
  }
  return pExp1;
}
CXFA_FMExpression* CXFA_FMParse::ParseExpExpression() {
  CXFA_FMExpression* e = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  CXFA_FMSimpleExpression* pExp1 = 0;
  pExp1 = ParseSimpleExpression();
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMExpExpression(line, pExp1);
  } else {
    delete pExp1;
    e = 0;
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseLogicalOrExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseLogicalAndExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKor:
      case TOKksor:
        NextToken();
        e2 = ParseLogicalAndExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMLogicalOrExpression(line, TOKor, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseLogicalAndExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseEqualityExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKand:
      case TOKksand:
        NextToken();
        e2 = ParseEqualityExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMLogicalAndExpression(line, TOKand, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseEqualityExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseRelationalExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKeq:
      case TOKkseq:
        NextToken();
        e2 = ParseRelationalExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMEqualityExpression(line, TOKeq, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKne:
      case TOKksne:
        NextToken();
        e2 = ParseRelationalExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMEqualityExpression(line, TOKne, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseRelationalExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseAddtiveExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKlt:
      case TOKkslt:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMRelationalExpression(line, TOKlt, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKgt:
      case TOKksgt:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMRelationalExpression(line, TOKgt, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKle:
      case TOKksle:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMRelationalExpression(line, TOKle, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKge:
      case TOKksge:
        NextToken();
        e2 = ParseAddtiveExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMRelationalExpression(line, TOKge, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseAddtiveExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseMultiplicativeExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKplus:
        NextToken();
        e2 = ParseMultiplicativeExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMAdditiveExpression(line, TOKplus, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKminus:
        NextToken();
        e2 = ParseMultiplicativeExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMAdditiveExpression(line, TOKminus, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseMultiplicativeExpression() {
  CXFA_FMSimpleExpression *e1 = 0, *e2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  e1 = ParseUnaryExpression();
  for (;;) {
    switch (m_pToken->m_type) {
      case TOKmul:
        NextToken();
        e2 = ParseUnaryExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMMultiplicativeExpression(line, TOKmul, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      case TOKdiv:
        NextToken();
        e2 = ParseUnaryExpression();
        if (m_pErrorInfo->message.IsEmpty()) {
          e1 = new CXFA_FMMultiplicativeExpression(line, TOKdiv, e1, e2);
        } else {
          delete e1;
          e1 = 0;
        }
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseUnaryExpression() {
  CXFA_FMSimpleExpression* e = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKplus:
      NextToken();
      e = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        e = new CXFA_FMPosExpression(line, e);
      } else {
        e = 0;
      }
      break;
    case TOKminus:
      NextToken();
      e = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        e = new CXFA_FMNegExpression(line, e);
      } else {
        e = 0;
      }
      break;
    case TOKksnot:
      NextToken();
      e = ParseUnaryExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        e = new CXFA_FMNotExpression(line, e);
      } else {
        e = 0;
      }
      break;
    default:
      e = ParsePrimaryExpression();
      break;
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParsePrimaryExpression() {
  CXFA_FMSimpleExpression* e = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  switch (m_pToken->m_type) {
    case TOKnumber:
      e = new CXFA_FMNumberExpression(line, m_pToken->m_wstring);
      NextToken();
      break;
    case TOKstring:
      e = new CXFA_FMStringExpression(line, m_pToken->m_wstring);
      NextToken();
      break;
    case TOKidentifier: {
      CFX_WideStringC wsIdentifier(m_pToken->m_wstring);
      NextToken();
      if (m_pToken->m_type == TOKlbracket) {
        CXFA_FMSimpleExpression* s = ParseIndexExpression();
        if (s) {
          e = new CXFA_FMDotAccessorExpression(line, NULL, TOKdot, wsIdentifier,
                                               s);
        }
        NextToken();
      } else {
        e = new CXFA_FMIdentifierExpressionn(line, wsIdentifier);
      }
    } break;
    case TOKif:
      e = new CXFA_FMIdentifierExpressionn(line, m_pToken->m_wstring);
      NextToken();
      break;
    case TOKnull:
      e = new CXFA_FMNullExpression(line);
      NextToken();
      break;
    case TOKlparen:
      e = ParseParenExpression();
      break;
    default:
      CFX_WideString ws_TempString = m_pToken->m_wstring;
      Error(m_pToken->m_uLinenum, FMERR_UNEXPECTED_EXPRESSION,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  e = ParsePostExpression(e);
  if (!(m_pErrorInfo->message.IsEmpty())) {
    delete e;
    e = 0;
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParsePostExpression(
    CXFA_FMSimpleExpression* e) {
  FX_DWORD line = m_pToken->m_uLinenum;
  while (1) {
    switch (m_pToken->m_type) {
      case TOKlparen: {
        NextToken();
        CFX_PtrArray* pArray = 0;
        if (m_pToken->m_type != TOKrparen) {
          pArray = new CFX_PtrArray();
          while (m_pToken->m_type != TOKrparen) {
            CXFA_FMSimpleExpression* e = ParseSimpleExpression();
            if (e) {
              pArray->Add(e);
            }
            if (m_pToken->m_type == TOKcomma) {
              NextToken();
            } else if (m_pToken->m_type == TOKeof) {
              break;
            }
          }
          if (m_pToken->m_type != TOKrparen) {
            CFX_WideString ws_TempString = m_pToken->m_wstring;
            Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
                  XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
          }
        }
        if (m_pErrorInfo->message.IsEmpty()) {
          e = new CXFA_FMCallExpression(line, e, pArray, FALSE);
          NextToken();
          if (m_pToken->m_type != TOKlbracket) {
            continue;
          }
          CXFA_FMSimpleExpression* s = ParseIndexExpression();
          if (s) {
            e = new CXFA_FMDotAccessorExpression(line, e, TOKcall,
                                                 FX_WSTRC(L""), s);
          } else {
            delete e;
            e = 0;
          }
        } else {
          int32_t iSize = pArray->GetSize();
          for (int32_t i = 0; i < iSize; ++i) {
            CXFA_FMSimpleExpression* pTemp =
                (CXFA_FMSimpleExpression*)pArray->GetAt(i);
            delete pTemp;
          }
          delete pArray;
          delete e;
          e = 0;
        }
      } break;
      case TOKdot:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          FX_DWORD tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlparen) {
            CXFA_FMSimpleExpression* pExpAccessor;
            CXFA_FMSimpleExpression* pExpCall;
            pExpAccessor = e;
            NextToken();
            CFX_PtrArray* pArray = 0;
            if (m_pToken->m_type != TOKrparen) {
              pArray = new CFX_PtrArray();
              while (m_pToken->m_type != TOKrparen) {
                CXFA_FMSimpleExpression* exp = ParseSimpleExpression();
                pArray->Add(exp);
                if (m_pToken->m_type == TOKcomma) {
                  NextToken();
                } else if (m_pToken->m_type == TOKeof) {
                  break;
                }
              }
              if (m_pToken->m_type != TOKrparen) {
                CFX_WideString ws_TempString = m_pToken->m_wstring;
                Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
                      XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
              }
            }
            if (m_pErrorInfo->message.IsEmpty()) {
              CXFA_FMSimpleExpression* pIdentifier =
                  new CXFA_FMIdentifierExpressionn(tempLine, tempStr);
              pExpCall =
                  new CXFA_FMCallExpression(line, pIdentifier, pArray, TRUE);
              e = new CXFA_FMMethodCallExpression(line, pExpAccessor, pExpCall);
              NextToken();
              if (m_pToken->m_type != TOKlbracket) {
                continue;
              }
              CXFA_FMSimpleExpression* s = ParseIndexExpression();
              if (s) {
                e = new CXFA_FMDotAccessorExpression(line, e, TOKcall,
                                                     FX_WSTRC(L""), s);
              } else {
                delete e;
                e = 0;
              }
            } else {
              int32_t iSize = pArray->GetSize();
              for (int32_t i = 0; i < iSize; ++i) {
                CXFA_FMSimpleExpression* pTemp =
                    (CXFA_FMSimpleExpression*)pArray->GetAt(i);
                delete pTemp;
              }
              delete pArray;
              delete e;
              e = 0;
            }
          } else if (m_pToken->m_type == TOKlbracket) {
            CXFA_FMSimpleExpression* s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty())) {
              if (s) {
                delete s;
                s = 0;
              }
              if (e) {
                delete e;
                e = 0;
              }
              return e;
            }
            e = new CXFA_FMDotAccessorExpression(tempLine, e, TOKdot, tempStr,
                                                 s);
          } else {
            CXFA_FMSimpleExpression* s = new CXFA_FMIndexExpression(
                tempLine, ACCESSOR_NO_INDEX, NULL, FALSE);
            e = new CXFA_FMDotAccessorExpression(line, e, TOKdot, tempStr, s);
            continue;
          }
        } else {
          CFX_WideString ws_TempString = m_pToken->m_wstring;
          Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
                ws_TempString.c_str());
          return e;
        }
        break;
      case TOKdotdot:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          FX_DWORD tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlbracket) {
            CXFA_FMSimpleExpression* s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty())) {
              if (s) {
                delete s;
                s = 0;
              }
              if (e) {
                delete e;
                e = 0;
              }
              return e;
            }
            e = new CXFA_FMDotDotAccessorExpression(tempLine, e, TOKdotdot,
                                                    tempStr, s);
          } else {
            CXFA_FMSimpleExpression* s = new CXFA_FMIndexExpression(
                tempLine, ACCESSOR_NO_INDEX, NULL, FALSE);
            e = new CXFA_FMDotDotAccessorExpression(line, e, TOKdotdot, tempStr,
                                                    s);
            continue;
          }
        } else {
          CFX_WideString ws_TempString = m_pToken->m_wstring;
          Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
                ws_TempString.c_str());
          return e;
        }
        break;
      case TOKdotscream:
        NextToken();
        if (m_pToken->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_pToken->m_wstring;
          FX_DWORD tempLine = m_pToken->m_uLinenum;
          NextToken();
          if (m_pToken->m_type == TOKlbracket) {
            CXFA_FMSimpleExpression* s = ParseIndexExpression();
            if (!(m_pErrorInfo->message.IsEmpty())) {
              if (s) {
                delete s;
                s = 0;
              }
              if (e) {
                delete e;
                e = 0;
              }
              return e;
            }
            e = new CXFA_FMDotAccessorExpression(tempLine, e, TOKdotscream,
                                                 tempStr, s);
          } else {
            CXFA_FMSimpleExpression* s = new CXFA_FMIndexExpression(
                tempLine, ACCESSOR_NO_INDEX, NULL, FALSE);
            e = new CXFA_FMDotAccessorExpression(line, e, TOKdotscream, tempStr,
                                                 s);
            continue;
          }
        } else {
          CFX_WideString ws_TempString = m_pToken->m_wstring;
          Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IDENTIFIER,
                ws_TempString.c_str());
          return e;
        }
        break;
      case TOKdotstar: {
        CXFA_FMSimpleExpression* s =
            new CXFA_FMIndexExpression(line, ACCESSOR_NO_INDEX, NULL, FALSE);
        e = new CXFA_FMDotAccessorExpression(line, e, TOKdotstar,
                                             FX_WSTRC(L"*"), s);
      } break;
      default:
        return e;
    }
    NextToken();
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseIndexExpression() {
  CXFA_FMSimpleExpression* pExp = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  CXFA_FMSimpleExpression* s = 0;
  XFA_FM_AccessorIndex accessorIndex = ACCESSOR_NO_RELATIVEINDEX;
  if (m_pToken->m_type == TOKmul) {
    pExp = new CXFA_FMIndexExpression(line, accessorIndex, s, TRUE);
    NextToken();
    if (m_pToken->m_type != TOKrbracket) {
      CFX_WideString ws_TempString = m_pToken->m_wstring;
      Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
            XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
      if (pExp) {
        delete pExp;
        pExp = 0;
      }
    }
    return pExp;
  }
  if (m_pToken->m_type == TOKplus) {
    accessorIndex = ACCESSOR_POSITIVE_INDEX;
    NextToken();
  } else if (m_pToken->m_type == TOKminus) {
    accessorIndex = ACCESSOR_NEGATIVE_INDEX;
    NextToken();
  }
  s = ParseSimpleExpression();
  if (m_pToken->m_type != TOKrbracket) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
          XFA_FM_KeywordToString(TOKrparen), ws_TempString.c_str());
    if (s) {
      delete s;
    }
  } else {
    pExp = new CXFA_FMIndexExpression(line, accessorIndex, s, FALSE);
  }
  return pExp;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseParenExpression() {
  CXFA_FMSimpleExpression *pExp1 = 0, *pExp2 = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  Check(TOKlparen);
  if (m_pToken->m_type != TOKrparen) {
    pExp1 = ParseLogicalOrExpression();
    while (m_pToken->m_type == TOKassign) {
      NextToken();
      pExp2 = ParseLogicalOrExpression();
      if (m_pErrorInfo->message.IsEmpty()) {
        pExp1 = new CXFA_FMAssignExpression(line, TOKassign, pExp1, pExp2);
      } else {
        delete pExp1;
        pExp1 = 0;
      }
    }
    Check(TOKrparen);
  } else {
    NextToken();
  }
  return pExp1;
}
CXFA_FMExpression* CXFA_FMParse::ParseBlockExpression() {
  FX_DWORD line = m_pToken->m_uLinenum;
  CXFA_FMExpression* e = 0;
  CFX_PtrArray* expression = new CFX_PtrArray();
  while (1) {
    switch (m_pToken->m_type) {
      case TOKeof:
      case TOKendif:
      case TOKelseif:
      case TOKelse:
      case TOKendwhile:
      case TOKendfor:
      case TOKend:
      case TOKendfunc:
        break;
      case TOKfunc:
        e = ParseFunction();
        if (e) {
          expression->Add(e);
        }
        continue;
      default:
        e = ParseExpression();
        if (e) {
          expression->Add(e);
        }
        continue;
    }
    break;
  }
  CXFA_FMBlockExpression* pExp = 0;
  if (m_pErrorInfo->message.IsEmpty()) {
    pExp = new CXFA_FMBlockExpression(line, expression);
  } else {
    int32_t size = expression->GetSize();
    int32_t index = 0;
    while (index < size) {
      e = (CXFA_FMExpression*)expression->GetAt(index);
      delete e;
      index++;
    }
    expression->RemoveAll();
    delete expression;
    expression = 0;
  }
  return pExp;
}
CXFA_FMExpression* CXFA_FMParse::ParseIfExpression() {
  CXFA_FMSimpleExpression* pExpression = 0;
  CXFA_FMExpression* pIfExpression = 0;
  CXFA_FMExpression* pElseExpression = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  const FX_WCHAR* pStartPos = m_lexer->SavePos();
  NextToken();
  Check(TOKlparen);
  while (m_pToken->m_type != TOKrparen) {
    if (pExpression) {
      delete pExpression;
    }
    pExpression = ParseSimpleExpression();
    if (m_pToken->m_type == TOKcomma) {
      NextToken();
    } else {
      break;
    }
  }
  Check(TOKrparen);
  if (m_pToken->m_type != TOKthen) {
    if (pExpression) {
      delete pExpression;
    }
    m_lexer->SetCurrentLine(line);
    m_pToken = new CXFA_FMToken(line);
    m_pToken->m_type = TOKidentifier;
    m_pToken->m_wstring = FX_WSTRC(L"if");
    m_lexer->SetToken(m_pToken);
    m_lexer->RestorePos(pStartPos);
    return ParseExpExpression();
  }
  Check(TOKthen);
  pIfExpression = ParseBlockExpression();
  switch (m_pToken->m_type) {
    case TOKeof:
    case TOKendif:
      Check(TOKendif);
      break;
    case TOKif:
      pElseExpression = ParseIfExpression();
      Check(TOKendif);
      break;
    case TOKelseif:
      pElseExpression = ParseIfExpression();
      break;
    case TOKelse:
      NextToken();
      pElseExpression = ParseBlockExpression();
      Check(TOKendif);
      break;
    default:
      CFX_WideString ws_TempString = m_pToken->m_wstring;
      Error(m_pToken->m_uLinenum, FMERR_EXPECTED_IFEND, ws_TempString.c_str());
      NextToken();
      break;
  }
  CXFA_FMIfExpression* pExp = 0;
  if (m_pErrorInfo->message.IsEmpty()) {
    pExp = new CXFA_FMIfExpression(line, pExpression, pIfExpression,
                                   pElseExpression);
  } else {
    if (pExpression) {
      delete pExpression;
    }
    if (pIfExpression) {
      delete pIfExpression;
    }
    if (pElseExpression) {
      delete pElseExpression;
    }
  }
  return pExp;
}
CXFA_FMExpression* CXFA_FMParse::ParseWhileExpression() {
  CXFA_FMExpression* e = 0;
  CXFA_FMSimpleExpression* pCondition = 0;
  CXFA_FMExpression* pExpression = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  pCondition = ParseParenExpression();
  Check(TOKdo);
  pExpression = ParseBlockExpression();
  Check(TOKendwhile);
  if (!m_pErrorInfo->message.IsEmpty()) {
    if (pCondition) {
      delete pCondition;
    }
    if (pExpression) {
      delete pExpression;
    }
    delete e;
    e = 0;
  } else {
    e = new CXFA_FMWhileExpression(line, pCondition, pExpression);
  }
  return e;
}
CXFA_FMSimpleExpression* CXFA_FMParse::ParseSubassignmentInForExpression() {
  CXFA_FMSimpleExpression* e = 0;
  switch (m_pToken->m_type) {
    case TOKidentifier:
      e = ParseSimpleExpression();
      break;
    default:
      CFX_WideString ws_TempString = m_pToken->m_wstring;
      Error(m_pToken->m_uLinenum, FMERR_UNEXPECTED_EXPRESSION,
            ws_TempString.c_str());
      NextToken();
      break;
  }
  return e;
}
CXFA_FMExpression* CXFA_FMParse::ParseForExpression() {
  CXFA_FMExpression* e = 0;
  CFX_WideStringC wsVariant;
  CXFA_FMSimpleExpression* pAssignment = 0;
  CXFA_FMSimpleExpression* pAccessor = 0;
  CXFA_FMSimpleExpression* pStep = 0;
  CXFA_FMExpression* pList = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  wsVariant = m_pToken->m_wstring;
  NextToken();
  if (m_pToken->m_type == TOKassign) {
    NextToken();
    pAssignment = ParseSimpleExpression();
  } else {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  int32_t iDirection = 0;
  if (m_pToken->m_type == TOKupto) {
    iDirection = 1;
  } else if (m_pToken->m_type == TOKdownto) {
    iDirection = -1;
  } else {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN, L"upto or downto",
          (const FX_WCHAR*)ws_TempString);
  }
  NextToken();
  pAccessor = ParseSimpleExpression();
  if (m_pToken->m_type == TOKstep) {
    NextToken();
    pStep = ParseSimpleExpression();
  }
  Check(TOKdo);
  pList = ParseBlockExpression();
  Check(TOKendfor);
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMForExpression(line, wsVariant, pAssignment, pAccessor,
                                 iDirection, pStep, pList);
  } else {
    if (pAssignment) {
      delete pAssignment;
    }
    if (pAccessor) {
      delete pAccessor;
    }
    if (pStep) {
      delete pStep;
    }
    if (pList) {
      delete pList;
    }
  }
  return e;
}
CXFA_FMExpression* CXFA_FMParse::ParseForeachExpression() {
  CXFA_FMExpression* e = 0;
  CFX_WideStringC wsIdentifier;
  CFX_PtrArray* pAccessors = 0;
  CXFA_FMExpression* pList = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  if (m_pToken->m_type != TOKidentifier) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_EXPECTED_TOKEN,
          XFA_FM_KeywordToString(m_pToken->m_type), ws_TempString.c_str());
  }
  wsIdentifier = m_pToken->m_wstring;
  NextToken();
  Check(TOKin);
  Check(TOKlparen);
  if (m_pToken->m_type == TOKrparen) {
    CFX_WideString ws_TempString = m_pToken->m_wstring;
    Error(m_pToken->m_uLinenum, FMERR_UNEXPECTED_EXPRESSION,
          ws_TempString.c_str());
    NextToken();
  } else {
    pAccessors = new CFX_PtrArray();
    while (m_pToken->m_type != TOKrparen) {
      CXFA_FMSimpleExpression* s = ParseSimpleExpression();
      if (s) {
        pAccessors->Add(s);
      }
      if (m_pToken->m_type == TOKcomma) {
        NextToken();
      } else {
        break;
      }
    }
    Check(TOKrparen);
  }
  Check(TOKdo);
  pList = ParseBlockExpression();
  Check(TOKendfor);
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMForeachExpression(line, wsIdentifier, pAccessors, pList);
  } else {
    if (pAccessors) {
      CXFA_FMSimpleExpression* s = 0;
      int32_t size = pAccessors->GetSize();
      int32_t index = 0;
      while (index < size) {
        s = (CXFA_FMSimpleExpression*)pAccessors->GetAt(index);
        delete s;
        index++;
      }
      pAccessors->RemoveAll();
      delete pAccessors;
      pAccessors = 0;
    }
    if (pList) {
      delete pList;
    }
  }
  return e;
}
CXFA_FMExpression* CXFA_FMParse::ParseDoExpression() {
  CXFA_FMExpression* e = 0;
  FX_DWORD line = m_pToken->m_uLinenum;
  NextToken();
  e = ParseBlockExpression();
  Check(TOKend);
  if (m_pErrorInfo->message.IsEmpty()) {
    e = new CXFA_FMDoExpression(line, e);
  } else {
    delete e;
    e = 0;
  }
  return e;
}
