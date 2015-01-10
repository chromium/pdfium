// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXDETECTOR_H_
#define _BC_DATAMATRIXDETECTOR_H_
class CBC_CommonBitMatrix;
class CBC_WhiteRectangleDetector;
class CBC_ResultPoint;
class CBC_QRDetectorResult;
class CBC_DataMatrixDetector;
class ResultPointsAndTransitions;
class CBC_ResultPointsAndTransitions : public CFX_Object
{
public:
    CBC_ResultPointsAndTransitions(CBC_ResultPoint *from, CBC_ResultPoint *to, FX_INT32 transitions)
    {
        m_from = from;
        m_to = to;
        m_transitions = transitions;
    }
    ~CBC_ResultPointsAndTransitions()
    {
    }
    CBC_ResultPoint *GetFrom()
    {
        return m_from;
    }
    CBC_ResultPoint *GetTo()
    {
        return m_to;
    }
    FX_INT32 GetTransitions()
    {
        return m_transitions;
    }
private:
    CBC_ResultPoint *m_from;
    CBC_ResultPoint *m_to;
    FX_INT32 m_transitions;
};
class CBC_DataMatrixDetector
{
public:
    CBC_DataMatrixDetector(CBC_CommonBitMatrix *image);
    virtual ~CBC_DataMatrixDetector();
    CBC_QRDetectorResult *Detect(FX_INT32 &e);
    CBC_ResultPoint *CorrectTopRightRectangular(CBC_ResultPoint *bottomLeft,
            CBC_ResultPoint *bottomRight,
            CBC_ResultPoint *topLeft,
            CBC_ResultPoint *topRight,
            FX_INT32 dimensionTop, FX_INT32 dimensionRight);
    CBC_ResultPoint *CorrectTopRight(CBC_ResultPoint *bottomLeft,
                                     CBC_ResultPoint *bottomRight,
                                     CBC_ResultPoint *topLeft,
                                     CBC_ResultPoint *topRight,
                                     FX_INT32 dimension);
    CBC_CommonBitMatrix *SampleGrid(CBC_CommonBitMatrix *image,
                                    CBC_ResultPoint *topLeft,
                                    CBC_ResultPoint *bottomLeft,
                                    CBC_ResultPoint *bottomRight,
                                    CBC_ResultPoint *topRight,
                                    FX_INT32 dimensionX, FX_INT32 dimensionY, FX_INT32 &e);
    CBC_ResultPointsAndTransitions *TransitionsBetween(CBC_ResultPoint *from, CBC_ResultPoint *to);
    FX_BOOL IsValid(CBC_ResultPoint *p);
    FX_INT32 Distance(CBC_ResultPoint *a, CBC_ResultPoint *b);
    void Increment(CFX_MapPtrTemplate<CBC_ResultPoint*, FX_INT32> &table, CBC_ResultPoint *key);
    FX_INT32 Round(FX_FLOAT d);
    void OrderBestPatterns(CFX_PtrArray *patterns);
    virtual void Init(FX_INT32 &e);
private:
    CBC_CommonBitMatrix *m_image;
    CBC_WhiteRectangleDetector *m_rectangleDetector;
    const static FX_INT32 INTEGERS[5];
};
#endif
