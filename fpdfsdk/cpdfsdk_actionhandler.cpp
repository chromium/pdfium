// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_actionhandler.h"

#include <set>
#include <vector>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "fxjs/ijs_event_context.h"
#include "fxjs/ijs_runtime.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"

bool CPDFSDK_ActionHandler::DoAction_DocOpen(
    const CPDF_Action& action,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentOpenAction(action, pFormFillEnv, &visited);
}

bool CPDFSDK_ActionHandler::DoAction_JavaScript(
    const CPDF_Action& JsAction,
    WideString csJSName,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  if (JsAction.GetType() == CPDF_Action::JavaScript) {
    WideString swJS = JsAction.GetJavaScript();
    if (!swJS.IsEmpty()) {
      RunDocumentOpenJavaScript(pFormFillEnv, csJSName, swJS);
      return true;
    }
  }

  return false;
}

bool CPDFSDK_ActionHandler::DoAction_FieldJavaScript(
    const CPDF_Action& JsAction,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CPDFSDK_FieldAction* data) {
  ASSERT(pFormFillEnv);
  if (pFormFillEnv->IsJSPlatformPresent() &&
      JsAction.GetType() == CPDF_Action::JavaScript) {
    WideString swJS = JsAction.GetJavaScript();
    if (!swJS.IsEmpty()) {
      RunFieldJavaScript(pFormFillEnv, pFormField, type, data, swJS);
      return true;
    }
  }
  return false;
}

bool CPDFSDK_ActionHandler::DoAction_Link(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* form_fill_env,
    int modifiers) {
  ASSERT(form_fill_env);

  if (!CPDF_AAction::IsUserInput(type))
    return false;

  switch (action.GetType()) {
    case CPDF_Action::GoTo:
      DoAction_GoTo(form_fill_env, action);
      return true;
    case CPDF_Action::URI:
      DoAction_URI(form_fill_env, action, modifiers);
      return true;
    default:
      return false;
  }
}

bool CPDFSDK_ActionHandler::DoAction_Destination(
    const CPDF_Dest& dest,
    CPDFSDK_FormFillEnvironment* form_fill_env) {
  ASSERT(form_fill_env);
  CPDF_Document* document = form_fill_env->GetPDFDocument();
  ASSERT(document);

  const CPDF_Array* dest_array = dest.GetArray();
  std::vector<float> dest_positions;
  // |dest_array| index 0 contains destination page details and index 1 contains
  // parameter that explains about the rest of |dest_array|.
  if (dest_array) {
    for (size_t i = 2; i < dest_array->size(); i++)
      dest_positions.push_back(dest_array->GetNumberAt(i));
  }

  form_fill_env->DoGoToAction(dest.GetDestPageIndex(document),
                              dest.GetZoomMode(), dest_positions.data(),
                              dest_positions.size());
  return true;
}

bool CPDFSDK_ActionHandler::DoAction_Page(
    const CPDF_Action& action,
    CPDF_AAction::AActionType eType,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentPageAction(action, eType, pFormFillEnv, &visited);
}

bool CPDFSDK_ActionHandler::DoAction_Document(
    const CPDF_Action& action,
    CPDF_AAction::AActionType eType,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentPageAction(action, eType, pFormFillEnv, &visited);
}

bool CPDFSDK_ActionHandler::DoAction_Field(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CPDFSDK_FieldAction* data) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteFieldAction(action, type, pFormFillEnv, pFormField, data,
                            &visited);
}

bool CPDFSDK_ActionHandler::ExecuteDocumentOpenAction(
    const CPDF_Action& action,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict))
    return false;

  visited->insert(pDict);

  ASSERT(pFormFillEnv);
  if (action.GetType() == CPDF_Action::JavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty())
        RunDocumentOpenJavaScript(pFormFillEnv, WideString(), swJS);
    }
  } else {
    DoAction_NoJs(action, CPDF_AAction::AActionType::kDocumentOpen,
                  pFormFillEnv, /*modifiers=*/0);
  }

  for (int32_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteDocumentOpenAction(subaction, pFormFillEnv, visited))
      return false;
  }

  return true;
}

bool CPDFSDK_ActionHandler::ExecuteDocumentPageAction(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict))
    return false;

  visited->insert(pDict);

  ASSERT(pFormFillEnv);
  if (action.GetType() == CPDF_Action::JavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty())
        RunDocumentPageJavaScript(pFormFillEnv, type, swJS);
    }
  } else {
    DoAction_NoJs(action, type, pFormFillEnv, /*modifiers=*/0);
  }

  ASSERT(pFormFillEnv);

  for (int32_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteDocumentPageAction(subaction, type, pFormFillEnv, visited))
      return false;
  }

  return true;
}

bool CPDFSDK_ActionHandler::IsValidField(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_Dictionary* pFieldDict) {
  ASSERT(pFieldDict);

  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  return !!pPDFForm->GetFieldByDict(pFieldDict);
}

bool CPDFSDK_ActionHandler::ExecuteFieldAction(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CPDFSDK_FieldAction* data,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict))
    return false;

  visited->insert(pDict);

  ASSERT(pFormFillEnv);
  if (action.GetType() == CPDF_Action::JavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty()) {
        RunFieldJavaScript(pFormFillEnv, pFormField, type, data, swJS);
        if (!IsValidField(pFormFillEnv, pFormField->GetFieldDict()))
          return false;
      }
    }
  } else {
    DoAction_NoJs(action, type, pFormFillEnv, /*modifiers=*/0);
  }

  for (int32_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteFieldAction(subaction, type, pFormFillEnv, pFormField, data,
                            visited))
      return false;
  }

  return true;
}

void CPDFSDK_ActionHandler::DoAction_NoJs(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    int modifiers) {
  ASSERT(pFormFillEnv);

  switch (action.GetType()) {
    case CPDF_Action::GoTo:
      DoAction_GoTo(pFormFillEnv, action);
      break;
    case CPDF_Action::URI:
      if (CPDF_AAction::IsUserInput(type))
        DoAction_URI(pFormFillEnv, action, modifiers);
      break;
    case CPDF_Action::Hide:
      DoAction_Hide(action, pFormFillEnv);
      break;
    case CPDF_Action::Named:
      DoAction_Named(pFormFillEnv, action);
      break;
    case CPDF_Action::SubmitForm:
      if (CPDF_AAction::IsUserInput(type))
        DoAction_SubmitForm(action, pFormFillEnv);
      break;
    case CPDF_Action::ResetForm:
      DoAction_ResetForm(action, pFormFillEnv);
      break;
    case CPDF_Action::JavaScript:
      NOTREACHED();
      break;
    case CPDF_Action::SetOCGState:
    case CPDF_Action::Thread:
    case CPDF_Action::Sound:
    case CPDF_Action::Movie:
    case CPDF_Action::Rendition:
    case CPDF_Action::Trans:
    case CPDF_Action::GoTo3DView:
    case CPDF_Action::GoToR:
    case CPDF_Action::GoToE:
    case CPDF_Action::Launch:
    case CPDF_Action::ImportData:
      // Unimplemented
      break;
    default:
      break;
  }
}

void CPDFSDK_ActionHandler::DoAction_GoTo(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action) {
  ASSERT(action.GetDict());

  CPDF_Document* pPDFDocument = pFormFillEnv->GetPDFDocument();
  ASSERT(pPDFDocument);

  CPDF_Dest MyDest = action.GetDest(pPDFDocument);
  DoAction_Destination(MyDest, pFormFillEnv);
}

void CPDFSDK_ActionHandler::DoAction_URI(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action,
    int modifiers) {
  ASSERT(action.GetDict());

  ByteString sURI = action.GetURI(pFormFillEnv->GetPDFDocument());
  pFormFillEnv->DoURIAction(sURI.c_str(), modifiers);
}

void CPDFSDK_ActionHandler::DoAction_Named(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action) {
  ASSERT(action.GetDict());

  ByteString csName = action.GetNamedAction();
  pFormFillEnv->ExecuteNamedAction(csName.c_str());
}

void CPDFSDK_ActionHandler::RunFieldJavaScript(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CPDF_AAction::AActionType type,
    CPDFSDK_FieldAction* data,
    const WideString& script) {
  ASSERT(type != CPDF_AAction::kCalculate);
  ASSERT(type != CPDF_AAction::kFormat);

  RunScript(pFormFillEnv, script,
            [type, data, pFormField](IJS_EventContext* context) {
              switch (type) {
                case CPDF_AAction::kCursorEnter:
                  context->OnField_MouseEnter(data->bModifier, data->bShift,
                                              pFormField);
                  break;
                case CPDF_AAction::kCursorExit:
                  context->OnField_MouseExit(data->bModifier, data->bShift,
                                             pFormField);
                  break;
                case CPDF_AAction::kButtonDown:
                  context->OnField_MouseDown(data->bModifier, data->bShift,
                                             pFormField);
                  break;
                case CPDF_AAction::kButtonUp:
                  context->OnField_MouseUp(data->bModifier, data->bShift,
                                           pFormField);
                  break;
                case CPDF_AAction::kGetFocus:
                  context->OnField_Focus(data->bModifier, data->bShift,
                                         pFormField, &data->sValue);
                  break;
                case CPDF_AAction::kLoseFocus:
                  context->OnField_Blur(data->bModifier, data->bShift,
                                        pFormField, &data->sValue);
                  break;
                case CPDF_AAction::kKeyStroke:
                  context->OnField_Keystroke(
                      &data->sChange, data->sChangeEx, data->bKeyDown,
                      data->bModifier, &data->nSelEnd, &data->nSelStart,
                      data->bShift, pFormField, &data->sValue,
                      data->bWillCommit, data->bFieldFull, &data->bRC);
                  break;
                case CPDF_AAction::kValidate:
                  context->OnField_Validate(&data->sChange, data->sChangeEx,
                                            data->bKeyDown, data->bModifier,
                                            data->bShift, pFormField,
                                            &data->sValue, &data->bRC);
                  break;
                default:
                  NOTREACHED();
                  break;
              }
            });
}

void CPDFSDK_ActionHandler::RunDocumentOpenJavaScript(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const WideString& sScriptName,
    const WideString& script) {
  RunScript(pFormFillEnv, script,
            [pFormFillEnv, sScriptName](IJS_EventContext* context) {
              context->OnDoc_Open(pFormFillEnv, sScriptName);
            });
}

void CPDFSDK_ActionHandler::RunDocumentPageJavaScript(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_AAction::AActionType type,
    const WideString& script) {
  RunScript(pFormFillEnv, script,
            [type, pFormFillEnv](IJS_EventContext* context) {
              switch (type) {
                case CPDF_AAction::kOpenPage:
                  context->OnPage_Open(pFormFillEnv);
                  break;
                case CPDF_AAction::kClosePage:
                  context->OnPage_Close(pFormFillEnv);
                  break;
                case CPDF_AAction::kCloseDocument:
                  context->OnDoc_WillClose(pFormFillEnv);
                  break;
                case CPDF_AAction::kSaveDocument:
                  context->OnDoc_WillSave(pFormFillEnv);
                  break;
                case CPDF_AAction::kDocumentSaved:
                  context->OnDoc_DidSave(pFormFillEnv);
                  break;
                case CPDF_AAction::kPrintDocument:
                  context->OnDoc_WillPrint(pFormFillEnv);
                  break;
                case CPDF_AAction::kDocumentPrinted:
                  context->OnDoc_DidPrint(pFormFillEnv);
                  break;
                case CPDF_AAction::kPageVisible:
                  context->OnPage_InView(pFormFillEnv);
                  break;
                case CPDF_AAction::kPageInvisible:
                  context->OnPage_OutView(pFormFillEnv);
                  break;
                default:
                  NOTREACHED();
                  break;
              }
            });
}

bool CPDFSDK_ActionHandler::DoAction_Hide(
    const CPDF_Action& action,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  if (pForm->DoAction_Hide(action)) {
    pFormFillEnv->SetChangeMark();
    return true;
  }
  return false;
}

bool CPDFSDK_ActionHandler::DoAction_SubmitForm(
    const CPDF_Action& action,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  return pForm->DoAction_SubmitForm(action);
}

void CPDFSDK_ActionHandler::DoAction_ResetForm(
    const CPDF_Action& action,
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  pForm->DoAction_ResetForm(action);
}

void CPDFSDK_ActionHandler::RunScript(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                      const WideString& script,
                                      const RunScriptCallback& cb) {
  IJS_Runtime::ScopedEventContext pContext(pFormFillEnv->GetIJSRuntime());
  cb(pContext.Get());
  pContext->RunScript(script);
  // TODO(dsinclair): Return error if RunScript returns a IJS_Runtime::JS_Error.
}
