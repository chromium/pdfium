// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fpdfdoc/fpdf_vt.h"
#include "pdf_vt.h"
IPDF_VariableText* IPDF_VariableText::NewVariableText()
{
    return new CPDF_VariableText();
}
void IPDF_VariableText::DelVariableText(IPDF_VariableText* pVT)
{
    delete (CPDF_VariableText*)pVT;
}
