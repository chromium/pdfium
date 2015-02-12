// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <limits.h>

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../src/fxcrt/fx_safe_types.h"
#include "../../../third_party/base/numerics/safe_conversions_impl.h"
#include "pageint.h"

class CPDF_PSEngine;
typedef enum {PSOP_ADD, PSOP_SUB, PSOP_MUL, PSOP_DIV, PSOP_IDIV, PSOP_MOD,
              PSOP_NEG, PSOP_ABS, PSOP_CEILING, PSOP_FLOOR, PSOP_ROUND, PSOP_TRUNCATE,
              PSOP_SQRT, PSOP_SIN, PSOP_COS, PSOP_ATAN, PSOP_EXP, PSOP_LN, PSOP_LOG,
              PSOP_CVI, PSOP_CVR, PSOP_EQ, PSOP_NE, PSOP_GT, PSOP_GE, PSOP_LT, PSOP_LE,
              PSOP_AND, PSOP_OR, PSOP_XOR, PSOP_NOT, PSOP_BITSHIFT, PSOP_TRUE, PSOP_FALSE,
              PSOP_IF, PSOP_IFELSE, PSOP_POP, PSOP_EXCH, PSOP_DUP, PSOP_COPY,
              PSOP_INDEX, PSOP_ROLL, PSOP_PROC, PSOP_CONST
             } PDF_PSOP;
class CPDF_PSProc : public CFX_Object
{
public:
    ~CPDF_PSProc();
    FX_BOOL	Parse(CPDF_SimpleParser& parser);
    FX_BOOL	Execute(CPDF_PSEngine* pEngine);
    CFX_PtrArray		m_Operators;
};
#define PSENGINE_STACKSIZE 100
class CPDF_PSEngine : public CFX_Object
{
public:
    CPDF_PSEngine();
    ~CPDF_PSEngine();
    FX_BOOL	Parse(const FX_CHAR* string, int size);
    FX_BOOL	Execute()
    {
        return m_MainProc.Execute(this);
    }
    FX_BOOL	DoOperator(PDF_PSOP op);
    void	Reset()
    {
        m_StackCount = 0;
    }
    void	Push(FX_FLOAT value);
    void	Push(int value)
    {
        Push((FX_FLOAT)value);
    }
    FX_FLOAT	Pop();
    int		GetStackSize()
    {
        return m_StackCount;
    }
private:
    FX_FLOAT	m_Stack[PSENGINE_STACKSIZE];
    int		m_StackCount;
    CPDF_PSProc	m_MainProc;
};
CPDF_PSProc::~CPDF_PSProc()
{
    int size = m_Operators.GetSize();
    for (int i = 0; i < size; i ++) {
        if (m_Operators[i] == (FX_LPVOID)PSOP_PROC) {
            delete (CPDF_PSProc*)m_Operators[i + 1];
            i ++;
        } else if (m_Operators[i] == (FX_LPVOID)PSOP_CONST) {
            FX_Free((FX_FLOAT*)m_Operators[i + 1]);
            i ++;
        }
    }
}
#pragma optimize( "", off )
FX_BOOL CPDF_PSProc::Execute(CPDF_PSEngine* pEngine)
{
    int size = m_Operators.GetSize();
    for (int i = 0; i < size; i ++) {
        PDF_PSOP op = (PDF_PSOP)(FX_UINTPTR)m_Operators[i];
        if (op == PSOP_PROC) {
            i ++;
        } else if (op == PSOP_CONST) {
            pEngine->Push(*(FX_FLOAT*)m_Operators[i + 1]);
            i ++;
        } else if (op == PSOP_IF) {
            if (i < 2 || m_Operators[i - 2] != (FX_LPVOID)PSOP_PROC) {
                return FALSE;
            }
            if ((int)pEngine->Pop()) {
                ((CPDF_PSProc*)m_Operators[i - 1])->Execute(pEngine);
            }
        } else if (op == PSOP_IFELSE) {
            if (i < 4 || m_Operators[i - 2] != (FX_LPVOID)PSOP_PROC ||
                    m_Operators[i - 4] != (FX_LPVOID)PSOP_PROC) {
                return FALSE;
            }
            if ((int)pEngine->Pop()) {
                ((CPDF_PSProc*)m_Operators[i - 3])->Execute(pEngine);
            } else {
                ((CPDF_PSProc*)m_Operators[i - 1])->Execute(pEngine);
            }
        } else {
            pEngine->DoOperator(op);
        }
    }
    return TRUE;
}
#pragma optimize( "", on )
CPDF_PSEngine::CPDF_PSEngine()
{
    m_StackCount = 0;
}
CPDF_PSEngine::~CPDF_PSEngine()
{
}
void CPDF_PSEngine::Push(FX_FLOAT v)
{
    if (m_StackCount == 100) {
        return;
    }
    m_Stack[m_StackCount++] = v;
}
FX_FLOAT CPDF_PSEngine::Pop()
{
    if (m_StackCount == 0) {
        return 0;
    }
    return m_Stack[--m_StackCount];
}
const struct _PDF_PSOpName {
    const FX_CHAR* name;
    PDF_PSOP op;
} _PDF_PSOpNames[] = {
    {"add", PSOP_ADD}, {"sub", PSOP_SUB}, {"mul", PSOP_MUL}, {"div", PSOP_DIV},
    {"idiv", PSOP_IDIV}, {"mod", PSOP_MOD}, {"neg", PSOP_NEG}, {"abs", PSOP_ABS},
    {"ceiling", PSOP_CEILING}, {"floor", PSOP_FLOOR}, {"round", PSOP_ROUND},
    {"truncate", PSOP_TRUNCATE}, {"sqrt", PSOP_SQRT}, {"sin", PSOP_SIN},
    {"cos", PSOP_COS}, {"atan", PSOP_ATAN}, {"exp", PSOP_EXP}, {"ln", PSOP_LN},
    {"log", PSOP_LOG}, {"cvi", PSOP_CVI}, {"cvr", PSOP_CVR}, {"eq", PSOP_EQ},
    {"ne", PSOP_NE}, {"gt", PSOP_GT}, {"ge", PSOP_GE}, {"lt", PSOP_LT},
    {"le", PSOP_LE}, {"and", PSOP_AND}, {"or", PSOP_OR}, {"xor", PSOP_XOR},
    {"not", PSOP_NOT}, {"bitshift", PSOP_BITSHIFT}, {"true", PSOP_TRUE},
    {"false", PSOP_FALSE}, {"if", PSOP_IF}, {"ifelse", PSOP_IFELSE},
    {"pop", PSOP_POP}, {"exch", PSOP_EXCH}, {"dup", PSOP_DUP},
    {"copy", PSOP_COPY}, {"index", PSOP_INDEX}, {"roll", PSOP_ROLL},
    {NULL, PSOP_PROC}
};
FX_BOOL CPDF_PSEngine::Parse(const FX_CHAR* string, int size)
{
    CPDF_SimpleParser parser((FX_LPBYTE)string, size);
    CFX_ByteStringC word = parser.GetWord();
    if (word != FX_BSTRC("{")) {
        return FALSE;
    }
    return m_MainProc.Parse(parser);
}
FX_BOOL CPDF_PSProc::Parse(CPDF_SimpleParser& parser)
{
    while (1) {
        CFX_ByteStringC word = parser.GetWord();
        if (word.IsEmpty()) {
            return FALSE;
        }
        if (word == FX_BSTRC("}")) {
            return TRUE;
        }
        if (word == FX_BSTRC("{")) {
            CPDF_PSProc* pProc = FX_NEW CPDF_PSProc;
            m_Operators.Add((FX_LPVOID)PSOP_PROC);
            m_Operators.Add(pProc);
            if (!pProc->Parse(parser)) {
                return FALSE;
            }
        } else {
            int i = 0;
            while (_PDF_PSOpNames[i].name) {
                if (word == CFX_ByteStringC(_PDF_PSOpNames[i].name)) {
                    m_Operators.Add((FX_LPVOID)_PDF_PSOpNames[i].op);
                    break;
                }
                i ++;
            }
            if (_PDF_PSOpNames[i].name == NULL) {
                FX_FLOAT* pd = FX_Alloc(FX_FLOAT, 1);
                *pd = FX_atof(word);
                m_Operators.Add((FX_LPVOID)PSOP_CONST);
                m_Operators.Add(pd);
            }
        }
    }
}
#define PI 3.1415926535897932384626433832795f
FX_BOOL CPDF_PSEngine::DoOperator(PDF_PSOP op)
{
    int i1, i2;
    FX_FLOAT d1, d2;
    switch (op) {
        case PSOP_ADD:
            d1 = Pop();
            d2 = Pop();
            Push(d1 + d2);
            break;
        case PSOP_SUB:
            d2 = Pop();
            d1 = Pop();
            Push(d1 - d2);
            break;
        case PSOP_MUL:
            d1 = Pop();
            d2 = Pop();
            Push(d1 * d2);
            break;
        case PSOP_DIV:
            d2 = Pop();
            d1 = Pop();
            Push(d1 / d2);
            break;
        case PSOP_IDIV:
            i2 = (int)Pop();
            i1 = (int)Pop();
            Push(i1 / i2);
            break;
        case PSOP_MOD:
            i2 = (int)Pop();
            i1 = (int)Pop();
            Push(i1 % i2);
            break;
        case PSOP_NEG:
            d1 = Pop();
            Push(-d1);
            break;
        case PSOP_ABS:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_fabs(d1));
            break;
        case PSOP_CEILING:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_ceil(d1));
            break;
        case PSOP_FLOOR:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_floor(d1));
            break;
        case PSOP_ROUND:
            d1 = Pop();
            Push(FXSYS_round(d1));
            break;
        case PSOP_TRUNCATE:
            i1 = (int)Pop();
            Push(i1);
            break;
        case PSOP_SQRT:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_sqrt(d1));
            break;
        case PSOP_SIN:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_sin(d1 * PI / 180.0f));
            break;
        case PSOP_COS:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_cos(d1 * PI / 180.0f));
            break;
        case PSOP_ATAN:
            d2 = Pop();
            d1 = Pop();
            d1 = (FX_FLOAT)(FXSYS_atan2(d1, d2) * 180.0 / PI);
            if (d1 < 0) {
                d1 += 360;
            }
            Push(d1);
            break;
        case PSOP_EXP:
            d2 = Pop();
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_pow(d1, d2));
            break;
        case PSOP_LN:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_log(d1));
            break;
        case PSOP_LOG:
            d1 = Pop();
            Push((FX_FLOAT)FXSYS_log10(d1));
            break;
        case PSOP_CVI:
            i1 = (int)Pop();
            Push(i1);
            break;
        case PSOP_CVR:
            break;
        case PSOP_EQ:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 == d2));
            break;
        case PSOP_NE:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 != d2));
            break;
        case PSOP_GT:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 > d2));
            break;
        case PSOP_GE:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 >= d2));
            break;
        case PSOP_LT:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 < d2));
            break;
        case PSOP_LE:
            d2 = Pop();
            d1 = Pop();
            Push((int)(d1 <= d2));
            break;
        case PSOP_AND:
            i1 = (int)Pop();
            i2 = (int)Pop();
            Push(i1 & i2);
            break;
        case PSOP_OR:
            i1 = (int)Pop();
            i2 = (int)Pop();
            Push(i1 | i2);
            break;
        case PSOP_XOR:
            i1 = (int)Pop();
            i2 = (int)Pop();
            Push(i1 ^ i2);
            break;
        case PSOP_NOT:
            i1 = (int)Pop();
            Push((int)!i1);
            break;
        case PSOP_BITSHIFT: {
                int shift = (int)Pop();
                int i = (int)Pop();
                if (shift > 0) {
                    Push(i << shift);
                } else {
                    Push(i >> -shift);
                }
                break;
            }
        case PSOP_TRUE:
            Push(1);
            break;
        case PSOP_FALSE:
            Push(0);
            break;
        case PSOP_POP:
            Pop();
            break;
        case PSOP_EXCH:
            d2 = Pop();
            d1 = Pop();
            Push(d2);
            Push(d1);
            break;
        case PSOP_DUP:
            d1 = Pop();
            Push(d1);
            Push(d1);
            break;
        case PSOP_COPY: {
                int n = (int)Pop();
                if (n < 0 || n > PSENGINE_STACKSIZE || m_StackCount + n > PSENGINE_STACKSIZE || n > m_StackCount) {
                    break;
                }
                for (int i = 0; i < n; i ++) {
                    m_Stack[m_StackCount + i] = m_Stack[m_StackCount + i - n];
                }
                m_StackCount += n;
                break;
            }
        case PSOP_INDEX: {
                int n = (int)Pop();
                if (n < 0 || n >= m_StackCount) {
                    break;
                }
                Push(m_Stack[m_StackCount - n - 1]);
                break;
            }
        case PSOP_ROLL: {
                int j = (int)Pop();
                int n = (int)Pop();
                if (m_StackCount == 0) {
                    break;
                }
                if (n < 0 || n > m_StackCount) {
                    break;
                }
                if (j < 0)
                    for (int i = 0; i < -j; i ++) {
                        FX_FLOAT first = m_Stack[m_StackCount - n];
                        for (int ii = 0; ii < n - 1; ii ++) {
                            m_Stack[m_StackCount - n + ii] = m_Stack[m_StackCount - n + ii + 1];
                        }
                        m_Stack[m_StackCount - 1] = first;
                    }
                else
                    for (int i = 0; i < j; i ++) {
                        FX_FLOAT last = m_Stack[m_StackCount - 1];
                        int ii;
                        for (ii = 0; ii < n - 1; ii ++) {
                            m_Stack[m_StackCount - ii - 1] = m_Stack[m_StackCount - ii - 2];
                        }
                        m_Stack[m_StackCount - ii - 1] = last;
                    }
                break;
            }
        default:
            break;
    }
    return TRUE;
}
static FX_FLOAT PDF_Interpolate(FX_FLOAT x, FX_FLOAT xmin, FX_FLOAT xmax, FX_FLOAT ymin, FX_FLOAT ymax)
{
    return ((x - xmin) * (ymax - ymin) / (xmax - xmin)) + ymin;
}
static FX_DWORD _GetBits32(FX_LPCBYTE pData, int bitpos, int nbits)
{
    int result = 0;
    for (int i = 0; i < nbits; i ++)
        if (pData[(bitpos + i) / 8] & (1 << (7 - (bitpos + i) % 8))) {
            result |= 1 << (nbits - i - 1);
        }
    return result;
}
typedef struct {
    FX_FLOAT	encode_max, encode_min;
    int			sizes;
} SampleEncodeInfo;
typedef struct {
    FX_FLOAT	decode_max, decode_min;
} SampleDecodeInfo;
class CPDF_SampledFunc : public CPDF_Function
{
public:
    CPDF_SampledFunc();
    virtual ~CPDF_SampledFunc();
    virtual FX_BOOL		v_Init(CPDF_Object* pObj);
    virtual FX_BOOL		v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const;
    SampleEncodeInfo*	m_pEncodeInfo;
    SampleDecodeInfo*	m_pDecodeInfo;
    FX_DWORD	m_nBitsPerSample, m_SampleMax;
    CPDF_StreamAcc*	m_pSampleStream;
};
CPDF_SampledFunc::CPDF_SampledFunc()
{
    m_pSampleStream = NULL;
    m_pEncodeInfo = NULL;
    m_pDecodeInfo = NULL;
}
CPDF_SampledFunc::~CPDF_SampledFunc()
{
    if (m_pSampleStream) {
        delete m_pSampleStream;
    }
    if (m_pEncodeInfo) {
        FX_Free(m_pEncodeInfo);
    }
    if (m_pDecodeInfo) {
        FX_Free(m_pDecodeInfo);
    }
}
FX_BOOL CPDF_SampledFunc::v_Init(CPDF_Object* pObj)
{
    if (pObj->GetType() != PDFOBJ_STREAM) {
        return FALSE;
    }
    CPDF_Stream* pStream = (CPDF_Stream*)pObj;
    CPDF_Dictionary* pDict = pStream->GetDict();
    CPDF_Array* pSize = pDict->GetArray(FX_BSTRC("Size"));
    CPDF_Array* pEncode = pDict->GetArray(FX_BSTRC("Encode"));
    CPDF_Array* pDecode = pDict->GetArray(FX_BSTRC("Decode"));
    m_nBitsPerSample = pDict->GetInteger(FX_BSTRC("BitsPerSample"));
    m_SampleMax = 0xffffffff >> (32 - m_nBitsPerSample);
    m_pSampleStream = FX_NEW CPDF_StreamAcc;
    m_pSampleStream->LoadAllData(pStream, FALSE);
    m_pEncodeInfo = FX_Alloc(SampleEncodeInfo, m_nInputs);
    int i;
    FX_DWORD nTotalSamples = 1;
    for (i = 0; i < m_nInputs; i ++) {
        m_pEncodeInfo[i].sizes = pSize ? pSize->GetInteger(i) : 0;
        if (!pSize && i == 0) {
            m_pEncodeInfo[i].sizes = pDict->GetInteger(FX_BSTRC("Size"));
        }
        if (nTotalSamples > 0 && (FX_UINT32)(m_pEncodeInfo[i].sizes) > UINT_MAX / nTotalSamples) {
            return FALSE;
        }
        nTotalSamples *= m_pEncodeInfo[i].sizes;
        if (pEncode) {
            m_pEncodeInfo[i].encode_min = pEncode->GetFloat(i * 2);
            m_pEncodeInfo[i].encode_max = pEncode->GetFloat(i * 2 + 1);
        } else {
            m_pEncodeInfo[i].encode_min = 0;
            if (m_pEncodeInfo[i].sizes == 1) {
                m_pEncodeInfo[i].encode_max = 1;
            } else {
                m_pEncodeInfo[i].encode_max = (FX_FLOAT)m_pEncodeInfo[i].sizes - 1;
            }
        }
    }
    if (nTotalSamples > 0 && m_nBitsPerSample > UINT_MAX / nTotalSamples) {
        return FALSE;
    }
    nTotalSamples *= m_nBitsPerSample;
    if (nTotalSamples > 0 && ((FX_UINT32)m_nOutputs) > UINT_MAX / nTotalSamples) {
        return FALSE;
    }
    nTotalSamples *= m_nOutputs;
    if (nTotalSamples == 0 || m_pSampleStream->GetSize() * 8 < nTotalSamples) {
        return FALSE;
    }
    m_pDecodeInfo = FX_Alloc(SampleDecodeInfo, m_nOutputs);
    for (i = 0; i < m_nOutputs; i ++) {
        if (pDecode) {
            m_pDecodeInfo[i].decode_min = pDecode->GetFloat(2 * i);
            m_pDecodeInfo[i].decode_max = pDecode->GetFloat(2 * i + 1);
        } else {
            m_pDecodeInfo[i].decode_min = m_pRanges[i * 2];
            m_pDecodeInfo[i].decode_max = m_pRanges[i * 2 + 1];
        }
    }
    return TRUE;
}
FX_BOOL CPDF_SampledFunc::v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const
{
    int pos = 0;
    CFX_FixedBufGrow<FX_FLOAT, 16> encoded_input_buf(m_nInputs);
    FX_FLOAT* encoded_input = encoded_input_buf;
    CFX_FixedBufGrow<int, 32> int_buf(m_nInputs * 2);
    int* index = int_buf;
    int* blocksize = index + m_nInputs;
    for (int i = 0; i < m_nInputs; i ++) {
        if (i == 0) {
            blocksize[i] = 1;
        } else {
            blocksize[i] = blocksize[i - 1] * m_pEncodeInfo[i - 1].sizes;
        }
        encoded_input[i] = PDF_Interpolate(inputs[i], m_pDomains[i * 2], m_pDomains[i * 2 + 1],
                                           m_pEncodeInfo[i].encode_min, m_pEncodeInfo[i].encode_max);
        index[i] = (int)encoded_input[i];
        if (index[i] < 0) {
            index[i] = 0;
        } else if (index[i] > m_pEncodeInfo[i].sizes - 1) {
            index[i] = m_pEncodeInfo[i].sizes - 1;
        }
        pos += index[i] * blocksize[i];
    }
    FX_SAFE_INT32 bitpos = pos;
    bitpos *= m_nBitsPerSample;
    bitpos *= m_nOutputs;
    if (!bitpos.IsValid()) {
        return FALSE;
    }
    FX_LPCBYTE pSampleData = m_pSampleStream->GetData();
    if (pSampleData == NULL) {
        return FALSE;
    }
    FX_SAFE_INT32 bitpos1 = m_nOutputs - 1 > 0 ? m_nOutputs - 1 : 0; 
    bitpos1 *= m_nBitsPerSample;
    bitpos1 += bitpos.ValueOrDie();
    if (!bitpos1.IsValid()) {
        return FALSE;
    }
    for (int j = 0; j < m_nOutputs; j ++) {
        FX_DWORD sample = _GetBits32(pSampleData, bitpos.ValueOrDie() + j * m_nBitsPerSample, m_nBitsPerSample);
        FX_FLOAT encoded = (FX_FLOAT)sample;
        for (int i = 0; i < m_nInputs; i ++) {
            if (index[i] == m_pEncodeInfo[i].sizes - 1) {
                if (index[i] == 0) {
                    encoded = encoded_input[i] * (FX_FLOAT)sample;
                }
            } else {
                FX_SAFE_INT32 bitpos2 = blocksize[i];
                bitpos2 += pos;
                bitpos2 *= m_nOutputs;
                bitpos2 += j;
                bitpos2 *= m_nBitsPerSample; 
                if (!bitpos2.IsValid()) {
                    return FALSE;
                }
                FX_DWORD sample1 = _GetBits32(pSampleData, bitpos2.ValueOrDie(), m_nBitsPerSample);
                encoded += (encoded_input[i] - index[i]) * ((FX_FLOAT)sample1 - (FX_FLOAT)sample);
            }
        }
        results[j] = PDF_Interpolate(encoded, 0, (FX_FLOAT)m_SampleMax,
                                     m_pDecodeInfo[j].decode_min, m_pDecodeInfo[j].decode_max);
    }
    return TRUE;
}
class CPDF_PSFunc : public CPDF_Function
{
public:
    virtual FX_BOOL		v_Init(CPDF_Object* pObj);
    virtual FX_BOOL		v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const;
    CPDF_PSEngine m_PS;
};
FX_BOOL CPDF_PSFunc::v_Init(CPDF_Object* pObj)
{
    CPDF_Stream* pStream = (CPDF_Stream*)pObj;
    CPDF_StreamAcc acc;
    acc.LoadAllData(pStream, FALSE);
    return m_PS.Parse((const FX_CHAR*)acc.GetData(), acc.GetSize());
}
FX_BOOL CPDF_PSFunc::v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const
{
    CPDF_PSEngine& PS = (CPDF_PSEngine&)m_PS;
    PS.Reset();
    int i;
    for (i = 0; i < m_nInputs; i ++) {
        PS.Push(inputs[i]);
    }
    PS.Execute();
    if (PS.GetStackSize() < m_nOutputs) {
        return FALSE;
    }
    for (i = 0; i < m_nOutputs; i ++) {
        results[m_nOutputs - i - 1] = PS.Pop();
    }
    return TRUE;
}
class CPDF_ExpIntFunc : public CPDF_Function
{
public:
    CPDF_ExpIntFunc();
    virtual ~CPDF_ExpIntFunc();
    virtual FX_BOOL		v_Init(CPDF_Object* pObj);
    virtual FX_BOOL		v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const;
    FX_FLOAT	m_Exponent;
    FX_FLOAT*	m_pBeginValues;
    FX_FLOAT*	m_pEndValues;
    int		m_nOrigOutputs;
};
CPDF_ExpIntFunc::CPDF_ExpIntFunc()
{
    m_pBeginValues = NULL;
    m_pEndValues = NULL;
}
CPDF_ExpIntFunc::~CPDF_ExpIntFunc()
{
    if (m_pBeginValues) {
        FX_Free(m_pBeginValues);
    }
    if (m_pEndValues) {
        FX_Free(m_pEndValues);
    }
}
FX_BOOL CPDF_ExpIntFunc::v_Init(CPDF_Object* pObj)
{
    CPDF_Dictionary* pDict = pObj->GetDict();
    if (pDict == NULL) {
        return FALSE;
    }
    CPDF_Array* pArray0 = pDict->GetArray(FX_BSTRC("C0"));
    if (m_nOutputs == 0) {
        m_nOutputs = 1;
        if (pArray0) {
            m_nOutputs = pArray0->GetCount();
        }
    }
    CPDF_Array* pArray1 = pDict->GetArray(FX_BSTRC("C1"));
    m_pBeginValues = FX_Alloc(FX_FLOAT, m_nOutputs * 2);
    m_pEndValues = FX_Alloc(FX_FLOAT, m_nOutputs * 2);
    for (int i = 0; i < m_nOutputs; i ++) {
        m_pBeginValues[i] = pArray0 ? pArray0->GetFloat(i) : 0.0f;
        m_pEndValues[i] = pArray1 ? pArray1->GetFloat(i) : 1.0f;
    }
    m_Exponent = pDict->GetFloat(FX_BSTRC("N"));
    m_nOrigOutputs = m_nOutputs;
    if (m_nOutputs && m_nInputs > INT_MAX / m_nOutputs) {
        return FALSE;
    }
    m_nOutputs *= m_nInputs;
    return TRUE;
}
FX_BOOL CPDF_ExpIntFunc::v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const
{
    for (int i = 0; i < m_nInputs; i ++)
        for (int j = 0; j < m_nOrigOutputs; j ++) {
            results[i * m_nOrigOutputs + j] = m_pBeginValues[j] + (FX_FLOAT)FXSYS_pow(inputs[i], m_Exponent) *
                                              (m_pEndValues[j] - m_pBeginValues[j]);
        }
    return TRUE;
}
class CPDF_StitchFunc : public CPDF_Function
{
public:
    CPDF_StitchFunc();
    virtual ~CPDF_StitchFunc();
    virtual FX_BOOL		v_Init(CPDF_Object* pObj);
    virtual FX_BOOL		v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const;
    int			m_nSubs;
    CPDF_Function** m_pSubFunctions;
    FX_FLOAT*	m_pBounds;
    FX_FLOAT*	m_pEncode;
};
CPDF_StitchFunc::CPDF_StitchFunc()
{
    m_nSubs = 0;
    m_pSubFunctions = NULL;
    m_pBounds = NULL;
    m_pEncode = NULL;
}
CPDF_StitchFunc::~CPDF_StitchFunc()
{
    for (int i = 0; i < m_nSubs; i ++)
        if (m_pSubFunctions[i]) {
            delete m_pSubFunctions[i];
        }
    if (m_pSubFunctions) {
        FX_Free(m_pSubFunctions);
    }
    if (m_pBounds) {
        FX_Free(m_pBounds);
    }
    if (m_pEncode) {
        FX_Free(m_pEncode);
    }
}
FX_BOOL CPDF_StitchFunc::v_Init(CPDF_Object* pObj)
{
    CPDF_Dictionary* pDict = pObj->GetDict();
    if (pDict == NULL) {
        return FALSE;
    }
    CPDF_Array* pArray = pDict->GetArray(FX_BSTRC("Functions"));
    if (pArray == NULL) {
        return FALSE;
    }
    m_nSubs = pArray->GetCount();
    if (m_nSubs == 0) {
        return FALSE;
    }
    m_pSubFunctions = FX_Alloc(CPDF_Function*, m_nSubs);
    m_nOutputs = 0;
    int i;
    for (i = 0; i < m_nSubs; i ++) {
        CPDF_Object* pSub = pArray->GetElementValue(i);
        if (pSub == pObj) {
            return FALSE;
        }
        m_pSubFunctions[i] = CPDF_Function::Load(pSub);
        if (m_pSubFunctions[i] == NULL) {
            return FALSE;
        }
        if (m_pSubFunctions[i]->CountOutputs() > m_nOutputs) {
            m_nOutputs = m_pSubFunctions[i]->CountOutputs();
        }
    }
    m_pBounds = FX_Alloc(FX_FLOAT, m_nSubs + 1);
    m_pBounds[0] = m_pDomains[0];
    pArray = pDict->GetArray(FX_BSTRC("Bounds"));
    if (pArray == NULL) {
        return FALSE;
    }
    for (i = 0; i < m_nSubs - 1; i ++) {
        m_pBounds[i + 1] = pArray->GetFloat(i);
    }
    m_pBounds[m_nSubs] = m_pDomains[1];
    m_pEncode = FX_Alloc(FX_FLOAT, m_nSubs * 2);
    pArray = pDict->GetArray(FX_BSTRC("Encode"));
    if (pArray == NULL) {
        return FALSE;
    }
    for (i = 0; i < m_nSubs * 2; i ++) {
        m_pEncode[i] = pArray->GetFloat(i);
    }
    return TRUE;
}
FX_BOOL CPDF_StitchFunc::v_Call(FX_FLOAT* inputs, FX_FLOAT* outputs) const
{
    FX_FLOAT input = inputs[0];
    int i;
    for (i = 0; i < m_nSubs - 1; i ++)
        if (input < m_pBounds[i + 1]) {
            break;
        }
    if (m_pSubFunctions[i] == NULL) {
        return FALSE;
    }
    input = PDF_Interpolate(input, m_pBounds[i], m_pBounds[i + 1], m_pEncode[i * 2], m_pEncode[i * 2 + 1]);
    int nresults;
    m_pSubFunctions[i]->Call(&input, m_nInputs, outputs, nresults);
    return TRUE;
}
CPDF_Function* CPDF_Function::Load(CPDF_Object* pFuncObj)
{
    if (pFuncObj == NULL) {
        return NULL;
    }
    CPDF_Function* pFunc = NULL;
    int type;
    if (pFuncObj->GetType() == PDFOBJ_STREAM) {
        type = ((CPDF_Stream*)pFuncObj)->GetDict()->GetInteger(FX_BSTRC("FunctionType"));
    } else if (pFuncObj->GetType() == PDFOBJ_DICTIONARY) {
        type = ((CPDF_Dictionary*)pFuncObj)->GetInteger(FX_BSTRC("FunctionType"));
    } else {
        return NULL;
    }
    if (type == 0) {
        pFunc = FX_NEW CPDF_SampledFunc;
    } else if (type == 2) {
        pFunc = FX_NEW CPDF_ExpIntFunc;
    } else if (type == 3) {
        pFunc = FX_NEW CPDF_StitchFunc;
    } else if (type == 4) {
        pFunc = FX_NEW CPDF_PSFunc;
    } else {
        return NULL;
    }
    if (!pFunc->Init(pFuncObj)) {
        delete pFunc;
        return NULL;
    }
    return pFunc;
}
CPDF_Function::CPDF_Function()
{
    m_pDomains = NULL;
    m_pRanges = NULL;
}
CPDF_Function::~CPDF_Function()
{
    if (m_pDomains) {
        FX_Free(m_pDomains);
        m_pDomains = NULL;
    }
    if (m_pRanges) {
        FX_Free(m_pRanges);
        m_pRanges = NULL;
    }
}
FX_BOOL CPDF_Function::Init(CPDF_Object* pObj)
{
    CPDF_Dictionary* pDict;
    if (pObj->GetType() == PDFOBJ_STREAM) {
        pDict = ((CPDF_Stream*)pObj)->GetDict();
    } else {
        pDict = (CPDF_Dictionary*)pObj;
    }
    CPDF_Array* pDomains = pDict->GetArray(FX_BSTRC("Domain"));
    if (pDomains == NULL) {
        return FALSE;
    }
    m_nInputs = pDomains->GetCount() / 2;
    if (m_nInputs == 0) {
        return FALSE;
    }
    m_pDomains = FX_Alloc(FX_FLOAT, m_nInputs * 2);
    for (int i = 0; i < m_nInputs * 2; i ++) {
        m_pDomains[i] = pDomains->GetFloat(i);
    }
    CPDF_Array* pRanges = pDict->GetArray(FX_BSTRC("Range"));
    m_nOutputs = 0;
    if (pRanges) {
        m_nOutputs = pRanges->GetCount() / 2;
        m_pRanges = FX_Alloc(FX_FLOAT, m_nOutputs * 2);
        for (int i = 0; i < m_nOutputs * 2; i ++) {
            m_pRanges[i] = pRanges->GetFloat(i);
        }
    }
    FX_DWORD old_outputs = m_nOutputs;
    FX_BOOL ret = v_Init(pObj);
    if (m_pRanges && m_nOutputs > (int)old_outputs) {
        m_pRanges = FX_Realloc(FX_FLOAT, m_pRanges, m_nOutputs * 2);
        if (m_pRanges) {
            FXSYS_memset32(m_pRanges + (old_outputs * 2), 0, sizeof(FX_FLOAT) * (m_nOutputs - old_outputs) * 2);
        }
    }
    return ret;
}
FX_BOOL CPDF_Function::Call(FX_FLOAT* inputs, int ninputs, FX_FLOAT* results, int& nresults) const
{
    if (m_nInputs != ninputs) {
        return FALSE;
    }
    nresults = m_nOutputs;
    for (int i = 0; i < m_nInputs; i ++) {
        if (inputs[i] < m_pDomains[i * 2]) {
            inputs[i] = m_pDomains[i * 2];
        } else if (inputs[i] > m_pDomains[i * 2 + 1]) {
            inputs[i] = m_pDomains[i * 2] + 1;
        }
    }
    v_Call(inputs, results);
    if (m_pRanges) {
        for (int i = 0; i < m_nOutputs; i ++) {
            if (results[i] < m_pRanges[i * 2]) {
                results[i] = m_pRanges[i * 2];
            } else if (results[i] > m_pRanges[i * 2 + 1]) {
                results[i] = m_pRanges[i * 2 + 1];
            }
        }
    }
    return TRUE;
}
