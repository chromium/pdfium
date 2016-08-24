// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_indirect_object_holder.h"

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

CPDF_Object* CPDF_IndirectObjectHolder::GetIndirectObject(
    uint32_t objnum) const {
  auto it = m_IndirectObjs.find(objnum);
  return it != m_IndirectObjs.end() ? it->second : nullptr;
}

CPDF_Object* CPDF_IndirectObjectHolder::GetOrParseIndirectObject(
    uint32_t objnum) {
  if (objnum == 0)
    return nullptr;

  CPDF_Object* pObj = GetIndirectObject(objnum);
  if (pObj)
    return pObj->GetObjNum() != CPDF_Object::kInvalidObjNum ? pObj : nullptr;

  if (!m_pParser)
    return nullptr;

  pObj = m_pParser->ParseIndirectObject(this, objnum);
  if (!pObj)
    return nullptr;

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

bool CPDF_IndirectObjectHolder::ReplaceIndirectObjectIfHigherGeneration(
    uint32_t objnum,
    CPDF_Object* pObj) {
  if (!objnum || !pObj)
    return false;

  CPDF_Object* pOldObj = GetIndirectObject(objnum);
  if (pOldObj) {
    if (pObj->GetGenNum() <= pOldObj->GetGenNum()) {
      pObj->Destroy();
      return false;
    }
    pOldObj->Destroy();
  }
  pObj->m_ObjNum = objnum;
  m_IndirectObjs[objnum] = pObj;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  return true;
}

void CPDF_IndirectObjectHolder::ReleaseIndirectObject(uint32_t objnum) {
  CPDF_Object* pObj = GetIndirectObject(objnum);
  if (!pObj || pObj->GetObjNum() == CPDF_Object::kInvalidObjNum)
    return;

  pObj->Destroy();
  m_IndirectObjs.erase(objnum);
}
