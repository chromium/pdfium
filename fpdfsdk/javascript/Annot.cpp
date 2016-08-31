// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Annot.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_context.h"

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

FX_BOOL Annot::hidden(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (vp.IsGetting()) {
    CPDF_Annot* pPDFAnnot = m_BAAnnot->GetPDFAnnot();
    vp << CPDF_Annot::IsAnnotationHidden(pPDFAnnot->GetAnnotDict());
    return TRUE;
  }

  bool bHidden;
  vp >> bHidden;

  uint32_t flags = m_BAAnnot->GetFlags();
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
  m_BAAnnot->SetFlags(flags);
  return TRUE;
}

FX_BOOL Annot::name(IJS_Context* cc,
                    CJS_PropValue& vp,
                    CFX_WideString& sError) {
  if (vp.IsGetting()) {
    vp << m_BAAnnot->GetAnnotName();
    return TRUE;
  }

  CFX_WideString annotName;
  vp >> annotName;
  m_BAAnnot->SetAnnotName(annotName);
  return TRUE;
}

FX_BOOL Annot::type(IJS_Context* cc,
                    CJS_PropValue& vp,
                    CFX_WideString& sError) {
  if (vp.IsSetting()) {
    CJS_Context* pContext = static_cast<CJS_Context*>(cc);
    sError = JSGetStringFromID(pContext, IDS_STRING_JSREADONLY);
    return FALSE;
  }

  vp << CPDF_Annot::AnnotSubtypeToString(m_BAAnnot->GetAnnotSubtype());
  return TRUE;
}

void Annot::SetSDKAnnot(CPDFSDK_BAAnnot* annot) {
  m_BAAnnot = annot;
}
