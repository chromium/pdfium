// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/cpdf_hint_tables.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_data_avail.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream_acc.h"
#include "core/fxcrt/include/fx_safe_types.h"

namespace {

bool CanReadFromBitStream(const CFX_BitStream* hStream,
                          const FX_SAFE_UINT32& bits) {
  return bits.IsValid() && hStream->BitsRemaining() >= bits.ValueOrDie();
}

}  // namespace

CPDF_HintTables::CPDF_HintTables(CPDF_DataAvail* pDataAvail,
                                 CPDF_Dictionary* pLinearized)
    : m_pDataAvail(pDataAvail),
      m_pLinearizedDict(pLinearized),
      m_nFirstPageSharedObjs(0),
      m_szFirstPageObjOffset(0) {
  ASSERT(m_pLinearizedDict);
}

CPDF_HintTables::~CPDF_HintTables() {}

uint32_t CPDF_HintTables::GetItemLength(
    int index,
    const std::vector<FX_FILESIZE>& szArray) {
  if (index < 0 || szArray.size() < 2 ||
      static_cast<size_t>(index) > szArray.size() - 2 ||
      szArray[index] > szArray[index + 1]) {
    return 0;
  }
  return szArray[index + 1] - szArray[index];
}

bool CPDF_HintTables::ReadPageHintTable(CFX_BitStream* hStream) {
  if (!hStream || hStream->IsEOF())
    return false;

  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return false;

  const uint32_t kHeaderSize = 288;
  if (hStream->BitsRemaining() < kHeaderSize)
    return false;

  // Item 1: The least number of objects in a page.
  const uint32_t dwObjLeastNum = hStream->GetBits(32);
  if (!dwObjLeastNum)
    return FALSE;

  // Item 2: The location of the first page's page object.
  const uint32_t dwFirstObjLoc = hStream->GetBits(32);
  if (dwFirstObjLoc > static_cast<uint32_t>(nStreamOffset)) {
    FX_SAFE_UINT32 safeLoc = pdfium::base::checked_cast<uint32_t>(nStreamLen);
    safeLoc += dwFirstObjLoc;
    if (!safeLoc.IsValid())
      return false;
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(safeLoc.ValueOrDie());
  } else {
    m_szFirstPageObjOffset =
        pdfium::base::checked_cast<FX_FILESIZE>(dwFirstObjLoc);
  }

  // Item 3: The number of bits needed to represent the difference
  // between the greatest and least number of objects in a page.
  const uint32_t dwDeltaObjectsBits = hStream->GetBits(16);
  if (!dwDeltaObjectsBits)
    return FALSE;

  // Item 4: The least length of a page in bytes.
  const uint32_t dwPageLeastLen = hStream->GetBits(32);
  if (!dwPageLeastLen)
    return FALSE;

  // Item 5: The number of bits needed to represent the difference
  // between the greatest and least length of a page, in bytes.
  const uint32_t dwDeltaPageLenBits = hStream->GetBits(16);
  if (!dwDeltaPageLenBits)
    return FALSE;

  // Skip Item 6, 7, 8, 9 total 96 bits.
  hStream->SkipBits(96);

  // Item 10: The number of bits needed to represent the greatest
  // number of shared object references.
  const uint32_t dwSharedObjBits = hStream->GetBits(16);

  // Item 11: The number of bits needed to represent the numerically
  // greatest shared object identifier used by the pages.
  const uint32_t dwSharedIdBits = hStream->GetBits(16);
  if (!dwSharedObjBits)
    return FALSE;

  // Item 12: The number of bits needed to represent the numerator of
  // the fractional position for each shared object reference. For each
  // shared object referenced from a page, there is an indication of
  // where in the page's content stream the object is first referenced.
  const uint32_t dwSharedNumeratorBits = hStream->GetBits(16);
  if (!dwSharedIdBits)
    return FALSE;

  // Item 13: Skip Item 13 which has 16 bits.
  hStream->SkipBits(16);

  // Sanity check values from the page table header. 2^|kMaxBits| should be more
  // than enough to represent most of the values here.
  constexpr uint32_t kMaxBits = 34;
  if (dwSharedObjBits > kMaxBits || dwDeltaObjectsBits > kMaxBits ||
      dwDeltaPageLenBits > kMaxBits || dwSharedIdBits > kMaxBits) {
    return false;
  }

  const int nPages = GetNumberOfPages();
  if (nPages < 1 || nPages >= FPDF_PAGE_MAX_NUM)
    return false;

  const uint32_t dwPages = pdfium::base::checked_cast<uint32_t>(nPages);
  FX_SAFE_UINT32 required_bits = dwDeltaObjectsBits;
  required_bits *= dwPages;
  if (!CanReadFromBitStream(hStream, required_bits))
    return false;

  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_UINT32 safeDeltaObj = hStream->GetBits(dwDeltaObjectsBits);
    safeDeltaObj += dwObjLeastNum;
    if (!safeDeltaObj.IsValid())
      return false;
    m_dwDeltaNObjsArray.push_back(safeDeltaObj.ValueOrDie());
  }
  hStream->ByteAlign();

  required_bits = dwDeltaPageLenBits;
  required_bits *= dwPages;
  if (!CanReadFromBitStream(hStream, required_bits))
    return false;

  std::vector<uint32_t> dwPageLenArray;
  for (int i = 0; i < nPages; ++i) {
    FX_SAFE_UINT32 safePageLen = hStream->GetBits(dwDeltaPageLenBits);
    safePageLen += dwPageLeastLen;
    if (!safePageLen.IsValid())
      return false;

    dwPageLenArray.push_back(safePageLen.ValueOrDie());
  }

  int nOffsetE = GetEndOfFirstPageOffset();
  if (nOffsetE < 0)
    return false;

  int nFirstPageNum = GetFirstPageNumber();
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

  m_szPageOffsetArray.push_back(m_szPageOffsetArray[nPages - 1] +
                                dwPageLenArray[nPages - 1]);
  hStream->ByteAlign();

  // Number of shared objects.
  required_bits = dwSharedObjBits;
  required_bits *= dwPages;
  if (!CanReadFromBitStream(hStream, required_bits))
    return false;

  for (int i = 0; i < nPages; i++)
    m_dwNSharedObjsArray.push_back(hStream->GetBits(dwSharedObjBits));
  hStream->ByteAlign();

  // Array of identifiers, size = nshared_objects.
  for (int i = 0; i < nPages; i++) {
    required_bits = dwSharedIdBits;
    required_bits *= m_dwNSharedObjsArray[i];
    if (!CanReadFromBitStream(hStream, required_bits))
      return false;

    for (uint32_t j = 0; j < m_dwNSharedObjsArray[i]; j++)
      m_dwIdentifierArray.push_back(hStream->GetBits(dwSharedIdBits));
  }
  hStream->ByteAlign();

  for (int i = 0; i < nPages; i++) {
    FX_SAFE_UINT32 safeSize = m_dwNSharedObjsArray[i];
    safeSize *= dwSharedNumeratorBits;
    if (!CanReadFromBitStream(hStream, safeSize))
      return false;

    hStream->SkipBits(safeSize.ValueOrDie());
  }
  hStream->ByteAlign();

  FX_SAFE_UINT32 safeTotalPageLen = dwPages;
  safeTotalPageLen *= dwDeltaPageLenBits;
  if (!CanReadFromBitStream(hStream, safeTotalPageLen))
    return false;

  hStream->SkipBits(safeTotalPageLen.ValueOrDie());
  hStream->ByteAlign();
  return true;
}

bool CPDF_HintTables::ReadSharedObjHintTable(CFX_BitStream* hStream,
                                             uint32_t offset) {
  if (!hStream || hStream->IsEOF())
    return false;

  int nStreamOffset = ReadPrimaryHintStreamOffset();
  int nStreamLen = ReadPrimaryHintStreamLength();
  if (nStreamOffset < 0 || nStreamLen < 1)
    return false;

  FX_SAFE_UINT32 bit_offset = offset;
  bit_offset *= 8;
  if (!bit_offset.IsValid() || hStream->GetPos() > bit_offset.ValueOrDie())
    return false;
  hStream->SkipBits(bit_offset.ValueOrDie() - hStream->GetPos());

  const uint32_t kHeaderSize = 192;
  if (hStream->BitsRemaining() < kHeaderSize)
    return false;

  // Item 1: The object number of the first object in the shared objects
  // section.
  uint32_t dwFirstSharedObjNum = hStream->GetBits(32);

  // Item 2: The location of the first object in the shared objects section.
  uint32_t dwFirstSharedObjLoc = hStream->GetBits(32);
  if (dwFirstSharedObjLoc > static_cast<uint32_t>(nStreamOffset))
    dwFirstSharedObjLoc += nStreamLen;

  // Item 3: The number of shared object entries for the first page.
  m_nFirstPageSharedObjs = hStream->GetBits(32);

  // Item 4: The number of shared object entries for the shared objects
  // section, including the number of shared object entries for the first page.
  uint32_t dwSharedObjTotal = hStream->GetBits(32);

  // Item 5: The number of bits needed to represent the greatest number of
  // objects in a shared object group. Skipped.
  hStream->SkipBits(16);

  // Item 6: The least length of a shared object group in bytes.
  uint32_t dwGroupLeastLen = hStream->GetBits(32);

  // Item 7: The number of bits needed to represent the difference between the
  // greatest and least length of a shared object group, in bytes.
  uint32_t dwDeltaGroupLen = hStream->GetBits(16);

  int nFirstPageObjNum = GetFirstPageObjectNumber();
  if (nFirstPageObjNum < 0)
    return false;

  uint32_t dwPrevObjLen = 0;
  uint32_t dwCurObjLen = 0;
  FX_SAFE_UINT32 required_bits = dwSharedObjTotal;
  required_bits *= dwDeltaGroupLen;
  if (!CanReadFromBitStream(hStream, required_bits))
    return false;

  for (uint32_t i = 0; i < dwSharedObjTotal; ++i) {
    dwPrevObjLen = dwCurObjLen;
    FX_SAFE_UINT32 safeObjLen = hStream->GetBits(dwDeltaGroupLen);
    safeObjLen += dwGroupLeastLen;
    if (!safeObjLen.IsValid())
      return false;

    dwCurObjLen = safeObjLen.ValueOrDie();
    if (i < m_nFirstPageSharedObjs) {
      m_dwSharedObjNumArray.push_back(nFirstPageObjNum + i);
      if (i == 0)
        m_szSharedObjOffsetArray.push_back(m_szFirstPageObjOffset);
    } else {
      FX_SAFE_UINT32 safeObjNum = dwFirstSharedObjNum;
      safeObjNum += i - m_nFirstPageSharedObjs;
      if (!safeObjNum.IsValid())
        return false;

      m_dwSharedObjNumArray.push_back(safeObjNum.ValueOrDie());
      if (i == m_nFirstPageSharedObjs) {
        FX_SAFE_FILESIZE safeLoc = dwFirstSharedObjLoc;
        if (!safeLoc.IsValid())
          return false;

        m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
      }
    }

    if (i != 0 && i != m_nFirstPageSharedObjs) {
      FX_SAFE_FILESIZE safeLoc = dwPrevObjLen;
      safeLoc += m_szSharedObjOffsetArray[i - 1];
      if (!safeLoc.IsValid())
        return false;

      m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
    }
  }

  if (dwSharedObjTotal > 0) {
    FX_SAFE_FILESIZE safeLoc = dwCurObjLen;
    safeLoc += m_szSharedObjOffsetArray[dwSharedObjTotal - 1];
    if (!safeLoc.IsValid())
      return false;

    m_szSharedObjOffsetArray.push_back(safeLoc.ValueOrDie());
  }

  hStream->ByteAlign();
  if (hStream->BitsRemaining() < dwSharedObjTotal)
    return false;

  hStream->SkipBits(dwSharedObjTotal);
  hStream->ByteAlign();
  return true;
}

bool CPDF_HintTables::GetPagePos(int index,
                                 FX_FILESIZE* szPageStartPos,
                                 FX_FILESIZE* szPageLength,
                                 uint32_t* dwObjNum) {
  *szPageStartPos = m_szPageOffsetArray[index];
  *szPageLength = GetItemLength(index, m_szPageOffsetArray);

  int nFirstPageObjNum = GetFirstPageObjectNumber();
  if (nFirstPageObjNum < 0)
    return false;

  int nFirstPageNum = GetFirstPageNumber();
  if (index == nFirstPageNum) {
    *dwObjNum = nFirstPageObjNum;
    return true;
  }

  // The object number of remaining pages starts from 1.
  *dwObjNum = 1;
  for (int i = 0; i < index; ++i) {
    if (i == nFirstPageNum)
      continue;
    *dwObjNum += m_dwDeltaNObjsArray[i];
  }
  return true;
}

CPDF_DataAvail::DocAvailStatus CPDF_HintTables::CheckPage(
    int index,
    CPDF_DataAvail::DownloadHints* pHints) {
  if (!pHints)
    return CPDF_DataAvail::DataError;

  int nFirstAvailPage = GetFirstPageNumber();
  if (index == nFirstAvailPage)
    return CPDF_DataAvail::DataAvailable;

  uint32_t dwLength = GetItemLength(index, m_szPageOffsetArray);
  // If two pages have the same offset, it should be treated as an error.
  if (!dwLength)
    return CPDF_DataAvail::DataError;

  if (!m_pDataAvail->IsDataAvail(m_szPageOffsetArray[index], dwLength, pHints))
    return CPDF_DataAvail::DataNotAvailable;

  // Download data of shared objects in the page.
  uint32_t offset = 0;
  for (int i = 0; i < index; ++i)
    offset += m_dwNSharedObjsArray[i];

  int nFirstPageObjNum = GetFirstPageObjectNumber();
  if (nFirstPageObjNum < 0)
    return CPDF_DataAvail::DataError;

  uint32_t dwIndex = 0;
  uint32_t dwObjNum = 0;
  for (uint32_t j = 0; j < m_dwNSharedObjsArray[index]; ++j) {
    dwIndex = m_dwIdentifierArray[offset + j];
    if (dwIndex >= m_dwSharedObjNumArray.size())
      return CPDF_DataAvail::DataNotAvailable;

    dwObjNum = m_dwSharedObjNumArray[dwIndex];
    if (dwObjNum >= static_cast<uint32_t>(nFirstPageObjNum) &&
        dwObjNum <
            static_cast<uint32_t>(nFirstPageObjNum) + m_nFirstPageSharedObjs) {
      continue;
    }

    dwLength = GetItemLength(dwIndex, m_szSharedObjOffsetArray);
    // If two objects have the same offset, it should be treated as an error.
    if (!dwLength)
      return CPDF_DataAvail::DataError;

    if (!m_pDataAvail->IsDataAvail(m_szSharedObjOffsetArray[dwIndex], dwLength,
                                   pHints)) {
      return CPDF_DataAvail::DataNotAvailable;
    }
  }
  return CPDF_DataAvail::DataAvailable;
}

bool CPDF_HintTables::LoadHintStream(CPDF_Stream* pHintStream) {
  if (!pHintStream)
    return false;

  CPDF_Dictionary* pDict = pHintStream->GetDict();
  CPDF_Object* pOffset = pDict ? pDict->GetObjectBy("S") : nullptr;
  if (!pOffset || !pOffset->IsNumber())
    return false;

  int shared_hint_table_offset = pOffset->GetInteger();
  if (shared_hint_table_offset <= 0)
    return false;

  CPDF_StreamAcc acc;
  acc.LoadAllData(pHintStream);

  uint32_t size = acc.GetSize();
  // The header section of page offset hint table is 36 bytes.
  // The header section of shared object hint table is 24 bytes.
  // Hint table has at least 60 bytes.
  const uint32_t kMinStreamLength = 60;
  if (size < kMinStreamLength)
    return false;

  FX_SAFE_UINT32 safe_shared_hint_table_offset = shared_hint_table_offset;
  if (!safe_shared_hint_table_offset.IsValid() ||
      size < safe_shared_hint_table_offset.ValueOrDie()) {
    return false;
  }

  CFX_BitStream bs;
  bs.Init(acc.GetData(), size);
  return ReadPageHintTable(&bs) &&
         ReadSharedObjHintTable(&bs, shared_hint_table_offset);
}

int CPDF_HintTables::GetEndOfFirstPageOffset() const {
  CPDF_Object* pOffsetE = m_pLinearizedDict->GetDirectObjectBy("E");
  return pOffsetE ? pOffsetE->GetInteger() : -1;
}

int CPDF_HintTables::GetNumberOfPages() const {
  CPDF_Object* pPageNum = m_pLinearizedDict->GetDirectObjectBy("N");
  return pPageNum ? pPageNum->GetInteger() : 0;
}

int CPDF_HintTables::GetFirstPageObjectNumber() const {
  CPDF_Object* pFirstPageObj = m_pLinearizedDict->GetDirectObjectBy("O");
  return pFirstPageObj ? pFirstPageObj->GetInteger() : -1;
}

int CPDF_HintTables::GetFirstPageNumber() const {
  CPDF_Object* pFirstPageNum = m_pLinearizedDict->GetDirectObjectBy("P");
  return pFirstPageNum ? pFirstPageNum->GetInteger() : 0;
}

int CPDF_HintTables::ReadPrimaryHintStreamOffset() const {
  return ReadPrimaryHintStream(0);
}

int CPDF_HintTables::ReadPrimaryHintStreamLength() const {
  return ReadPrimaryHintStream(1);
}

int CPDF_HintTables::ReadPrimaryHintStream(int index) const {
  CPDF_Array* pRange = m_pLinearizedDict->GetArrayBy("H");
  if (!pRange)
    return -1;

  CPDF_Object* pStreamLen = pRange->GetDirectObjectAt(index);
  return pStreamLen ? pStreamLen->GetInteger() : -1;
}
