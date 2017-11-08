// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_OBJECT_H_
#define FXJS_CJX_OBJECT_H_

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Value;
class CXFA_Object;
class CXFA_Document;

class CJX_Object {
 public:
  virtual ~CJX_Object();

  CXFA_Object* GetXFAObject() { return object_.Get(); }
  const CXFA_Object* GetXFAObject() const { return object_.Get(); }

  CXFA_Document* GetDocument() const;

  void Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);

  void ThrowInvalidPropertyException() const;
  void ThrowArgumentMismatchException() const;
  void ThrowIndexOutOfBoundsException() const;
  void ThrowParamCountMismatchException(const WideString& method) const;

 protected:
  explicit CJX_Object(CXFA_Object* object);

  void ThrowException(const wchar_t* str, ...) const;

 private:
  UnownedPtr<CXFA_Object> object_;
};

#endif  // FXJS_CJX_OBJECT_H_
