// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Annot.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_context.h"

namespace {

CPDFSDK_BAAnnot* ToBAAnnot(CPDFSDK_Annot* annot) {
  return static_cast<CPDFSDK_BAAnnot*>(annot);
}

}  // namespace

BEGIN_JS_STATIC_CONST(CJS_Annot)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Annot)
JS_STATIC_PROP_ENTRY(hidden)
JS_STATIC_PROP_ENTRY(name)
JS_STATIC_PROP_ENTRY(type)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Annot)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Annot, Annot)

Annot::Annot(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {}

Annot::~Annot() {}

bool Annot::hidden(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (vp.IsGetting()) {
    if (!m_pAnnot) {
      sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
      return false;
    }
    CPDF_Annot* pPDFAnnot = ToBAAnnot(m_pAnnot.Get())->GetPDFAnnot();
    vp << CPDF_Annot::IsAnnotationHidden(pPDFAnnot->GetAnnotDict());
    return true;
  }

  bool bHidden;
  vp >> bHidden;  // May invalidate m_pAnnot.
  if (!m_pAnnot) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
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

bool Annot::name(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (vp.IsGetting()) {
    if (!m_pAnnot) {
      sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
      return false;
    }
    vp << ToBAAnnot(m_pAnnot.Get())->GetAnnotName();
    return true;
  }

  CFX_WideString annotName;
  vp >> annotName;  // May invalidate m_pAnnot.
  if (!m_pAnnot) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }

  ToBAAnnot(m_pAnnot.Get())->SetAnnotName(annotName);
  return true;
}

bool Annot::type(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  if (vp.IsSetting()) {
    sError = JSGetStringFromID(IDS_STRING_JSREADONLY);
    return false;
  }
  if (!m_pAnnot) {
    sError = JSGetStringFromID(IDS_STRING_JSBADOBJECT);
    return false;
  }
  vp << CPDF_Annot::AnnotSubtypeToString(
      ToBAAnnot(m_pAnnot.Get())->GetAnnotSubtype());
  return true;
}

void Annot::SetSDKAnnot(CPDFSDK_BAAnnot* annot) {
  m_pAnnot.Reset(annot);
}
