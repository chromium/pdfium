// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_ARITHMETIC_DECODER_H_
#define _JBIG2_ARITHMETIC_DECODER_H_
#include "JBig2_Define.h"
#include "JBig2_BitStream.h"
#include "JBig2_ArithQe.h"
typedef struct {
    unsigned int MPS;
    unsigned int I;
} JBig2ArithCtx;
class CJBig2_ArithDecoder : public CJBig2_Object
{
public:

    CJBig2_ArithDecoder(CJBig2_BitStream *pStream);

    ~CJBig2_ArithDecoder();

    int DECODE(JBig2ArithCtx *pCX);
private:

    void INITDEC();

    void BYTEIN();
    unsigned char B;
    unsigned int C;
    unsigned int A;
    unsigned int CT;
    CJBig2_BitStream *m_pStream;
};
inline CJBig2_ArithDecoder::CJBig2_ArithDecoder(CJBig2_BitStream *pStream)
{
    m_pStream = pStream;
    INITDEC();
}
inline CJBig2_ArithDecoder::~CJBig2_ArithDecoder()
{
}
inline void CJBig2_ArithDecoder::INITDEC()
{
    B = m_pStream->getCurByte_arith();
    C = (B ^ 0xff) << 16;;
    BYTEIN();
    C = C << 7;
    CT = CT - 7;
    A = 0x8000;
}
inline void CJBig2_ArithDecoder::BYTEIN()
{
    unsigned char B1;
    if(B == 0xff) {
        B1 = m_pStream->getNextByte_arith();
        if(B1 > 0x8f) {
            CT = 8;
        } else {
            m_pStream->incByteIdx();
            B = B1;
            C = C + 0xfe00 - (B << 9);
            CT = 7;
        }
    } else {
        m_pStream->incByteIdx();
        B = m_pStream->getCurByte_arith();
        C = C + 0xff00 - (B << 8);
        CT = 8;
    }
}
inline int CJBig2_ArithDecoder::DECODE(JBig2ArithCtx *pCX)
{
    if (!pCX || pCX->I >= JBIG2_QE_NUM) {
        return 0;
    }

    int D;
    const JBig2ArithQe * qe = &QeTable[pCX->I];
    A = A - qe->Qe;
    if((C >> 16) < A) {
        if(A & 0x8000) {
            D = pCX->MPS;
        } else {
            if(A < qe->Qe) {
                D = 1 - pCX->MPS;
                if(qe->nSwitch == 1) {
                    pCX->MPS = 1 - pCX->MPS;
                }
                pCX->I = qe->NLPS;
            } else {
                D = pCX->MPS;
                pCX->I = qe->NMPS;
            }
            do {
                if (CT == 0) {
                    BYTEIN();
                }
                A <<= 1;
                C <<= 1;
                CT--;
            } while ((A & 0x8000) == 0);
        }
    } else {
        C -= A << 16;
        if(A < qe->Qe) {
            A = qe->Qe;
            D = pCX->MPS;
            pCX->I = qe->NMPS;
        } else {
            A = qe->Qe;
            D = 1 - pCX->MPS;
            if(qe->nSwitch == 1) {
                pCX->MPS = 1 - pCX->MPS;
            }
            pCX->I = qe->NLPS;
        }
        do {
            if (CT == 0) {
                BYTEIN();
            }
            A <<= 1;
            C <<= 1;
            CT--;
        } while ((A & 0x8000) == 0);
    }
    return D;
}
#endif
