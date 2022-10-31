// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_hostpseudomodel.h"

#include <vector>

#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "third_party/base/check.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cscript_hostpseudomodel.h"
#include "xfa/fxfa/parser/cxfa_node.h"

namespace {

size_t FilterName(WideStringView wsExpression,
                  size_t nStart,
                  WideString& wsFilter) {
  const size_t nLength = wsExpression.GetLength();
  if (nStart >= nLength)
    return nLength;

  size_t nCount = 0;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<wchar_t> pBuf = wsFilter.GetBuffer(nLength - nStart);
    const wchar_t* pSrc = wsExpression.unterminated_c_str();
    while (nStart < nLength) {
      wchar_t wCur = pSrc[nStart++];
      if (wCur == ',')
        break;

      pBuf[nCount++] = wCur;
    }
  }
  wsFilter.ReleaseBuffer(nCount);
  wsFilter.Trim();
  return nStart;
}

}  // namespace

const CJX_MethodSpec CJX_HostPseudoModel::MethodSpecs[] = {
    {"beep", beep_static},
    {"documentCountInBatch", documentCountInBatch_static},
    {"documentInBatch", documentInBatch_static},
    {"exportData", exportData_static},
    {"getFocus", getFocus_static},
    {"gotoURL", gotoURL_static},
    {"importData", importData_static},
    {"messageBox", messageBox_static},
    {"openList", openList_static},
    {"pageDown", pageDown_static},
    {"pageUp", pageUp_static},
    {"print", print_static},
    {"resetData", resetData_static},
    {"response", response_static},
    {"setFocus", setFocus_static}};

CJX_HostPseudoModel::CJX_HostPseudoModel(CScript_HostPseudoModel* model)
    : CJX_Object(model) {
  DefineMethods(MethodSpecs);
}

CJX_HostPseudoModel::~CJX_HostPseudoModel() = default;

bool CJX_HostPseudoModel::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

void CJX_HostPseudoModel::appType(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  *pValue = fxv8::NewStringHelper(pIsolate, "Exchange");
}

void CJX_HostPseudoModel::calculationsEnabled(v8::Isolate* pIsolate,
                                              v8::Local<v8::Value>* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  if (bSetting) {
    hDoc->SetCalculationsEnabled(
        fxv8::ReentrantToBooleanHelper(pIsolate, *pValue));
    return;
  }
  *pValue = fxv8::NewBooleanHelper(pIsolate, hDoc->IsCalculationsEnabled());
}

void CJX_HostPseudoModel::currentPage(v8::Isolate* pIsolate,
                                      v8::Local<v8::Value>* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  if (bSetting) {
    hDoc->SetCurrentPage(fxv8::ReentrantToInt32Helper(pIsolate, *pValue));
    return;
  }
  *pValue = fxv8::NewNumberHelper(pIsolate, hDoc->GetCurrentPage());
}

void CJX_HostPseudoModel::language(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set language value."));
    return;
  }
  ByteString lang = pNotify->GetAppProvider()->GetLanguage().ToUTF8();
  *pValue = fxv8::NewStringHelper(pIsolate, lang.AsStringView());
}

void CJX_HostPseudoModel::numPages(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set numPages value."));
    return;
  }
  *pValue = fxv8::NewNumberHelper(pIsolate, hDoc->CountPages());
}

void CJX_HostPseudoModel::platform(v8::Isolate* pIsolate,
                                   v8::Local<v8::Value>* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set platform value."));
    return;
  }
  ByteString plat = pNotify->GetAppProvider()->GetPlatform().ToUTF8();
  *pValue = fxv8::NewStringHelper(pIsolate, plat.AsStringView());
}

void CJX_HostPseudoModel::title(v8::Isolate* pIsolate,
                                v8::Local<v8::Value>* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  if (bSetting) {
    hDoc->SetTitle(fxv8::ReentrantToWideStringHelper(pIsolate, *pValue));
    return;
  }

  ByteString bsTitle = hDoc->GetTitle().ToUTF8();
  *pValue = fxv8::NewStringHelper(pIsolate, bsTitle.AsStringView());
}

void CJX_HostPseudoModel::validationsEnabled(v8::Isolate* pIsolate,
                                             v8::Local<v8::Value>* pValue,
                                             bool bSetting,
                                             XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  if (bSetting) {
    hDoc->SetValidationsEnabled(
        fxv8::ReentrantToBooleanHelper(pIsolate, *pValue));
    return;
  }

  *pValue = fxv8::NewBooleanHelper(pIsolate, hDoc->IsValidationsEnabled());
}

void CJX_HostPseudoModel::variation(v8::Isolate* pIsolate,
                                    v8::Local<v8::Value>* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return;

  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set variation value."));
    return;
  }
  *pValue = fxv8::NewStringHelper(pIsolate, "Full");
}

void CJX_HostPseudoModel::version(v8::Isolate* pIsolate,
                                  v8::Local<v8::Value>* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowException(pIsolate,
                   WideString::FromASCII("Unable to set version value."));
    return;
  }
  *pValue = fxv8::NewStringHelper(pIsolate, "11");
}

void CJX_HostPseudoModel::name(v8::Isolate* pIsolate,
                               v8::Local<v8::Value>* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  ByteString bsName = pNotify->GetAppProvider()->GetAppName().ToUTF8();
  *pValue = fxv8::NewStringHelper(pIsolate, bsName.AsStringView());
}

CJS_Result CJX_HostPseudoModel::gotoURL(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  pNotify->GetFFDoc()->GotoURL(runtime->ToWideString(params[0]));
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::openList(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_Node* pNode = nullptr;
  if (params[0]->IsObject()) {
    pNode = ToNode(runtime->ToXFAObject(params[0]));
  } else if (params[0]->IsString()) {
    CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
    CXFA_Object* pObject = pScriptContext->GetThisObject();
    if (!pObject)
      return CJS_Result::Success();

    constexpr Mask<XFA_ResolveFlag> kFlags = {XFA_ResolveFlag::kChildren,
                                              XFA_ResolveFlag::kParent,
                                              XFA_ResolveFlag::kSiblings};
    absl::optional<CFXJSE_Engine::ResolveResult> maybeResult =
        pScriptContext->ResolveObjects(
            pObject, runtime->ToWideString(params[0]).AsStringView(), kFlags);
    if (!maybeResult.has_value() ||
        !maybeResult.value().objects.front()->IsNode()) {
      return CJS_Result::Success();
    }
    pNode = maybeResult.value().objects.front()->AsNode();
  }
  if (pNode)
    pNotify->OpenDropDownList(pNode);

  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::response(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 4)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  WideString question;
  if (params.size() >= 1)
    question = runtime->ToWideString(params[0]);

  WideString title;
  if (params.size() >= 2)
    title = runtime->ToWideString(params[1]);

  WideString defaultAnswer;
  if (params.size() >= 3)
    defaultAnswer = runtime->ToWideString(params[2]);

  bool mark = false;
  if (params.size() >= 4)
    mark = runtime->ToInt32(params[3]) != 0;

  WideString answer =
      pNotify->GetAppProvider()->Response(question, title, defaultAnswer, mark);
  return CJS_Result::Success(
      runtime->NewString(answer.ToUTF8().AsStringView()));
}

CJS_Result CJX_HostPseudoModel::documentInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_HostPseudoModel::resetData(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() > 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  WideString expression;
  if (params.size() >= 1)
    expression = runtime->ToWideString(params[0]);

  if (expression.IsEmpty()) {
    pNotify->ResetData(nullptr);
    return CJS_Result::Success();
  }

  WideString wsName;
  CXFA_Node* pNode = nullptr;
  size_t nStart = 0;
  const size_t nExpLength = expression.GetLength();
  while (nStart < nExpLength) {
    nStart = FilterName(expression.AsStringView(), nStart, wsName);
    CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
    CXFA_Object* pObject = pScriptContext->GetThisObject();
    if (!pObject)
      return CJS_Result::Success();

    constexpr Mask<XFA_ResolveFlag> kFlags = {XFA_ResolveFlag::kChildren,
                                              XFA_ResolveFlag::kParent,
                                              XFA_ResolveFlag::kSiblings};
    absl::optional<CFXJSE_Engine::ResolveResult> maybeResult =
        pScriptContext->ResolveObjects(pObject, wsName.AsStringView(), kFlags);
    if (!maybeResult.has_value() ||
        !maybeResult.value().objects.front()->IsNode())
      continue;

    pNode = maybeResult.value().objects.front()->AsNode();
    pNotify->ResetData(pNode->IsWidgetReady() ? pNode : nullptr);
  }
  if (!pNode)
    pNotify->ResetData(nullptr);

  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::beep(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.size() > 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  uint32_t dwType = 4;
  if (params.size() >= 1)
    dwType = runtime->ToInt32(params[0]);

  pNotify->GetAppProvider()->Beep(dwType);
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::setFocus(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_Node* pNode = nullptr;
  if (params.size() >= 1) {
    if (params[0]->IsObject()) {
      pNode = ToNode(runtime->ToXFAObject(params[0]));
    } else if (params[0]->IsString()) {
      CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
      CXFA_Object* pObject = pScriptContext->GetThisObject();
      if (!pObject)
        return CJS_Result::Success();

      constexpr Mask<XFA_ResolveFlag> kFlags = {XFA_ResolveFlag::kChildren,
                                                XFA_ResolveFlag::kParent,
                                                XFA_ResolveFlag::kSiblings};
      absl::optional<CFXJSE_Engine::ResolveResult> maybeResult =
          pScriptContext->ResolveObjects(
              pObject, runtime->ToWideString(params[0]).AsStringView(), kFlags);
      if (!maybeResult.has_value() ||
          !maybeResult.value().objects.front()->IsNode()) {
        return CJS_Result::Success();
      }
      pNode = maybeResult.value().objects.front()->AsNode();
    }
  }
  pNotify->SetFocusWidgetNode(pNode);
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::getFocus(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_Node* pNode = pNotify->GetFocusWidgetNode();
  if (!pNode)
    return CJS_Result::Success();

  v8::Local<v8::Value> value =
      GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(pNode);

  return CJS_Result::Success(value);
}

CJS_Result CJX_HostPseudoModel::messageBox(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.empty() || params.size() > 4)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  WideString message;
  if (params.size() >= 1)
    message = runtime->ToWideString(params[0]);

  WideString title;
  if (params.size() >= 2)
    title = runtime->ToWideString(params[1]);

  uint32_t messageType = static_cast<uint32_t>(AlertIcon::kDefault);
  if (params.size() >= 3) {
    messageType = runtime->ToInt32(params[2]);
    if (messageType > static_cast<uint32_t>(AlertIcon::kStatus))
      messageType = static_cast<uint32_t>(AlertIcon::kDefault);
  }

  uint32_t buttonType = static_cast<uint32_t>(AlertButton::kDefault);
  if (params.size() >= 4) {
    buttonType = runtime->ToInt32(params[3]);
    if (buttonType > static_cast<uint32_t>(AlertButton::kYesNoCancel))
      buttonType = static_cast<uint32_t>(AlertButton::kDefault);
  }

  int32_t iValue = pNotify->GetAppProvider()->MsgBox(message, title,
                                                     messageType, buttonType);
  return CJS_Result::Success(runtime->NewNumber(iValue));
}

CJS_Result CJX_HostPseudoModel::documentCountInBatch(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Result::Success(runtime->NewNumber(0));
}

CJS_Result CJX_HostPseudoModel::print(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (!GetDocument()->GetScriptContext()->IsRunAtClient())
    return CJS_Result::Success();

  if (params.size() != 8)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  Mask<XFA_PrintOpt> dwOptions;
  if (runtime->ToBoolean(params[0]))
    dwOptions |= XFA_PrintOpt::kShowDialog;
  if (runtime->ToBoolean(params[3]))
    dwOptions |= XFA_PrintOpt::kCanCancel;
  if (runtime->ToBoolean(params[4]))
    dwOptions |= XFA_PrintOpt::kShrinkPage;
  if (runtime->ToBoolean(params[5]))
    dwOptions |= XFA_PrintOpt::kAsImage;
  if (runtime->ToBoolean(params[6]))
    dwOptions |= XFA_PrintOpt::kReverseOrder;
  if (runtime->ToBoolean(params[7]))
    dwOptions |= XFA_PrintOpt::kPrintAnnot;

  int32_t nStartPage = runtime->ToInt32(params[1]);
  int32_t nEndPage = runtime->ToInt32(params[2]);
  pNotify->GetFFDoc()->Print(nStartPage, nEndPage, dwOptions);
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::importData(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::exportData(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  WideString filePath;
  if (params.size() >= 1)
    filePath = runtime->ToWideString(params[0]);

  bool XDP = true;
  if (params.size() >= 2)
    XDP = runtime->ToBoolean(params[1]);

  pNotify->GetFFDoc()->ExportData(filePath, XDP);
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::pageUp(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  int32_t nCurPage = hDoc->GetCurrentPage();
  if (nCurPage <= 1)
    return CJS_Result::Success();

  hDoc->SetCurrentPage(nCurPage - 1);
  return CJS_Result::Success();
}

CJS_Result CJX_HostPseudoModel::pageDown(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return CJS_Result::Success();

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  int32_t nCurPage = hDoc->GetCurrentPage();
  int32_t nPageCount = hDoc->CountPages();
  if (!nPageCount || nCurPage == nPageCount)
    return CJS_Result::Success();

  int32_t nNewPage = 0;
  if (nCurPage >= nPageCount)
    nNewPage = nPageCount - 1;
  else
    nNewPage = nCurPage + 1;

  hDoc->SetCurrentPage(nNewPage);
  return CJS_Result::Success();
}
