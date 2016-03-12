// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/src/fpdfapi/fpdf_parser/cpdf_hint_tables.h"

#include "core/include/fxcrt/fx_safe_types.h"
#include "core/include/fpdfapi/cpdf_array.h"
#include "core/include/fpdfapi/cpdf_dictionary.h"
#include "core/include/fpdfapi/cpdf_stream.h"
#include "core/src/fpdfapi/fpdf_parser/cpdf_data_avail.h"

namespace {

bool CanReadFromBitStream(const CFX_BitStream* hStream,
                          const FX_SAFE_DWORD& num_bits) {
  return num_bits.IsValid() &&
         hStream->BitsRemaining() >= num_bits.ValueOrDie();
}

}  // namespace

CPDF_HintTables::~CPDF_HintTables() {
  m_dwDeltaNObjsArray.RemoveAll();
  m_dwNSharedObjsArray.RemoveAll();
  m_dwSharedObjNumArray.RemoveAll();
  m_dwIdentifierArray.RemoveAll();
}

FX_DWORD CPDF_HintTables::GetItemLength(
    int index,
    const std::vector<FX_FILESIZE>& szArray) {
  if (index < 0 || szArray.size() < 2 ||
      static_cast<size_t>(index) > szArray.size() - 2 ||
      szArray[index] > szArray[index + 1]) {
    return 0;
  }
  return szArray[index + 1] - szArray[index];
}

FX_BOOL CPDF_HintTables::ReadPageHintTable(CFX_BitStream* hStream) {
  if (!hStream || hStream->IsEOF())
    return FALSE;

  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return FALSE;

  const FX_DWORD kHeaderSize = 288;
  if (hStream->BitsRemaining() < kHeaderSize)
    return FALSE;

  // Item 1: The least number of objects in a page.
  FX_DWORD dwObjLeastNum = hStream->GetBits(32);

  // Item 2: The location of the first page's page object.
  FX_DWORD dwFirstObjLoc = hStream->GetBits(32);
  if (dwFirstObjLoc > nStreamOffset) {
    FX_SAFE_DWORD safeLoc = pdfium::base::checked_cast<FX_DWORD>(nStreamLen);
    safeLoc += dwFirstObjLoc;
    if (!safeLoc.IsValid())
      return FALSE;
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(safeLoc.ValueOrDie());
  } else {
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(dwFirstObjLoc);
  }

  // Item 3: The number of bits needed to represent the difference
  // between the greatest and least number of objects in a page.
  FX_DWORD dwDeltaObjectsBits = hStream->GetBits(16);

  // Item 4: The least length of a page in bytes.
  FX_DWORD dwPageLeastLen = hStream->GetBits(32);

  // Item 5: The number of bits needed to represent the difference
  // between the greatest and least length of a page, in bytes.
  FX_DWORD dwDeltaPageLenBits = hStream->GetBits(16);

  // Skip Item 6, 7, 8, 9 total 96 bits.
  hStream->SkipBits(96);

  // Item 10: The number of bits needed to represent the greatest
  // number of shared object references.
  FX_DWORD dwSharedObjBits = hStream->GetBits(16);

  // Item 11: The number of bits needed to represent the numerically
  // greatest shared object identifier used by the pages.
  FX_DWORD dwSharedIdBits = hStream->GetBits(16);

  // Item 12: The number of bits needed to represent the numerator of
  // the fractional position for each shared object reference. For each
  // shared object referenced from a page, there is an indication of
  // where in the page's content stream the object is first referenced.
  FX_DWORD dwSharedNumeratorBits = hStream->GetBits(16);

  // Item 13: Skip Item 13 which has 16 bits.
  hStream->SkipBits(16);

  CPDF_Object* pPageNum = m_pLinearizedDict->GetElementValue("N");
  int nPages = pPageNum ? pPageNum->GetInteger() : 0;
  if (nPages < 1)
    return FALSE;

  FX_SAFE_DWORD required_bits = dwDeltaObjectsBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_DWORD safeDeltaObj = hStream->GetBits(dwDeltaObjectsBits);
    safeDeltaObj += dwObjLeastNum;
    if (!safeDeltaObj.IsValid())
      return FALSE;
    m_dwDeltaNObjsArray.Add(safeDeltaObj.ValueOrDie());
  }
  hStream->ByteAlign();

  required_bits = dwDeltaPageLenBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  CFX_DWordArray dwPageLenArray;
  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_DWORD safePageLen = hStream->GetBits(dwDeltaPageLenBits);
    safePageLen += dwPageLeastLen;
    if (!safePageLen.IsValid())
      return FALSE;
    dwPageLenArray.Add(safePageLen.ValueOrDie());
  }

  CPDF_Object* pOffsetE = m_pLinearizedDict->GetElementValue("E");
  int nOffsetE = pOffsetE ? pOffsetE->GetInteger() : -1;
  if (nOffsetE < 0)
    return FALSE;

  CPDF_Object* pFirstPageNum = m_pLinearizedDict->GetElementValue("P");
  int nFirstPageNum = pFirstPageNum ? pFirstPageNum->GetInteger() : 0;
  for (int i = 0; i < nPages; ++i) {
    if (i == nFirstPageNum) {
      m_szPageOffsetArray.push_back(m_szFirstPageObjOffset);
    } else if (i == nFirstPageNum + 1) {
      if (i == 1) {
        m_szPageOffsetArray.push_back(nOffsetE);
      } else {
        m_szPageOffsetArray.push_back(m_szPageOffsetArray[i - 2] +
                                      dwPageLenArray[i - 2]);
      }
    } else {
      if (i == 0) {
        m_szPageOffsetArray.push_back(nOffsetE);
      } else {
        m_szPageOffsetArray.push_back(m_szPageOffsetArray[i - 1] +
                                      dwPageLenArray[i - 1]);
      }
    }
  }

  if (nPages > 0) {
    m_szPageOffsetArray.push_back(m_szPageOffsetArray[nPages - 1] +
                                  dwPageLenArray[nPages - 1]);
  }
  hStream->ByteAlign();

  // Number of shared objects.
  required_bits = dwSharedObjBits;
  required_bits *= pdfium::base::checked_cast<FX_DWORD>(nPages);
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  for (int i = 0; i < nPages; i++)
    m_dwNSharedObjsArray.Add(hStream->GetBits(dwSharedObjBits));
  hStream->ByteAlign();

  // Array of identifiers, size = nshared_objects.
  for (int i = 0; i < nPages; i++) {
    required_bits = dwSharedIdBits;
    required_bits *= m_dwNSharedObjsArray[i];
    if (!CanReadFromBitStream(hStream, required_bits))
      return FALSE;

    for (int j = 0; j < m_dwNSharedObjsArray[i]; j++)
      m_dwIdentifierArray.Add(hStream->GetBits(dwSharedIdBits));
  }
  hStream->ByteAlign();

  for (int i = 0; i < nPages; i++) {
    FX_SAFE_DWORD safeSize = m_dwNSharedObjsArray[i];
    safeSize *= dwSharedNumeratorBits;
    if (!CanReadFromBitStream(hStream, safeSize))
      return FALSE;

    hStream->SkipBits(safeSize.ValueOrDie());
  }
  hStream->ByteAlign();

  FX_SAFE_DWORD safeTotalPageLen = pdfium::base::checked_cast<FX_DWORD>(nPages);
  safeTotalPageLen *= dwDeltaPageLenBits;
  if (!CanReadFromBitStream(hStream, safeTotalPageLen))
    return FALSE;

  hStream->SkipBits(safeTotalPageLen.ValueOrDie());
  hStream->ByteAlign();
  return TRUE;
}

FX_BOOL CPDF_HintTables::ReadSharedObjHintTable(CFX_BitStream* hStream,
                                                FX_DWORD offset) {
  if (!hStream || hStream->IsEOF())
    return FALSE;

  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return FALSE;

  FX_SAFE_DWORD bit_offset = offset;
  bit_offset *= 8;
  if (!bit_offset.IsValid() || hStream->GetPos() > bit_offset.ValueOrDie())
    return FALSE;
  hStream->SkipBits(bit_offset.ValueOrDie() - hStream->GetPos());

  const FX_DWORD kHeaderSize = 192;
  if (hStream->BitsRemaining() < kHeaderSize)
    return FALSE;

  // Item 1: The object number of the first object in the shared objects
  // section.
  FX_DWORD dwFirstSharedObjNum = hStream->GetBits(32);

  // Item 2: The location of the first object in the shared objects section.
  FX_DWORD dwFirstSharedObjLoc = hStream->GetBits(32);
  if (dwFirstSharedObjLoc > nStreamOffset)
    dwFirstSharedObjLoc += nStreamLen;

  // Item 3: The number of shared object entries for the first page.
  m_nFirstPageSharedObjs = hStream->GetBits(32);

  // Item 4: The number of shared object entries for the shared objects
  // section, including the number of shared object entries for the first page.
  FX_DWORD dwSharedObjTotal = hStream->GetBits(32);

  // Item 5: The number of bits needed to represent the greatest number of
  // objects in a shared object group. Skipped.
  hStream->SkipBits(16);

  // Item 6: The least length of a shared object group in bytes.
  FX_DWORD dwGroupLeastLen = hStream->GetBits(32);

  // Item 7: The number of bits needed to represent the difference between the
  // greatest and least length of a shared object group, in bytes.
  FX_DWORD dwDeltaGroupLen = hStream->GetBits(16);
  CPDF_Object* pFirstPageObj = m_pLinearizedDict->GetElementValue("O");
  int nFirstPageObjNum = pFirstPageObj ? pFirstPageObj->GetInteger() : -1;
  if (nFirstPageObjNum < 0)
    return FALSE;

  FX_DWORD dwPrevObjLen = 0;
  FX_DWORD dwCurObjLen = 0;
  FX_SAFE_DWORD required_bits = dwSharedObjTotal;
  required_bits *= dwDeltaGroupLen;
  if (!CanReadFromBitStream(hStream, required_bits))
    return FALSE;

  for (FX_DWORD i = 0; i < dwSharedObjTotal; ++i) {
    dwPrevObjLen = dwCurObjLen;
    FX_SAFE_DWORD safeObjLen = hStream->GetBits(dwDeltaGroupLen);
    safeObjLen += dwGroupLeastLen;
    if (!safeObjLen.IsValid())
      return FALSE;

    dwCurObjLen = safeObjLen.ValueOrDie();
    if (i < m_nFirstPageSharedObjs) {
      m_dwSharedObjNumArray.Add(nFirstPageObjNum + i);
      if (i == 0)
        m_szSharedObjOffsetArray.push_back(m_szFirstPageObjOffset);
    } else {
      FX_SAFE_DWORD safeObjNum = dwFirstSharedObjNum;
      safeObjNum += i - m_nFirstPageSharedObjs;
      if (!safeObjNum.IsValid())
        return FALSE;

      m_dwSharedObjNumArray.Add(safeObjNum.ValueOrDie());
      if (i == m_nFirstPageSharedObjs) {
        m_szSharedObjOffsetArray.push_back(
            pdfium::base::checked_cast<int32_t>(dwFirstSharedObjLoc));
      }
    }

    if (i != 0 && i != m_nFirstPageSharedObjs) {
      FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwPrevObjLen);
      safeLoc += m_szSharedObjOffsetArray[i - 1];
      if (!safeLoc.IsValid())
        return FALSE;

      m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
    }
  }

  if (dwSharedObjTotal > 0) {
    FX_SAFE_INT32 safeLoc = pdfium::base::checked_cast<int32_t>(dwCurObjLen);
    safeLoc += m_szSharedObjOffsetArray[dwSharedObjTotal - 1];
    if (!safeLoc.IsValid())
      return FALSE;

    m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
  }

  hStream->ByteAlign();
  if (hStream->BitsRemaining() < dwSharedObjTotal)
    return FALSE;

  hStream->SkipBits(dwSharedObjTotal);
  hStream->ByteAlign();
  return TRUE;
}

FX_BOOL CPDF_HintTables::GetPagePos(int index,
                                    FX_FILESIZE& szPageStartPos,
                                    FX_FILESIZE& szPageLength,
                                    FX_DWORD& dwObjNum) {
  if (!m_pLinearizedDict)
    return FALSE;

  szPageStartPos = m_szPageOffsetArray[index];
  szPageLength = GetItemLength(index, m_szPageOffsetArray);

  CPDF_Object* pFirstPageNum = m_pLinearizedDict->GetElementValue("P");
  int nFirstPageNum = pFirstPageNum ? pFirstPageNum->GetInteger() : 0;

  CPDF_Object* pFirstPageObjNum = m_pLinearizedDict->GetElementValue("O");
  if (!pFirstPageObjNum)
    return FALSE;

  int nFirstPageObjNum = pFirstPageObjNum->GetInteger();
  if (index == nFirstPageNum) {
    dwObjNum = nFirstPageObjNum;
    return TRUE;
  }

  // The object number of remaining pages starts from 1.
  dwObjNum = 1;
  for (int i = 0; i < index; ++i) {
    if (i == nFirstPageNum)
      continue;
    dwObjNum += m_dwDeltaNObjsArray[i];
  }
  return TRUE;
}

IPDF_DataAvail::DocAvailStatus CPDF_HintTables::CheckPage(
    int index,
    IPDF_DataAvail::DownloadHints* pHints) {
  if (!m_pLinearizedDict || !pHints)
    return IPDF_DataAvail::DataError;

  CPDF_Object* pFirstAvailPage = m_pLinearizedDict->GetElementValue("P");
  int nFirstAvailPage = pFirstAvailPage ? pFirstAvailPage->GetInteger() : 0;
  if (index == nFirstAvailPage)
    return IPDF_DataAvail::DataAvailable;

  FX_DWORD dwLength = GetItemLength(index, m_szPageOffsetArray);
  // If two pages have the same offset, it should be treated as an error.
  if (!dwLength)
    return IPDF_DataAvail::DataError;

  if (!m_pDataAvail->IsDataAvail(m_szPageOffsetArray[index], dwLength, pHints))
    return IPDF_DataAvail::DataNotAvailable;

  // Download data of shared objects in the page.
  FX_DWORD offset = 0;
  for (int i = 0; i < index; ++i)
    offset += m_dwNSharedObjsArray[i];

  CPDF_Object* pFirstPageObj = m_pLinearizedDict->GetElementValue("O");
  int nFirstPageObjNum = pFirstPageObj ? pFirstPageObj->GetInteger() : -1;
  if (nFirstPageObjNum < 0)
    return IPDF_DataAvail::DataError;

  FX_DWORD dwIndex = 0;
  FX_DWORD dwObjNum = 0;
  for (int j = 0; j < m_dwNSharedObjsArray[index]; ++j) {
    dwIndex = m_dwIdentifierArray[offset + j];
    if (dwIndex >= m_dwSharedObjNumArray.GetSize())
      return IPDF_DataAvail::DataNotAvailable;

    dwObjNum = m_dwSharedObjNumArray[dwIndex];
    if (dwObjNum >= nFirstPageObjNum &&
        dwObjNum < nFirstPageObjNum + m_nFirstPageSharedObjs) {
      continue;
    }

    dwLength = GetItemLength(dwIndex, m_szSharedObjOffsetArray);
    // If two objects have the same offset, it should be treated as an error.
    if (!dwLength)
      return IPDF_DataAvail::DataError;

    if (!m_pDataAvail->IsDataAvail(m_szSharedObjOffsetArray[dwIndex], dwLength,
                                   pHints)) {
      return IPDF_DataAvail::DataNotAvailable;
    }
  }
  return IPDF_DataAvail::DataAvailable;
}

FX_BOOL CPDF_HintTables::LoadHintStream(CPDF_Stream* pHintStream) {
  if (!pHintStream || !m_pLinearizedDict)
    return FALSE;

  CPDF_Dictionary* pDict = pHintStream->GetDict();
  CPDF_Object* pOffset = pDict ? pDict->GetElement("S") : nullptr;
  if (!pOffset || !pOffset->IsNumber())
    return FALSE;

  int shared_hint_table_offset = pOffset->GetInteger();
  CPDF_StreamAcc acc;
  acc.LoadAllData(pHintStream);

  FX_DWORD size = acc.GetSize();
  // The header section of page offset hint table is 36 bytes.
  // The header section of shared object hint table is 24 bytes.
  // Hint table has at least 60 bytes.
  const FX_DWORD MIN_STREAM_LEN = 60;
  if (size < MIN_STREAM_LEN || shared_hint_table_offset <= 0 ||
      size < shared_hint_table_offset) {
    return FALSE;
  }

  CFX_BitStream bs;
  bs.Init(acc.GetData(), size);
  return ReadPageHintTable(&bs) &&
         ReadSharedObjHintTable(&bs, pdfium::base::checked_cast<FX_DWORD>(
                                         shared_hint_table_offset));
}

int CPDF_HintTables::ReadPrimaryHintStreamOffset() const {
  if (!m_pLinearizedDict)
    return -1;

  CPDF_Array* pRange = m_pLinearizedDict->GetArrayBy("H");
  if (!pRange)
    return -1;

  CPDF_Object* pStreamOffset = pRange->GetElementValue(0);
  if (!pStreamOffset)
    return -1;

  return pStreamOffset->GetInteger();
}

int CPDF_HintTables::ReadPrimaryHintStreamLength() const {
  if (!m_pLinearizedDict)
    return -1;

  CPDF_Array* pRange = m_pLinearizedDict->GetArrayBy("H");
  if (!pRange)
    return -1;

  CPDF_Object* pStreamLen = pRange->GetElementValue(1);
  if (!pStreamLen)
    return -1;

  return pStreamLen->GetInteger();
}
