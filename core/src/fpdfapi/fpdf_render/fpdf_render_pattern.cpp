// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_render.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../../../include/fxge/fx_ge.h"
#include "../fpdf_page/pageint.h"
#include "render_int.h"
#define SHADING_STEPS 256
static void _DrawAxialShading(CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                              CPDF_Dictionary* pDict, CPDF_Function** pFuncs, int nFuncs,
                              CPDF_ColorSpace* pCS, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    CPDF_Array* pCoords = pDict->GetArray(FX_BSTRC("Coords"));
    if (pCoords == NULL) {
        return;
    }
    FX_FLOAT start_x = pCoords->GetNumber(0);
    FX_FLOAT start_y = pCoords->GetNumber(1);
    FX_FLOAT end_x = pCoords->GetNumber(2);
    FX_FLOAT end_y = pCoords->GetNumber(3);
    FX_FLOAT t_min = 0, t_max = 1.0f;
    CPDF_Array* pArray = pDict->GetArray(FX_BSTRC("Domain"));
    if (pArray) {
        t_min = pArray->GetNumber(0);
        t_max = pArray->GetNumber(1);
    }
    FX_BOOL bStartExtend = FALSE, bEndExtend = FALSE;
    pArray = pDict->GetArray(FX_BSTRC("Extend"));
    if (pArray) {
        bStartExtend = pArray->GetInteger(0);
        bEndExtend = pArray->GetInteger(1);
    }
    int width = pBitmap->GetWidth();
    int height = pBitmap->GetHeight();
    FX_FLOAT x_span = end_x - start_x;
    FX_FLOAT y_span = end_y - start_y;
    FX_FLOAT axis_len_square = FXSYS_Mul(x_span, x_span) + FXSYS_Mul(y_span, y_span);
    CFX_AffineMatrix matrix;
    matrix.SetReverse(*pObject2Bitmap);
    int total_results = 0;
    for (int j = 0; j < nFuncs; j ++) {
        if (pFuncs[j]) {
            total_results += pFuncs[j]->CountOutputs();
        }
    }
    if (pCS->CountComponents() > total_results) {
        total_results = pCS->CountComponents();
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> result_array(total_results);
    FX_FLOAT* pResults = result_array;
    FXSYS_memset32(pResults, 0, total_results * sizeof(FX_FLOAT));
    FX_DWORD rgb_array[SHADING_STEPS];
    for (int i = 0; i < SHADING_STEPS; i ++) {
        FX_FLOAT input = (t_max - t_min) * i / SHADING_STEPS + t_min;
        int offset = 0;
        for (int j = 0; j < nFuncs; j ++) {
            if (pFuncs[j]) {
                int nresults = 0;
                if (pFuncs[j]->Call(&input, 1, pResults + offset, nresults)) {
                    offset += nresults;
                }
            }
        }
        FX_FLOAT R = 0.0f, G = 0.0f, B = 0.0f;
        pCS->GetRGB(pResults, R, G, B);
        rgb_array[i] = FXARGB_TODIB(FXARGB_MAKE(alpha, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255)));
    }
    int pitch = pBitmap->GetPitch();
    for (int row = 0; row < height; row ++) {
        FX_DWORD* dib_buf = (FX_DWORD*)(pBitmap->GetBuffer() + row * pitch);
        for (int column = 0; column < width; column ++) {
            FX_FLOAT x = (FX_FLOAT)column, y = (FX_FLOAT)row;
            matrix.Transform(x, y);
            FX_FLOAT scale = FXSYS_Div(FXSYS_Mul(x - start_x, x_span) + FXSYS_Mul(y - start_y, y_span), axis_len_square);
            int index = (FX_INT32)(scale * (SHADING_STEPS - 1));
            if (index < 0) {
                if (!bStartExtend) {
                    continue;
                }
                index = 0;
            } else if (index >= SHADING_STEPS) {
                if (!bEndExtend) {
                    continue;
                }
                index = SHADING_STEPS - 1;
            }
            dib_buf[column] = rgb_array[index];
        }
    }
}
static void _DrawRadialShading(CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                               CPDF_Dictionary* pDict, CPDF_Function** pFuncs, int nFuncs,
                               CPDF_ColorSpace* pCS, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    CPDF_Array* pCoords = pDict->GetArray(FX_BSTRC("Coords"));
    if (pCoords == NULL) {
        return;
    }
    FX_FLOAT start_x = pCoords->GetNumber(0);
    FX_FLOAT start_y = pCoords->GetNumber(1);
    FX_FLOAT start_r = pCoords->GetNumber(2);
    FX_FLOAT end_x = pCoords->GetNumber(3);
    FX_FLOAT end_y = pCoords->GetNumber(4);
    FX_FLOAT end_r = pCoords->GetNumber(5);
    CFX_AffineMatrix matrix;
    matrix.SetReverse(*pObject2Bitmap);
    FX_FLOAT t_min = 0, t_max = 1.0f;
    CPDF_Array* pArray = pDict->GetArray(FX_BSTRC("Domain"));
    if (pArray) {
        t_min = pArray->GetNumber(0);
        t_max = pArray->GetNumber(1);
    }
    FX_BOOL bStartExtend = FALSE, bEndExtend = FALSE;
    pArray = pDict->GetArray(FX_BSTRC("Extend"));
    if (pArray) {
        bStartExtend = pArray->GetInteger(0);
        bEndExtend = pArray->GetInteger(1);
    }
    int total_results = 0;
    for (int j = 0; j < nFuncs; j ++) {
        if (pFuncs[j]) {
            total_results += pFuncs[j]->CountOutputs();
        }
    }
    if (pCS->CountComponents() > total_results) {
        total_results = pCS->CountComponents();
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> result_array(total_results);
    FX_FLOAT* pResults = result_array;
    FXSYS_memset32(pResults, 0, total_results * sizeof(FX_FLOAT));
    FX_DWORD rgb_array[SHADING_STEPS];
    for (int i = 0; i < SHADING_STEPS; i ++) {
        FX_FLOAT input = (t_max - t_min) * i / SHADING_STEPS + t_min;
        int offset = 0;
        for (int j = 0; j < nFuncs; j ++) {
            if (pFuncs[j]) {
                int nresults;
                if (pFuncs[j]->Call(&input, 1, pResults + offset, nresults)) {
                    offset += nresults;
                }
            }
        }
        FX_FLOAT R = 0.0f, G = 0.0f, B = 0.0f;
        pCS->GetRGB(pResults, R, G, B);
        rgb_array[i] = FXARGB_TODIB(FXARGB_MAKE(alpha, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255)));
    }
    FX_FLOAT a = FXSYS_Mul(start_x - end_x, start_x - end_x) +
                 FXSYS_Mul(start_y - end_y, start_y - end_y) - FXSYS_Mul(start_r - end_r, start_r - end_r);
    int width = pBitmap->GetWidth();
    int height = pBitmap->GetHeight();
    int pitch = pBitmap->GetPitch();
    FX_BOOL bDecreasing = FALSE;
    if (start_r > end_r) {
        int length = (int)FXSYS_sqrt((FXSYS_Mul(start_x - end_x, start_x - end_x) + FXSYS_Mul(start_y - end_y, start_y - end_y)));
        if (length < start_r - end_r) {
            bDecreasing = TRUE;
        }
    }
    for (int row = 0; row < height; row ++) {
        FX_DWORD* dib_buf = (FX_DWORD*)(pBitmap->GetBuffer() + row * pitch);
        for (int column = 0; column < width; column ++) {
            FX_FLOAT x = (FX_FLOAT)column, y = (FX_FLOAT)row;
            matrix.Transform(x, y);
            FX_FLOAT b = -2 * (FXSYS_Mul(x - start_x, end_x - start_x) + FXSYS_Mul(y - start_y, end_y - start_y) +
                               FXSYS_Mul(start_r, end_r - start_r));
            FX_FLOAT c = FXSYS_Mul(x - start_x, x - start_x) + FXSYS_Mul(y - start_y, y - start_y) -
                         FXSYS_Mul(start_r, start_r);
            FX_FLOAT s;
            if (a == 0) {
                s = FXSYS_Div(-c, b);
            } else {
                FX_FLOAT b2_4ac = FXSYS_Mul(b, b) - 4 * FXSYS_Mul(a, c);
                if (b2_4ac < 0) {
                    continue;
                }
                FX_FLOAT root = FXSYS_sqrt(b2_4ac);
                FX_FLOAT s1, s2;
                if (a > 0) {
                    s1 = FXSYS_Div(-b - root, 2 * a);
                    s2 = FXSYS_Div(-b + root, 2 * a);
                } else {
                    s2 = FXSYS_Div(-b - root, 2 * a);
                    s1 = FXSYS_Div(-b + root, 2 * a);
                }
                if (bDecreasing) {
                    if (s1 >= 0 || bStartExtend) {
                        s = s1;
                    } else {
                        s = s2;
                    }
                } else {
                    if (s2 <= 1.0f || bEndExtend) {
                        s = s2;
                    } else {
                        s = s1;
                    }
                }
                if ((start_r + s * (end_r - start_r)) < 0) {
                    continue;
                }
            }
            int index = (FX_INT32)(s * (SHADING_STEPS - 1));
            if (index < 0) {
                if (!bStartExtend) {
                    continue;
                }
                index = 0;
            }
            if (index >= SHADING_STEPS) {
                if (!bEndExtend) {
                    continue;
                }
                index = SHADING_STEPS - 1;
            }
            dib_buf[column] = rgb_array[index];
        }
    }
}
static void _DrawFuncShading(CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                             CPDF_Dictionary* pDict, CPDF_Function** pFuncs, int nFuncs,
                             CPDF_ColorSpace* pCS, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    CPDF_Array* pDomain = pDict->GetArray(FX_BSTRC("Domain"));
    FX_FLOAT xmin = 0, ymin = 0, xmax = 1.0f, ymax = 1.0f;
    if (pDomain) {
        xmin = pDomain->GetNumber(0);
        xmax = pDomain->GetNumber(1);
        ymin = pDomain->GetNumber(2);
        ymax = pDomain->GetNumber(3);
    }
    CFX_AffineMatrix mtDomain2Target = pDict->GetMatrix(FX_BSTRC("Matrix"));
    CFX_AffineMatrix matrix, reverse_matrix;
    matrix.SetReverse(*pObject2Bitmap);
    reverse_matrix.SetReverse(mtDomain2Target);
    matrix.Concat(reverse_matrix);
    int width = pBitmap->GetWidth();
    int height = pBitmap->GetHeight();
    int pitch = pBitmap->GetPitch();
    int total_results = 0;
    for (int j = 0; j < nFuncs; j ++) {
        if (pFuncs[j]) {
            total_results += pFuncs[j]->CountOutputs();
        }
    }
    if (pCS->CountComponents() > total_results) {
        total_results = pCS->CountComponents();
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> result_array(total_results);
    FX_FLOAT* pResults = result_array;
    FXSYS_memset32(pResults, 0, total_results * sizeof(FX_FLOAT));
    for (int row = 0; row < height; row ++) {
        FX_DWORD* dib_buf = (FX_DWORD*)(pBitmap->GetBuffer() + row * pitch);
        for (int column = 0; column < width; column ++) {
            FX_FLOAT x = (FX_FLOAT)column, y = (FX_FLOAT)row;
            matrix.Transform(x, y);
            if (x < xmin || x > xmax || y < ymin || y > ymax) {
                continue;
            }
            FX_FLOAT input[2];
            int offset = 0;
            input[0] = x;
            input[1] = y;
            for (int j = 0; j < nFuncs; j ++) {
                if (pFuncs[j]) {
                    int nresults;
                    if (pFuncs[j]->Call(input, 2, pResults + offset, nresults)) {
                        offset += nresults;
                    }
                }
            }
            FX_FLOAT R = 0.0f, G = 0.0f, B = 0.0f;
            pCS->GetRGB(pResults, R, G, B);
            dib_buf[column] = FXARGB_TODIB(FXARGB_MAKE(alpha, (FX_INT32)(R * 255), (FX_INT32)(G * 255), (FX_INT32)(B * 255)));
        }
    }
}
FX_BOOL _GetScanlineIntersect(int y, FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_FLOAT& x)
{
    if (y1 == y2) {
        return FALSE;
    }
    if (y1 < y2) {
        if (y < y1 || y > y2) {
            return FALSE;
        }
    } else {
        if (y < y2 || y > y1) {
            return FALSE;
        }
    }
    x = x1 + FXSYS_MulDiv(x2 - x1, y - y1, y2 - y1);
    return TRUE;
}
static void _DrawGouraud(CFX_DIBitmap* pBitmap, int alpha, CPDF_MeshVertex triangle[3])
{
    FX_FLOAT min_y = triangle[0].y, max_y = triangle[0].y;
    for (int i = 1; i < 3; i ++) {
        if (min_y > triangle[i].y) {
            min_y = triangle[i].y;
        }
        if (max_y < triangle[i].y) {
            max_y = triangle[i].y;
        }
    }
    if (min_y == max_y) {
        return;
    }
    int min_yi = (int)FXSYS_floor(min_y), max_yi = (int)FXSYS_ceil(max_y);
    if (min_yi < 0) {
        min_yi = 0;
    }
    if (max_yi >= pBitmap->GetHeight()) {
        max_yi = pBitmap->GetHeight() - 1;
    }
    for (int y = min_yi; y <= max_yi; y ++) {
        int nIntersects = 0;
        FX_FLOAT inter_x[3], r[3], g[3], b[3];
        for (int i = 0; i < 3; i ++) {
            CPDF_MeshVertex& vertex1 = triangle[i];
            CPDF_MeshVertex& vertex2 = triangle[(i + 1) % 3];
            FX_BOOL bIntersect = _GetScanlineIntersect(y, vertex1.x, vertex1.y,
                                 vertex2.x, vertex2.y, inter_x[nIntersects]);
            if (!bIntersect) {
                continue;
            }
            r[nIntersects] = vertex1.r + FXSYS_MulDiv(vertex2.r - vertex1.r, y - vertex1.y, vertex2.y - vertex1.y);
            g[nIntersects] = vertex1.g + FXSYS_MulDiv(vertex2.g - vertex1.g, y - vertex1.y, vertex2.y - vertex1.y);
            b[nIntersects] = vertex1.b + FXSYS_MulDiv(vertex2.b - vertex1.b, y - vertex1.y, vertex2.y - vertex1.y);
            nIntersects ++;
        }
        if (nIntersects != 2) {
            continue;
        }
        int min_x, max_x, start_index, end_index;
        if (inter_x[0] < inter_x[1]) {
            min_x = (int)FXSYS_floor(inter_x[0]);
            max_x = (int)FXSYS_ceil(inter_x[1]);
            start_index = 0;
            end_index = 1;
        } else {
            min_x = (int)FXSYS_floor(inter_x[1]);
            max_x = (int)FXSYS_ceil(inter_x[0]);
            start_index = 1;
            end_index = 0;
        }
        int start_x = min_x, end_x = max_x;
        if (start_x < 0) {
            start_x = 0;
        }
        if (end_x > pBitmap->GetWidth()) {
            end_x = pBitmap->GetWidth();
        }
        FX_LPBYTE dib_buf = pBitmap->GetBuffer() + y * pBitmap->GetPitch() + start_x * 4;
        FX_FLOAT r_unit = (r[end_index] - r[start_index]) / (max_x - min_x);
        FX_FLOAT g_unit = (g[end_index] - g[start_index]) / (max_x - min_x);
        FX_FLOAT b_unit = (b[end_index] - b[start_index]) / (max_x - min_x);
        FX_FLOAT R = r[start_index] + (start_x - min_x) * r_unit;
        FX_FLOAT G = g[start_index] + (start_x - min_x) * g_unit;
        FX_FLOAT B = b[start_index] + (start_x - min_x) * b_unit;
        for (int x = start_x; x < end_x; x ++) {
            R += r_unit;
            G += g_unit;
            B += b_unit;
            FXARGB_SETDIB(dib_buf, FXARGB_MAKE(alpha, (FX_INT32)(R * 255), (FX_INT32)(G * 255), (FX_INT32)(B * 255)));
            dib_buf += 4;
        }
    }
}
static void _DrawFreeGouraudShading(CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                                    CPDF_Stream* pShadingStream, CPDF_Function** pFuncs, int nFuncs,
                                    CPDF_ColorSpace* pCS, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    if (pShadingStream->GetType() != PDFOBJ_STREAM) {
        return;
    }
    CPDF_MeshStream stream;
    if (!stream.Load(pShadingStream, pFuncs, nFuncs, pCS)) {
        return;
    }
    CPDF_MeshVertex triangle[3];
    FXSYS_memset32(triangle, 0, sizeof(triangle));

    while (!stream.m_BitStream.IsEOF()) {
        CPDF_MeshVertex vertex;
        FX_DWORD flag = stream.GetVertex(vertex, pObject2Bitmap);
        if (flag == 0) {
            triangle[0] = vertex;
            for (int j = 1; j < 3; j ++) {
                stream.GetVertex(triangle[j], pObject2Bitmap);
            }
        } else {
            if (flag == 1) {
                triangle[0] = triangle[1];
            }
            triangle[1] = triangle[2];
            triangle[2] = vertex;
        }
        _DrawGouraud(pBitmap, alpha, triangle);
    }
}
static void _DrawLatticeGouraudShading(CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                                       CPDF_Stream* pShadingStream, CPDF_Function** pFuncs, int nFuncs,
                                       CPDF_ColorSpace* pCS, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    if (pShadingStream->GetType() != PDFOBJ_STREAM) {
        return;
    }
    int row_verts = pShadingStream->GetDict()->GetInteger("VerticesPerRow");
    if (row_verts < 2) {
        return;
    }
    CPDF_MeshStream stream;
    if (!stream.Load(pShadingStream, pFuncs, nFuncs, pCS)) {
        return;
    }
    CPDF_MeshVertex* vertex = FX_Alloc2D(CPDF_MeshVertex, row_verts, 2);
    if (!stream.GetVertexRow(vertex, row_verts, pObject2Bitmap)) {
        FX_Free(vertex);
        return;
    }
    int last_index = 0;
    while (1) {
        CPDF_MeshVertex* last_row = vertex + last_index * row_verts;
        CPDF_MeshVertex* this_row = vertex + (1 - last_index) * row_verts;
        if (!stream.GetVertexRow(this_row, row_verts, pObject2Bitmap)) {
            FX_Free(vertex);
            return;
        }
        CPDF_MeshVertex triangle[3];
        for (int i = 1; i < row_verts; i ++) {
            triangle[0] = last_row[i];
            triangle[1] = this_row[i - 1];
            triangle[2] = last_row[i - 1];
            _DrawGouraud(pBitmap, alpha, triangle);
            triangle[2] = this_row[i];
            _DrawGouraud(pBitmap, alpha, triangle);
        }
        last_index = 1 - last_index;
    }
    FX_Free(vertex);
}
struct Coon_BezierCoeff {
    float a, b, c, d;
    void FromPoints(float p0, float p1, float p2, float p3)
    {
        a = -p0 + 3 * p1 - 3 * p2 + p3;
        b = 3 * p0 - 6 * p1 + 3 * p2;
        c = -3 * p0 + 3 * p1;
        d = p0;
    }
    Coon_BezierCoeff first_half()
    {
        Coon_BezierCoeff result;
        result.a = a / 8;
        result.b = b / 4;
        result.c = c / 2;
        result.d = d;
        return result;
    }
    Coon_BezierCoeff second_half()
    {
        Coon_BezierCoeff result;
        result.a = a / 8;
        result.b = 3 * a / 8 + b / 4;
        result.c = 3 * a / 8 + b / 2 + c / 2;
        result.d = a / 8 + b / 4 + c / 2 + d;
        return result;
    }
    void GetPoints(float p[4])
    {
        p[0] = d;
        p[1] = c / 3 + p[0];
        p[2] = b / 3 - p[0] + 2 * p[1];
        p[3] = a + p[0] - 3 * p[1] + 3 * p[2];
    }
    void GetPointsReverse(float p[4])
    {
        p[3] = d;
        p[2] = c / 3 + p[3];
        p[1] = b / 3 - p[3] + 2 * p[2];
        p[0] = a + p[3] - 3 * p[2] + 3 * p[1];
    }
    void BezierInterpol(Coon_BezierCoeff& C1, Coon_BezierCoeff& C2, Coon_BezierCoeff& D1, Coon_BezierCoeff& D2)
    {
        a = (D1.a + D2.a) / 2;
        b = (D1.b + D2.b) / 2;
        c = (D1.c + D2.c) / 2 - (C1.a / 8 + C1.b / 4 + C1.c / 2) + (C2.a / 8 + C2.b / 4) + (-C1.d + D2.d) / 2 - (C2.a + C2.b) / 2;
        d = C1.a / 8 + C1.b / 4 + C1.c / 2 + C1.d;
    }
    float Distance()
    {
        float dis = a + b + c;
        return dis < 0 ? -dis : dis;
    }
};
struct Coon_Bezier {
    Coon_BezierCoeff x, y;
    void FromPoints(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3)
    {
        x.FromPoints(x0, x1, x2, x3);
        y.FromPoints(y0, y1, y2, y3);
    }
    Coon_Bezier first_half()
    {
        Coon_Bezier result;
        result.x = x.first_half();
        result.y = y.first_half();
        return result;
    }
    Coon_Bezier second_half()
    {
        Coon_Bezier result;
        result.x = x.second_half();
        result.y = y.second_half();
        return result;
    }
    void BezierInterpol(Coon_Bezier& C1, Coon_Bezier& C2, Coon_Bezier& D1, Coon_Bezier& D2)
    {
        x.BezierInterpol(C1.x, C2.x, D1.x, D2.x);
        y.BezierInterpol(C1.y, C2.y, D1.y, D2.y);
    }
    void GetPoints(FX_PATHPOINT* pPoints)
    {
        float p[4];
        int i;
        x.GetPoints(p);
        for (i = 0; i < 4; i ++) {
            pPoints[i].m_PointX = p[i];
        }
        y.GetPoints(p);
        for (i = 0; i < 4; i ++) {
            pPoints[i].m_PointY = p[i];
        }
    }
    void GetPointsReverse(FX_PATHPOINT* pPoints)
    {
        float p[4];
        int i;
        x.GetPointsReverse(p);
        for (i = 0; i < 4; i ++) {
            pPoints[i].m_PointX = p[i];
        }
        y.GetPointsReverse(p);
        for (i = 0; i < 4; i ++) {
            pPoints[i].m_PointY = p[i];
        }
    }
    float Distance()
    {
        return x.Distance() + y.Distance();
    }
};
static int _BiInterpol(int c0, int c1, int c2, int c3, int x, int y, int x_scale, int y_scale)
{
    int x1 = c0 + (c3 - c0) * x / x_scale;
    int x2 = c1 + (c2 - c1) * x / x_scale;
    return x1 + (x2 - x1) * y / y_scale;
}
struct Coon_Color {
    Coon_Color()
    {
        FXSYS_memset32(comp, 0, sizeof(int) * 3);
    }
    int		comp[3];
    void	BiInterpol(Coon_Color colors[4], int x, int y, int x_scale, int y_scale)
    {
        for (int i = 0; i < 3; i ++)
            comp[i] = _BiInterpol(colors[0].comp[i], colors[1].comp[i], colors[2].comp[i], colors[3].comp[i],
                                  x, y, x_scale, y_scale);
    }
    int		Distance(Coon_Color& o)
    {
        int max, diff;
        max = FXSYS_abs(comp[0] - o.comp[0]);
        diff = FXSYS_abs(comp[1] - o.comp[1]);
        if (max < diff) {
            max = diff;
        }
        diff = FXSYS_abs(comp[2] - o.comp[2]);
        if (max < diff) {
            max = diff;
        }
        return max;
    }
};
struct CPDF_PatchDrawer {
    Coon_Color			patch_colors[4];
    int					max_delta;
    CFX_PathData		path;
    CFX_RenderDevice*	pDevice;
    int					fill_mode;
    int					alpha;
    void Draw(int x_scale, int y_scale, int left, int bottom, Coon_Bezier C1, Coon_Bezier C2, Coon_Bezier D1, Coon_Bezier D2)
    {
        FX_BOOL bSmall = C1.Distance() < 2 && C2.Distance() < 2 && D1.Distance() < 2 && D2.Distance() < 2;
        Coon_Color div_colors[4];
        int d_bottom, d_left, d_top, d_right;
        div_colors[0].BiInterpol(patch_colors, left, bottom, x_scale, y_scale);
        if (!bSmall) {
            div_colors[1].BiInterpol(patch_colors, left, bottom + 1, x_scale, y_scale);
            div_colors[2].BiInterpol(patch_colors, left + 1, bottom + 1, x_scale, y_scale);
            div_colors[3].BiInterpol(patch_colors, left + 1, bottom, x_scale, y_scale);
            d_bottom = div_colors[3].Distance(div_colors[0]);
            d_left = div_colors[1].Distance(div_colors[0]);
            d_top = div_colors[1].Distance(div_colors[2]);
            d_right = div_colors[2].Distance(div_colors[3]);
        }
#define COONCOLOR_THRESHOLD 4
        if (bSmall || (d_bottom < COONCOLOR_THRESHOLD && d_left < COONCOLOR_THRESHOLD &&
                       d_top < COONCOLOR_THRESHOLD && d_right < COONCOLOR_THRESHOLD)) {
            FX_PATHPOINT* pPoints = path.GetPoints();
            C1.GetPoints(pPoints);
            D2.GetPoints(pPoints + 3);
            C2.GetPointsReverse(pPoints + 6);
            D1.GetPointsReverse(pPoints + 9);
            int fillFlags = FXFILL_WINDING | FXFILL_FULLCOVER;
            if (fill_mode & RENDER_NOPATHSMOOTH) {
                fillFlags |= FXFILL_NOPATHSMOOTH;
            }
            pDevice->DrawPath(&path, NULL, NULL, FXARGB_MAKE(alpha, div_colors[0].comp[0], div_colors[0].comp[1], div_colors[0].comp[2]), 0, fillFlags);
        } else {
            if (d_bottom < COONCOLOR_THRESHOLD && d_top < COONCOLOR_THRESHOLD) {
                Coon_Bezier m1;
                m1.BezierInterpol(D1, D2, C1, C2);
                y_scale *= 2;
                bottom *= 2;
                Draw(x_scale, y_scale, left, bottom, C1, m1, D1.first_half(), D2.first_half());
                Draw(x_scale, y_scale, left, bottom + 1, m1, C2, D1.second_half(), D2.second_half());
            } else if (d_left < COONCOLOR_THRESHOLD && d_right < COONCOLOR_THRESHOLD) {
                Coon_Bezier m2;
                m2.BezierInterpol(C1, C2, D1, D2);
                x_scale *= 2;
                left *= 2;
                Draw(x_scale, y_scale, left, bottom, C1.first_half(), C2.first_half(), D1, m2);
                Draw(x_scale, y_scale, left + 1, bottom, C1.second_half(), C2.second_half(), m2, D2);
            } else {
                Coon_Bezier m1, m2;
                m1.BezierInterpol(D1, D2, C1, C2);
                m2.BezierInterpol(C1, C2, D1, D2);
                Coon_Bezier m1f = m1.first_half();
                Coon_Bezier m1s = m1.second_half();
                Coon_Bezier m2f = m2.first_half();
                Coon_Bezier m2s = m2.second_half();
                x_scale *= 2;
                y_scale *= 2;
                left *= 2;
                bottom *= 2;
                Draw(x_scale, y_scale, left, bottom, C1.first_half(), m1f, D1.first_half(), m2f);
                Draw(x_scale, y_scale, left, bottom + 1, m1f, C2.first_half(), D1.second_half(), m2s);
                Draw(x_scale, y_scale, left + 1, bottom, C1.second_half(), m1s, m2f, D2.first_half());
                Draw(x_scale, y_scale, left + 1, bottom + 1, m1s, C2.second_half(), m2s, D2.second_half());
            }
        }
    }
};

FX_BOOL _CheckCoonTensorPara(const CPDF_MeshStream &stream)
{
    FX_BOOL bCoorBits = ( stream.m_nCoordBits== 1   ||
                          stream.m_nCoordBits == 2  ||
                          stream.m_nCoordBits == 4  ||
                          stream.m_nCoordBits == 8  ||
                          stream.m_nCoordBits == 12 ||
                          stream.m_nCoordBits == 16 ||
                          stream.m_nCoordBits == 24 ||
                          stream.m_nCoordBits == 32   );

    FX_BOOL bCompBits = ( stream.m_nCompBits == 1  || 
                          stream.m_nCompBits == 2  ||
                          stream.m_nCompBits == 4  ||
                          stream.m_nCompBits == 8  ||
                          stream.m_nCompBits == 12 ||
                          stream.m_nCompBits == 16   );

    FX_BOOL bFlagBits = ( stream.m_nFlagBits == 2  ||
                          stream.m_nFlagBits == 4  ||
                          stream.m_nFlagBits == 8    );

    return bCoorBits && bCompBits && bFlagBits; 
}

static void _DrawCoonPatchMeshes(FX_BOOL bTensor, CFX_DIBitmap* pBitmap, CFX_AffineMatrix* pObject2Bitmap,
                                 CPDF_Stream* pShadingStream, CPDF_Function** pFuncs, int nFuncs,
                                 CPDF_ColorSpace* pCS, int fill_mode, int alpha)
{
    ASSERT(pBitmap->GetFormat() == FXDIB_Argb);
    if (pShadingStream->GetType() != PDFOBJ_STREAM) {
        return;
    }
    CFX_FxgeDevice device;
    device.Attach(pBitmap);
    CPDF_MeshStream stream;
    if (!stream.Load(pShadingStream, pFuncs, nFuncs, pCS)) {
        return;
    }

    if (!_CheckCoonTensorPara(stream)) {
        return;
    }
 
    CPDF_PatchDrawer patch;
    patch.alpha = alpha;
    patch.pDevice = &device;
    patch.fill_mode = fill_mode;
    patch.path.SetPointCount(13);
    FX_PATHPOINT* pPoints = patch.path.GetPoints();
    pPoints[0].m_Flag = FXPT_MOVETO;
    for (int i = 1; i < 13; i ++) {
        pPoints[i].m_Flag = FXPT_BEZIERTO;
    }
    CFX_FloatPoint coords[16];
    for (int i = 0; i < 16; i ++) {
        coords[i].Set(0.0f, 0.0f);
    }

    int point_count = bTensor ? 16 : 12;
    while (!stream.m_BitStream.IsEOF()) {
        FX_DWORD flag = stream.GetFlag();
        int iStartPoint = 0, iStartColor = 0, i = 0;
        if (flag) {
            iStartPoint = 4;
            iStartColor = 2;
            CFX_FloatPoint tempCoords[4];
            for (i = 0; i < 4; i ++) {
                tempCoords[i] = coords[(flag * 3 + i) % 12];
            }
            FXSYS_memcpy32(coords, tempCoords, sizeof(CFX_FloatPoint) * 4);
            Coon_Color tempColors[2];
            tempColors[0] = patch.patch_colors[flag];
            tempColors[1] = patch.patch_colors[(flag + 1) % 4];
            FXSYS_memcpy32(patch.patch_colors, tempColors, sizeof(Coon_Color) * 2);
        }
        for (i = iStartPoint; i < point_count; i ++) {
            stream.GetCoords(coords[i].x, coords[i].y);
            pObject2Bitmap->Transform(coords[i].x, coords[i].y);
        }
        for (i = iStartColor; i < 4; i ++) {
            FX_FLOAT r=0.0f, g=0.0f, b=0.0f;
            stream.GetColor(r, g, b);
            patch.patch_colors[i].comp[0] = (FX_INT32)(r * 255);
            patch.patch_colors[i].comp[1] = (FX_INT32)(g * 255);
            patch.patch_colors[i].comp[2] = (FX_INT32)(b * 255);
        }
        CFX_FloatRect bbox = CFX_FloatRect::GetBBox(coords, point_count);
        if (bbox.right <= 0 || bbox.left >= (FX_FLOAT)pBitmap->GetWidth() || bbox.top <= 0 ||
                bbox.bottom >= (FX_FLOAT)pBitmap->GetHeight()) {
            continue;
        }
        Coon_Bezier C1, C2, D1, D2;
        C1.FromPoints(coords[0].x, coords[0].y, coords[11].x, coords[11].y, coords[10].x, coords[10].y,
                      coords[9].x, coords[9].y);
        C2.FromPoints(coords[3].x, coords[3].y, coords[4].x, coords[4].y, coords[5].x, coords[5].y,
                      coords[6].x, coords[6].y);
        D1.FromPoints(coords[0].x, coords[0].y, coords[1].x, coords[1].y, coords[2].x, coords[2].y,
                      coords[3].x, coords[3].y);
        D2.FromPoints(coords[9].x, coords[9].y, coords[8].x, coords[8].y, coords[7].x, coords[7].y,
                      coords[6].x, coords[6].y);
        patch.Draw(1, 1, 0, 0, C1, C2, D1, D2);
    }
}
void CPDF_RenderStatus::DrawShading(CPDF_ShadingPattern* pPattern, CFX_AffineMatrix* pMatrix,
                                    FX_RECT& clip_rect, int alpha, FX_BOOL bAlphaMode)
{
    CPDF_Function** pFuncs = pPattern->m_pFunctions;
    int nFuncs = pPattern->m_nFuncs;
    CPDF_Dictionary* pDict = pPattern->m_pShadingObj->GetDict();
    CPDF_ColorSpace* pColorSpace = pPattern->m_pCS;
    if (pColorSpace == NULL) {
        return;
    }
    FX_ARGB background = 0;
    if (!pPattern->m_bShadingObj && pPattern->m_pShadingObj->GetDict()->KeyExist(FX_BSTRC("Background"))) {
        CPDF_Array* pBackColor = pPattern->m_pShadingObj->GetDict()->GetArray(FX_BSTRC("Background"));
        if (pBackColor && pBackColor->GetCount() >= (FX_DWORD)pColorSpace->CountComponents()) {
            CFX_FixedBufGrow<FX_FLOAT, 16> comps(pColorSpace->CountComponents());
            for (int i = 0; i < pColorSpace->CountComponents(); i ++) {
                comps[i] = pBackColor->GetNumber(i);
            }
            FX_FLOAT R = 0.0f, G = 0.0f, B = 0.0f;
            pColorSpace->GetRGB(comps, R, G, B);
            background = ArgbEncode(255, (FX_INT32)(R * 255), (FX_INT32)(G * 255), (FX_INT32)(B * 255));
        }
    }
    if (pDict->KeyExist(FX_BSTRC("BBox"))) {
        CFX_FloatRect rect = pDict->GetRect(FX_BSTRC("BBox"));
        rect.Transform(pMatrix);
        clip_rect.Intersect(rect.GetOutterRect());
    }
    CPDF_DeviceBuffer buffer;
    buffer.Initialize(m_pContext, m_pDevice, &clip_rect, m_pCurObj, 150);
    CFX_AffineMatrix FinalMatrix = *pMatrix;
    FinalMatrix.Concat(*buffer.GetMatrix());
    CFX_DIBitmap* pBitmap = buffer.GetBitmap();
    if (pBitmap->GetBuffer() == NULL) {
        return;
    }
    pBitmap->Clear(background);
    int fill_mode = m_Options.m_Flags;
    switch (pPattern->m_ShadingType) {
        case 1:
            _DrawFuncShading(pBitmap, &FinalMatrix, pDict, pFuncs, nFuncs, pColorSpace, alpha);
            break;
        case 2:
            _DrawAxialShading(pBitmap, &FinalMatrix, pDict, pFuncs, nFuncs, pColorSpace, alpha);
            break;
        case 3:
            _DrawRadialShading(pBitmap, &FinalMatrix, pDict, pFuncs, nFuncs, pColorSpace, alpha);
            break;
        case 4: {
                _DrawFreeGouraudShading(pBitmap, &FinalMatrix, (CPDF_Stream*)pPattern->m_pShadingObj,
                                        pFuncs, nFuncs, pColorSpace, alpha);
            }
            break;
        case 5: {
                _DrawLatticeGouraudShading(pBitmap, &FinalMatrix, (CPDF_Stream*)pPattern->m_pShadingObj,
                                           pFuncs, nFuncs, pColorSpace, alpha);
            }
            break;
        case 6:
        case 7: {
                _DrawCoonPatchMeshes(pPattern->m_ShadingType - 6, pBitmap, &FinalMatrix, (CPDF_Stream*)pPattern->m_pShadingObj,
                                     pFuncs, nFuncs, pColorSpace, fill_mode, alpha);
            }
            break;
    }
    if (bAlphaMode) {
        pBitmap->LoadChannel(FXDIB_Red, pBitmap, FXDIB_Alpha);
    }
    if (m_Options.m_ColorMode == RENDER_COLOR_GRAY) {
        pBitmap->ConvertColorScale(m_Options.m_ForeColor, m_Options.m_BackColor);
    }
    buffer.OutputToDevice();
}
void CPDF_RenderStatus::DrawShadingPattern(CPDF_ShadingPattern* pattern, CPDF_PageObject* pPageObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStroke)
{
    if (!pattern->Load()) {
        return;
    }
    m_pDevice->SaveState();
    if (pPageObj->m_Type == PDFPAGE_PATH) {
        if (!SelectClipPath((CPDF_PathObject*)pPageObj, pObj2Device, bStroke)) {
            m_pDevice->RestoreState();
            return;
        }
    } else if (pPageObj->m_Type == PDFPAGE_IMAGE) {
        FX_RECT rect = pPageObj->GetBBox(pObj2Device);
        m_pDevice->SetClip_Rect(&rect);
    } else {
        return;
    }
    FX_RECT rect;
    if (GetObjectClippedRect(pPageObj, pObj2Device, FALSE, rect)) {
        m_pDevice->RestoreState();
        return;
    }
    CFX_AffineMatrix matrix = pattern->m_Pattern2Form;
    matrix.Concat(*pObj2Device);
    GetScaledMatrix(matrix);
    int alpha = pPageObj->m_GeneralState.GetAlpha(bStroke);
    DrawShading(pattern, &matrix, rect, alpha, m_Options.m_ColorMode == RENDER_COLOR_ALPHA);
    m_pDevice->RestoreState();
}
FX_BOOL CPDF_RenderStatus::ProcessShading(CPDF_ShadingObject* pShadingObj, const CFX_AffineMatrix* pObj2Device)
{
    FX_RECT rect = pShadingObj->GetBBox(pObj2Device);
    FX_RECT clip_box = m_pDevice->GetClipBox();
    rect.Intersect(clip_box);
    if (rect.IsEmpty()) {
        return TRUE;
    }
    CFX_AffineMatrix matrix = pShadingObj->m_Matrix;
    matrix.Concat(*pObj2Device);
    DrawShading(pShadingObj->m_pShading, &matrix, rect, pShadingObj->m_GeneralState.GetAlpha(FALSE),
                m_Options.m_ColorMode == RENDER_COLOR_ALPHA);
    return TRUE;
}
static CFX_DIBitmap* DrawPatternBitmap(CPDF_Document* pDoc, CPDF_PageRenderCache* pCache,
                                       CPDF_TilingPattern* pPattern, const CFX_AffineMatrix* pObject2Device,
                                       int width, int height, int flags)
{
    CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
    if (!pBitmap->Create(width, height, pPattern->m_bColored ? FXDIB_Argb : FXDIB_8bppMask)) {
        delete pBitmap;
        return NULL;
    }
    CFX_FxgeDevice bitmap_device;
    bitmap_device.Attach(pBitmap);
    pBitmap->Clear(0);
    CFX_FloatRect cell_bbox = pPattern->m_BBox;
    pPattern->m_Pattern2Form.TransformRect(cell_bbox);
    pObject2Device->TransformRect(cell_bbox);
    CFX_FloatRect bitmap_rect(0.0f, 0.0f, (FX_FLOAT)width, (FX_FLOAT)height);
    CFX_AffineMatrix mtAdjust;
    mtAdjust.MatchRect(bitmap_rect, cell_bbox);
    CFX_AffineMatrix mtPattern2Bitmap = *pObject2Device;
    mtPattern2Bitmap.Concat(mtAdjust);
    CPDF_RenderOptions options;
    if (!pPattern->m_bColored) {
        options.m_ColorMode = RENDER_COLOR_ALPHA;
    }
    flags |= RENDER_FORCE_HALFTONE;
    options.m_Flags = flags;
    CPDF_RenderContext context;
    context.Create(pDoc, pCache, NULL);
    context.DrawObjectList(&bitmap_device, pPattern->m_pForm, &mtPattern2Bitmap, &options);
    return pBitmap;
}
void CPDF_RenderStatus::DrawTilingPattern(CPDF_TilingPattern* pPattern, CPDF_PageObject* pPageObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStroke)
{
    if (!pPattern->Load()) {
        return;
    }
    m_pDevice->SaveState();
    if (pPageObj->m_Type == PDFPAGE_PATH) {
        if (!SelectClipPath((CPDF_PathObject*)pPageObj, pObj2Device, bStroke)) {
            m_pDevice->RestoreState();
            return;
        }
    } else if (pPageObj->m_Type == PDFPAGE_IMAGE) {
        FX_RECT rect = pPageObj->GetBBox(pObj2Device);
        m_pDevice->SetClip_Rect(&rect);
    } else {
        return;
    }
    FX_RECT clip_box = m_pDevice->GetClipBox();
    if (clip_box.IsEmpty()) {
        m_pDevice->RestoreState();
        return;
    }
    CFX_Matrix dCTM = m_pDevice->GetCTM();
    FX_FLOAT sa = FXSYS_fabs(dCTM.a);
    FX_FLOAT sd = FXSYS_fabs(dCTM.d);
    clip_box.right = clip_box.left + (FX_INT32)FXSYS_ceil(clip_box.Width() * sa);
    clip_box.bottom = clip_box.top + (FX_INT32)FXSYS_ceil(clip_box.Height() * sd);
    CFX_AffineMatrix mtPattern2Device = pPattern->m_Pattern2Form;
    mtPattern2Device.Concat(*pObj2Device);
    GetScaledMatrix(mtPattern2Device);
    FX_BOOL bAligned = FALSE;
    if (pPattern->m_BBox.left == 0 && pPattern->m_BBox.bottom == 0 &&
            pPattern->m_BBox.right == pPattern->m_XStep && pPattern->m_BBox.top == pPattern->m_YStep &&
            (mtPattern2Device.IsScaled() || mtPattern2Device.Is90Rotated())) {
        bAligned = TRUE;
    }
    CFX_FloatRect cell_bbox = pPattern->m_BBox;
    mtPattern2Device.TransformRect(cell_bbox);
    int width = (int)FXSYS_ceil(cell_bbox.Width());
    int height = (int)FXSYS_ceil(cell_bbox.Height());
    if (width == 0) {
        width = 1;
    }
    if (height == 0) {
        height = 1;
    }
    int min_col, max_col, min_row, max_row;
    CFX_AffineMatrix mtDevice2Pattern;
    mtDevice2Pattern.SetReverse(mtPattern2Device);
    CFX_FloatRect clip_box_p(clip_box);
    clip_box_p.Transform(&mtDevice2Pattern);
    min_col = (int)FXSYS_ceil(FXSYS_Div(clip_box_p.left - pPattern->m_BBox.right, pPattern->m_XStep));
    max_col = (int)FXSYS_floor(FXSYS_Div(clip_box_p.right - pPattern->m_BBox.left, pPattern->m_XStep));
    min_row = (int)FXSYS_ceil(FXSYS_Div(clip_box_p.bottom - pPattern->m_BBox.top, pPattern->m_YStep));
    max_row = (int)FXSYS_floor(FXSYS_Div(clip_box_p.top - pPattern->m_BBox.bottom, pPattern->m_YStep));
    if (width > clip_box.Width() || height > clip_box.Height() || width * height > clip_box.Width() * clip_box.Height()) {
        CPDF_GraphicStates* pStates = NULL;
        if (!pPattern->m_bColored) {
            pStates = CloneObjStates(pPageObj, bStroke);
        }
        CPDF_Dictionary* pFormResource = NULL;
        if (pPattern->m_pForm->m_pFormDict) {
            pFormResource = pPattern->m_pForm->m_pFormDict->GetDict(FX_BSTRC("Resources"));
        }
        for (int col = min_col; col <= max_col; col ++)
            for (int row = min_row; row <= max_row; row ++) {
                FX_FLOAT orig_x, orig_y;
                orig_x = col * pPattern->m_XStep;
                orig_y = row * pPattern->m_YStep;
                mtPattern2Device.Transform(orig_x, orig_y);
                CFX_AffineMatrix matrix = *pObj2Device;
                matrix.Translate(orig_x - mtPattern2Device.e, orig_y - mtPattern2Device.f);
                m_pDevice->SaveState();
                CPDF_RenderStatus status;
                status.Initialize(m_pContext, m_pDevice, NULL, NULL, this, pStates, &m_Options,
                                  pPattern->m_pForm->m_Transparency, m_bDropObjects, pFormResource);
                status.RenderObjectList(pPattern->m_pForm, &matrix);
                m_pDevice->RestoreState();
            }
        m_pDevice->RestoreState();
        if (pStates) {
            delete pStates;
        }
        return;
    }
    if (bAligned) {
        int orig_x = FXSYS_round(mtPattern2Device.e);
        int orig_y = FXSYS_round(mtPattern2Device.f);
        min_col = (clip_box.left - orig_x) / width;
        if (clip_box.left < orig_x) {
            min_col --;
        }
        max_col = (clip_box.right - orig_x) / width;
        if (clip_box.right <= orig_x) {
            max_col --;
        }
        min_row = (clip_box.top - orig_y) / height;
        if (clip_box.top < orig_y) {
            min_row --;
        }
        max_row = (clip_box.bottom - orig_y) / height;
        if (clip_box.bottom <= orig_y) {
            max_row --;
        }
    }
    FX_FLOAT left_offset = cell_bbox.left - mtPattern2Device.e;
    FX_FLOAT top_offset = cell_bbox.bottom - mtPattern2Device.f;
    CFX_DIBitmap* pPatternBitmap = NULL;
    if (width * height < 16) {
        CFX_DIBitmap* pEnlargedBitmap = DrawPatternBitmap(m_pContext->m_pDocument, m_pContext->m_pPageCache, pPattern, pObj2Device, 8, 8, m_Options.m_Flags);
        pPatternBitmap = pEnlargedBitmap->StretchTo(width, height);
        delete pEnlargedBitmap;
    } else {
        pPatternBitmap = DrawPatternBitmap(m_pContext->m_pDocument, m_pContext->m_pPageCache, pPattern, pObj2Device, width, height, m_Options.m_Flags);
    }
    if (pPatternBitmap == NULL) {
        m_pDevice->RestoreState();
        return;
    }
    if (m_Options.m_ColorMode == RENDER_COLOR_GRAY) {
        pPatternBitmap->ConvertColorScale(m_Options.m_ForeColor, m_Options.m_BackColor);
    }
    FX_ARGB fill_argb = GetFillArgb(pPageObj);
    int clip_width = clip_box.right - clip_box.left;
    int clip_height = clip_box.bottom - clip_box.top;
    CFX_DIBitmap screen;
    if (!screen.Create(clip_width, clip_height, FXDIB_Argb)) {
        return;
    }
    screen.Clear(0);
    FX_DWORD* src_buf = (FX_DWORD*)pPatternBitmap->GetBuffer();
    for (int col = min_col; col <= max_col; col ++) {
        for (int row = min_row; row <= max_row; row ++) {
            int start_x, start_y;
            if (bAligned) {
                start_x = FXSYS_round(mtPattern2Device.e) + col * width - clip_box.left;
                start_y = FXSYS_round(mtPattern2Device.f) + row * height - clip_box.top;
            } else {
                FX_FLOAT orig_x = col * pPattern->m_XStep;
                FX_FLOAT orig_y = row * pPattern->m_YStep;
                mtPattern2Device.Transform(orig_x, orig_y);
                start_x = FXSYS_round(orig_x + left_offset) - clip_box.left;
                start_y = FXSYS_round(orig_y + top_offset) - clip_box.top;
            }
            if (width == 1 && height == 1) {
                if (start_x < 0 || start_x >= clip_box.Width() || start_y < 0 || start_y >= clip_box.Height()) {
                    continue;
                }
                FX_DWORD* dest_buf = (FX_DWORD*)(screen.GetBuffer() + screen.GetPitch() * start_y + start_x * 4);
                if (pPattern->m_bColored) {
                    *dest_buf = *src_buf;
                } else {
                    *dest_buf = (*(FX_LPBYTE)src_buf << 24) | (fill_argb & 0xffffff);
                }
            } else {
                if (pPattern->m_bColored) {
                    screen.CompositeBitmap(start_x, start_y, width, height, pPatternBitmap, 0, 0);
                } else {
                    screen.CompositeMask(start_x, start_y, width, height, pPatternBitmap, fill_argb, 0, 0);
                }
            }
        }
    }
    CompositeDIBitmap(&screen, clip_box.left, clip_box.top, 0, 255, FXDIB_BLEND_NORMAL, FALSE);
    m_pDevice->RestoreState();
    delete pPatternBitmap;
}
void CPDF_RenderStatus::DrawPathWithPattern(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device, CPDF_Color* pColor, FX_BOOL bStroke)
{
    CPDF_Pattern* pattern = pColor->GetPattern();
    if (pattern == NULL) {
        return;
    }
    if(pattern->m_PatternType == PATTERN_TILING) {
        DrawTilingPattern((CPDF_TilingPattern*)pattern, pPathObj, pObj2Device, bStroke);
    } else {
        DrawShadingPattern((CPDF_ShadingPattern*)pattern, pPathObj, pObj2Device, bStroke);
    }
}
void CPDF_RenderStatus::ProcessPathPattern(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device, int& filltype, FX_BOOL& bStroke)
{
    if(filltype) {
        CPDF_Color& FillColor = *pPathObj->m_ColorState.GetFillColor();
        if(FillColor.m_pCS && FillColor.m_pCS->GetFamily() == PDFCS_PATTERN) {
            DrawPathWithPattern(pPathObj, pObj2Device, &FillColor, FALSE);
            filltype = 0;
        }
    }
    if(bStroke) {
        CPDF_Color& StrokeColor = *pPathObj->m_ColorState.GetStrokeColor();
        if(StrokeColor.m_pCS && StrokeColor.m_pCS->GetFamily() == PDFCS_PATTERN) {
            DrawPathWithPattern(pPathObj, pObj2Device, &StrokeColor, TRUE);
            bStroke = FALSE;
        }
    }
}
