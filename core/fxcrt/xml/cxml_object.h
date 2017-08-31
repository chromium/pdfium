// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_OBJECT_H_
#define CORE_FXCRT_XML_CXML_OBJECT_H_

class CXML_Content;
class CXML_Element;

class CXML_Object {
 public:
  virtual ~CXML_Object();

  virtual CXML_Content* AsContent();
  virtual const CXML_Content* AsContent() const;

  virtual CXML_Element* AsElement();
  virtual const CXML_Element* AsElement() const;

 protected:
  CXML_Object() {}
};

inline CXML_Content* ToContent(CXML_Object* pObj) {
  return pObj ? pObj->AsContent() : nullptr;
}

inline const CXML_Content* ToContent(const CXML_Object* pObj) {
  return pObj ? pObj->AsContent() : nullptr;
}

inline CXML_Element* ToElement(CXML_Object* pObj) {
  return pObj ? pObj->AsElement() : nullptr;
}

inline const CXML_Element* ToElement(const CXML_Object* pObj) {
  return pObj ? pObj->AsElement() : nullptr;
}

#endif  // CORE_FXCRT_XML_CXML_OBJECT_H_
