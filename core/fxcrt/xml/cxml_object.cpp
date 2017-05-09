// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_object.h"

CXML_Object::~CXML_Object() {}

CXML_Content* CXML_Object::AsContent() {
  return nullptr;
}

CXML_Element* CXML_Object::AsElement() {
  return nullptr;
}

const CXML_Content* CXML_Object::AsContent() const {
  return nullptr;
}

const CXML_Element* CXML_Object::AsElement() const {
  return nullptr;
}
