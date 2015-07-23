// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_HUFFMAN_TABLE_STANDARD_H_
#define _JBIG2_HUFFMAN_TABLE_STANDARD_H_
typedef struct {
    int PREFLEN;
    int RANDELEN;
    int RANGELOW;
} JBig2TableLine;
const FX_BOOL HuffmanTable_HTOOB_B1 = FALSE;
const JBig2TableLine HuffmanTable_B1[] = {
    { 1,	4,		0 },
    { 2,	8,	   16 },
    { 3,   16,	  272 },
    { 0,   32,	   -1 },
    { 3,   32,	65808 }
};
const FX_BOOL HuffmanTable_HTOOB_B2 = TRUE;
const JBig2TableLine HuffmanTable_B2[] = {
    { 1,	0,		0 },
    { 2,	0,		1 },
    { 3,	0,		2 },
    { 4,	3,		3 },
    { 5,	6,	   11 },
    { 0,   32,	   -1 },
    { 6,   32,     75 },
    { 6,	0,      0 }
};
const FX_BOOL HuffmanTable_HTOOB_B3 = TRUE;
const JBig2TableLine HuffmanTable_B3[] = {
    { 8,	8,   -256 },
    { 1,	0,		0 },
    { 2,	0,		1 },
    { 3,	0,		2 },
    { 4,	3,		3 },
    { 5,	6,	   11 },
    { 8,   32,   -257 },
    { 7,   32,     75 },
    { 6,	0,		0 }
};
const FX_BOOL HuffmanTable_HTOOB_B4 = FALSE;
const JBig2TableLine HuffmanTable_B4[] = {
    { 1,	0,		1 },
    { 2,	0,		2 },
    { 3,	0,		3 },
    { 4,	3,		4 },
    { 5,	6,	   12 },
    { 0,   32,     -1 },
    { 5,   32,	   76 },
};
const FX_BOOL HuffmanTable_HTOOB_B5 = FALSE;
const JBig2TableLine HuffmanTable_B5[] = {
    { 7,	8,	 -255 },
    { 1,	0,	    1 },
    { 2,	0,	    2 },
    { 3,    0,      3 },
    { 4,	3,	    4 },
    { 5,	6,	   12 },
    { 7,   32,   -256 },
    { 6,   32,     76 }
};
const FX_BOOL HuffmanTable_HTOOB_B6 = FALSE;
const JBig2TableLine HuffmanTable_B6[] = {
    { 5,   10,	-2048 },
    { 4,    9,	-1024 },
    { 4,	8,	 -512 },
    { 4,	7,	 -256 },
    { 5,	6,	 -128 },
    { 5,	5,	  -64 },
    { 4,	5,	  -32 },
    { 2,	7,		0 },
    { 3,	7,	  128 },
    { 3,	8,	  256 },
    { 4,	9,	  512 },
    { 4,   10,	 1024 },
    { 6,   32,	-2049 },
    { 6,   32,	 2048 }
};
const FX_BOOL HuffmanTable_HTOOB_B7 = FALSE;
const JBig2TableLine HuffmanTable_B7[] = {
    { 4,	9,	-1024 },
    { 3,	8,	 -512 },
    { 4,	7,	 -256 },
    { 5,	6,	 -128 },
    { 5,	5,	  -64 },
    { 4,	5,	  -32 },
    { 4,	5,		0 },
    { 5,	5,	   32 },
    { 5,	6,	   64 },
    { 4,	7,	  128 },
    { 3,	8,	  256 },
    { 3,	9,	  512 },
    { 3,   10,	 1024 },
    { 5,   32,	-1025 },
    { 5,   32,	 2048 },
};
const FX_BOOL HuffmanTable_HTOOB_B8 = TRUE;
const JBig2TableLine HuffmanTable_B8[] = {
    { 8,	3,	  -15 },
    { 9,	1,	   -7 },
    { 8,	1,	   -5 },
    { 9,	0,	   -3 },
    { 7,	0,	   -2 },
    { 4,	0,	   -1 },
    { 2,	1,		0 },
    { 5,	0,		2 },
    { 6,	0,		3 },
    { 3,	4,		4 },
    { 6,	1,	   20 },
    { 4,	4,	   22 },
    { 4,	5,	   38 },
    { 5,	6,	   70 },
    { 5,	7,	  134 },
    { 6,	7,	  262 },
    { 7,	8,	  390 },
    { 6,   10,	  646 },
    { 9,   32,	  -16 },
    { 9,   32,	 1670 },
    { 2,	0,		0 }
};
const FX_BOOL HuffmanTable_HTOOB_B9 = TRUE;
const JBig2TableLine HuffmanTable_B9[] = {
    { 8,	4,	  -31 },
    { 9,	2,	  -15 },
    { 8,	2,	  -11 },
    { 9,	1,	   -7 },
    { 7,	1,	   -5 },
    { 4,	1,	   -3 },
    { 3,	1,	   -1 },
    { 3,	1,		1 },
    { 5,	1,		3 },
    { 6,	1,		5 },
    { 3,	5,		7 },
    { 6,	2,	   39 },
    { 4,	5,	   43 },
    { 4,	6,	   75 },
    { 5,	7,	  139 },
    { 5,	8,	  267 },
    { 6,	8,	  523 },
    { 7,	9,	  779 },
    { 6,   11,	 1291 },
    { 9,   32,	  -32 },
    { 9,   32,	 3339 },
    { 2,	0,		0 }
};
const FX_BOOL HuffmanTable_HTOOB_B10 = TRUE;
const JBig2TableLine HuffmanTable_B10[] = {
    { 7,	4,	  -21 },
    { 8,	0,	   -5 },
    { 7,	0,	   -4 },
    { 5,	0,	   -3 },
    { 2,	2,	   -2 },
    { 5,	0,		2 },
    { 6,	0,		3 },
    { 7,	0,		4 },
    { 8,	0,		5 },
    { 2,	6,		6 },
    { 5,	5,	   70 },
    { 6,	5,	  102 },
    { 6,	6,	  134 },
    { 6,	7,	  198 },
    { 6,	8,	  326 },
    { 6,	9,	  582 },
    { 6,   10,	 1094 },
    { 7,   11,	 2118 },
    { 8,   32,	  -22 },
    { 8,   32,	 4166 },
    { 2,	0,		0 }
};
const FX_BOOL HuffmanTable_HTOOB_B11 = FALSE;
const JBig2TableLine HuffmanTable_B11[] = {
    { 1,	0,		1 },
    { 2,	1,		2 },
    { 4,	0,		4 },
    { 4,	1,		5 },
    { 5,	1,		7 },
    { 5,	2,		9 },
    { 6,	2,	   13 },
    { 7,	2,	   17 },
    { 7,	3,	   21 },
    { 7,	4,	   29 },
    { 7,	5,	   45 },
    { 7,	6,	   77 },
    { 0,   32,		0 },
    { 7,   32,	  141 }
};
const FX_BOOL HuffmanTable_HTOOB_B12 = FALSE;
const JBig2TableLine HuffmanTable_B12[] = {
    { 1,	0,		1 },
    { 2,	0,		2 },
    { 3,	1,		3 },
    { 5,	0,		5 },
    { 5,	1,		6 },
    { 6,	1,		8 },
    { 7,	0,	   10 },
    { 7,	1,	   11 },
    { 7,	2,	   13 },
    { 7,	3,	   17 },
    { 7,	4,	   25 },
    { 8,	5,	   41 },
    { 0,   32,		0 },
    { 8,   32,	   73 }
};
const FX_BOOL HuffmanTable_HTOOB_B13 = FALSE;
const JBig2TableLine HuffmanTable_B13[] = {
    { 1,	0,		1 },
    { 3,	0,		2 },
    { 4,	0,		3 },
    { 5,	0,		4 },
    { 4,	1,		5 },
    { 3,	3,		7 },
    { 6,	1,	   15 },
    { 6,	2,	   17 },
    { 6,	3,	   21 },
    { 6,	4,	   29 },
    { 6,	5,	   45 },
    { 7,	6,	   77 },
    { 0,   32,		0 },
    { 7,   32,	  141 }
};
const FX_BOOL HuffmanTable_HTOOB_B14 = FALSE;
const JBig2TableLine HuffmanTable_B14[] = {
    { 3,	0,	   -2 },
    { 3,	0,	   -1 },
    { 1,	0,		0 },
    { 3,	0,		1 },
    { 3,	0,		2 },
    { 0,   32,	   -3 },
    { 0,   32,		3 }
};
const FX_BOOL HuffmanTable_HTOOB_B15 = FALSE;
const JBig2TableLine HuffmanTable_B15[] = {
    { 7,	4,	  -24 },
    { 6,	2,	   -8 },
    { 5,	1,	   -4 },
    { 4,	0,	   -2 },
    { 3,	0,	   -1 },
    { 1,	0,		0 },
    { 3,	0,		1 },
    { 4,	0,		2 },
    { 5,	1,		3 },
    { 6,	2,		5 },
    { 7,	4,		9 },
    { 7,   32,	  -25 },
    { 7,   32,	   25 }
};
#endif
