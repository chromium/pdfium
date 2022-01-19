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
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"

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
  if (JsAction.GetType() == CPDF_Action::Type::kJavaScript) {
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
    CFFL_FieldAction* data) {
  DCHECK(pFormFillEnv);
  if (pFormFillEnv->IsJSPlatformPresent() &&
      JsAction.GetType() == CPDF_Action::Type::kJavaScript) {
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
    Mask<FWL_EVENTFLAG> modifiers) {
  DCHECK(form_fill_env);

  if (!CPDF_AAction::IsUserInput(type))
    return false;

  switch (action.GetType()) {
    case CPDF_Action::Type::kGoTo:
      DoAction_GoTo(form_fill_env, action);
      return true;
    case CPDF_Action::Type::kURI:
      DoAction_URI(form_fill_env, action, modifiers);
      return true;
    default:
      return false;
  }
}

bool CPDFSDK_ActionHandler::DoAction_Destination(
    const CPDF_Dest& dest,
    CPDFSDK_FormFillEnvironment* form_fill_env) {
  DCHECK(form_fill_env);
  CPDF_Document* document = form_fill_env->GetPDFDocument();
  DCHECK(document);

  const CPDF_Array* dest_array = dest.GetArray();
  std::vector<float> dest_positions;
  // |dest_array| index 0 contains destination page details and index 1 contains
  // parameter that explains about the rest of |dest_array|.
  if (dest_array) {
    for (size_t i = 2; i < dest_array->size(); i++)
      dest_positions.push_back(dest_array->GetNumberAt(i));
  }

  form_fill_env->DoGoToAction(dest.GetDestPageIndex(document),
                              dest.GetZoomMode(), dest_positions);
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
    CFFL_FieldAction* data) {
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

  DCHECK(pFormFillEnv);
  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty())
        RunDocumentOpenJavaScript(pFormFillEnv, WideString(), swJS);
    }
  } else {
    DoAction_NoJs(action, CPDF_AAction::AActionType::kDocumentOpen,
                  pFormFillEnv);
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

  DCHECK(pFormFillEnv);
  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty())
        RunDocumentPageJavaScript(pFormFillEnv, type, swJS);
    }
  } else {
    DoAction_NoJs(action, type, pFormFillEnv);
  }

  DCHECK(pFormFillEnv);

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
  DCHECK(pFieldDict);

  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  return !!pPDFForm->GetFieldByDict(pFieldDict);
}

bool CPDFSDK_ActionHandler::ExecuteFieldAction(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CFFL_FieldAction* data,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict))
    return false;

  visited->insert(pDict);

  DCHECK(pFormFillEnv);
  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (pFormFillEnv->IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty()) {
        RunFieldJavaScript(pFormFillEnv, pFormField, type, data, swJS);
        if (!IsValidField(pFormFillEnv, pFormField->GetFieldDict()))
          return false;
      }
    }
  } else {
    DoAction_NoJs(action, type, pFormFillEnv);
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
    CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  DCHECK(pFormFillEnv);

  switch (action.GetType()) {
    case CPDF_Action::Type::kGoTo:
      DoAction_GoTo(pFormFillEnv, action);
      break;
    case CPDF_Action::Type::kURI:
      if (CPDF_AAction::IsUserInput(type))
        DoAction_URI(pFormFillEnv, action, Mask<FWL_EVENTFLAG>{});
      break;
    case CPDF_Action::Type::kHide:
      DoAction_Hide(action, pFormFillEnv);
      break;
    case CPDF_Action::Type::kNamed:
      DoAction_Named(pFormFillEnv, action);
      break;
    case CPDF_Action::Type::kSubmitForm:
      if (CPDF_AAction::IsUserInput(type))
        DoAction_SubmitForm(action, pFormFillEnv);
      break;
    case CPDF_Action::Type::kResetForm:
      DoAction_ResetForm(action, pFormFillEnv);
      break;
    case CPDF_Action::Type::kJavaScript:
      NOTREACHED();
      break;
    case CPDF_Action::Type::kSetOCGState:
    case CPDF_Action::Type::kThread:
    case CPDF_Action::Type::kSound:
    case CPDF_Action::Type::kMovie:
    case CPDF_Action::Type::kRendition:
    case CPDF_Action::Type::kTrans:
    case CPDF_Action::Type::kGoTo3DView:
    case CPDF_Action::Type::kGoToR:
    case CPDF_Action::Type::kGoToE:
    case CPDF_Action::Type::kLaunch:
    case CPDF_Action::Type::kImportData:
      // Unimplemented
      break;
    default:
      break;
  }
}

void CPDFSDK_ActionHandler::DoAction_GoTo(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action) {
  DCHECK(action.GetDict());

  CPDF_Document* pPDFDocument = pFormFillEnv->GetPDFDocument();
  DCHECK(pPDFDocument);

  CPDF_Dest MyDest = action.GetDest(pPDFDocument);
  DoAction_Destination(MyDest, pFormFillEnv);
}

void CPDFSDK_ActionHandler::DoAction_URI(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action,
    Mask<FWL_EVENTFLAG> modifiers) {
  DCHECK(action.GetDict());
  pFormFillEnv->DoURIAction(action.GetURI(pFormFillEnv->GetPDFDocument()),
                            modifiers);
}

void CPDFSDK_ActionHandler::DoAction_Named(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const CPDF_Action& action) {
  DCHECK(action.GetDict());
  pFormFillEnv->ExecuteNamedAction(action.GetNamedAction());
}

void CPDFSDK_ActionHandler::RunFieldJavaScript(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_FormField* pFormField,
    CPDF_AAction::AActionType type,
    CFFL_FieldAction* data,
    const WideString& script) {
  DCHECK(type != CPDF_AAction::kCalculate);
  DCHECK(type != CPDF_AAction::kFormat);

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
  RunScript(pFormFillEnv, script, [sScriptName](IJS_EventContext* context) {
    context->OnDoc_Open(sScriptName);
  });
}

void CPDFSDK_ActionHandler::RunDocumentPageJavaScript(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    CPDF_AAction::AActionType type,
    const WideString& script) {
  RunScript(pFormFillEnv, script, [type](IJS_EventContext* context) {
    switch (type) {
      case CPDF_AAction::kOpenPage:
        context->OnPage_Open();
        break;
      case CPDF_AAction::kClosePage:
        context->OnPage_Close();
        break;
      case CPDF_AAction::kCloseDocument:
        context->OnDoc_WillClose();
        break;
      case CPDF_AAction::kSaveDocument:
        context->OnDoc_WillSave();
        break;
      case CPDF_AAction::kDocumentSaved:
        context->OnDoc_DidSave();
        break;
      case CPDF_AAction::kPrintDocument:
        context->OnDoc_WillPrint();
        break;
      case CPDF_AAction::kDocumentPrinted:
        context->OnDoc_DidPrint();
        break;
      case CPDF_AAction::kPageVisible:
        context->OnPage_InView();
        break;
      case CPDF_AAction::kPageInvisible:
        context->OnPage_OutView();
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
