// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_structtree.h"

#include <memory>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_structelement.h"
#include "core/fpdfdoc/cpdf_structtree.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/stl_util.h"
#include "fpdfsdk/cpdfsdk_helpers.h"

namespace {

unsigned long WideStringToBuffer(const WideString& str,
                                 void* buffer,
                                 unsigned long buflen) {
  if (str.IsEmpty())
    return 0;

  ByteString encodedStr = str.ToUTF16LE();
  const unsigned long len = encodedStr.GetLength();
  if (buffer && len <= buflen)
    memcpy(buffer, encodedStr.c_str(), len);
  return len;
}

}  // namespace

FPDF_EXPORT FPDF_STRUCTTREE FPDF_CALLCONV
FPDF_StructTree_GetForPage(FPDF_PAGE page) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  // Caller takes onwership.
  return FPDFStructTreeFromCPDFStructTree(
      CPDF_StructTree::LoadPage(pPage->GetDocument(), pPage->GetDict())
          .release());
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_StructTree_Close(FPDF_STRUCTTREE struct_tree) {
  std::unique_ptr<CPDF_StructTree>(
      CPDFStructTreeFromFPDFStructTree(struct_tree));
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StructTree_CountChildren(FPDF_STRUCTTREE struct_tree) {
  CPDF_StructTree* tree = CPDFStructTreeFromFPDFStructTree(struct_tree);
  if (!tree)
    return -1;

  FX_SAFE_INT32 tmp_size = tree->CountTopElements();
  return tmp_size.ValueOrDefault(-1);
}

FPDF_EXPORT FPDF_STRUCTELEMENT FPDF_CALLCONV
FPDF_StructTree_GetChildAtIndex(FPDF_STRUCTTREE struct_tree, int index) {
  CPDF_StructTree* tree = CPDFStructTreeFromFPDFStructTree(struct_tree);
  if (!tree || index < 0 ||
      static_cast<size_t>(index) >= tree->CountTopElements()) {
    return nullptr;
  }
  return FPDFStructElementFromCPDFStructElement(
      tree->GetTopElement(static_cast<size_t>(index)));
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetAltText(FPDF_STRUCTELEMENT struct_element,
                              void* buffer,
                              unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  return elem ? WideStringToBuffer(elem->GetAltText(), buffer, buflen) : 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetActualText(FPDF_STRUCTELEMENT struct_element,
                                 void* buffer,
                                 unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  return elem ? WideStringToBuffer(elem->GetActualText(), buffer, buflen) : 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetID(FPDF_STRUCTELEMENT struct_element,
                         void* buffer,
                         unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Dictionary* dict = elem ? elem->GetDict() : nullptr;
  if (!dict)
    return 0;
  const CPDF_Object* obj = dict->GetObjectFor("ID");
  if (!obj || !obj->IsString())
    return 0;
  return Utf16EncodeMaybeCopyAndReturnLength(obj->GetUnicodeText(), buffer,
                                             buflen);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetLang(FPDF_STRUCTELEMENT struct_element,
                           void* buffer,
                           unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Dictionary* dict = elem ? elem->GetDict() : nullptr;
  if (!dict)
    return 0;
  const CPDF_Object* obj = dict->GetObjectFor("Lang");
  if (!obj || !obj->IsString())
    return 0;
  return Utf16EncodeMaybeCopyAndReturnLength(obj->GetUnicodeText(), buffer,
                                             buflen);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StructElement_GetAttributeCount(FPDF_STRUCTELEMENT struct_element) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Dictionary* dict = elem ? elem->GetDict() : nullptr;
  const CPDF_Object* attr_obj = dict ? dict->GetObjectFor("A") : nullptr;
  if (!attr_obj)
    return -1;

  if (attr_obj->IsArray())
    return attr_obj->AsArray()->size();
  return attr_obj->IsDictionary() ? 1 : -1;
}

FPDF_EXPORT FPDF_STRUCTELEMENT_ATTR FPDF_CALLCONV
FPDF_StructElement_GetAttributeAtIndex(FPDF_STRUCTELEMENT struct_element,
                                       int index) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Dictionary* dict = elem ? elem->GetDict() : nullptr;
  const CPDF_Object* attr_obj = dict ? dict->GetObjectFor("A") : nullptr;
  if (!attr_obj)
    return nullptr;

  if (attr_obj->IsDictionary()) {
    return index == 0 ? FPDFStructElementAttrFromCPDFDictionary(
                            attr_obj->AsDictionary())
                      : nullptr;
  }

  if (attr_obj->IsArray()) {
    const CPDF_Array* array = attr_obj->AsArray();
    if (index < 0 || static_cast<size_t>(index) >= array->size())
      return nullptr;
    return FPDFStructElementAttrFromCPDFDictionary(array->GetDictAt(index));
  }

  return nullptr;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetStringAttribute(FPDF_STRUCTELEMENT struct_element,
                                      FPDF_BYTESTRING attr_name,
                                      void* buffer,
                                      unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Dictionary* dict = elem ? elem->GetDict() : nullptr;
  const CPDF_Array* array = dict ? dict->GetArrayFor("A") : nullptr;
  if (!array)
    return 0;
  CPDF_ArrayLocker locker(array);
  for (const RetainPtr<CPDF_Object>& obj : locker) {
    const CPDF_Dictionary* obj_dict = obj->AsDictionary();
    if (!obj_dict)
      continue;
    const CPDF_Object* attr = obj_dict->GetObjectFor(attr_name);
    if (!attr || !(attr->IsString() || attr->IsName()))
      continue;
    return Utf16EncodeMaybeCopyAndReturnLength(attr->GetUnicodeText(), buffer,
                                               buflen);
  }
  return 0;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StructElement_GetMarkedContentID(FPDF_STRUCTELEMENT struct_element) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  const CPDF_Object* p = elem ? elem->GetDict()->GetObjectFor("K") : nullptr;
  return p && p->IsNumber() ? p->GetInteger() : -1;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetType(FPDF_STRUCTELEMENT struct_element,
                           void* buffer,
                           unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  return elem ? WideStringToBuffer(
                    WideString::FromUTF8(elem->GetType().AsStringView()),
                    buffer, buflen)
              : 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetObjType(FPDF_STRUCTELEMENT struct_element,
                              void* buffer,
                              unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  return elem ? WideStringToBuffer(
                    WideString::FromUTF8(elem->GetObjType().AsStringView()),
                    buffer, buflen)
              : 0;
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDF_StructElement_GetTitle(FPDF_STRUCTELEMENT struct_element,
                            void* buffer,
                            unsigned long buflen) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  return elem ? WideStringToBuffer(elem->GetTitle(), buffer, buflen) : 0;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StructElement_CountChildren(FPDF_STRUCTELEMENT struct_element) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  if (!elem)
    return -1;

  FX_SAFE_INT32 tmp_size = elem->CountKids();
  return tmp_size.ValueOrDefault(-1);
}

FPDF_EXPORT FPDF_STRUCTELEMENT FPDF_CALLCONV
FPDF_StructElement_GetChildAtIndex(FPDF_STRUCTELEMENT struct_element,
                                   int index) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  if (!elem || index < 0 || static_cast<size_t>(index) >= elem->CountKids())
    return nullptr;

  return FPDFStructElementFromCPDFStructElement(elem->GetKidIfElement(index));
}

FPDF_EXPORT FPDF_STRUCTELEMENT FPDF_CALLCONV
FPDF_StructElement_GetParent(FPDF_STRUCTELEMENT struct_element) {
  CPDF_StructElement* elem =
      CPDFStructElementFromFPDFStructElement(struct_element);
  CPDF_StructElement* parent = elem ? elem->GetParent() : nullptr;
  if (!parent) {
    return nullptr;
  }
  return FPDFStructElementFromCPDFStructElement(parent);
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StructElement_Attr_GetCount(FPDF_STRUCTELEMENT_ATTR struct_attribute) {
  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return -1;
  return fxcrt::CollectionSize<int>(*dict);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StructElement_Attr_GetName(FPDF_STRUCTELEMENT_ATTR struct_attribute,
                                int index,
                                void* buffer,
                                unsigned long buflen,
                                unsigned long* out_buflen) {
  if (!out_buflen || !buffer)
    return false;

  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return false;

  CPDF_DictionaryLocker locker(dict);
  for (auto& it : locker) {
    if (index == 0) {
      *out_buflen =
          NulTerminateMaybeCopyAndReturnLength(it.first, buffer, buflen);
      return true;
    }
    --index;
  }
  return false;
}

FPDF_EXPORT FPDF_OBJECT_TYPE FPDF_CALLCONV
FPDF_StructElement_Attr_GetType(FPDF_STRUCTELEMENT_ATTR struct_attribute,
                                FPDF_BYTESTRING name) {
  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return FPDF_OBJECT_UNKNOWN;

  const CPDF_Object* obj = dict->GetObjectFor(name);
  return obj ? obj->GetType() : FPDF_OBJECT_UNKNOWN;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_StructElement_Attr_GetBooleanValue(
    FPDF_STRUCTELEMENT_ATTR struct_attribute,
    FPDF_BYTESTRING name,
    FPDF_BOOL* out_value) {
  if (!out_value)
    return false;

  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return false;

  const CPDF_Object* obj = dict->GetObjectFor(name);
  if (!obj || !obj->IsBoolean())
    return false;

  *out_value = obj->GetInteger();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StructElement_Attr_GetNumberValue(FPDF_STRUCTELEMENT_ATTR struct_attribute,
                                       FPDF_BYTESTRING name,
                                       float* out_value) {
  if (!out_value)
    return false;

  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return false;

  const CPDF_Object* obj = dict->GetObjectFor(name);
  if (!obj || !obj->IsNumber())
    return false;

  *out_value = obj->GetNumber();
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StructElement_Attr_GetStringValue(FPDF_STRUCTELEMENT_ATTR struct_attribute,
                                       FPDF_BYTESTRING name,
                                       void* buffer,
                                       unsigned long buflen,
                                       unsigned long* out_buflen) {
  if (!out_buflen)
    return false;

  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return false;

  const CPDF_Object* obj = dict->GetObjectFor(name);
  if (!obj || !(obj->IsString() || obj->IsName()))
    return false;

  *out_buflen = Utf16EncodeMaybeCopyAndReturnLength(
      WideString::FromUTF8(obj->GetString().AsStringView()), buffer, buflen);
  return true;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StructElement_Attr_GetBlobValue(FPDF_STRUCTELEMENT_ATTR struct_attribute,
                                     FPDF_BYTESTRING name,
                                     void* buffer,
                                     unsigned long buflen,
                                     unsigned long* out_buflen) {
  if (!out_buflen)
    return false;

  const CPDF_Dictionary* dict =
      CPDFDictionaryFromFPDFStructElementAttr(struct_attribute);
  if (!dict)
    return false;

  const CPDF_Object* obj = dict->GetObjectFor(name);
  if (!obj || !obj->IsString())
    return false;

  ByteString result = obj->GetString();
  unsigned long len = result.GetLength();

  if (buffer && len <= buflen)
    memcpy(buffer, result.c_str(), len);

  *out_buflen = len;
  return true;
}
