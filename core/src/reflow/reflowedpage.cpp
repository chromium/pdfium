// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/reflow/reflowengine.h"
#include "reflowedpage.h"
CPDF_ProgressiveReflowPageParser::CPDF_ProgressiveReflowPageParser()
{
    m_nObjProcessed = 0;
    m_pReflowEngine = NULL;
    m_pProvider = NULL;
}
CPDF_ProgressiveReflowPageParser::~CPDF_ProgressiveReflowPageParser()
{
    if(m_pProvider) {
        delete m_pProvider;
    }
    m_pProvider = NULL;
    if(m_pReflowEngine) {
        delete m_pReflowEngine;
    }
    m_pReflowEngine = NULL;
}
void CPDF_ProgressiveReflowPageParser::Init()
{
    m_Status = Ready;
}
CPDF_ReflowedPage::CPDF_ReflowedPage(CFX_GrowOnlyPool*	pMemoryPool)
{
    m_PageWidth = 0;
    m_PageHeight = 0;
    m_bWaiting = TRUE;
    if(pMemoryPool) {
        m_pMemoryPool = pMemoryPool;
        m_bCreateMemoryPool = FALSE;
    } else {
        m_pMemoryPool = new CFX_GrowOnlyPool;
        m_bCreateMemoryPool = TRUE;
    }
    m_pCharState = new CRF_CharStateArray(10);
    m_pReflowed = new CRF_DataPtrArray(500);
    m_pPageInfos = NULL;
}
CPDF_ReflowedPage::~CPDF_ReflowedPage()
{
    if (m_pReflowed) {
        for(int i = 0; i < m_pReflowed->GetSize(); i++) {
            CRF_Data* pData = (*m_pReflowed)[i];
            if(pData->m_Type == CRF_Data::Image) {
                delete ((CRF_ImageData*)pData)->m_pBitmap;
            }
        }
        m_pReflowed->RemoveAll();
        delete m_pReflowed;
    }
    m_pReflowed = NULL;
    if (m_pCharState) {
        m_pCharState->RemoveAll();
        delete m_pCharState;
    }
    m_pCharState = NULL;
    if(m_bCreateMemoryPool && m_pMemoryPool) {
        m_pMemoryPool->FreeAll();
    }
    if (m_pMemoryPool) {
        delete m_pMemoryPool;
    }
    m_pMemoryPool = NULL;
    m_pPDFPage = NULL;
    if (m_pPageInfos) {
        ReleasePageObjsMemberShip();
    }
}
FX_BOOL CPDF_ReflowedPage::RetainPageObjsMemberShip()
{
    if (NULL == m_pPDFPage) {
        return FALSE;
    }
    if (NULL == m_pPageInfos) {
        m_pPageInfos = new CFX_MapPtrToPtr();
    } else {
        return TRUE;
    }
    FX_POSITION	pos = m_pPDFPage->GetFirstObjectPosition();
    if (!pos)	{
        return FALSE;
    }
    CPDF_PageObject* pPageObj = NULL;
    while (pos) {
        pPageObj = m_pPDFPage->GetNextObject(pos);
        MarkPageObjMemberShip(pPageObj, NULL);
        pPageObj = NULL;
    }
    return TRUE;
}
void CPDF_ReflowedPage::MarkPageObjMemberShip(CPDF_PageObject* pObj, CRF_PageInfo* pParent)
{
    if (NULL == m_pPageInfos) {
        return;
    }
    CRF_PageInfo* pPageInfo = new CRF_PageInfo(pObj, pParent);
    m_pPageInfos->SetAt(pObj, pPageInfo);
    if (PDFPAGE_FORM != pObj->m_Type) {
        return;
    }
    CPDF_FormObject* pFormObj = (CPDF_FormObject*)pObj;
    FX_POSITION	pos;
    pos = pFormObj->m_pForm->GetFirstObjectPosition();
    if (!pos)	{
        return;
    }
    CPDF_PageObject* pPageObj = NULL;
    while (pos) {
        pPageObj = pFormObj->m_pForm->GetNextObject(pos);
        MarkPageObjMemberShip(pPageObj, pPageInfo);
        pPageObj = NULL;
    }
}
void CPDF_ReflowedPage::ReleasePageObjsMemberShip()
{
    if (NULL == m_pPageInfos) {
        return;
    }
    CPDF_PageObject* pPageObj = NULL;
    CRF_PageInfo* pPageInfo = NULL;
    FX_POSITION pos = m_pPageInfos->GetStartPosition();
    while (pos) {
        m_pPageInfos->GetNextAssoc(pos, (void*&)pPageObj, (void*&)pPageInfo);
        delete pPageInfo;
    }
    m_pPageInfos->RemoveAll();
    delete m_pPageInfos;
    m_pPageInfos = NULL;
}
CPDF_Dictionary* CPDF_ReflowedPage::GetFormResDict(CPDF_PageObject* pObj)
{
    if (NULL == m_pPageInfos) {
        return NULL;
    }
    if (FALSE == RetainPageObjsMemberShip()) {
        return NULL;
    }
    CRF_PageInfo* pPageInfo = (CRF_PageInfo*)m_pPageInfos->GetValueAt(pObj);
    if (NULL == pPageInfo) {
        return NULL;
    }
    return pPageInfo->GetFormDict();
}
void CPDF_ReflowedPage::GetDisplayMatrix(CFX_AffineMatrix& matrix, int32_t xPos, int32_t yPos, int32_t xSize, int32_t ySize, int32_t iRotate, const CFX_AffineMatrix* pPageMatrix)
{
    CFX_AffineMatrix display_matrix;
    if(m_PageHeight == 0) {
        matrix.Set(1, 0, 0, -1, 0, 0);
        return;
    }
    int32_t x0, y0, x1, y1, x2, y2;
    iRotate %= 4;
    switch (iRotate) {
        case 0:
            x0 = xPos;
            y0 = yPos;
            x1 = xPos;
            y1 = yPos + ySize;
            x2 = xPos + xSize;
            y2 = yPos;
            break;
        case 3:
            x0 = xPos;
            y0 = ySize + yPos;
            x1 =  xPos + xSize;
            y1 = yPos + ySize;
            x2 = xPos;
            y2 = yPos;
            break;
        case 2:
            x0 = xSize + xPos;
            y0 = ySize + yPos;
            x1 = xSize + xPos ;
            y1 = yPos;
            x2 = xPos;
            y2 =  ySize + yPos;
            break;
        case 1:
            x0 = xPos + xSize;
            y0 = yPos;
            x1 = xPos;
            y1 = yPos;
            x2 = xPos + xSize;
            y2 = yPos + ySize;
            break;
    }
    display_matrix.Set(FXSYS_Div((FX_FLOAT)(x2 - x0), m_PageWidth),
                       FXSYS_Div((FX_FLOAT)(y2 - y0), m_PageWidth),
                       FXSYS_Div((FX_FLOAT)(x1 - x0), m_PageHeight),
                       FXSYS_Div((FX_FLOAT)(y1 - y0), m_PageHeight),
                       (FX_FLOAT)(x0), (FX_FLOAT)(y0));
    matrix.Set(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
    matrix.Concat(display_matrix);
    return;
}
FX_FLOAT CPDF_ReflowedPage::GetPageHeight()
{
    return m_PageHeight;
}
void CPDF_ReflowedPage::FocusGetData(const CFX_AffineMatrix matrix, int32_t x, int32_t y, CFX_ByteString& str)
{
    if (NULL == m_pReflowed) {
        return;
    }
    CFX_AffineMatrix revMatrix;
    revMatrix.SetReverse(matrix);
    FX_FLOAT x1, y1;
    revMatrix.Transform((float)x, (float)y, x1, y1);
    int count = m_pReflowed->GetSize();
    FX_FLOAT dx = 1000, dy = 1000;
    int32_t pos = 0;
    int32_t i;
    for(i = 0; i < count; i++) {
        CRF_Data* pData = (*m_pReflowed)[i];
        FX_FLOAT tempdy = FXSYS_fabs(pData->m_PosY - y1);
        if(FXSYS_fabs(tempdy - dy) < 1) {
            continue;
        }
        CFX_FloatRect rect (0, pData->m_PosY + pData->m_Height, m_PageWidth, pData->m_PosY);
        if(rect.Contains(x1, y1)) {
            pos = i;
            dx = 0;
            dy = 0;
            break;
        } else if(tempdy < dy) {
            dy = tempdy;
            dx = FXSYS_fabs(pData->m_PosX - x1);
            pos = i;
        } else if (tempdy == dy) {
            FX_FLOAT tempdx = FXSYS_fabs(pData->m_PosX - x1);
            if(tempdx < dx) {
                dx = tempdx;
                pos = i;
            }
        } else if (tempdy > dy) {
            break;
        }
    }
    if(dx != 0 || dy != 0) {
        count = count < (pos + 10) ? count : (pos + 10);
        for(i = 0 > (pos - 10) ? 0 : (pos - 10); i < count; i++) {
            CRF_Data* pData = (*m_pReflowed)[i];
            FX_FLOAT tempdy = FXSYS_fabs(pData->m_PosY - y1);
            if(tempdy < dy) {
                dy = tempdy;
                dx = FXSYS_fabs(pData->m_PosX - x1);
                pos = i;
            } else if (tempdy == dy) {
                FX_FLOAT tempdx = FXSYS_fabs(pData->m_PosX - x1);
                if(tempdx < dx) {
                    dx = tempdx;
                    pos = i;
                }
            }
        }
    }
    str.Format("%d", pos);
}
FX_BOOL CPDF_ReflowedPage::FocusGetPosition(const CFX_AffineMatrix matrix, CFX_ByteString str, int32_t& x, int32_t& y)
{
    if (NULL == m_pReflowed) {
        return FALSE;
    }
    int32_t pos = FXSYS_atoi(str);
    if(pos < 0 || pos >= m_pReflowed->GetSize()) {
        return FALSE;
    }
    CRF_Data* pData = (*m_pReflowed)[pos];
    FX_FLOAT x1, y1;
    matrix.Transform(pData->m_PosX, pData->m_PosY + pData->m_Height, x1, y1);
    x = (int)x1;
    y = (int)y1;
    return TRUE;
}
int CPDF_ProgressiveReflowPageParser::GetPosition()
{
    if(!m_pProvider) {
        return 0;
    }
    if(!m_pReflowEngine) {
        return m_pProvider->GetPosition() / 2;
    }
    return m_pProvider->GetPosition() / 2 + m_pReflowEngine->GetPosition() / 2;
}
void CPDF_ProgressiveReflowPageParser::Continue(IFX_Pause* pPause)
{
    if (NULL == m_pReflowPage) {
        return;
    }
    if(m_Status != ToBeContinued) {
        return;
    }
    m_pPause = pPause;
    if(m_pReflowEngine) {
        if(m_pReflowEngine->Continue() != LayoutToBeContinued) {
            m_Status = Done;
        }
    } else {
        if(m_pProvider->Continue() == LayoutFinished) {
            m_pReflowEngine = IPDF_LayoutProcessor::Create_LayoutProcessor_Reflow(m_TopIndent, m_ReflowedWidth, m_fScreenHeight, m_pReflowPage, m_flags, m_ParseStyle.m_LineSpace);
            CFX_AffineMatrix matrix;
            m_pPDFPage->GetDisplayMatrix(matrix, 0, 0, (int)(m_pPDFPage->GetPageWidth()), (int)(m_pPDFPage->GetPageHeight()), 0);
            if(m_pReflowEngine->StartProcess(m_pProvider->GetRoot(), m_pPause, &matrix) != LayoutToBeContinued) {
                m_Status = Done;
            }
        }
    }
    if(m_TopIndent && m_Status == Done) {
        m_pReflowPage->m_PageHeight -= m_TopIndent;
    }
}
void CPDF_ProgressiveReflowPageParser::Clear()
{
    Init();
    return;
}
FX_BOOL IPDF_ProgressiveReflowPageParser::IsTaggedPage(CPDF_PageObjects*pPage)
{
    if(!pPage) {
        return FALSE;
    }
    CPDF_StructTree* pPageTree = CPDF_StructTree::LoadPage(pPage->m_pDocument, pPage->m_pFormDict);
    if(pPageTree) {
        int count = pPageTree->CountTopElements();
        if(count) {
            for(int i = 0; i < count; i++) {
                CPDF_StructElement* pElm = pPageTree->GetTopElement(i);
                if(pElm) {
                    delete pPageTree;
                    pPageTree = NULL;
                    return TRUE;
                }
            }
        }
        delete pPageTree;
        pPageTree = NULL;
        return FALSE;
    }
    return FALSE;
}
void CPDF_ProgressiveReflowPageParser::Start(IPDF_ReflowedPage* pReflowPage, CPDF_Page* pPage, FX_FLOAT topIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, IFX_Pause* pPause, int flags)
{
    if (NULL == pReflowPage) {
        m_Status = Failed;
        return;
    }
    m_flags = flags;
    m_pReflowPage = (CPDF_ReflowedPage*)pReflowPage;
    m_pReflowPage->m_pPDFPage = pPage;
    m_pReflowPage->ReleasePageObjsMemberShip();
    m_pPDFPage = pPage;
    m_TopIndent = topIndent;
    m_pPause = pPause;
    m_fScreenHeight = fHeight;
    m_ReflowedWidth = fWidth;
    m_pProvider = IPDF_LayoutProvider::Create_LayoutProvider_TaggedPDF(m_pPDFPage);
    LayoutStatus status = m_pProvider->StartLoad(pPause);
    if(status == LayoutError) {
        delete m_pProvider;
        m_pProvider = IPDF_LayoutProvider::Create_LayoutProvider_AutoReflow(m_pPDFPage, m_flags & RF_PARSER_READERORDER);
        if (NULL == m_pProvider) {
            m_Status = Failed;
            return;
        }
        status = m_pProvider->StartLoad(pPause);
    }
    if(status == LayoutError) {
        delete m_pProvider;
        m_pProvider = NULL;
        m_Status = Failed;
        return;
    }
    if(status == LayoutToBeContinued) {
        m_Status = ToBeContinued;
    } else if (status == LayoutFinished) {
        m_pReflowEngine = IPDF_LayoutProcessor::Create_LayoutProcessor_Reflow(topIndent, fWidth, fHeight, pReflowPage, m_flags, m_ParseStyle.m_LineSpace);
        if(NULL == m_pReflowEngine) {
            delete m_pProvider;
            m_pProvider = NULL;
            m_Status = Failed;
            return;
        }
        CFX_AffineMatrix matrix;
        pPage->GetDisplayMatrix(matrix, 0, 0, (int)(pPage->GetPageWidth()), (int)(pPage->GetPageHeight()), 0);
        CFX_AffineMatrix matrix1 = pPage->GetPageMatrix();
        if((status = m_pReflowEngine->StartProcess(m_pProvider->GetRoot(), pPause, &matrix)) != LayoutToBeContinued) {
            delete m_pReflowEngine;
            m_pReflowEngine = NULL;
            m_Status = Done;
        } else {
            m_Status = ToBeContinued;
        }
    }
    if(status != LayoutToBeContinued) {
        delete m_pProvider;
        m_pProvider = NULL;
    }
    if(m_TopIndent && m_Status == Done) {
        m_pReflowPage->m_PageHeight -= m_TopIndent;
    }
    return;
}
CPDF_ProgressiveReflowPageRender::~CPDF_ProgressiveReflowPageRender()
{
    if(m_pDisplayMatrix) {
        delete m_pDisplayMatrix;
    }
    m_pDisplayMatrix = NULL;
}
CPDF_ProgressiveReflowPageRender::CPDF_ProgressiveReflowPageRender()
{
    m_Status = Ready;
    m_pReflowPage = NULL;
    m_pDisplayMatrix = NULL;
    m_CurrNum = 0;
    m_pFontEncoding = NULL;
    m_DisplayColor = -1;
}
static FX_FLOAT _CIDTransformToFloat(uint8_t ch)
{
    if (ch < 128) {
        return ch * 1.0f / 127;
    }
    return (-255 + ch) * 1.0f / 127;
}
int	CPDF_ProgressiveReflowPageRender::GetPosition()
{
    if(m_CurrNum == 0 || NULL == m_pReflowPage) {
        return 0;
    }
    int size = m_pReflowPage->m_pReflowed->GetSize();
    if(size == 0 || m_CurrNum >= size) {
        return 100;
    }
    return (int)(m_CurrNum * 100 / size);
}
void CPDF_ProgressiveReflowPageRender::Display(IFX_Pause* pPause)
{
    if (NULL == m_pReflowPage) {
        m_Status = Done;
        return;
    }
    FX_RECT clipBox = m_pFXDevice->GetClipBox();
    int size = m_pReflowPage->m_pReflowed->GetSize();
    if (size < 1 || NULL == m_pDisplayMatrix) {
        m_Status = Done;
        return;
    }
    for(int i = m_CurrNum; i < size; i++) {
        CRF_Data* pData = (*m_pReflowPage->m_pReflowed)[i];
        if(!pData) {
            continue;
        }
        CFX_FloatRect rect (pData->m_PosX, pData->m_PosY + pData->m_Height, pData->m_PosX + pData->m_Width, pData->m_PosY);
        m_pDisplayMatrix->TransformRect(rect);
        if(rect.left > clipBox.right || rect.right < clipBox.left || rect.bottom > clipBox.bottom || rect.top < clipBox.top) {
            continue;
        }
        if(pData->GetType() == CRF_Data::Text) {
            CRF_CharData* pCharData = (CRF_CharData*)pData;
            CPDF_Font* pPDFFont = pCharData->m_pCharState->m_pFont;
            if(pPDFFont->GetFontType() == PDFFONT_TYPE3) {
                continue;
            }
            FX_FLOAT x = pData->m_PosX, y = pData->m_PosY - pCharData->m_pCharState->m_fDescent;
            FXTEXT_CHARPOS charpos ;
            charpos.m_GlyphIndex = pPDFFont->GlyphFromCharCode(pCharData->m_CharCode);
            charpos.m_FontCharWidth = pPDFFont->m_Font.GetGlyphWidth(charpos.m_GlyphIndex);
            charpos.m_OriginX       = x;
            charpos.m_OriginY       = y;
            FX_FLOAT charW = pData->m_Width * 1000 / pData->m_Height;
            if(charW != charpos.m_FontCharWidth) {
                charpos.m_bGlyphAdjust  = TRUE;
                charpos.m_AdjustMatrix[0] = charW / charpos.m_FontCharWidth;
                charpos.m_AdjustMatrix[1] = 0;
                charpos.m_AdjustMatrix[2] = 0;
                charpos.m_AdjustMatrix[3] = 1;
            } else {
                charpos.m_bGlyphAdjust  = FALSE;
            }
            FX_BOOL bRet = FALSE;
            if(m_DisplayColor == -1)
                bRet = m_pFXDevice->DrawNormalText(1, &charpos, &(pPDFFont->m_Font),
                                                   NULL, pCharData->m_pCharState->m_fFontSize,
                                                   m_pDisplayMatrix, pCharData->m_pCharState->m_Color + 0xff000000, FXTEXT_CLEARTYPE);
            else
                bRet = m_pFXDevice->DrawNormalText(1, &charpos, &(pPDFFont->m_Font),
                                                   NULL, pCharData->m_pCharState->m_fFontSize, m_pDisplayMatrix, m_DisplayColor, FXTEXT_CLEARTYPE);
        } else if(pData->GetType() == CRF_Data::Image) {
            CRF_ImageData* pImageData = (CRF_ImageData*)pData;
            if(!pImageData->m_pBitmap) {
                continue;
            }
            int left = 0, top = 0;
            CFX_DIBitmap* pDiBmp = NULL;
            CFX_DIBSource* pDispSource = pImageData->m_pBitmap;
            if(pImageData->m_Matrix.d < 0) {
                CFX_AffineMatrix matrix(pImageData->m_Matrix.a, 0, 0, -pImageData->m_Matrix.d, 0, 0);
                int left, top;
                pDiBmp = pImageData->m_pBitmap->TransformTo(&matrix, left, top);
                pDispSource = pDiBmp;
            }
            if (NULL == pDispSource) {
                continue;
            }
            if (pDispSource->GetFormat() == FXDIB_1bppMask || pDispSource->GetFormat() == FXDIB_8bppMask) {
                m_pFXDevice->StretchBitMask(pDispSource, (int)(rect.left + 0.5), (int)(rect.bottom + 0.5), (int)(rect.Width() + 0.5), (int)(rect.Height() + 0.5), 0xff000000);
            } else {
                m_pFXDevice->StretchDIBits(pDispSource, (int)(rect.left + 0.5), (int)(rect.bottom + 0.5), (int)(rect.Width() + 0.5), (int)(rect.Height() + 0.5));
            }
            if(m_pFXDevice->GetBitmap() && m_pFXDevice->GetBitmap()->GetFormat() == FXDIB_8bppRgb &&
                    m_pFXDevice->GetBitmap()->GetPalette() == NULL) {
                int nPalette = 0;
                switch(m_DitherBits) {
                    case 0:
                        nPalette = 0;
                        break;
                    case 1:
                        nPalette = 2;
                        break;
                    case 2:
                        nPalette = 4;
                        break;
                    case 3:
                        nPalette = 8;
                        break;
                    case 4:
                        nPalette = 16;
                        break;
                    case 5:
                        nPalette = 32;
                        break;
                    case 6:
                        nPalette = 64;
                        break;
                    case 7:
                        nPalette = 128;
                        break;
                    default:
                        nPalette = 256;
                        break;
                }
                if(nPalette >= 2) {
                    FX_ARGB * palette = FX_Alloc(FX_ARGB, nPalette);
                    nPalette --;
                    palette[0] = 0;
                    palette[nPalette] = 255;
                    FX_FLOAT Dither = (FX_FLOAT)255 / (nPalette);
                    for(int i = 1; i < nPalette; i++) {
                        palette[i] = (FX_ARGB)(Dither * i + 0.5);
                    }
                    FX_RECT tmpRect = rect.GetOutterRect();
                    m_pFXDevice->GetBitmap()->DitherFS(palette, nPalette + 1, &tmpRect);
                    FX_Free (palette);
                }
            }
            if(pDiBmp) {
                delete pDiBmp;
            }
        } else if(pData->GetType() == CRF_Data::Path) {
        }
        if(!(i % 10)) {
            if(pPause && pPause->NeedToPauseNow()) {
                i++;
                m_CurrNum = i;
                m_Status = ToBeContinued;
                return;
            }
        }
    }
    m_CurrNum = size;
    m_Status = Done;
}
void CPDF_ProgressiveReflowPageRender::Start(IPDF_ReflowedPage* pReflowPage, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pMatrix, IFX_Pause* pPause, int DitherBits)
{
    if(!pReflowPage || !pDevice || !pMatrix) {
        m_Status = Failed;
        return;
    }
    m_DitherBits = DitherBits;
    m_Status = Ready;
    m_CurrNum = 0;
    m_pReflowPage = (CPDF_ReflowedPage*)pReflowPage;
    m_pFXDevice = pDevice;
    if(NULL == m_pDisplayMatrix) {
        m_pDisplayMatrix = new CFX_AffineMatrix;
    }
    m_pDisplayMatrix->Copy(*pMatrix);
    m_Status = ToBeContinued;
    Display(pPause);
}
void CPDF_ProgressiveReflowPageRender::Continue(IFX_Pause* pPause)
{
    Display(pPause);
}
void CPDF_ProgressiveReflowPageRender::SetDisplayColor(FX_COLORREF color)
{
    m_DisplayColor = color;
}
void CPDF_ProgressiveReflowPageRender::Clear()
{
    if (m_pDisplayMatrix) {
        delete m_pDisplayMatrix;
    }
    m_pDisplayMatrix = NULL;
    m_pReflowPage = NULL;
    m_pFXDevice = NULL;
    m_CurrNum = 0;
    m_Status = Ready;
}
