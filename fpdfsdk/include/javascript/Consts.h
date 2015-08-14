// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_

#include "JS_Define.h"

/* ------------------------------ border ------------------------------ */

class CJS_Border : public CJS_Object {
 public:
  explicit CJS_Border(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Border() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ display ------------------------------ */

class CJS_Display : public CJS_Object {
 public:
  explicit CJS_Display(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Display() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ font ------------------------------ */

class CJS_Font : public CJS_Object {
 public:
  explicit CJS_Font(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Font() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ highlight ------------------------------ */

class CJS_Highlight : public CJS_Object {
 public:
  explicit CJS_Highlight(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Highlight() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ position ------------------------------ */

class CJS_Position : public CJS_Object {
 public:
  explicit CJS_Position(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Position() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ scaleHow ------------------------------ */

class CJS_ScaleHow : public CJS_Object {
 public:
  explicit CJS_ScaleHow(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_ScaleHow() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ scaleWhen ------------------------------ */

class CJS_ScaleWhen : public CJS_Object {
 public:
  explicit CJS_ScaleWhen(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_ScaleWhen() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ style ------------------------------ */

class CJS_Style : public CJS_Object {
 public:
  explicit CJS_Style(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Style() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ zoomtype ------------------------------ */

class CJS_Zoomtype : public CJS_Object {
 public:
  explicit CJS_Zoomtype(JSFXObject pObject) : CJS_Object(pObject) {}
  ~CJS_Zoomtype() override {}

  DECLARE_JS_CLASS_CONST();
};

/* ------------------------------ CJS_GlobalConsts
 * ------------------------------ */

class CJS_GlobalConsts : public CJS_Object {
 public:
  static int Init(IJS_Runtime* pRuntime);
};

/* ------------------------------ CJS_GlobalArrays
 * ------------------------------ */

class CJS_GlobalArrays : public CJS_Object {
 public:
  static int Init(IJS_Runtime* pRuntime);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_CONSTS_H_
