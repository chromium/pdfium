// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_indirect_object_holder.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_parser.h"

CPDF_IndirectObjectHolder::CPDF_IndirectObjectHolder(CPDF_Parser* pParser)
    : m_pParser(pParser), m_LastObjNum(0) {
  if (pParser)
    m_LastObjNum = m_pParser->GetLastObjNum();
}

CPDF_IndirectObjectHolder::~CPDF_IndirectObjectHolder() {
  for (const auto& pair : m_IndirectObjs)
    pair.second->Destroy();
}

CPDF_Object* CPDF_IndirectObjectHolder::GetIndirectObject(uint32_t objnum) {
  if (objnum == 0)
    return nullptr;

  CPDF_Object* result_obj = nullptr;
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end()) {
    CPDF_Object* obj = it->second;
    result_obj =
        obj->GetObjNum() != CPDF_Object::kInvalidObjNum ? it->second : nullptr;
    // Xref object is not used by the pdf document itself. Some software thus
    // reuse an object number for xref object. So when we get an xref object,
    // try again to see whether another object with the same number is defined.
    // If so, use that object instead. See chromium:596947.
    CPDF_Dictionary* dict =
        obj->IsStream() ? obj->GetDict() : obj->AsDictionary();
    if (!dict || dict->GetStringBy("Type") != "XRef")
      return result_obj;
  }

  if (!m_pParser)
    return nullptr;

  CPDF_Object* pObj = m_pParser->ParseIndirectObject(this, objnum);
  if (!pObj)
    return result_obj;

  pObj->m_ObjNum = objnum;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  if (m_IndirectObjs[objnum])
    m_IndirectObjs[objnum]->Destroy();

  m_IndirectObjs[objnum] = pObj;
  return pObj;
}

uint32_t CPDF_IndirectObjectHolder::AddIndirectObject(CPDF_Object* pObj) {
  if (pObj->m_ObjNum)
    return pObj->m_ObjNum;

  m_LastObjNum++;
  m_IndirectObjs[m_LastObjNum] = pObj;
  pObj->m_ObjNum = m_LastObjNum;
  return m_LastObjNum;
}

void CPDF_IndirectObjectHolder::ReleaseIndirectObject(uint32_t objnum) {
  auto it = m_IndirectObjs.find(objnum);
  if (it == m_IndirectObjs.end() ||
      it->second->GetObjNum() == CPDF_Object::kInvalidObjNum) {
    return;
  }
  it->second->Destroy();
  m_IndirectObjs.erase(it);
}

FX_BOOL CPDF_IndirectObjectHolder::InsertIndirectObject(uint32_t objnum,
                                                        CPDF_Object* pObj) {
  if (!objnum || !pObj)
    return FALSE;
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end()) {
    if (pObj->GetGenNum() <= it->second->GetGenNum()) {
      pObj->Destroy();
      return FALSE;
    }
    it->second->Destroy();
  }
  pObj->m_ObjNum = objnum;
  m_IndirectObjs[objnum] = pObj;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  return TRUE;
}
