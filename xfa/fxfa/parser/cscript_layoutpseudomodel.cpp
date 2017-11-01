// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cscript_layoutpseudomodel.h"

#include "fxjs/cfxjse_arguments.h"
#include "third_party/base/ptr_util.h"

CScript_LayoutPseudoModel::CScript_LayoutPseudoModel(CXFA_Document* pDocument)
    : CXFA_Object(pDocument,
                  XFA_ObjectType::Object,
                  XFA_Element::LayoutPseudoModel,
                  WideStringView(L"layoutPseudoModel"),
                  pdfium::MakeUnique<CJX_LayoutPseudoModel>(this)) {}

CScript_LayoutPseudoModel::~CScript_LayoutPseudoModel() {}

void CScript_LayoutPseudoModel::Ready(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_ATTRIBUTE eAttribute) {
  JSLayoutPseudoModel()->Ready(pValue, bSetting, eAttribute);
}

void CScript_LayoutPseudoModel::H(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->H(pArguments);
}

void CScript_LayoutPseudoModel::W(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->W(pArguments);
}

void CScript_LayoutPseudoModel::X(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->X(pArguments);
}

void CScript_LayoutPseudoModel::Y(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->Y(pArguments);
}

void CScript_LayoutPseudoModel::PageCount(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->PageCount(pArguments);
}

void CScript_LayoutPseudoModel::PageSpan(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->PageSpan(pArguments);
}

void CScript_LayoutPseudoModel::Page(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->Page(pArguments);
}

void CScript_LayoutPseudoModel::PageContent(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->PageContent(pArguments);
}

void CScript_LayoutPseudoModel::AbsPageCount(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->AbsPageCount(pArguments);
}

void CScript_LayoutPseudoModel::AbsPageCountInBatch(
    CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->AbsPageCountInBatch(pArguments);
}

void CScript_LayoutPseudoModel::SheetCountInBatch(
    CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->SheetCountInBatch(pArguments);
}

void CScript_LayoutPseudoModel::Relayout(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->Relayout(pArguments);
}

void CScript_LayoutPseudoModel::AbsPageSpan(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->AbsPageSpan(pArguments);
}

void CScript_LayoutPseudoModel::AbsPageInBatch(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->AbsPageInBatch(pArguments);
}

void CScript_LayoutPseudoModel::SheetInBatch(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->SheetInBatch(pArguments);
}

void CScript_LayoutPseudoModel::Sheet(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->Sheet(pArguments);
}

void CScript_LayoutPseudoModel::RelayoutPageArea(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->RelayoutPageArea(pArguments);
}

void CScript_LayoutPseudoModel::SheetCount(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->SheetCount(pArguments);
}

void CScript_LayoutPseudoModel::AbsPage(CFXJSE_Arguments* pArguments) {
  JSLayoutPseudoModel()->AbsPage(pArguments);
}
