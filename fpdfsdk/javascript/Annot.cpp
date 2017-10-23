// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Annot.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"

namespace {

CPDFSDK_BAAnnot* ToBAAnnot(CPDFSDK_Annot* annot) {
  return static_cast<CPDFSDK_BAAnnot*>(annot);
}

}  // namespace

JSConstSpec CJS_Annot::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Annot::PropertySpecs[] = {
    {"hidden", get_hidden_static, set_hidden_static},
    {"name", get_name_static, set_name_static},
    {"type", get_type_static, set_type_static},
    {0, 0, 0}};

JSMethodSpec CJS_Annot::MethodSpecs[] = {{0, 0}};

IMPLEMENT_JS_CLASS(CJS_Annot, Annot)

Annot::Annot(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {}

Annot::~Annot() {}

bool Annot::get_hidden(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  if (!m_pAnnot) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  CPDF_Annot* pPDFAnnot = ToBAAnnot(m_pAnnot.Get())->GetPDFAnnot();
  vp->Set(pRuntime, CPDF_Annot::IsAnnotationHidden(pPDFAnnot->GetAnnotDict()));
  return true;
}

bool Annot::set_hidden(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  bool bHidden = vp.ToBool(pRuntime);  // May invalidate m_pAnnot.
  if (!m_pAnnot) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  uint32_t flags = ToBAAnnot(m_pAnnot.Get())->GetFlags();
  if (bHidden) {
    flags |= ANNOTFLAG_HIDDEN;
    flags |= ANNOTFLAG_INVISIBLE;
    flags |= ANNOTFLAG_NOVIEW;
    flags &= ~ANNOTFLAG_PRINT;
  } else {
    flags &= ~ANNOTFLAG_HIDDEN;
    flags &= ~ANNOTFLAG_INVISIBLE;
    flags &= ~ANNOTFLAG_NOVIEW;
    flags |= ANNOTFLAG_PRINT;
  }
  ToBAAnnot(m_pAnnot.Get())->SetFlags(flags);

  return true;
}

bool Annot::get_name(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  if (!m_pAnnot) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  vp->Set(pRuntime, ToBAAnnot(m_pAnnot.Get())->GetAnnotName());
  return true;
}

bool Annot::set_name(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  WideString annotName = vp.ToWideString(pRuntime);  // May invalidate m_pAnnot.
  if (!m_pAnnot) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  ToBAAnnot(m_pAnnot.Get())->SetAnnotName(annotName);
  return true;
}

bool Annot::get_type(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  if (!m_pAnnot) {
    *sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  vp->Set(pRuntime, CPDF_Annot::AnnotSubtypeToString(
                        ToBAAnnot(m_pAnnot.Get())->GetAnnotSubtype()));
  return true;
}

bool Annot::set_type(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  *sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
  return false;
}

void Annot::SetSDKAnnot(CPDFSDK_BAAnnot* annot) {
  m_pAnnot.Reset(annot);
}
