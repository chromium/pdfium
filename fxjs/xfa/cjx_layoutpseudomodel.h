// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_LAYOUTPSEUDOMODEL_H_
#define FXJS_XFA_CJX_LAYOUTPSEUDOMODEL_H_

#include <vector>

#include "core/fxcrt/span.h"
#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"

class CScript_LayoutPseudoModel;
class CXFA_LayoutProcessor;
class CXFA_Node;

class CJX_LayoutPseudoModel final : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_LayoutPseudoModel() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(absPage);
  JSE_METHOD(absPageCount);
  JSE_METHOD(absPageCountInBatch);
  JSE_METHOD(absPageInBatch);
  JSE_METHOD(absPageSpan);
  JSE_METHOD(h);
  JSE_METHOD(page);
  JSE_METHOD(pageContent);
  JSE_METHOD(pageCount);
  JSE_METHOD(pageSpan);
  JSE_METHOD(relayout);
  JSE_METHOD(relayoutPageArea);
  JSE_METHOD(sheet);
  JSE_METHOD(sheetCount);
  JSE_METHOD(sheetCountInBatch);
  JSE_METHOD(sheetInBatch);
  JSE_METHOD(w);
  JSE_METHOD(x);
  JSE_METHOD(y);

  JSE_PROP(ready);

 private:
  enum class HWXY { kH, kW, kX, kY };

  explicit CJX_LayoutPseudoModel(CScript_LayoutPseudoModel* model);

  using Type__ = CJX_LayoutPseudoModel;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::LayoutPseudoModel;
  static const CJX_MethodSpec MethodSpecs[];

  CJS_Result AllPageCount(CFXJSE_Engine* runtime);
  CJS_Result NumberedPageCount(CFXJSE_Engine* runtime);
  CJS_Result DoHWXYInternal(CFXJSE_Engine* runtime,
                            pdfium::span<v8::Local<v8::Value>> params,
                            HWXY layoutModel);
  std::vector<CXFA_Node*> GetObjArray(CXFA_LayoutProcessor* pDocLayout,
                                      int32_t iPageNo,
                                      const WideString& wsType,
                                      bool bOnPageArea);
  CJS_Result PageInternals(CFXJSE_Engine* runtime,
                           pdfium::span<v8::Local<v8::Value>> params,
                           bool bAbsPage);
};

#endif  // FXJS_XFA_CJX_LAYOUTPSEUDOMODEL_H_
