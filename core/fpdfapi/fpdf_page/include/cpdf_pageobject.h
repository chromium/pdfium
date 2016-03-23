// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGEOBJECT_H_
#define CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGEOBJECT_H_

#include "core/fpdfapi/fpdf_page/cpdf_contentmark.h"
#include "core/fpdfapi/fpdf_page/cpdf_graphicstates.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_TextObject;
class CPDF_PathObject;
class CPDF_ImageObject;
class CPDF_ShadingObject;
class CPDF_FormObject;

class CPDF_PageObject : public CPDF_GraphicStates {
 public:
  enum Type {
    TEXT = 1,
    PATH,
    IMAGE,
    SHADING,
    FORM,
  };

  CPDF_PageObject();
  virtual ~CPDF_PageObject();

  virtual CPDF_PageObject* Clone() const = 0;
  virtual Type GetType() const = 0;
  virtual void Transform(const CFX_Matrix& matrix) = 0;
  virtual bool IsText() const { return false; }
  virtual bool IsPath() const { return false; }
  virtual bool IsImage() const { return false; }
  virtual bool IsShading() const { return false; }
  virtual bool IsForm() const { return false; }
  virtual CPDF_TextObject* AsText() { return nullptr; }
  virtual const CPDF_TextObject* AsText() const { return nullptr; }
  virtual CPDF_PathObject* AsPath() { return nullptr; }
  virtual const CPDF_PathObject* AsPath() const { return nullptr; }
  virtual CPDF_ImageObject* AsImage() { return nullptr; }
  virtual const CPDF_ImageObject* AsImage() const { return nullptr; }
  virtual CPDF_ShadingObject* AsShading() { return nullptr; }
  virtual const CPDF_ShadingObject* AsShading() const { return nullptr; }
  virtual CPDF_FormObject* AsForm() { return nullptr; }
  virtual const CPDF_FormObject* AsForm() const { return nullptr; }

  void TransformClipPath(CFX_Matrix& matrix);
  void TransformGeneralState(CFX_Matrix& matrix);
  FX_RECT GetBBox(const CFX_Matrix* pMatrix) const;

  FX_FLOAT m_Left;
  FX_FLOAT m_Right;
  FX_FLOAT m_Top;
  FX_FLOAT m_Bottom;
  CPDF_ContentMark m_ContentMark;

 protected:
  void CopyData(const CPDF_PageObject* pSrcObject);

 private:
  CPDF_PageObject(const CPDF_PageObject& src) = delete;
  void operator=(const CPDF_PageObject& src) = delete;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_INCLUDE_CPDF_PAGEOBJECT_H_
