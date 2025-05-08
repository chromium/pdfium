// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_streamparser.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "constants/stream_dict_common.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_null.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcodec/data_and_bytes_consumed.h"
#include "core/fxcodec/jpeg/jpegmodule.h"
#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/calculate_pitch.h"

namespace {

const uint32_t kMaxNestedParsingLevel = 512;
const size_t kMaxStringLength = 32767;

const char kTrue[] = "true";
const char kFalse[] = "false";
const char kNull[] = "null";

uint32_t DecodeAllScanlines(std::unique_ptr<ScanlineDecoder> pDecoder) {
  if (!pDecoder) {
    return FX_INVALID_OFFSET;
  }

  int ncomps = pDecoder->CountComps();
  int bpc = pDecoder->GetBPC();
  int width = pDecoder->GetWidth();
  int height = pDecoder->GetHeight();
  if (width <= 0 || height <= 0) {
    return FX_INVALID_OFFSET;
  }

  std::optional<uint32_t> maybe_size =
      fxge::CalculatePitch8(bpc, ncomps, width);
  if (!maybe_size.has_value()) {
    return FX_INVALID_OFFSET;
  }

  FX_SAFE_UINT32 size = maybe_size.value();
  size *= height;
  if (size.ValueOrDefault(0) == 0) {
    return FX_INVALID_OFFSET;
  }

  for (int row = 0; row < height; ++row) {
    if (pDecoder->GetScanline(row).empty()) {
      break;
    }
  }
  return pDecoder->GetSrcOffset();
}

uint32_t DecodeInlineStream(pdfium::span<const uint8_t> src_span,
                            int width,
                            int height,
                            const ByteString& decoder,
                            RetainPtr<const CPDF_Dictionary> pParam,
                            uint32_t orig_size) {
  // |decoder| should not be an abbreviation.
  DCHECK(decoder != "A85");
  DCHECK(decoder != "AHx");
  DCHECK(decoder != "CCF");
  DCHECK(decoder != "DCT");
  DCHECK(decoder != "Fl");
  DCHECK(decoder != "LZW");
  DCHECK(decoder != "RL");

  if (decoder == "FlateDecode") {
    return FlateOrLZWDecode(/*use_lzw=*/false, src_span, pParam.Get(),
                            /*estimated_size=*/orig_size)
        .bytes_consumed;
  }
  if (decoder == "LZWDecode") {
    return FlateOrLZWDecode(
               /*use_lzw=*/true, src_span, pParam.Get(),
               /*estimated_size=*/0)
        .bytes_consumed;
  }
  if (decoder == "DCTDecode") {
    std::unique_ptr<ScanlineDecoder> pDecoder = JpegModule::CreateDecoder(
        src_span, width, height, 0,
        !pParam || pParam->GetIntegerFor("ColorTransform", 1));
    return DecodeAllScanlines(std::move(pDecoder));
  }
  if (decoder == "CCITTFaxDecode") {
    std::unique_ptr<ScanlineDecoder> pDecoder =
        CreateFaxDecoder(src_span, width, height, pParam.Get());
    return DecodeAllScanlines(std::move(pDecoder));
  }

  if (decoder == "ASCII85Decode") {
    return A85Decode(src_span).bytes_consumed;
  }
  if (decoder == "ASCIIHexDecode") {
    return HexDecode(src_span).bytes_consumed;
  }
  if (decoder == "RunLengthDecode") {
    return RunLengthDecode(src_span).bytes_consumed;
  }

  return FX_INVALID_OFFSET;
}

}  // namespace

CPDF_StreamParser::CPDF_StreamParser(pdfium::span<const uint8_t> span)
    : buf_(span) {}

CPDF_StreamParser::CPDF_StreamParser(pdfium::span<const uint8_t> span,
                                     const WeakPtr<ByteStringPool>& pPool)
    : pool_(pPool), buf_(span) {}

CPDF_StreamParser::~CPDF_StreamParser() = default;

RetainPtr<CPDF_Stream> CPDF_StreamParser::ReadInlineStream(
    CPDF_Document* pDoc,
    RetainPtr<CPDF_Dictionary> pDict,
    const CPDF_Object* pCSObj) {
  auto stream_span = buf_.subspan(pos_);
  if (stream_span.empty()) {
    return nullptr;
  }

  if (PDFCharIsWhitespace(stream_span.front())) {
    pos_++;
    stream_span = stream_span.subspan<1>();
    if (stream_span.empty()) {
      return nullptr;
    }
  }

  ByteString decoder;
  RetainPtr<const CPDF_Dictionary> param_dict;
  RetainPtr<const CPDF_Object> filter = pDict->GetDirectObjectFor("Filter");
  if (filter) {
    const CPDF_Array* array = filter->AsArray();
    if (array) {
      decoder = array->GetByteStringAt(0);
      RetainPtr<const CPDF_Array> params =
          pDict->GetArrayFor(pdfium::stream::kDecodeParms);
      if (params) {
        param_dict = params->GetDictAt(0);
      }
    } else {
      decoder = filter->GetString();
      param_dict = pDict->GetDictFor(pdfium::stream::kDecodeParms);
    }
  }
  uint32_t width = pDict->GetIntegerFor("Width");
  uint32_t height = pDict->GetIntegerFor("Height");
  uint32_t bpc = 1;
  uint32_t nComponents = 1;
  if (pCSObj) {
    RetainPtr<CPDF_ColorSpace> pCS =
        CPDF_DocPageData::FromDocument(pDoc)->GetColorSpace(pCSObj, nullptr);
    nComponents = pCS ? pCS->ComponentCount() : 3;
    bpc = pDict->GetIntegerFor("BitsPerComponent");
  }
  std::optional<uint32_t> maybe_size =
      fxge::CalculatePitch8(bpc, nComponents, width);
  if (!maybe_size.has_value()) {
    return nullptr;
  }

  FX_SAFE_UINT32 size = maybe_size.value();
  size *= height;
  if (!size.IsValid()) {
    return nullptr;
  }

  uint32_t original_size = size.ValueOrDie();
  DataVector<uint8_t> data;
  uint32_t actual_stream_size;
  if (decoder.IsEmpty()) {
    original_size = std::min<uint32_t>(original_size, stream_span.size());
    auto src_span = stream_span.first(original_size);
    data = DataVector<uint8_t>(src_span.begin(), src_span.end());
    actual_stream_size = original_size;
    pos_ += original_size;
  } else {
    actual_stream_size =
        DecodeInlineStream(stream_span, width, height, decoder,
                           std::move(param_dict), original_size);
    if (!pdfium::IsValueInRangeForNumericType<int>(actual_stream_size)) {
      return nullptr;
    }

    {
      AutoRestorer<uint32_t> saved_position(&pos_);
      pos_ += actual_stream_size;
      while (true) {
        uint32_t saved_iteration_position = pos_;
        ElementType type = ParseNextElement();
        if (type == ElementType::kEndOfData) {
          return nullptr;
        }

        if (type == ElementType::kKeyword && GetWord() == "EI") {
          break;
        }

        actual_stream_size += pos_ - saved_iteration_position;
      }
    }
    auto src_span = stream_span.first(actual_stream_size);
    data = DataVector<uint8_t>(src_span.begin(), src_span.end());
    pos_ += actual_stream_size;
  }
  pDict->SetNewFor<CPDF_Number>("Length", static_cast<int>(actual_stream_size));
  return pdfium::MakeRetain<CPDF_Stream>(std::move(data), std::move(pDict));
}

CPDF_StreamParser::ElementType CPDF_StreamParser::ParseNextElement() {
  last_obj_.Reset();
  word_size_ = 0;
  if (!PositionIsInBounds()) {
    return ElementType::kEndOfData;
  }

  uint8_t ch = buf_[pos_++];
  while (true) {
    while (PDFCharIsWhitespace(ch)) {
      if (!PositionIsInBounds()) {
        return ElementType::kEndOfData;
      }

      ch = buf_[pos_++];
    }

    if (ch != '%') {
      break;
    }

    while (true) {
      if (!PositionIsInBounds()) {
        return ElementType::kEndOfData;
      }

      ch = buf_[pos_++];
      if (PDFCharIsLineEnding(ch)) {
        break;
      }
    }
  }

  if (PDFCharIsDelimiter(ch) && ch != '/') {
    pos_--;
    last_obj_ = ReadNextObject(false, false, 0);
    return ElementType::kOther;
  }

  bool bIsNumber = true;
  while (true) {
    if (word_size_ < kMaxWordLength) {
      word_buffer_[word_size_++] = ch;
    }

    if (!PDFCharIsNumeric(ch)) {
      bIsNumber = false;
    }

    if (!PositionIsInBounds()) {
      break;
    }

    ch = buf_[pos_++];

    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      pos_--;
      break;
    }
  }

  word_buffer_[word_size_] = 0;
  if (bIsNumber) {
    return ElementType::kNumber;
  }

  if (word_buffer_[0] == '/') {
    return ElementType::kName;
  }

  if (word_size_ == 4) {
    if (GetWord() == kTrue) {
      last_obj_ = pdfium::MakeRetain<CPDF_Boolean>(true);
      return ElementType::kOther;
    }
    if (GetWord() == kNull) {
      last_obj_ = pdfium::MakeRetain<CPDF_Null>();
      return ElementType::kOther;
    }
  } else if (word_size_ == 5) {
    if (GetWord() == kFalse) {
      last_obj_ = pdfium::MakeRetain<CPDF_Boolean>(false);
      return ElementType::kOther;
    }
  }
  return ElementType::kKeyword;
}

RetainPtr<CPDF_Object> CPDF_StreamParser::ReadNextObject(
    bool bAllowNestedArray,
    bool bInArray,
    uint32_t dwRecursionLevel) {
  bool bIsNumber;
  // Must get the next word before returning to avoid infinite loops.
  GetNextWord(bIsNumber);
  if (!word_size_ || dwRecursionLevel > kMaxNestedParsingLevel) {
    return nullptr;
  }

  if (bIsNumber) {
    word_buffer_[word_size_] = 0;
    return pdfium::MakeRetain<CPDF_Number>(GetWord());
  }

  int first_char = word_buffer_[0];
  if (first_char == '/') {
    ByteString name = PDF_NameDecode(GetWord().Substr(1));
    return pdfium::MakeRetain<CPDF_Name>(pool_, name);
  }

  if (first_char == '(') {
    return pdfium::MakeRetain<CPDF_String>(pool_, ReadString());
  }

  if (first_char == '<') {
    if (word_size_ == 1) {
      return pdfium::MakeRetain<CPDF_String>(pool_, ReadHexString(),
                                             CPDF_String::DataType::kIsHex);
    }

    auto pDict = pdfium::MakeRetain<CPDF_Dictionary>(pool_);
    while (true) {
      GetNextWord(bIsNumber);
      if (word_size_ == 2 && word_buffer_[0] == '>') {
        break;
      }

      if (!word_size_ || word_buffer_[0] != '/') {
        return nullptr;
      }

      ByteString key = PDF_NameDecode(GetWord().Substr(1));
      RetainPtr<CPDF_Object> pObj =
          ReadNextObject(true, bInArray, dwRecursionLevel + 1);
      if (!pObj) {
        return nullptr;
      }

      pDict->SetFor(key, std::move(pObj));
    }
    return pDict;
  }

  if (first_char == '[') {
    if ((!bAllowNestedArray && bInArray)) {
      return nullptr;
    }

    auto pArray = pdfium::MakeRetain<CPDF_Array>();
    while (true) {
      RetainPtr<CPDF_Object> pObj =
          ReadNextObject(bAllowNestedArray, true, dwRecursionLevel + 1);
      if (pObj) {
        pArray->Append(std::move(pObj));
        continue;
      }
      if (!word_size_ || word_buffer_[0] == ']') {
        break;
      }
    }
    return pArray;
  }

  if (GetWord() == kFalse) {
    return pdfium::MakeRetain<CPDF_Boolean>(false);
  }
  if (GetWord() == kTrue) {
    return pdfium::MakeRetain<CPDF_Boolean>(true);
  }
  if (GetWord() == kNull) {
    return pdfium::MakeRetain<CPDF_Null>();
  }
  return nullptr;
}

// TODO(npm): the following methods are almost identical in cpdf_syntaxparser
void CPDF_StreamParser::GetNextWord(bool& bIsNumber) {
  word_size_ = 0;
  bIsNumber = true;
  if (!PositionIsInBounds()) {
    return;
  }

  uint8_t ch = buf_[pos_++];
  while (true) {
    while (PDFCharIsWhitespace(ch)) {
      if (!PositionIsInBounds()) {
        return;
      }
      ch = buf_[pos_++];
    }

    if (ch != '%') {
      break;
    }

    while (true) {
      if (!PositionIsInBounds()) {
        return;
      }
      ch = buf_[pos_++];
      if (PDFCharIsLineEnding(ch)) {
        break;
      }
    }
  }

  if (PDFCharIsDelimiter(ch)) {
    bIsNumber = false;
    word_buffer_[word_size_++] = ch;
    if (ch == '/') {
      while (true) {
        if (!PositionIsInBounds()) {
          return;
        }
        ch = buf_[pos_++];
        if (!PDFCharIsOther(ch) && !PDFCharIsNumeric(ch)) {
          pos_--;
          return;
        }
        if (word_size_ < kMaxWordLength) {
          word_buffer_[word_size_++] = ch;
        }
      }
    } else if (ch == '<') {
      if (!PositionIsInBounds()) {
        return;
      }
      ch = buf_[pos_++];
      if (ch == '<') {
        word_buffer_[word_size_++] = ch;
      } else {
        pos_--;
      }
    } else if (ch == '>') {
      if (!PositionIsInBounds()) {
        return;
      }
      ch = buf_[pos_++];
      if (ch == '>') {
        word_buffer_[word_size_++] = ch;
      } else {
        pos_--;
      }
    }
    return;
  }

  while (true) {
    if (word_size_ < kMaxWordLength) {
      word_buffer_[word_size_++] = ch;
    }
    if (!PDFCharIsNumeric(ch)) {
      bIsNumber = false;
    }
    if (!PositionIsInBounds()) {
      return;
    }

    ch = buf_[pos_++];
    if (PDFCharIsDelimiter(ch) || PDFCharIsWhitespace(ch)) {
      pos_--;
      break;
    }
  }
}

ByteString CPDF_StreamParser::ReadString() {
  if (!PositionIsInBounds()) {
    return ByteString();
  }

  ByteString buf;
  int parlevel = 0;
  int status = 0;
  int iEscCode = 0;
  uint8_t ch = buf_[pos_++];
  while (true) {
    switch (status) {
      case 0:
        if (ch == ')') {
          if (parlevel == 0) {
            return buf.First(std::min(buf.GetLength(), kMaxStringLength));
          }
          parlevel--;
          buf += ')';
        } else if (ch == '(') {
          parlevel++;
          buf += '(';
        } else if (ch == '\\') {
          status = 1;
        } else {
          buf += static_cast<char>(ch);
        }
        break;
      case 1:
        if (FXSYS_IsOctalDigit(ch)) {
          iEscCode = FXSYS_DecimalCharToInt(static_cast<char>(ch));
          status = 2;
          break;
        }
        if (ch == '\r') {
          status = 4;
          break;
        }
        if (ch == '\n') {
          // Do nothing.
        } else if (ch == 'n') {
          buf += '\n';
        } else if (ch == 'r') {
          buf += '\r';
        } else if (ch == 't') {
          buf += '\t';
        } else if (ch == 'b') {
          buf += '\b';
        } else if (ch == 'f') {
          buf += '\f';
        } else {
          buf += static_cast<char>(ch);
        }
        status = 0;
        break;
      case 2:
        if (FXSYS_IsOctalDigit(ch)) {
          iEscCode =
              iEscCode * 8 + FXSYS_DecimalCharToInt(static_cast<char>(ch));
          status = 3;
        } else {
          buf += static_cast<char>(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 3:
        if (FXSYS_IsOctalDigit(ch)) {
          iEscCode =
              iEscCode * 8 + FXSYS_DecimalCharToInt(static_cast<char>(ch));
          buf += static_cast<char>(iEscCode);
          status = 0;
        } else {
          buf += static_cast<char>(iEscCode);
          status = 0;
          continue;
        }
        break;
      case 4:
        status = 0;
        if (ch != '\n') {
          continue;
        }
        break;
    }
    if (!PositionIsInBounds()) {
      return buf.First(std::min(buf.GetLength(), kMaxStringLength));
    }

    ch = buf_[pos_++];
  }
}

DataVector<uint8_t> CPDF_StreamParser::ReadHexString() {
  if (!PositionIsInBounds()) {
    return DataVector<uint8_t>();
  }

  // TODO(thestig): Deduplicate CPDF_SyntaxParser::ReadHexString()?
  DataVector<uint8_t> buf;
  bool bFirst = true;
  uint8_t code = 0;
  while (PositionIsInBounds()) {
    uint8_t ch = buf_[pos_++];
    if (ch == '>') {
      break;
    }

    if (!FXSYS_IsHexDigit(ch)) {
      continue;
    }

    int val = FXSYS_HexCharToInt(ch);
    if (bFirst) {
      code = val * 16;
    } else {
      code += val;
      buf.push_back(code);
    }
    bFirst = !bFirst;
  }
  if (!bFirst) {
    buf.push_back(code);
  }

  if (buf.size() > kMaxStringLength) {
    buf.resize(kMaxStringLength);
  }
  return buf;
}

bool CPDF_StreamParser::PositionIsInBounds() const {
  return pos_ < buf_.size();
}
