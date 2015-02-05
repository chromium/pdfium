// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_serial.h"
CFDF_Document::CFDF_Document() : CPDF_IndirectObjects(NULL)
{
    m_pRootDict = NULL;
    m_pFile = NULL;
    m_bOwnFile = FALSE;
}
CFDF_Document::~CFDF_Document()
{
    if (m_bOwnFile && m_pFile) {
        m_pFile->Release();
    }
}
CFDF_Document* CFDF_Document::CreateNewDoc()
{
    CFDF_Document* pDoc = FX_NEW CFDF_Document;
    pDoc->m_pRootDict = FX_NEW CPDF_Dictionary;
    pDoc->AddIndirectObject(pDoc->m_pRootDict);
    CPDF_Dictionary* pFDFDict = FX_NEW CPDF_Dictionary;
    pDoc->m_pRootDict->SetAt(FX_BSTRC("FDF"), pFDFDict);
    return pDoc;
}
CFDF_Document* CFDF_Document::ParseFile(IFX_FileRead *pFile, FX_BOOL bOwnFile)
{
  if (!pFile) {
    return NULL;
  }
  CFDF_Document* pDoc = FX_NEW CFDF_Document;
  pDoc->ParseStream(pFile, bOwnFile);
  if (pDoc->m_pRootDict == NULL) {
    delete pDoc;
    return NULL;
  }
  return pDoc;
}
CFDF_Document* CFDF_Document::ParseMemory(FX_LPCBYTE pData, FX_DWORD size)
{
    return CFDF_Document::ParseFile(FX_CreateMemoryStream((FX_LPBYTE)pData, size), TRUE);
}
void CFDF_Document::ParseStream(IFX_FileRead *pFile, FX_BOOL bOwnFile)
{
    m_pFile = pFile;
    m_bOwnFile = bOwnFile;
    CPDF_SyntaxParser parser;
    parser.InitParser(m_pFile, 0);
    while (1) {
        FX_BOOL bNumber;
        CFX_ByteString word = parser.GetNextWord(bNumber);
        if (bNumber) {
            FX_DWORD objnum = FXSYS_atoi(word);
            word = parser.GetNextWord(bNumber);
            if (!bNumber) {
                break;
            }
            word = parser.GetNextWord(bNumber);
            if (word != FX_BSTRC("obj")) {
                break;
            }
            CPDF_Object* pObj = parser.GetObject(this, objnum, 0, FALSE);
            if (pObj == NULL) {
                break;
            }
            InsertIndirectObject(objnum, pObj);
            word = parser.GetNextWord(bNumber);
            if (word != FX_BSTRC("endobj")) {
                break;
            }
        } else {
            if (word != FX_BSTRC("trailer")) {
                break;
            }
            CPDF_Dictionary* pMainDict = (CPDF_Dictionary*)parser.GetObject(this, 0, 0, 0);
            if (pMainDict == NULL || pMainDict->GetType() != PDFOBJ_DICTIONARY) {
                break;
            }
            m_pRootDict = pMainDict->GetDict(FX_BSTRC("Root"));
            pMainDict->Release();
            break;
        }
    }
}
FX_BOOL CFDF_Document::WriteBuf(CFX_ByteTextBuf& buf) const
{
    if (m_pRootDict == NULL) {
        return FALSE;
    }
    buf << FX_BSTRC("%FDF-1.2\r\n");
    FX_POSITION pos = m_IndirectObjs.GetStartPosition();
    while(pos) {
        size_t objnum;
        CPDF_Object* pObj;
        m_IndirectObjs.GetNextAssoc(pos, (FX_LPVOID&)objnum, (FX_LPVOID&)pObj);
        buf << (FX_DWORD)objnum << FX_BSTRC(" 0 obj\r\n") << pObj << FX_BSTRC("\r\nendobj\r\n\r\n");
    }
    buf << FX_BSTRC("trailer\r\n<</Root ") << m_pRootDict->GetObjNum() << FX_BSTRC(" 0 R>>\r\n%%EOF\r\n");
    return TRUE;
}
CFX_WideString CFDF_Document::GetWin32Path() const
{
    CPDF_Dictionary* pDict = m_pRootDict ? m_pRootDict->GetDict(FX_BSTRC("FDF")) : NULL;
    CPDF_Object* pFileSpec = pDict ? pDict->GetElementValue(FX_BSTRC("F")) : NULL;
    if (pFileSpec == NULL) {
        return CFX_WideString();
    }
    if (pFileSpec->GetType() == PDFOBJ_STRING) {
        return FPDF_FileSpec_GetWin32Path(m_pRootDict->GetDict(FX_BSTRC("FDF")));
    }
    return FPDF_FileSpec_GetWin32Path(pFileSpec);
}
static CFX_WideString ChangeSlash(FX_LPCWSTR str)
{
    CFX_WideString result;
    while (*str) {
        if (*str == '\\') {
            result += '/';
        } else if (*str == '/') {
            result += '\\';
        } else {
            result += *str;
        }
        str ++;
    }
    return result;
}
void FPDF_FileSpec_SetWin32Path(CPDF_Object* pFileSpec, const CFX_WideString& filepath)
{
    CFX_WideString result;
    if (filepath.GetLength() > 1 && filepath[1] == ':') {
        result = L"/";
        result += filepath[0];
        if (filepath[2] != '\\') {
            result += '/';
        }
        result += ChangeSlash(filepath.c_str() + 2);
    } else if (filepath.GetLength() > 1 && filepath[0] == '\\' && filepath[1] == '\\') {
        result = ChangeSlash(filepath.c_str() + 1);
    } else {
        result = ChangeSlash(filepath);
    }
    if (pFileSpec->GetType() == PDFOBJ_STRING) {
        pFileSpec->SetString(CFX_ByteString::FromUnicode(result));
    } else if (pFileSpec->GetType() == PDFOBJ_DICTIONARY) {
        ((CPDF_Dictionary*)pFileSpec)->SetAtString(FX_BSTRC("F"), CFX_ByteString::FromUnicode(result));
        ((CPDF_Dictionary*)pFileSpec)->SetAtString(FX_BSTRC("UF"), PDF_EncodeText(result));
        ((CPDF_Dictionary*)pFileSpec)->RemoveAt(FX_BSTRC("FS"));
    }
}
CFX_WideString	FPDF_FileSpec_GetWin32Path(const CPDF_Object* pFileSpec)
{
    CFX_WideString wsFileName;
    if (pFileSpec->GetType() == PDFOBJ_DICTIONARY) {
        CPDF_Dictionary* pDict = (CPDF_Dictionary*)pFileSpec;
        wsFileName = pDict->GetUnicodeText(FX_BSTRC("UF"));
        if (wsFileName.IsEmpty()) {
            wsFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("F")));
        }
        if (pDict->GetString(FX_BSTRC("FS")) == FX_BSTRC("URL")) {
            return wsFileName;
        }
        if (wsFileName.IsEmpty() && pDict->KeyExist(FX_BSTRC("DOS"))) {
            wsFileName = CFX_WideString::FromLocal(pDict->GetString(FX_BSTRC("DOS")));
        }
    }
    else if (!pFileSpec)
        wsFileName = CFX_WideString();
    else {
        wsFileName = CFX_WideString::FromLocal(pFileSpec->GetString());
    }
    if (wsFileName[0] != '/') {
        return ChangeSlash(wsFileName);
    }
    if (wsFileName[2] == '/') {
        CFX_WideString result;
        result += wsFileName[1];
        result += ':';
        result += ChangeSlash(wsFileName.c_str() + 2);
        return result;
    } else {
        CFX_WideString result;
        result += '\\';
        result += ChangeSlash(wsFileName);
        return result;
    }
}
