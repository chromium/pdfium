// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/pageint.h"

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/edit/cpdf_creator.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_meshstream.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_shadingobject.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"

namespace {

struct PDF_AbbrPair {
  const FX_CHAR* abbr;
  const FX_CHAR* full_name;
};

const PDF_AbbrPair PDF_InlineKeyAbbr[] = {
    {"BPC", "BitsPerComponent"}, {"CS", "ColorSpace"}, {"D", "Decode"},
    {"DP", "DecodeParms"},       {"F", "Filter"},      {"H", "Height"},
    {"IM", "ImageMask"},         {"I", "Interpolate"}, {"W", "Width"},
};

const PDF_AbbrPair PDF_InlineValueAbbr[] = {
    {"G", "DeviceGray"},       {"RGB", "DeviceRGB"},
    {"CMYK", "DeviceCMYK"},    {"I", "Indexed"},
    {"AHx", "ASCIIHexDecode"}, {"A85", "ASCII85Decode"},
    {"LZW", "LZWDecode"},      {"Fl", "FlateDecode"},
    {"RL", "RunLengthDecode"}, {"CCF", "CCITTFaxDecode"},
    {"DCT", "DCTDecode"},
};

struct AbbrReplacementOp {
  bool is_replace_key;
  CFX_ByteString key;
  CFX_ByteStringC replacement;
};

CFX_ByteStringC PDF_FindFullName(const PDF_AbbrPair* table,
                                 size_t count,
                                 const CFX_ByteStringC& abbr) {
  auto it = std::find_if(
      table, table + count,
      [abbr](const PDF_AbbrPair& pair) { return pair.abbr == abbr; });
  return it != table + count ? CFX_ByteStringC(it->full_name)
                             : CFX_ByteStringC();
}

}  // namespace

CFX_ByteStringC PDF_FindKeyAbbreviationForTesting(const CFX_ByteStringC& abbr) {
  return PDF_FindFullName(PDF_InlineKeyAbbr, FX_ArraySize(PDF_InlineKeyAbbr),
                          abbr);
}

CFX_ByteStringC PDF_FindValueAbbreviationForTesting(
    const CFX_ByteStringC& abbr) {
  return PDF_FindFullName(PDF_InlineValueAbbr,
                          FX_ArraySize(PDF_InlineValueAbbr), abbr);
}

void PDF_ReplaceAbbr(CPDF_Object* pObj) {
  switch (pObj->GetType()) {
    case CPDF_Object::DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      std::vector<AbbrReplacementOp> replacements;
      for (const auto& it : *pDict) {
        CFX_ByteString key = it.first;
        CPDF_Object* value = it.second;
        CFX_ByteStringC fullname =
            PDF_FindFullName(PDF_InlineKeyAbbr, FX_ArraySize(PDF_InlineKeyAbbr),
                             key.AsStringC());
        if (!fullname.IsEmpty()) {
          AbbrReplacementOp op;
          op.is_replace_key = true;
          op.key = key;
          op.replacement = fullname;
          replacements.push_back(op);
          key = fullname;
        }

        if (value->IsName()) {
          CFX_ByteString name = value->GetString();
          fullname = PDF_FindFullName(PDF_InlineValueAbbr,
                                      FX_ArraySize(PDF_InlineValueAbbr),
                                      name.AsStringC());
          if (!fullname.IsEmpty()) {
            AbbrReplacementOp op;
            op.is_replace_key = false;
            op.key = key;
            op.replacement = fullname;
            replacements.push_back(op);
          }
        } else {
          PDF_ReplaceAbbr(value);
        }
      }
      for (const auto& op : replacements) {
        if (op.is_replace_key)
          pDict->ReplaceKey(op.key, CFX_ByteString(op.replacement));
        else
          pDict->SetNameFor(op.key, CFX_ByteString(op.replacement));
      }
      break;
    }
    case CPDF_Object::ARRAY: {
      CPDF_Array* pArray = pObj->AsArray();
      for (size_t i = 0; i < pArray->GetCount(); i++) {
        CPDF_Object* pElement = pArray->GetObjectAt(i);
        if (pElement->IsName()) {
          CFX_ByteString name = pElement->GetString();
          CFX_ByteStringC fullname = PDF_FindFullName(
              PDF_InlineValueAbbr, FX_ArraySize(PDF_InlineValueAbbr),
              name.AsStringC());
          if (!fullname.IsEmpty()) {
            pArray->SetAt(i, new CPDF_Name(CFX_ByteString(fullname)));
          }
        } else {
          PDF_ReplaceAbbr(pElement);
        }
      }
      break;
    }
    default:
      break;
  }
}
