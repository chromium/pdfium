// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_resource.h"
#include "fpdfsdk/include/fxedit/fx_edit.h"
#include "fpdfsdk/include/fxedit/fxet_edit.h"

CFX_ByteString GetPDFWordString(IFX_Edit_FontMap* pFontMap,
                                int32_t nFontIndex,
                                FX_WORD Word,
                                FX_WORD SubWord) {
  CFX_ByteString sWord;
  if (CPDF_Font* pPDFFont = pFontMap->GetPDFFont(nFontIndex)) {
    if (SubWord > 0) {
      Word = SubWord;
    } else {
      FX_DWORD dwCharCode = -1;

      if (pPDFFont->IsUnicodeCompatible())
        dwCharCode = pPDFFont->CharCodeFromUnicode(Word);
      else
        dwCharCode = pFontMap->CharCodeFromUnicode(nFontIndex, Word);

      if (dwCharCode > 0) {
        pPDFFont->AppendChar(sWord, dwCharCode);
        return sWord;
      }
    }

    pPDFFont->AppendChar(sWord, Word);
  }

  return sWord;
}

static CFX_ByteString GetWordRenderString(const CFX_ByteString& strWords) {
  if (strWords.GetLength() > 0)
    return PDF_EncodeString(strWords) + " Tj\n";

  return "";
}

static CFX_ByteString GetFontSetString(IFX_Edit_FontMap* pFontMap,
                                       int32_t nFontIndex,
                                       FX_FLOAT fFontSize) {
  CFX_ByteTextBuf sRet;

  if (pFontMap) {
    CFX_ByteString sFontAlias = pFontMap->GetPDFFontAlias(nFontIndex);

    if (sFontAlias.GetLength() > 0 && fFontSize > 0)
      sRet << "/" << sFontAlias << " " << fFontSize << " Tf\n";
  }

  return sRet.GetByteString();
}

CFX_ByteString IFX_Edit::GetEditAppearanceStream(
    IFX_Edit* pEdit,
    const CPDF_Point& ptOffset,
    const CPVT_WordRange* pRange /* = NULL*/,
    FX_BOOL bContinuous /* = TRUE*/,
    FX_WORD SubWord /* = 0*/) {
  CFX_ByteTextBuf sEditStream, sWords;

  CPDF_Point ptOld(0.0f, 0.0f), ptNew(0.0f, 0.0f);
  int32_t nCurFontIndex = -1;

  if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator()) {
    if (pRange)
      pIterator->SetAt(pRange->BeginPos);
    else
      pIterator->SetAt(0);

    CPVT_WordPlace oldplace;

    while (pIterator->NextWord()) {
      CPVT_WordPlace place = pIterator->GetAt();

      if (pRange && place.WordCmp(pRange->EndPos) > 0)
        break;

      if (bContinuous) {
        if (place.LineCmp(oldplace) != 0) {
          if (sWords.GetSize() > 0) {
            sEditStream << GetWordRenderString(sWords.GetByteString());
            sWords.Clear();
          }

          CPVT_Word word;
          if (pIterator->GetWord(word)) {
            ptNew = CPDF_Point(word.ptWord.x + ptOffset.x,
                               word.ptWord.y + ptOffset.y);
          } else {
            CPVT_Line line;
            pIterator->GetLine(line);
            ptNew = CPDF_Point(line.ptLine.x + ptOffset.x,
                               line.ptLine.y + ptOffset.y);
          }

          if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
            sEditStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y
                        << " Td\n";

            ptOld = ptNew;
          }
        }

        CPVT_Word word;
        if (pIterator->GetWord(word)) {
          if (word.nFontIndex != nCurFontIndex) {
            if (sWords.GetSize() > 0) {
              sEditStream << GetWordRenderString(sWords.GetByteString());
              sWords.Clear();
            }
            sEditStream << GetFontSetString(pEdit->GetFontMap(),
                                            word.nFontIndex, word.fFontSize);
            nCurFontIndex = word.nFontIndex;
          }

          sWords << GetPDFWordString(pEdit->GetFontMap(), nCurFontIndex,
                                     word.Word, SubWord);
        }

        oldplace = place;
      } else {
        CPVT_Word word;
        if (pIterator->GetWord(word)) {
          ptNew = CPDF_Point(word.ptWord.x + ptOffset.x,
                             word.ptWord.y + ptOffset.y);

          if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
            sEditStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y
                        << " Td\n";
            ptOld = ptNew;
          }

          if (word.nFontIndex != nCurFontIndex) {
            sEditStream << GetFontSetString(pEdit->GetFontMap(),
                                            word.nFontIndex, word.fFontSize);
            nCurFontIndex = word.nFontIndex;
          }

          sEditStream << GetWordRenderString(GetPDFWordString(
              pEdit->GetFontMap(), nCurFontIndex, word.Word, SubWord));
        }
      }
    }

    if (sWords.GetSize() > 0) {
      sEditStream << GetWordRenderString(sWords.GetByteString());
      sWords.Clear();
    }
  }

  CFX_ByteTextBuf sAppStream;
  if (sEditStream.GetSize() > 0) {
    int32_t nHorzScale = pEdit->GetHorzScale();
    if (nHorzScale != 100) {
      sAppStream << nHorzScale << " Tz\n";
    }

    FX_FLOAT fCharSpace = pEdit->GetCharSpace();
    if (!FX_EDIT_IsFloatZero(fCharSpace)) {
      sAppStream << fCharSpace << " Tc\n";
    }

    sAppStream << sEditStream;
  }

  return sAppStream.GetByteString();
}

CFX_ByteString IFX_Edit::GetSelectAppearanceStream(
    IFX_Edit* pEdit,
    const CPDF_Point& ptOffset,
    const CPVT_WordRange* pRange) {
  CFX_ByteTextBuf sRet;

  if (pRange && pRange->IsExist()) {
    if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator()) {
      pIterator->SetAt(pRange->BeginPos);

      while (pIterator->NextWord()) {
        CPVT_WordPlace place = pIterator->GetAt();

        if (pRange && place.WordCmp(pRange->EndPos) > 0)
          break;

        CPVT_Word word;
        CPVT_Line line;
        if (pIterator->GetWord(word) && pIterator->GetLine(line)) {
          sRet << word.ptWord.x + ptOffset.x << " "
               << line.ptLine.y + line.fLineDescent << " " << word.fWidth << " "
               << line.fLineAscent - line.fLineDescent << " re\nf\n";
        }
      }
    }
  }

  return sRet.GetByteString();
}
