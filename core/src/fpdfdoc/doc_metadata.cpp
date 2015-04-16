// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fxcrt/fx_xml.h"
typedef struct _PDFDOC_METADATA {
    CPDF_Document *m_pDoc;
    CXML_Element *m_pXmlElmnt;
    CXML_Element *m_pElmntRdf;
    CFX_CMapByteStringToPtr *m_pStringMap;
} PDFDOC_METADATA, * PDFDOC_LPMETADATA;
typedef PDFDOC_METADATA const * PDFDOC_LPCMETADATA;
const FX_LPCSTR gs_FPDFDOC_Metadata_Titles[] = {
    "Title", "title",
    "Subject", "description",
    "Author", "creator",
    "Keywords", "Keywords",
    "Producer", "Producer",
    "Creator", "CreatorTool",
    "CreationDate", "CreateDate",
    "ModDate", "ModifyDate",
    "MetadataDate", "MetadataDate"
};
CPDF_Metadata::CPDF_Metadata()
{
    m_pData = FX_Alloc(PDFDOC_METADATA, 1);
    CFX_CMapByteStringToPtr *&pStringMap = ((PDFDOC_LPMETADATA)m_pData)->m_pStringMap;
    pStringMap = new CFX_CMapByteStringToPtr;
    CFX_ByteString bstr;
    for (int i = 0; i < 18; i += 2) {
        bstr = gs_FPDFDOC_Metadata_Titles[i];
        pStringMap->AddValue(bstr, (void*)gs_FPDFDOC_Metadata_Titles[i + 1]);
    }

}
CPDF_Metadata::~CPDF_Metadata()
{
    FXSYS_assert(m_pData != NULL);
    CXML_Element *&p = ((PDFDOC_LPMETADATA)m_pData)->m_pXmlElmnt;
    delete p;
    CFX_CMapByteStringToPtr *pStringMap = ((PDFDOC_LPMETADATA)m_pData)->m_pStringMap;
    if (pStringMap) {
        pStringMap->RemoveAll();
        delete pStringMap;
    }
    FX_Free(m_pData);
}
void CPDF_Metadata::LoadDoc(CPDF_Document *pDoc)
{
    FXSYS_assert(pDoc != NULL);
    ((PDFDOC_LPMETADATA)m_pData)->m_pDoc = pDoc;
    CPDF_Dictionary *pRoot = pDoc->GetRoot();
    CPDF_Stream *pStream = pRoot->GetStream(FX_BSTRC("Metadata"));
    if (!pStream) {
        return;
    }
    CPDF_StreamAcc acc;
    acc.LoadAllData(pStream, FALSE);
    int size = acc.GetSize();
    FX_LPCBYTE pBuf = acc.GetData();
    CXML_Element *&pXmlElmnt = ((PDFDOC_LPMETADATA)m_pData)->m_pXmlElmnt;
    pXmlElmnt = CXML_Element::Parse(pBuf, size);
    if (!pXmlElmnt) {
        return;
    }
    CXML_Element *&pElmntRdf = ((PDFDOC_LPMETADATA)m_pData)->m_pElmntRdf;
    if (pXmlElmnt->GetTagName() == FX_BSTRC("RDF")) {
        pElmntRdf = pXmlElmnt;
    } else {
        pElmntRdf = pXmlElmnt->GetElement(NULL, FX_BSTRC("RDF"));
    }
}
FX_INT32 CPDF_Metadata::GetString(FX_BSTR bsItem, CFX_WideString &wsStr)
{
    if (!((PDFDOC_LPMETADATA)m_pData)->m_pXmlElmnt) {
        return -1;
    }
    if (!((PDFDOC_LPMETADATA)m_pData)->m_pStringMap) {
        return -1;
    }
    void *szTag;
    if (!((PDFDOC_LPMETADATA)m_pData)->m_pStringMap->Lookup(bsItem, szTag)) {
        return -1;
    }
    CFX_ByteString bsTag = (FX_LPCSTR)szTag;
    wsStr = L"";
    CXML_Element *pElmntRdf = ((PDFDOC_LPMETADATA)m_pData)->m_pElmntRdf;
    if (!pElmntRdf) {
        return -1;
    }
    int nChild = pElmntRdf->CountChildren();
    for (int i = 0; i < nChild; i++) {
        CXML_Element *pTag = pElmntRdf->GetElement(NULL, FX_BSTRC("Description"), i);
        if (!pTag) {
            continue;
        }
        if (bsItem == FX_BSTRC("Title") || bsItem == FX_BSTRC("Subject")) {
            CXML_Element *pElmnt = pTag->GetElement(NULL, bsTag);
            if (!pElmnt) {
                continue;
            }
            pElmnt = pElmnt->GetElement(NULL, FX_BSTRC("Alt"));
            if (!pElmnt) {
                continue;
            }
            pElmnt = pElmnt->GetElement(NULL, FX_BSTRC("li"));
            if (!pElmnt) {
                continue;
            }
            wsStr = pElmnt->GetContent(0);
            return wsStr.GetLength();
        } else if (bsItem == FX_BSTRC("Author")) {
            CXML_Element *pElmnt = pTag->GetElement(NULL, bsTag);
            if (!pElmnt) {
                continue;
            }
            pElmnt = pElmnt->GetElement(NULL, FX_BSTRC("Seq"));
            if (!pElmnt) {
                continue;
            }
            pElmnt = pElmnt->GetElement(NULL, FX_BSTRC("li"));
            if (!pElmnt) {
                continue;
            }
            wsStr = pElmnt->GetContent(0);
            return wsStr.GetLength();
        } else {
            CXML_Element *pElmnt = pTag->GetElement(NULL, bsTag);
            if (!pElmnt) {
                continue;
            }
            wsStr = pElmnt->GetContent(0);
            return wsStr.GetLength();
        }
    }
    return -1;
}
CXML_Element* CPDF_Metadata::GetRoot() const
{
    return ((PDFDOC_LPMETADATA)m_pData)->m_pXmlElmnt;
}
CXML_Element* CPDF_Metadata::GetRDF() const
{
    return ((PDFDOC_LPMETADATA)m_pData)->m_pElmntRdf;
}
