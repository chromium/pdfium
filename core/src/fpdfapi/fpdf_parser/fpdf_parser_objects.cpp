// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fxcrt/fx_string.h"

//static
int CPDF_Object::s_nCurRefDepth = 0;

void CPDF_Object::Release()
{
    if (m_ObjNum) {
        return;
    }
    Destroy();
}
void CPDF_Object::Destroy()
{
    switch (m_Type) {
        case PDFOBJ_STRING:
            delete (CPDF_String*)this;
            break;
        case PDFOBJ_NAME:
            delete (CPDF_Name*)this;
            break;
        case PDFOBJ_ARRAY:
            delete (CPDF_Array*)this;
            break;
        case PDFOBJ_DICTIONARY:
            delete (CPDF_Dictionary*)this;
            break;
        case PDFOBJ_STREAM:
            delete (CPDF_Stream*)this;
            break;
        default:
            delete this;
    }
}
CFX_ByteString CPDF_Object::GetString() const
{
    switch (m_Type) {
        case PDFOBJ_BOOLEAN:
            return ((CPDF_Boolean*)this)->m_bValue ? "true" : "false";
        case PDFOBJ_NUMBER:
            return ((CPDF_Number*)this)->GetString();
        case PDFOBJ_STRING:
            return ((CPDF_String*)this)->m_String;
        case PDFOBJ_NAME:
            return ((CPDF_Name*)this)->m_Name;
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)(FX_LPVOID)this;
                if (pRef->m_pObjList == NULL) {
                    break;
                }
                CPDF_Object* pObj = pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum);
                if (pObj == NULL) {
                    return CFX_ByteString();
                }
                return pObj->GetString();
            }
    }
    return CFX_ByteString();
}
CFX_ByteStringC CPDF_Object::GetConstString() const
{
    switch (m_Type) {
        case PDFOBJ_STRING:
            return CFX_ByteStringC((FX_LPCBYTE)((CPDF_String*)this)->m_String, ((CPDF_String*)this)->m_String.GetLength());
        case PDFOBJ_NAME:
            return CFX_ByteStringC((FX_LPCBYTE)((CPDF_Name*)this)->m_Name, ((CPDF_Name*)this)->m_Name.GetLength());
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)(FX_LPVOID)this;
                if (pRef->m_pObjList == NULL) {
                    break;
                }
                CPDF_Object* pObj = pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum);
                if (pObj == NULL) {
                    return CFX_ByteStringC();
                }
                return pObj->GetConstString();
            }
    }
    return CFX_ByteStringC();
}
FX_FLOAT CPDF_Object::GetNumber() const
{
    switch (m_Type) {
        case PDFOBJ_NUMBER:
            return ((CPDF_Number*)this)->GetNumber();
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)(FX_LPVOID)this;
                if (pRef->m_pObjList == NULL) {
                    break;
                }
                CPDF_Object* pObj = pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum);
                if (pObj == NULL) {
                    return 0;
                }
                return pObj->GetNumber();
            }
    }
    return 0;
}
FX_FLOAT CPDF_Object::GetNumber16() const
{
    return GetNumber();
}
int CPDF_Object::GetInteger() const
{
    CFX_AutoRestorer<int> restorer(&s_nCurRefDepth);
    if (++s_nCurRefDepth > OBJECT_REF_MAX_DEPTH) {
        return 0;
    }
    switch (m_Type) {
        case PDFOBJ_BOOLEAN:
            return ((CPDF_Boolean*)this)->m_bValue;
        case PDFOBJ_NUMBER:
            return ((CPDF_Number*)this)->GetInteger();
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)(FX_LPVOID)this;
                PARSE_CONTEXT context;
                FXSYS_memset32(&context, 0, sizeof(PARSE_CONTEXT));
                if (pRef->m_pObjList == NULL) {
                    return 0;
                }
                CPDF_Object* pObj = pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum, &context);
                if (pObj == NULL) {
                    return 0;
                }
                return pObj->GetInteger();
            }
    }
    return 0;
}
CPDF_Dictionary* CPDF_Object::GetDict() const
{
    switch (m_Type) {
        case PDFOBJ_DICTIONARY:
            return (CPDF_Dictionary*)this;
        case PDFOBJ_STREAM:
            return ((CPDF_Stream*)this)->GetDict();
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)this;
                if (pRef->m_pObjList == NULL) {
                    break;
                }
                CPDF_Object* pObj = pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum);
                if (pObj == NULL) {
                    return NULL;
                }
                return pObj->GetDict();
            }
    }
    return NULL;
}
CPDF_Array* CPDF_Object::GetArray() const
{
    if (m_Type == PDFOBJ_ARRAY)
        return (CPDF_Array*)this;
    else
        return NULL;
}
void CPDF_Object::SetString(const CFX_ByteString& str)
{
    ASSERT(this != NULL);
    switch (m_Type) {
        case PDFOBJ_BOOLEAN:
            ((CPDF_Boolean*)this)->m_bValue = str == FX_BSTRC("true") ? 1 : 0;
            return;
        case PDFOBJ_NUMBER:
            ((CPDF_Number*)this)->SetString(str);
            return;
        case PDFOBJ_STRING:
            ((CPDF_String*)this)->m_String = str;
            return;
        case PDFOBJ_NAME:
            ((CPDF_Name*)this)->m_Name = str;
            return;
    }
    ASSERT(FALSE);
}
int CPDF_Object::GetDirectType() const
{
    if (m_Type != PDFOBJ_REFERENCE) {
        return m_Type;
    }
    CPDF_Reference* pRef = (CPDF_Reference*)this;
    return pRef->m_pObjList->GetIndirectType(pRef->m_RefObjNum);
}
FX_BOOL CPDF_Object::IsIdentical(CPDF_Object* pOther) const
{
    if (this == pOther) {
        return TRUE;
    }
    if (pOther == NULL) {
        return FALSE;
    }
    if (pOther->m_Type != m_Type) {
        if (m_Type == PDFOBJ_REFERENCE && GetDirect()) {
            return GetDirect()->IsIdentical(pOther);
        } else if (pOther->m_Type == PDFOBJ_REFERENCE) {
            return IsIdentical(pOther->GetDirect());
        }
        return FALSE;
    }
    switch (m_Type) {
        case PDFOBJ_BOOLEAN:
            return (((CPDF_Boolean*)this)->Identical((CPDF_Boolean*)pOther));
        case PDFOBJ_NUMBER:
            return (((CPDF_Number*)this)->Identical((CPDF_Number*)pOther));
        case PDFOBJ_STRING:
            return (((CPDF_String*)this)->Identical((CPDF_String*)pOther));
        case PDFOBJ_NAME:
            return (((CPDF_Name*)this)->Identical((CPDF_Name*)pOther));
        case PDFOBJ_ARRAY:
            return (((CPDF_Array*)this)->Identical((CPDF_Array*)pOther));
        case PDFOBJ_DICTIONARY:
            return (((CPDF_Dictionary*)this)->Identical((CPDF_Dictionary*)pOther));
        case PDFOBJ_NULL:
            return TRUE;
        case PDFOBJ_STREAM:
            return (((CPDF_Stream*)this)->Identical((CPDF_Stream*)pOther));
        case PDFOBJ_REFERENCE:
            return (((CPDF_Reference*)this)->Identical((CPDF_Reference*)pOther));
    }
    return FALSE;
}
CPDF_Object* CPDF_Object::GetDirect() const
{
    if (m_Type != PDFOBJ_REFERENCE) {
        return (CPDF_Object*)this;
    }
    CPDF_Reference* pRef = (CPDF_Reference*)(FX_LPVOID)this;
    if (pRef->m_pObjList == NULL) {
        return NULL;
    }
    return pRef->m_pObjList->GetIndirectObject(pRef->m_RefObjNum);
}
CPDF_Object* CPDF_Object::Clone(FX_BOOL bDirect) const
{
    CFX_MapPtrToPtr visited;
    return CloneInternal(bDirect, &visited);
}
CPDF_Object* CPDF_Object::CloneInternal(FX_BOOL bDirect, CFX_MapPtrToPtr* visited) const
{
    switch (m_Type) {
        case PDFOBJ_BOOLEAN:
            return new CPDF_Boolean(((CPDF_Boolean*)this)->m_bValue);
        case PDFOBJ_NUMBER:
            return new CPDF_Number(((CPDF_Number*)this)->m_bInteger, &((CPDF_Number*)this)->m_Integer);
        case PDFOBJ_STRING:
            return new CPDF_String(((CPDF_String*)this)->m_String, ((CPDF_String*)this)->IsHex());
        case PDFOBJ_NAME:
            return new CPDF_Name(((CPDF_Name*)this)->m_Name);
        case PDFOBJ_ARRAY: {
                CPDF_Array* pCopy = new CPDF_Array();
                CPDF_Array* pThis = (CPDF_Array*)this;
                int n = pThis->GetCount();
                for (int i = 0; i < n; i ++) {
                    CPDF_Object* value = (CPDF_Object*)pThis->m_Objects.GetAt(i);
                    pCopy->m_Objects.Add(value->CloneInternal(bDirect, visited));
                }
                return pCopy;
            }
        case PDFOBJ_DICTIONARY: {
                CPDF_Dictionary* pCopy = new CPDF_Dictionary();
                CPDF_Dictionary* pThis = (CPDF_Dictionary*)this;
                FX_POSITION pos = pThis->m_Map.GetStartPosition();
                while (pos) {
                    CFX_ByteString key;
                    CPDF_Object* value;
                    pThis->m_Map.GetNextAssoc(pos, key, (void*&)value);
                    pCopy->m_Map.SetAt(key, value->CloneInternal(bDirect, visited));
                }
                return pCopy;
            }
        case PDFOBJ_NULL: {
                return new CPDF_Null;
            }
        case PDFOBJ_STREAM: {
                CPDF_Stream* pThis = (CPDF_Stream*)this;
                CPDF_StreamAcc acc;
                acc.LoadAllData(pThis, TRUE);
                FX_DWORD streamSize = acc.GetSize();
                CPDF_Stream* pObj;
                if (pThis->GetDict())
                    pObj = new CPDF_Stream(acc.DetachData(), streamSize, (CPDF_Dictionary*)((CPDF_Object*)pThis->GetDict())->CloneInternal(bDirect, visited));
                else
                    pObj = new CPDF_Stream(acc.DetachData(), streamSize, NULL);
                return pObj;
            }
        case PDFOBJ_REFERENCE: {
                CPDF_Reference* pRef = (CPDF_Reference*)this;
                FX_DWORD obj_num = pRef->m_RefObjNum;
                if (bDirect && !visited->GetValueAt((void*)(FX_UINTPTR)obj_num)) {
                    visited->SetAt((void*)(FX_UINTPTR)obj_num, (void*)1);
                    CPDF_Object* ret;
                    if (pRef->GetDirect())
                        ret = pRef->GetDirect()->CloneInternal(TRUE, visited);
                    else
                        ret = NULL;
                    return ret;
                } else {
                    return new CPDF_Reference(pRef->m_pObjList, obj_num);
                }
            }
    }
    return NULL;
}
CPDF_Object* CPDF_Object::CloneRef(CPDF_IndirectObjects* pDoc) const
{
    if (m_ObjNum) {
        return new CPDF_Reference(pDoc, m_ObjNum);
    }
    return Clone();
}
CFX_WideString CPDF_Object::GetUnicodeText(CFX_CharMap* pCharMap) const
{
    if (m_Type == PDFOBJ_STRING) {
        return PDF_DecodeText(((CPDF_String*)this)->m_String, pCharMap);
    } else if (m_Type == PDFOBJ_STREAM) {
        CPDF_StreamAcc stream;
        stream.LoadAllData((CPDF_Stream*)this, FALSE);
        CFX_WideString result = PDF_DecodeText(stream.GetData(), stream.GetSize(), pCharMap);
        return result;
    } else if (m_Type == PDFOBJ_NAME) {
        return PDF_DecodeText(((CPDF_Name*)this)->m_Name, pCharMap);
    }
    return CFX_WideString();
}
void CPDF_Object::SetUnicodeText(FX_LPCWSTR pUnicodes, int len)
{
    if (m_Type == PDFOBJ_STRING) {
        ((CPDF_String*)this)->m_String = PDF_EncodeText(pUnicodes, len);
    } else if (m_Type == PDFOBJ_STREAM) {
        CFX_ByteString result = PDF_EncodeText(pUnicodes, len);
        ((CPDF_Stream*)this)->SetData((FX_LPBYTE)result.c_str(), result.GetLength(), FALSE, FALSE);
    }
}

CPDF_Number::CPDF_Number(int value)
    : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(TRUE), m_Integer(value) {
}

CPDF_Number::CPDF_Number(FX_FLOAT value)
    : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(FALSE), m_Float(value) {
}

CPDF_Number::CPDF_Number(FX_BOOL bInteger, void* pData)
    : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(bInteger), m_Integer(*(int*)pData) {
}

CPDF_Number::CPDF_Number(FX_BSTR str) : CPDF_Object(PDFOBJ_NUMBER) {
    FX_atonum(str, m_bInteger, &m_Integer);
}

void CPDF_Number::SetString(FX_BSTR str)
{
    FX_atonum(str, m_bInteger, &m_Integer);
}
FX_BOOL CPDF_Number::Identical(CPDF_Number* pOther) const
{
    return m_bInteger == pOther->m_bInteger && m_Integer == pOther->m_Integer;
}
CFX_ByteString CPDF_Number::GetString() const
{
    return m_bInteger ? CFX_ByteString::FormatInteger(m_Integer, FXFORMAT_SIGNED) : CFX_ByteString::FormatFloat(m_Float);
}
void CPDF_Number::SetNumber(FX_FLOAT value)
{
    m_bInteger = FALSE;
    m_Float = value;
}
CPDF_String::CPDF_String(const CFX_WideString& str) : CPDF_Object(PDFOBJ_STRING), m_bHex(FALSE) {
    m_String = PDF_EncodeText(str);
}
CPDF_Array::~CPDF_Array()
{
    int size = m_Objects.GetSize();
    CPDF_Object** pList = (CPDF_Object**)m_Objects.GetData();
    for (int i = 0; i < size; i ++) {
        if (pList[i])
            pList[i]->Release();
    }
}
CFX_FloatRect CPDF_Array::GetRect()
{
    CFX_FloatRect rect;
    if (m_Type != PDFOBJ_ARRAY || m_Objects.GetSize() != 4) {
        return rect;
    }
    rect.left = GetNumber(0);
    rect.bottom = GetNumber(1);
    rect.right = GetNumber(2);
    rect.top = GetNumber(3);
    return rect;
}
CFX_AffineMatrix CPDF_Array::GetMatrix()
{
    CFX_AffineMatrix matrix;
    if (m_Type != PDFOBJ_ARRAY || m_Objects.GetSize() != 6) {
        return matrix;
    }
    matrix.Set(GetNumber(0), GetNumber(1), GetNumber(2), GetNumber(3), GetNumber(4), GetNumber(5));
    return matrix;
}
CPDF_Object* CPDF_Array::GetElement(FX_DWORD i) const
{
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return NULL;
    }
    return (CPDF_Object*)m_Objects.GetAt(i);
}
CPDF_Object* CPDF_Array::GetElementValue(FX_DWORD i) const
{
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return NULL;
    }
    return ((CPDF_Object*)m_Objects.GetAt(i))->GetDirect();
}
CFX_ByteString CPDF_Array::GetString(FX_DWORD i) const
{
    if (i < (FX_DWORD)m_Objects.GetSize()) {
        CPDF_Object* p = (CPDF_Object*)m_Objects.GetAt(i);
        return p->GetString();
    }
    else
        return CFX_ByteString();
}
CFX_ByteStringC CPDF_Array::GetConstString(FX_DWORD i) const
{
    if (i < (FX_DWORD)m_Objects.GetSize()) {
        CPDF_Object* p = (CPDF_Object*)m_Objects.GetAt(i);
        return p->GetConstString();
    }
    else
        return CFX_ByteStringC();
}
int CPDF_Array::GetInteger(FX_DWORD i) const
{
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return 0;
    }
    CPDF_Object* p = (CPDF_Object*)m_Objects.GetAt(i);
    return p->GetInteger();
}
FX_FLOAT CPDF_Array::GetNumber(FX_DWORD i) const
{
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return 0;
    }
    CPDF_Object* p = (CPDF_Object*)m_Objects.GetAt(i);
    return p->GetNumber();
}
CPDF_Dictionary* CPDF_Array::GetDict(FX_DWORD i) const
{
    CPDF_Object* p = GetElementValue(i);
    if (p == NULL) {
        return NULL;
    } else if (p->GetType() == PDFOBJ_DICTIONARY) {
        return (CPDF_Dictionary*)p;
    } else if (p->GetType() == PDFOBJ_STREAM) {
        return ((CPDF_Stream*)p)->GetDict();
    }
    return NULL;
}
CPDF_Stream* CPDF_Array::GetStream(FX_DWORD i) const
{
    CPDF_Object* p = GetElementValue(i);
    if (p == NULL || p->GetType() != PDFOBJ_STREAM) {
        return NULL;
    }
    return (CPDF_Stream*)p;
}
CPDF_Array* CPDF_Array::GetArray(FX_DWORD i) const
{
    CPDF_Object* p = GetElementValue(i);
    if (p == NULL || p->GetType() != PDFOBJ_ARRAY) {
        return NULL;
    }
    return (CPDF_Array*)p;
}
void CPDF_Array::RemoveAt(FX_DWORD i)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return;
    }
    CPDF_Object* p = (CPDF_Object*)m_Objects.GetAt(i);
    if (p)
        p->Release();
    m_Objects.RemoveAt(i);
}
void CPDF_Array::SetAt(FX_DWORD i, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    ASSERT(i < (FX_DWORD)m_Objects.GetSize());
    if (i >= (FX_DWORD)m_Objects.GetSize()) {
        return;
    }
    CPDF_Object* pOld = (CPDF_Object*)m_Objects.GetAt(i);
    if (pOld)
        pOld->Release();
    if (pObj->GetObjNum()) {
        ASSERT(pObjs != NULL);
        pObj = CPDF_Reference::Create(pObjs, pObj->GetObjNum());
    }
    m_Objects.SetAt(i, pObj);
}
void CPDF_Array::InsertAt(FX_DWORD index, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs)
{
    ASSERT(pObj != NULL);
    if (pObj->GetObjNum()) {
        ASSERT(pObjs != NULL);
        pObj = CPDF_Reference::Create(pObjs, pObj->GetObjNum());
    }
    m_Objects.InsertAt(index, pObj);
}
void CPDF_Array::Add(CPDF_Object* pObj, CPDF_IndirectObjects* pObjs)
{
    ASSERT(pObj != NULL);
    if (pObj->GetObjNum()) {
        ASSERT(pObjs != NULL);
        pObj = CPDF_Reference::Create(pObjs, pObj->GetObjNum());
    }
    m_Objects.Add(pObj);
}
void CPDF_Array::AddName(const CFX_ByteString& str)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    Add(new CPDF_Name(str));
}
void CPDF_Array::AddString(const CFX_ByteString& str)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    Add(new CPDF_String(str));
}
void CPDF_Array::AddInteger(int i)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    Add(new CPDF_Number(i));
}
void CPDF_Array::AddNumber(FX_FLOAT f)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    CPDF_Number* pNumber = new CPDF_Number;
    pNumber->SetNumber(f);
    Add(pNumber);
}
void CPDF_Array::AddReference(CPDF_IndirectObjects* pDoc, FX_DWORD objnum)
{
    ASSERT(m_Type == PDFOBJ_ARRAY);
    Add(new CPDF_Reference(pDoc, objnum));
}
FX_BOOL CPDF_Array::Identical(CPDF_Array* pOther) const
{
    if (m_Objects.GetSize() != pOther->m_Objects.GetSize()) {
        return FALSE;
    }
    for (int i = 0; i < m_Objects.GetSize(); i ++)
        if (!((CPDF_Object*)m_Objects[i])->IsIdentical((CPDF_Object*)pOther->m_Objects[i])) {
            return FALSE;
        }
    return TRUE;
}
CPDF_Dictionary::~CPDF_Dictionary()
{
    FX_POSITION pos = m_Map.GetStartPosition();
    while (pos) {
        FX_LPVOID value = m_Map.GetNextValue(pos);
        if (value)
            ((CPDF_Object*)value)->Release();
    }
}
FX_POSITION CPDF_Dictionary::GetStartPos() const
{
    return m_Map.GetStartPosition();
}
CPDF_Object* CPDF_Dictionary::GetNextElement(FX_POSITION& pos, CFX_ByteString& key) const
{
    if (pos == NULL) {
        return NULL;
    }
    CPDF_Object* p;
    m_Map.GetNextAssoc(pos, key, (FX_LPVOID&)p);
    return p;
}
CPDF_Object* CPDF_Dictionary::GetElement(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    return p;
}
CPDF_Object* CPDF_Dictionary::GetElementValue(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    return p ? p->GetDirect() : NULL;
}
CFX_ByteString CPDF_Dictionary::GetString(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p)
        return p->GetString();
    else
        return CFX_ByteString();
}
CFX_ByteStringC CPDF_Dictionary::GetConstString(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p)
        return p->GetConstString();
    else
        return CFX_ByteStringC();
}
CFX_WideString CPDF_Dictionary::GetUnicodeText(FX_BSTR key, CFX_CharMap* pCharMap) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p) {
        if(p->GetType() == PDFOBJ_REFERENCE) {
            p = ((CPDF_Reference*)p)->GetDirect();
            if (p) {
                return p->GetUnicodeText(pCharMap);
            }
        } else {
            return p->GetUnicodeText(pCharMap);
        }
    }
    return CFX_WideString();
}
CFX_ByteString CPDF_Dictionary::GetString(FX_BSTR key, FX_BSTR def) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p) {
        return p->GetString();
    }
    return CFX_ByteString(def);
}
CFX_ByteStringC CPDF_Dictionary::GetConstString(FX_BSTR key, FX_BSTR def) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p)
        return p->GetConstString();
    else
        return CFX_ByteStringC(def);
}
int CPDF_Dictionary::GetInteger(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p) {
        return p->GetInteger();
    }
    return 0;
}
int CPDF_Dictionary::GetInteger(FX_BSTR key, int def) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p) {
        return p->GetInteger();
    }
    return def;
}
FX_FLOAT CPDF_Dictionary::GetNumber(FX_BSTR key) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p) {
        return p->GetNumber();
    }
    return 0;
}
FX_BOOL CPDF_Dictionary::GetBoolean(FX_BSTR key, FX_BOOL bDefault) const
{
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p && p->GetType() == PDFOBJ_BOOLEAN) {
        return p->GetInteger();
    }
    return bDefault;
}
CPDF_Dictionary* CPDF_Dictionary::GetDict(FX_BSTR key) const
{
    CPDF_Object* p = GetElementValue(key);
    if (p == NULL) {
        return NULL;
    } else if (p->GetType() == PDFOBJ_DICTIONARY) {
        return (CPDF_Dictionary*)p;
    } else if (p->GetType() == PDFOBJ_STREAM) {
        return ((CPDF_Stream*)p)->GetDict();
    }
    return NULL;
}
CPDF_Array* CPDF_Dictionary::GetArray(FX_BSTR key) const
{
    CPDF_Object* p = GetElementValue(key);
    if (p == NULL || p->GetType() != PDFOBJ_ARRAY) {
        return NULL;
    }
    return (CPDF_Array*)p;
}
CPDF_Stream* CPDF_Dictionary::GetStream(FX_BSTR key) const
{
    CPDF_Object* p = GetElementValue(key);
    if (p == NULL || p->GetType() != PDFOBJ_STREAM) {
        return NULL;
    }
    return (CPDF_Stream*)p;
}
CFX_FloatRect CPDF_Dictionary::GetRect(FX_BSTR key) const
{
    CFX_FloatRect rect;
    CPDF_Array* pArray = GetArray(key);
    if (pArray) {
        rect = pArray->GetRect();
    }
    return rect;
}
CFX_AffineMatrix CPDF_Dictionary::GetMatrix(FX_BSTR key) const
{
    CFX_AffineMatrix matrix;
    CPDF_Array* pArray = GetArray(key);
    if (pArray) {
        matrix = pArray->GetMatrix();
    }
    return matrix;
}
FX_BOOL CPDF_Dictionary::KeyExist(FX_BSTR key) const
{
    FX_LPVOID value;
    return m_Map.Lookup(key, value);
}
void CPDF_Dictionary::SetAt(FX_BSTR key, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs)
{
    ASSERT(m_Type == PDFOBJ_DICTIONARY);
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p == pObj) {
        return;
    }
    if (p)
        p->Release();
    if (pObj) {
        if (pObj->GetObjNum()) {
            ASSERT(pObjs != NULL);
            pObj = CPDF_Reference::Create(pObjs, pObj->GetObjNum());
        }
        m_Map.SetAt(key, pObj);
    } else {
        m_Map.RemoveKey(key);
    }
}
void CPDF_Dictionary::AddValue(FX_BSTR key, CPDF_Object* pObj)
{
    ASSERT(m_Type == PDFOBJ_DICTIONARY);
    m_Map.AddValue(key, pObj);
}
void CPDF_Dictionary::RemoveAt(FX_BSTR key)
{
    ASSERT(m_Type == PDFOBJ_DICTIONARY);
    CPDF_Object* p = NULL;
    m_Map.Lookup(key, (void*&)p);
    if (p == NULL) {
        return;
    }
    p->Release();
    m_Map.RemoveKey(key);
}
void CPDF_Dictionary::ReplaceKey(FX_BSTR oldkey, FX_BSTR newkey)
{
    ASSERT(m_Type == PDFOBJ_DICTIONARY);
    CPDF_Object* p = NULL;
    m_Map.Lookup(oldkey, (void*&)p);
    if (p == NULL) {
        return;
    }
    m_Map.RemoveKey(oldkey);
    m_Map.SetAt(newkey, p);
}
FX_BOOL CPDF_Dictionary::Identical(CPDF_Dictionary* pOther) const
{
    if (pOther == NULL) {
        return FALSE;
    }
    if (m_Map.GetCount() != pOther->m_Map.GetCount()) {
        return FALSE;
    }
    FX_POSITION pos = m_Map.GetStartPosition();
    while (pos) {
        CFX_ByteString key;
        FX_LPVOID value;
        m_Map.GetNextAssoc(pos, key, value);
        if (!value)
            return FALSE;
        if (!((CPDF_Object*)value)->IsIdentical(pOther->GetElement(key))) {
            return FALSE;
        }
    }
    return TRUE;
}
void CPDF_Dictionary::SetAtInteger(FX_BSTR key, int i)
{
    SetAt(key, new CPDF_Number(i));
}
void CPDF_Dictionary::SetAtName(FX_BSTR key, const CFX_ByteString& name)
{
    SetAt(key, new CPDF_Name(name));
}
void CPDF_Dictionary::SetAtString(FX_BSTR key, const CFX_ByteString& str)
{
    SetAt(key, new CPDF_String(str));
}
void CPDF_Dictionary::SetAtReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, FX_DWORD objnum)
{
    SetAt(key, new CPDF_Reference(pDoc, objnum));
}
void CPDF_Dictionary::AddReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, FX_DWORD objnum)
{
    AddValue(key, new CPDF_Reference(pDoc, objnum));
}
void CPDF_Dictionary::SetAtNumber(FX_BSTR key, FX_FLOAT f)
{
    CPDF_Number* pNumber = new CPDF_Number;
    pNumber->SetNumber(f);
    SetAt(key, pNumber);
}
void CPDF_Dictionary::SetAtBoolean(FX_BSTR key, FX_BOOL bValue)
{
    SetAt(key, new CPDF_Boolean(bValue));
}
void CPDF_Dictionary::SetAtRect(FX_BSTR key, const CFX_FloatRect& rect)
{
    CPDF_Array* pArray = new CPDF_Array;
    pArray->AddNumber(rect.left);
    pArray->AddNumber(rect.bottom);
    pArray->AddNumber(rect.right);
    pArray->AddNumber(rect.top);
    SetAt(key, pArray);
}
void CPDF_Dictionary::SetAtMatrix(FX_BSTR key, const CFX_AffineMatrix& matrix)
{
    CPDF_Array* pArray = new CPDF_Array;
    pArray->AddNumber16(matrix.a);
    pArray->AddNumber16(matrix.b);
    pArray->AddNumber16(matrix.c);
    pArray->AddNumber16(matrix.d);
    pArray->AddNumber(matrix.e);
    pArray->AddNumber(matrix.f);
    SetAt(key, pArray);
}
CPDF_Stream::CPDF_Stream(FX_LPBYTE pData, FX_DWORD size, CPDF_Dictionary* pDict)
    : CPDF_Object(PDFOBJ_STREAM) {
    m_pDict = pDict;
    m_dwSize = size;
    m_GenNum = (FX_DWORD) - 1;
    m_pDataBuf = pData;
    m_pCryptoHandler = NULL;
}
CPDF_Stream::~CPDF_Stream()
{
    if (m_GenNum == (FX_DWORD) - 1 && m_pDataBuf != NULL) {
        FX_Free(m_pDataBuf);
    }
    if (m_pDict) {
        m_pDict->Release();
    }
}
void CPDF_Stream::InitStream(CPDF_Dictionary* pDict)
{
    if (pDict) {
        if (m_pDict) {
            m_pDict->Release();
        }
        m_pDict = pDict;
    }
    if (m_GenNum == (FX_DWORD) - 1) {
        if (m_pDataBuf) {
            FX_Free(m_pDataBuf);
        }
    }
    m_GenNum = 0;
    m_pFile = NULL;
    m_pCryptoHandler = NULL;
    m_FileOffset = 0;
}
void CPDF_Stream::InitStream(FX_LPBYTE pData, FX_DWORD size, CPDF_Dictionary* pDict)
{
    InitStream(pDict);
    m_GenNum = (FX_DWORD) - 1;
    m_pDataBuf = FX_Alloc(FX_BYTE, size);
    if (pData) {
        FXSYS_memcpy32(m_pDataBuf, pData, size);
    }
    m_dwSize = size;
    if (m_pDict) {
        m_pDict->SetAtInteger(FX_BSTRC("Length"), size);
    }
}
void CPDF_Stream::SetData(FX_LPCBYTE pData, FX_DWORD size, FX_BOOL bCompressed, FX_BOOL bKeepBuf)
{
    if (m_GenNum == (FX_DWORD) - 1) {
        if (m_pDataBuf) {
            FX_Free(m_pDataBuf);
        }
    } else {
        m_GenNum = (FX_DWORD) - 1;
        m_pCryptoHandler = NULL;
    }
    if (bKeepBuf) {
        m_pDataBuf = (FX_LPBYTE)pData;
    } else {
        m_pDataBuf = FX_Alloc(FX_BYTE, size);
        if (pData) {
            FXSYS_memcpy32(m_pDataBuf, pData, size);
        }
    }
    m_dwSize = size;
    if (m_pDict == NULL) {
        m_pDict = new CPDF_Dictionary;
    }
    m_pDict->SetAtInteger(FX_BSTRC("Length"), size);
    if (!bCompressed) {
        m_pDict->RemoveAt(FX_BSTRC("Filter"));
        m_pDict->RemoveAt(FX_BSTRC("DecodeParms"));
    }
}
FX_BOOL CPDF_Stream::ReadRawData(FX_FILESIZE offset, FX_LPBYTE buf, FX_DWORD size) const
{
    if ((m_GenNum != (FX_DWORD) - 1) && m_pFile) {
        return m_pFile->ReadBlock(buf, m_FileOffset + offset, size);
    }
    if (m_pDataBuf) {
        FXSYS_memcpy32(buf, m_pDataBuf + offset, size);
    }
    return TRUE;
}
void CPDF_Stream::InitStream(IFX_FileRead *pFile, CPDF_Dictionary* pDict)
{
    InitStream(pDict);
    m_pFile = pFile;
    m_dwSize = (FX_DWORD)pFile->GetSize();
    if (m_pDict) {
        m_pDict->SetAtInteger(FX_BSTRC("Length"), m_dwSize);
    }
}
FX_BOOL CPDF_Stream::Identical(CPDF_Stream* pOther) const
{
    if (!m_pDict)
        return pOther->m_pDict ? FALSE : TRUE;

    if (!m_pDict->Identical(pOther->m_pDict)) {
        return FALSE;
    }
    if (m_dwSize != pOther->m_dwSize) {
        return FALSE;
    }
    if (m_GenNum != (FX_DWORD) - 1 && pOther->m_GenNum != (FX_DWORD) - 1) {
        if (m_pFile == pOther->m_pFile && m_pFile == NULL) {
            return TRUE;
        }
        if (!m_pFile || !pOther->m_pFile) {
            return FALSE;
        }
        FX_BYTE srcBuf[1024];
        FX_BYTE destBuf[1024];
        FX_DWORD size = m_dwSize;
        FX_DWORD srcOffset = m_FileOffset;
        FX_DWORD destOffset = pOther->m_FileOffset;
        if (m_pFile == pOther->m_pFile && srcOffset == destOffset) {
            return TRUE;
        }
        while (size > 0) {
            FX_DWORD actualSize = size > 1024 ? 1024 : size;
            m_pFile->ReadBlock(srcBuf, srcOffset, actualSize);
            pOther->m_pFile->ReadBlock(destBuf, destOffset, actualSize);
            if (FXSYS_memcmp32(srcBuf, destBuf, actualSize) != 0) {
                return FALSE;
            }
            size -= actualSize;
            srcOffset += actualSize;
            destOffset += actualSize;
        }
        return TRUE;
    }
    if (m_GenNum != (FX_DWORD) - 1 || pOther->m_GenNum != (FX_DWORD) - 1) {
        IFX_FileRead* pFile = NULL;
        FX_LPBYTE pBuf = NULL;
        FX_DWORD offset = 0;
        if (pOther->m_GenNum != (FX_DWORD) - 1) {
            pFile = pOther->m_pFile;
            pBuf = m_pDataBuf;
            offset = pOther->m_FileOffset;
        } else if (m_GenNum != (FX_DWORD) - 1) {
            pFile = m_pFile;
            pBuf = pOther->m_pDataBuf;
            offset = m_FileOffset;
        }
        if (NULL == pBuf) {
            return FALSE;
        }
        FX_BYTE srcBuf[1024];
        FX_DWORD size = m_dwSize;
        while (size > 0) {
            FX_DWORD actualSize = std::min(size, 1024U);
            pFile->ReadBlock(srcBuf, offset, actualSize);
            if (FXSYS_memcmp32(srcBuf, pBuf, actualSize) != 0) {
                return FALSE;
            }
            pBuf += actualSize;
            size -= actualSize;
            offset += actualSize;
        }
        return TRUE;
    }
    return FXSYS_memcmp32(m_pDataBuf, pOther->m_pDataBuf, m_dwSize) == 0;
}
CPDF_Stream* CPDF_Stream::Clone(FX_BOOL bDirect, FPDF_LPFCloneStreamCallback lpfCallback, FX_LPVOID pUserData) const
{
    CPDF_Dictionary *pCloneDict = (CPDF_Dictionary*)m_pDict->Clone(bDirect);
    IFX_FileStream *pFS = NULL;
    if (lpfCallback) {
        pFS = lpfCallback((CPDF_Stream*)this, pUserData);
    }
    if (!pFS) {
        CPDF_StreamAcc acc;
        acc.LoadAllData(this, TRUE);
        FX_DWORD streamSize = acc.GetSize();
        return new CPDF_Stream(acc.DetachData(), streamSize, pCloneDict);
    }
    CPDF_Stream* pObj = new CPDF_Stream(NULL, 0, NULL);
    CPDF_StreamFilter *pSF = GetStreamFilter(TRUE);
    if (pSF) {
        FX_LPBYTE pBuf = FX_Alloc(FX_BYTE, 4096);
        FX_DWORD dwRead;
        do {
            dwRead = pSF->ReadBlock(pBuf, 4096);
            if (dwRead) {
                pFS->WriteBlock(pBuf, dwRead);
            }
        } while (dwRead == 4096);
        pFS->Flush();
        FX_Free(pBuf);
        delete pSF;
    }
    pObj->InitStream((IFX_FileRead*)pFS, pCloneDict);
    return pObj;
}
extern FX_BOOL PDF_DataDecode(FX_LPCBYTE src_buf, FX_DWORD src_size, const CPDF_Dictionary* pDict,
                              FX_LPBYTE& dest_buf, FX_DWORD& dest_size, CFX_ByteString& ImageEncoding,
                              CPDF_Dictionary*& pImageParms, FX_DWORD estimated_size, FX_BOOL bImageAcc);
CPDF_StreamAcc::CPDF_StreamAcc()
{
    m_bNewBuf = FALSE;
    m_pData = NULL;
    m_dwSize = 0;
    m_pImageParam = NULL;
    m_pStream = NULL;
    m_pSrcData = NULL;
}
void CPDF_StreamAcc::LoadAllData(const CPDF_Stream* pStream, FX_BOOL bRawAccess, FX_DWORD estimated_size,
                                 FX_BOOL bImageAcc)
{
    if (pStream == NULL || pStream->GetType() != PDFOBJ_STREAM) {
        return;
    }
    m_pStream = pStream;
    if (pStream->IsMemoryBased() &&
            (!pStream->GetDict()->KeyExist(FX_BSTRC("Filter")) || bRawAccess)) {
        m_dwSize = pStream->m_dwSize;
        m_pData = (FX_LPBYTE)pStream->m_pDataBuf;
        return;
    }
    FX_LPBYTE pSrcData;
    FX_DWORD dwSrcSize = pStream->m_dwSize;
    if (dwSrcSize == 0) {
        return;
    }
    if (!pStream->IsMemoryBased()) {
        pSrcData = m_pSrcData = FX_Alloc(FX_BYTE, dwSrcSize);
        if (!pStream->ReadRawData(0, pSrcData, dwSrcSize)) {
            return;
        }
    } else {
        pSrcData = pStream->m_pDataBuf;
    }
    FX_LPBYTE pDecryptedData;
    FX_DWORD dwDecryptedSize;
    if (pStream->m_pCryptoHandler) {
        CFX_BinaryBuf dest_buf;
        dest_buf.EstimateSize(pStream->m_pCryptoHandler->DecryptGetSize(dwSrcSize));
        FX_LPVOID context = pStream->m_pCryptoHandler->DecryptStart(pStream->GetObjNum(), pStream->m_GenNum);
        pStream->m_pCryptoHandler->DecryptStream(context, pSrcData, dwSrcSize, dest_buf);
        pStream->m_pCryptoHandler->DecryptFinish(context, dest_buf);
        pDecryptedData = dest_buf.GetBuffer();
        dwDecryptedSize = dest_buf.GetSize();
        dest_buf.DetachBuffer();
    } else {
        pDecryptedData = pSrcData;
        dwDecryptedSize = dwSrcSize;
    }
    if (!pStream->GetDict()->KeyExist(FX_BSTRC("Filter")) || bRawAccess) {
        m_pData = pDecryptedData;
        m_dwSize = dwDecryptedSize;
    } else {
        FX_BOOL bRet = PDF_DataDecode(pDecryptedData, dwDecryptedSize, m_pStream->GetDict(),
                                      m_pData, m_dwSize, m_ImageDecoder, m_pImageParam, estimated_size, bImageAcc);
        if (!bRet) {
            m_pData = pDecryptedData;
            m_dwSize = dwDecryptedSize;
        }
    }
    if (pSrcData != pStream->m_pDataBuf && pSrcData != m_pData) {
        FX_Free(pSrcData);
    }
    if (pDecryptedData != pSrcData && pDecryptedData != m_pData) {
        FX_Free(pDecryptedData);
    }
    m_pSrcData = NULL;
    m_bNewBuf = m_pData != pStream->m_pDataBuf;
}
CPDF_StreamAcc::~CPDF_StreamAcc()
{
    if (m_bNewBuf && m_pData) {
        FX_Free(m_pData);
    }
    if (m_pSrcData) {
        FX_Free(m_pSrcData);
    }
}
FX_LPCBYTE CPDF_StreamAcc::GetData() const
{
    if (m_bNewBuf) {
        return m_pData;
    }
    if (!m_pStream) {
        return NULL;
    }
    return m_pStream->m_pDataBuf;
}
FX_DWORD CPDF_StreamAcc::GetSize() const
{
    if (m_bNewBuf) {
        return m_dwSize;
    }
    if (!m_pStream) {
        return 0;
    }
    return m_pStream->m_dwSize;
}
FX_LPBYTE CPDF_StreamAcc::DetachData()
{
    if (m_bNewBuf) {
        FX_LPBYTE p = m_pData;
        m_pData = NULL;
        m_dwSize = 0;
        return p;
    }
    FX_LPBYTE p = FX_Alloc(FX_BYTE, m_dwSize);
    FXSYS_memcpy32(p, m_pData, m_dwSize);
    return p;
}
void CPDF_Reference::SetRef(CPDF_IndirectObjects* pDoc, FX_DWORD objnum)
{
    m_pObjList = pDoc;
    m_RefObjNum = objnum;
}
CPDF_IndirectObjects::CPDF_IndirectObjects(CPDF_Parser* pParser)
{
    m_pParser = pParser;
    m_IndirectObjs.InitHashTable(1013);
    if (pParser) {
        m_LastObjNum = m_pParser->GetLastObjNum();
    } else {
        m_LastObjNum = 0;
    }
}
CPDF_IndirectObjects::~CPDF_IndirectObjects()
{
    FX_POSITION pos = m_IndirectObjs.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_IndirectObjs.GetNextAssoc(pos, key, value);
        ((CPDF_Object*)value)->Destroy();
    }
}
CPDF_Object* CPDF_IndirectObjects::GetIndirectObject(FX_DWORD objnum, struct PARSE_CONTEXT* pContext)
{
    if (objnum == 0) {
        return NULL;
    }
    FX_LPVOID value;
    {
        if (m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, value)) {
            if (((CPDF_Object*)value)->GetObjNum() == -1) {
                return NULL;
            }
            return (CPDF_Object*)value;
        }
    }
    CPDF_Object* pObj = NULL;
    if (m_pParser) {
        pObj = m_pParser->ParseIndirectObject(this, objnum, pContext);
    }
    if (pObj == NULL) {
        return NULL;
    }
    pObj->m_ObjNum = objnum;
    if (m_LastObjNum < objnum) {
        m_LastObjNum = objnum;
    }
    if (m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, value)) {
        if (value) {
            ((CPDF_Object *)value)->Destroy();
        }
    }
    m_IndirectObjs.SetAt((FX_LPVOID)(FX_UINTPTR)objnum, pObj);
    return pObj;
}
int CPDF_IndirectObjects::GetIndirectType(FX_DWORD objnum)
{
    FX_LPVOID value;
    if (m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, value)) {
        return ((CPDF_Object*)value)->GetType();
    }
    if (m_pParser) {
        PARSE_CONTEXT context;
        FXSYS_memset32(&context, 0, sizeof(PARSE_CONTEXT));
        context.m_Flags = PDFPARSE_TYPEONLY;
        return (int)(FX_UINTPTR)m_pParser->ParseIndirectObject(this, objnum, &context);
    }
    return 0;
}
FX_DWORD CPDF_IndirectObjects::AddIndirectObject(CPDF_Object* pObj)
{
    if (pObj->m_ObjNum) {
        return pObj->m_ObjNum;
    }
    m_LastObjNum ++;
    m_IndirectObjs.SetAt((FX_LPVOID)(FX_UINTPTR)m_LastObjNum, pObj);
    pObj->m_ObjNum = m_LastObjNum;
    return m_LastObjNum;
}
void CPDF_IndirectObjects::ReleaseIndirectObject(FX_DWORD objnum)
{
    FX_LPVOID value;
    if (!m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, value)) {
        return;
    }
    if (((CPDF_Object*)value)->GetObjNum() == -1) {
        return;
    }
    ((CPDF_Object*)value)->Destroy();
    m_IndirectObjs.RemoveKey((FX_LPVOID)(FX_UINTPTR)objnum);
}
void CPDF_IndirectObjects::InsertIndirectObject(FX_DWORD objnum, CPDF_Object* pObj)
{
    if (objnum == 0 || pObj == NULL) {
        return;
    }
    FX_LPVOID value = NULL;
    if (m_IndirectObjs.Lookup((FX_LPVOID)(FX_UINTPTR)objnum, value)) {
        if (value)
        {
            if (pObj->GetGenNum() <= ((CPDF_Object*)value)->GetGenNum())
                return;
            else
                ((CPDF_Object*)value)->Destroy();
         }
    }
    pObj->m_ObjNum = objnum;
    m_IndirectObjs.SetAt((FX_LPVOID)(FX_UINTPTR)objnum, pObj);
    if (m_LastObjNum < objnum) {
        m_LastObjNum = objnum;
    }
}
FX_DWORD CPDF_IndirectObjects::GetLastObjNum() const
{
    return m_LastObjNum;
}
