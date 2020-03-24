// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_formcalc_context.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>

#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_random.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/crt/cfgas_decimal.h"
#include "xfa/fgas/crt/locale_iface.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fm2js/cxfa_fmparser.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/xfa_utils.h"

using pdfium::fxjse::kClassTag;
using pdfium::fxjse::kFuncTag;

namespace {

const double kFinancialPrecision = 0.00000001;

const wchar_t kStrCode[] = L"0123456789abcdef";

struct XFA_FMHtmlReserveCode {
  uint32_t m_uCode;
  const char* m_htmlReserve;
};

// Sorted by |m_htmlReserve|.
const XFA_FMHtmlReserveCode kReservesForDecode[] = {
    {198, "AElig"},   {193, "Aacute"},   {194, "Acirc"},    {192, "Agrave"},
    {913, "Alpha"},   {197, "Aring"},    {195, "Atilde"},   {196, "Auml"},
    {914, "Beta"},    {199, "Ccedil"},   {935, "Chi"},      {8225, "Dagger"},
    {916, "Delta"},   {208, "ETH"},      {201, "Eacute"},   {202, "Ecirc"},
    {200, "Egrave"},  {917, "Epsilon"},  {919, "Eta"},      {203, "Euml"},
    {915, "Gamma"},   {922, "Kappa"},    {923, "Lambda"},   {924, "Mu"},
    {209, "Ntilde"},  {925, "Nu"},       {338, "OElig"},    {211, "Oacute"},
    {212, "Ocirc"},   {210, "Ograve"},   {937, "Omega"},    {927, "Omicron"},
    {216, "Oslash"},  {213, "Otilde"},   {214, "Ouml"},     {934, "Phi"},
    {928, "Pi"},      {936, "Psi"},      {929, "Rho"},      {352, "Scaron"},
    {931, "Sigma"},   {222, "THORN"},    {932, "Tau"},      {920, "Theta"},
    {218, "Uacute"},  {219, "Ucirc"},    {217, "Ugrave"},   {933, "Upsilon"},
    {220, "Uuml"},    {926, "Xi"},       {221, "Yacute"},   {376, "Yuml"},
    {918, "Zeta"},    {225, "aacute"},   {226, "acirc"},    {180, "acute"},
    {230, "aelig"},   {224, "agrave"},   {8501, "alefsym"}, {945, "alpha"},
    {38, "amp"},      {8743, "and"},     {8736, "ang"},     {39, "apos"},
    {229, "aring"},   {8776, "asymp"},   {227, "atilde"},   {228, "auml"},
    {8222, "bdquo"},  {946, "beta"},     {166, "brvbar"},   {8226, "bull"},
    {8745, "cap"},    {231, "ccedil"},   {184, "cedil"},    {162, "cent"},
    {967, "chi"},     {710, "circ"},     {9827, "clubs"},   {8773, "cong"},
    {169, "copy"},    {8629, "crarr"},   {8746, "cup"},     {164, "current"},
    {8659, "dArr"},   {8224, "dagger"},  {8595, "darr"},    {176, "deg"},
    {948, "delta"},   {9830, "diams"},   {247, "divide"},   {233, "eacute"},
    {234, "ecirc"},   {232, "egrave"},   {8709, "empty"},   {8195, "emsp"},
    {8194, "ensp"},   {949, "epsilon"},  {8801, "equiv"},   {951, "eta"},
    {240, "eth"},     {235, "euml"},     {8364, "euro"},    {8707, "exist"},
    {402, "fnof"},    {8704, "forall"},  {189, "frac12"},   {188, "frac14"},
    {190, "frac34"},  {8260, "frasl"},   {947, "gamma"},    {8805, "ge"},
    {62, "gt"},       {8660, "hArr"},    {8596, "harr"},    {9829, "hearts"},
    {8230, "hellip"}, {237, "iacute"},   {238, "icirc"},    {161, "iexcl"},
    {236, "igrave"},  {8465, "image"},   {8734, "infin"},   {8747, "int"},
    {953, "iota"},    {191, "iquest"},   {8712, "isin"},    {239, "iuml"},
    {954, "kappa"},   {8656, "lArr"},    {205, "lacute"},   {955, "lambda"},
    {9001, "lang"},   {171, "laquo"},    {8592, "larr"},    {8968, "lceil"},
    {206, "lcirc"},   {8220, "ldquo"},   {8804, "le"},      {8970, "lfloor"},
    {204, "lgrave"},  {921, "lota"},     {8727, "lowast"},  {9674, "loz"},
    {8206, "lrm"},    {8249, "lsaquo"},  {8216, "lsquo"},   {60, "lt"},
    {207, "luml"},    {175, "macr"},     {8212, "mdash"},   {181, "micro"},
    {183, "middot"},  {8722, "minus"},   {956, "mu"},       {8711, "nabla"},
    {160, "nbsp"},    {8211, "ndash"},   {8800, "ne"},      {8715, "ni"},
    {172, "not"},     {8713, "notin"},   {8836, "nsub"},    {241, "ntilde"},
    {957, "nu"},      {243, "oacute"},   {244, "ocirc"},    {339, "oelig"},
    {242, "ograve"},  {8254, "oline"},   {969, "omega"},    {959, "omicron"},
    {8853, "oplus"},  {8744, "or"},      {170, "ordf"},     {186, "ordm"},
    {248, "oslash"},  {245, "otilde"},   {8855, "otimes"},  {246, "ouml"},
    {182, "para"},    {8706, "part"},    {8240, "permil"},  {8869, "perp"},
    {966, "phi"},     {960, "pi"},       {982, "piv"},      {177, "plusmn"},
    {8242, "prime"},  {8719, "prod"},    {8733, "prop"},    {968, "psi"},
    {163, "pund"},    {34, "quot"},      {8658, "rArr"},    {8730, "radic"},
    {9002, "rang"},   {187, "raquo"},    {8594, "rarr"},    {8969, "rceil"},
    {8476, "real"},   {174, "reg"},      {8971, "rfloor"},  {961, "rho"},
    {8207, "rlm"},    {8250, "rsaquo"},  {8217, "rsquo"},   {353, "saron"},
    {8218, "sbquo"},  {8901, "sdot"},    {167, "sect"},     {173, "shy"},
    {963, "sigma"},   {962, "sigmaf"},   {8764, "sim"},     {9824, "spades"},
    {8834, "sub"},    {8838, "sube"},    {8721, "sum"},     {8835, "sup"},
    {185, "sup1"},    {178, "sup2"},     {179, "sup3"},     {8839, "supe"},
    {223, "szlig"},   {964, "tau"},      {8221, "tdquo"},   {8756, "there4"},
    {952, "theta"},   {977, "thetasym"}, {8201, "thinsp"},  {254, "thorn"},
    {732, "tilde"},   {215, "times"},    {8482, "trade"},   {8657, "uArr"},
    {250, "uacute"},  {8593, "uarr"},    {251, "ucirc"},    {249, "ugrave"},
    {168, "uml"},     {978, "upsih"},    {965, "upsilon"},  {252, "uuml"},
    {8472, "weierp"}, {958, "xi"},       {253, "yacute"},   {165, "yen"},
    {255, "yuml"},    {950, "zeta"},     {8205, "zwj"},     {8204, "zwnj"},
};

// Sorted by |m_uCode|.
const XFA_FMHtmlReserveCode kReservesForEncode[] = {
    {34, "quot"},     {38, "amp"},      {39, "apos"},      {60, "lt"},
    {62, "gt"},       {160, "nbsp"},    {161, "iexcl"},    {162, "cent"},
    {163, "pund"},    {164, "current"}, {165, "yen"},      {166, "brvbar"},
    {167, "sect"},    {168, "uml"},     {169, "copy"},     {170, "ordf"},
    {171, "laquo"},   {172, "not"},     {173, "shy"},      {174, "reg"},
    {175, "macr"},    {176, "deg"},     {177, "plusmn"},   {178, "sup2"},
    {179, "sup3"},    {180, "acute"},   {181, "micro"},    {182, "para"},
    {183, "middot"},  {184, "cedil"},   {185, "sup1"},     {186, "ordm"},
    {187, "raquo"},   {188, "frac14"},  {189, "frac12"},   {190, "frac34"},
    {191, "iquest"},  {192, "Agrave"},  {193, "Aacute"},   {194, "Acirc"},
    {195, "Atilde"},  {196, "Auml"},    {197, "Aring"},    {198, "AElig"},
    {199, "Ccedil"},  {200, "Egrave"},  {201, "Eacute"},   {202, "Ecirc"},
    {203, "Euml"},    {204, "lgrave"},  {205, "lacute"},   {206, "lcirc"},
    {207, "luml"},    {208, "ETH"},     {209, "Ntilde"},   {210, "Ograve"},
    {211, "Oacute"},  {212, "Ocirc"},   {213, "Otilde"},   {214, "Ouml"},
    {215, "times"},   {216, "Oslash"},  {217, "Ugrave"},   {218, "Uacute"},
    {219, "Ucirc"},   {220, "Uuml"},    {221, "Yacute"},   {222, "THORN"},
    {223, "szlig"},   {224, "agrave"},  {225, "aacute"},   {226, "acirc"},
    {227, "atilde"},  {228, "auml"},    {229, "aring"},    {230, "aelig"},
    {231, "ccedil"},  {232, "egrave"},  {233, "eacute"},   {234, "ecirc"},
    {235, "euml"},    {236, "igrave"},  {237, "iacute"},   {238, "icirc"},
    {239, "iuml"},    {240, "eth"},     {241, "ntilde"},   {242, "ograve"},
    {243, "oacute"},  {244, "ocirc"},   {245, "otilde"},   {246, "ouml"},
    {247, "divide"},  {248, "oslash"},  {249, "ugrave"},   {250, "uacute"},
    {251, "ucirc"},   {252, "uuml"},    {253, "yacute"},   {254, "thorn"},
    {255, "yuml"},    {338, "OElig"},   {339, "oelig"},    {352, "Scaron"},
    {353, "saron"},   {376, "Yuml"},    {402, "fnof"},     {710, "circ"},
    {732, "tilde"},   {913, "Alpha"},   {914, "Beta"},     {915, "Gamma"},
    {916, "Delta"},   {917, "Epsilon"}, {918, "Zeta"},     {919, "Eta"},
    {920, "Theta"},   {921, "lota"},    {922, "Kappa"},    {923, "Lambda"},
    {924, "Mu"},      {925, "Nu"},      {926, "Xi"},       {927, "Omicron"},
    {928, "Pi"},      {929, "Rho"},     {931, "Sigma"},    {932, "Tau"},
    {933, "Upsilon"}, {934, "Phi"},     {935, "Chi"},      {936, "Psi"},
    {937, "Omega"},   {945, "alpha"},   {946, "beta"},     {947, "gamma"},
    {948, "delta"},   {949, "epsilon"}, {950, "zeta"},     {951, "eta"},
    {952, "theta"},   {953, "iota"},    {954, "kappa"},    {955, "lambda"},
    {956, "mu"},      {957, "nu"},      {958, "xi"},       {959, "omicron"},
    {960, "pi"},      {961, "rho"},     {962, "sigmaf"},   {963, "sigma"},
    {964, "tau"},     {965, "upsilon"}, {966, "phi"},      {967, "chi"},
    {968, "psi"},     {969, "omega"},   {977, "thetasym"}, {978, "upsih"},
    {982, "piv"},     {8194, "ensp"},   {8195, "emsp"},    {8201, "thinsp"},
    {8204, "zwnj"},   {8205, "zwj"},    {8206, "lrm"},     {8207, "rlm"},
    {8211, "ndash"},  {8212, "mdash"},  {8216, "lsquo"},   {8217, "rsquo"},
    {8218, "sbquo"},  {8220, "ldquo"},  {8221, "tdquo"},   {8222, "bdquo"},
    {8224, "dagger"}, {8225, "Dagger"}, {8226, "bull"},    {8230, "hellip"},
    {8240, "permil"}, {8242, "prime"},  {8249, "lsaquo"},  {8250, "rsaquo"},
    {8254, "oline"},  {8260, "frasl"},  {8364, "euro"},    {8465, "image"},
    {8472, "weierp"}, {8476, "real"},   {8482, "trade"},   {8501, "alefsym"},
    {8592, "larr"},   {8593, "uarr"},   {8594, "rarr"},    {8595, "darr"},
    {8596, "harr"},   {8629, "crarr"},  {8656, "lArr"},    {8657, "uArr"},
    {8658, "rArr"},   {8659, "dArr"},   {8660, "hArr"},    {8704, "forall"},
    {8706, "part"},   {8707, "exist"},  {8709, "empty"},   {8711, "nabla"},
    {8712, "isin"},   {8713, "notin"},  {8715, "ni"},      {8719, "prod"},
    {8721, "sum"},    {8722, "minus"},  {8727, "lowast"},  {8730, "radic"},
    {8733, "prop"},   {8734, "infin"},  {8736, "ang"},     {8743, "and"},
    {8744, "or"},     {8745, "cap"},    {8746, "cup"},     {8747, "int"},
    {8756, "there4"}, {8764, "sim"},    {8773, "cong"},    {8776, "asymp"},
    {8800, "ne"},     {8801, "equiv"},  {8804, "le"},      {8805, "ge"},
    {8834, "sub"},    {8835, "sup"},    {8836, "nsub"},    {8838, "sube"},
    {8839, "supe"},   {8853, "oplus"},  {8855, "otimes"},  {8869, "perp"},
    {8901, "sdot"},   {8968, "lceil"},  {8969, "rceil"},   {8970, "lfloor"},
    {8971, "rfloor"}, {9001, "lang"},   {9002, "rang"},    {9674, "loz"},
    {9824, "spades"}, {9827, "clubs"},  {9829, "hearts"},  {9830, "diams"},
};

const FXJSE_FUNCTION_DESCRIPTOR kFormCalcFM2JSFunctions[] = {
    {kFuncTag, "Abs", CFXJSE_FormCalcContext::Abs},
    {kFuncTag, "Avg", CFXJSE_FormCalcContext::Avg},
    {kFuncTag, "Ceil", CFXJSE_FormCalcContext::Ceil},
    {kFuncTag, "Count", CFXJSE_FormCalcContext::Count},
    {kFuncTag, "Floor", CFXJSE_FormCalcContext::Floor},
    {kFuncTag, "Max", CFXJSE_FormCalcContext::Max},
    {kFuncTag, "Min", CFXJSE_FormCalcContext::Min},
    {kFuncTag, "Mod", CFXJSE_FormCalcContext::Mod},
    {kFuncTag, "Round", CFXJSE_FormCalcContext::Round},
    {kFuncTag, "Sum", CFXJSE_FormCalcContext::Sum},
    {kFuncTag, "Date", CFXJSE_FormCalcContext::Date},
    {kFuncTag, "Date2Num", CFXJSE_FormCalcContext::Date2Num},
    {kFuncTag, "DateFmt", CFXJSE_FormCalcContext::DateFmt},
    {kFuncTag, "IsoDate2Num", CFXJSE_FormCalcContext::IsoDate2Num},
    {kFuncTag, "IsoTime2Num", CFXJSE_FormCalcContext::IsoTime2Num},
    {kFuncTag, "LocalDateFmt", CFXJSE_FormCalcContext::LocalDateFmt},
    {kFuncTag, "LocalTimeFmt", CFXJSE_FormCalcContext::LocalTimeFmt},
    {kFuncTag, "Num2Date", CFXJSE_FormCalcContext::Num2Date},
    {kFuncTag, "Num2GMTime", CFXJSE_FormCalcContext::Num2GMTime},
    {kFuncTag, "Num2Time", CFXJSE_FormCalcContext::Num2Time},
    {kFuncTag, "Time", CFXJSE_FormCalcContext::Time},
    {kFuncTag, "Time2Num", CFXJSE_FormCalcContext::Time2Num},
    {kFuncTag, "TimeFmt", CFXJSE_FormCalcContext::TimeFmt},
    {kFuncTag, "Apr", CFXJSE_FormCalcContext::Apr},
    {kFuncTag, "Cterm", CFXJSE_FormCalcContext::CTerm},
    {kFuncTag, "FV", CFXJSE_FormCalcContext::FV},
    {kFuncTag, "Ipmt", CFXJSE_FormCalcContext::IPmt},
    {kFuncTag, "NPV", CFXJSE_FormCalcContext::NPV},
    {kFuncTag, "Pmt", CFXJSE_FormCalcContext::Pmt},
    {kFuncTag, "PPmt", CFXJSE_FormCalcContext::PPmt},
    {kFuncTag, "PV", CFXJSE_FormCalcContext::PV},
    {kFuncTag, "Rate", CFXJSE_FormCalcContext::Rate},
    {kFuncTag, "Term", CFXJSE_FormCalcContext::Term},
    {kFuncTag, "Choose", CFXJSE_FormCalcContext::Choose},
    {kFuncTag, "Exists", CFXJSE_FormCalcContext::Exists},
    {kFuncTag, "HasValue", CFXJSE_FormCalcContext::HasValue},
    {kFuncTag, "Oneof", CFXJSE_FormCalcContext::Oneof},
    {kFuncTag, "Within", CFXJSE_FormCalcContext::Within},
    {kFuncTag, "If", CFXJSE_FormCalcContext::If},
    {kFuncTag, "Eval", CFXJSE_FormCalcContext::Eval},
    {kFuncTag, "Translate", CFXJSE_FormCalcContext::eval_translation},
    {kFuncTag, "Ref", CFXJSE_FormCalcContext::Ref},
    {kFuncTag, "UnitType", CFXJSE_FormCalcContext::UnitType},
    {kFuncTag, "UnitValue", CFXJSE_FormCalcContext::UnitValue},
    {kFuncTag, "At", CFXJSE_FormCalcContext::At},
    {kFuncTag, "Concat", CFXJSE_FormCalcContext::Concat},
    {kFuncTag, "Decode", CFXJSE_FormCalcContext::Decode},
    {kFuncTag, "Encode", CFXJSE_FormCalcContext::Encode},
    {kFuncTag, "Format", CFXJSE_FormCalcContext::Format},
    {kFuncTag, "Left", CFXJSE_FormCalcContext::Left},
    {kFuncTag, "Len", CFXJSE_FormCalcContext::Len},
    {kFuncTag, "Lower", CFXJSE_FormCalcContext::Lower},
    {kFuncTag, "Ltrim", CFXJSE_FormCalcContext::Ltrim},
    {kFuncTag, "Parse", CFXJSE_FormCalcContext::Parse},
    {kFuncTag, "Replace", CFXJSE_FormCalcContext::Replace},
    {kFuncTag, "Right", CFXJSE_FormCalcContext::Right},
    {kFuncTag, "Rtrim", CFXJSE_FormCalcContext::Rtrim},
    {kFuncTag, "Space", CFXJSE_FormCalcContext::Space},
    {kFuncTag, "Str", CFXJSE_FormCalcContext::Str},
    {kFuncTag, "Stuff", CFXJSE_FormCalcContext::Stuff},
    {kFuncTag, "Substr", CFXJSE_FormCalcContext::Substr},
    {kFuncTag, "Uuid", CFXJSE_FormCalcContext::Uuid},
    {kFuncTag, "Upper", CFXJSE_FormCalcContext::Upper},
    {kFuncTag, "WordNum", CFXJSE_FormCalcContext::WordNum},
    {kFuncTag, "Get", CFXJSE_FormCalcContext::Get},
    {kFuncTag, "Post", CFXJSE_FormCalcContext::Post},
    {kFuncTag, "Put", CFXJSE_FormCalcContext::Put},
    {kFuncTag, "pos_op", CFXJSE_FormCalcContext::positive_operator},
    {kFuncTag, "neg_op", CFXJSE_FormCalcContext::negative_operator},
    {kFuncTag, "log_or_op", CFXJSE_FormCalcContext::logical_or_operator},
    {kFuncTag, "log_and_op", CFXJSE_FormCalcContext::logical_and_operator},
    {kFuncTag, "log_not_op", CFXJSE_FormCalcContext::logical_not_operator},
    {kFuncTag, "eq_op", CFXJSE_FormCalcContext::equality_operator},
    {kFuncTag, "neq_op", CFXJSE_FormCalcContext::notequality_operator},
    {kFuncTag, "lt_op", CFXJSE_FormCalcContext::less_operator},
    {kFuncTag, "le_op", CFXJSE_FormCalcContext::lessequal_operator},
    {kFuncTag, "gt_op", CFXJSE_FormCalcContext::greater_operator},
    {kFuncTag, "ge_op", CFXJSE_FormCalcContext::greaterequal_operator},
    {kFuncTag, "plus_op", CFXJSE_FormCalcContext::plus_operator},
    {kFuncTag, "minus_op", CFXJSE_FormCalcContext::minus_operator},
    {kFuncTag, "mul_op", CFXJSE_FormCalcContext::multiple_operator},
    {kFuncTag, "div_op", CFXJSE_FormCalcContext::divide_operator},
    {kFuncTag, "asgn_val_op", CFXJSE_FormCalcContext::assign_value_operator},
    {kFuncTag, "dot_acc", CFXJSE_FormCalcContext::dot_accessor},
    {kFuncTag, "dotdot_acc", CFXJSE_FormCalcContext::dotdot_accessor},
    {kFuncTag, "concat_obj", CFXJSE_FormCalcContext::concat_fm_object},
    {kFuncTag, "is_obj", CFXJSE_FormCalcContext::is_fm_object},
    {kFuncTag, "is_ary", CFXJSE_FormCalcContext::is_fm_array},
    {kFuncTag, "get_val", CFXJSE_FormCalcContext::get_fm_value},
    {kFuncTag, "get_jsobj", CFXJSE_FormCalcContext::get_fm_jsobj},
    {kFuncTag, "var_filter", CFXJSE_FormCalcContext::fm_var_filter},
};

const uint8_t kAltTableDate[] = {
    255, 255, 255, 3,   9,   255, 255, 255, 255, 255, 255,
    255, 2,   255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 1,   255, 255, 255, 255, 255, 255, 255, 255,
};
static_assert(FX_ArraySize(kAltTableDate) == L'a' - L'A' + 1,
              "Invalid kAltTableDate size.");

const uint8_t kAltTableTime[] = {
    14,  255, 255, 3,   9,   255, 255, 15,  255, 255, 255,
    255, 6,   255, 255, 255, 255, 255, 7,   255, 255, 255,
    255, 255, 1,   17,  255, 255, 255, 255, 255, 255, 255,
};
static_assert(FX_ArraySize(kAltTableTime) == L'a' - L'A' + 1,
              "Invalid kAltTableTime size.");

void AlternateDateTimeSymbols(WideString* pPattern,
                              const WideString& wsAltSymbols,
                              bool bIsDate) {
  const uint8_t* pAltTable = bIsDate ? kAltTableDate : kAltTableTime;
  int32_t nLength = pPattern->GetLength();
  bool bInConstRange = false;
  bool bEscape = false;
  int32_t i = 0;
  while (i < nLength) {
    wchar_t wc = (*pPattern)[i];
    if (wc == L'\'') {
      bInConstRange = !bInConstRange;
      if (bEscape) {
        i++;
      } else {
        pPattern->Delete(i);
        nLength--;
      }
      bEscape = !bEscape;
      continue;
    }
    if (!bInConstRange && wc >= L'A' && wc <= L'a') {
      uint8_t nAlt = pAltTable[wc - L'A'];
      if (nAlt != 255)
        pPattern->SetAt(i, wsAltSymbols[nAlt]);
    }
    i++;
    bEscape = false;
  }
}

std::pair<bool, uint32_t> PatternStringType(ByteStringView bsPattern) {
  WideString wsPattern = WideString::FromUTF8(bsPattern);
  if (L"datetime" == wsPattern.First(8))
    return {true, XFA_VT_DATETIME};
  if (L"date" == wsPattern.First(4)) {
    auto pos = wsPattern.Find(L"time");
    uint32_t type =
        pos.has_value() && pos.value() != 0 ? XFA_VT_DATETIME : XFA_VT_DATE;
    return {true, type};
  }
  if (L"time" == wsPattern.First(4))
    return {true, XFA_VT_TIME};
  if (L"text" == wsPattern.First(4))
    return {true, XFA_VT_TEXT};
  if (L"num" == wsPattern.First(3)) {
    uint32_t type;
    if (L"integer" == wsPattern.Substr(4, 7)) {
      type = XFA_VT_INTEGER;
    } else if (L"decimal" == wsPattern.Substr(4, 7)) {
      type = XFA_VT_DECIMAL;
    } else if (L"currency" == wsPattern.Substr(4, 8)) {
      type = XFA_VT_FLOAT;
    } else if (L"percent" == wsPattern.Substr(4, 7)) {
      type = XFA_VT_FLOAT;
    } else {
      type = XFA_VT_FLOAT;
    }
    return {true, type};
  }

  uint32_t type = XFA_VT_NULL;
  wsPattern.MakeLower();
  const wchar_t* pData = wsPattern.c_str();
  int32_t iLength = wsPattern.GetLength();
  int32_t iIndex = 0;
  bool bSingleQuotation = false;
  while (iIndex < iLength) {
    wchar_t wsPatternChar = pData[iIndex];
    if (wsPatternChar == 0x27) {
      bSingleQuotation = !bSingleQuotation;
      iIndex++;
      continue;
    }
    if (bSingleQuotation) {
      iIndex++;
      continue;
    }

    if (wsPatternChar == 'h' || wsPatternChar == 'k')
      return {false, XFA_VT_TIME};
    if (wsPatternChar == 'x' || wsPatternChar == 'o' || wsPatternChar == '0')
      return {false, XFA_VT_TEXT};
    if (wsPatternChar == 'v' || wsPatternChar == '8' || wsPatternChar == '$')
      return {false, XFA_VT_FLOAT};
    if (wsPatternChar == 'y' || wsPatternChar == 'j') {
      iIndex++;
      wchar_t timePatternChar;
      while (iIndex < iLength) {
        timePatternChar = pData[iIndex];
        if (timePatternChar == 0x27) {
          bSingleQuotation = !bSingleQuotation;
          iIndex++;
          continue;
        }
        if (!bSingleQuotation && timePatternChar == 't')
          return {false, XFA_VT_DATETIME};
        iIndex++;
      }
      return {false, XFA_VT_DATE};
    }

    if (wsPatternChar == 'a') {
      type = XFA_VT_TEXT;
    } else if (wsPatternChar == 'z' || wsPatternChar == 's' ||
               wsPatternChar == 'e' || wsPatternChar == ',' ||
               wsPatternChar == '.') {
      type = XFA_VT_FLOAT;
    }
    iIndex++;
  }

  if (type == XFA_VT_NULL)
    type = XFA_VT_TEXT | XFA_VT_FLOAT;
  return {false, type};
}

CFXJSE_FormCalcContext* ToFormCalcContext(CFXJSE_HostObject* pHostObj) {
  return pHostObj ? pHostObj->AsFormCalcContext() : nullptr;
}

LocaleIface* LocaleFromString(CXFA_Document* pDoc,
                              CXFA_LocaleMgr* pMgr,
                              ByteStringView bsLocale) {
  if (!bsLocale.IsEmpty())
    return pMgr->GetLocaleByName(WideString::FromUTF8(bsLocale));

  CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
  return pThisNode->GetLocale();
}

WideString FormatFromString(LocaleIface* pLocale, ByteStringView bsFormat) {
  if (!bsFormat.IsEmpty())
    return WideString::FromUTF8(bsFormat);

  return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Default);
}

FX_LOCALEDATETIMESUBCATEGORY SubCategoryFromInt(int32_t iStyle) {
  switch (iStyle) {
    case 1:
      return FX_LOCALEDATETIMESUBCATEGORY_Short;
    case 3:
      return FX_LOCALEDATETIMESUBCATEGORY_Long;
    case 4:
      return FX_LOCALEDATETIMESUBCATEGORY_Full;
    case 0:
    case 2:
    default:
      return FX_LOCALEDATETIMESUBCATEGORY_Medium;
  }
}

ByteString GetLocalDateTimeFormat(CXFA_Document* pDoc,
                                  int32_t iStyle,
                                  ByteStringView bsLocale,
                                  bool bStandard,
                                  bool bIsDate) {
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  LocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  FX_LOCALEDATETIMESUBCATEGORY category = SubCategoryFromInt(iStyle);
  WideString wsLocal = bIsDate ? pLocale->GetDatePattern(category)
                               : pLocale->GetTimePattern(category);
  if (!bStandard)
    AlternateDateTimeSymbols(&wsLocal, pLocale->GetDateTimeSymbols(), bIsDate);
  return wsLocal.ToUTF8();
}

bool IsWhitespace(char c) {
  return c == 0x20 || c == 0x09 || c == 0x0B || c == 0x0C || c == 0x0A ||
         c == 0x0D;
}

bool IsPartOfNumber(char ch) {
  return std::isdigit(ch) || ch == '-' || ch == '.';
}

bool IsPartOfNumberW(wchar_t ch) {
  return FXSYS_IsDecimalDigit(ch) || ch == L'-' || ch == L'.';
}

ByteString GUIDString(bool bSeparator) {
  uint8_t data[16];
  FX_Random_GenerateMT(reinterpret_cast<uint32_t*>(data), 4);
  data[6] = (data[6] & 0x0F) | 0x40;

  ByteString bsGUID;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<char> pBuf = bsGUID.GetBuffer(40);
    size_t out_index = 0;
    for (size_t i = 0; i < 16; ++i, out_index += 2) {
      if (bSeparator && (i == 4 || i == 6 || i == 8 || i == 10))
        pBuf[out_index++] = L'-';

      FXSYS_IntToTwoHexChars(data[i], &pBuf[out_index]);
    }
  }
  bsGUID.ReleaseBuffer(bSeparator ? 36 : 32);
  return bsGUID;
}

bool IsIsoDateFormat(pdfium::span<const char> pData,
                     int32_t* pStyle,
                     int32_t* pYear,
                     int32_t* pMonth,
                     int32_t* pDay) {
  int32_t& iStyle = *pStyle;
  int32_t& iYear = *pYear;
  int32_t& iMonth = *pMonth;
  int32_t& iDay = *pDay;

  iYear = 0;
  iMonth = 1;
  iDay = 1;

  if (pData.size() < 4)
    return false;

  char szYear[5];
  szYear[4] = '\0';
  for (int32_t i = 0; i < 4; ++i) {
    if (!std::isdigit(pData[i]))
      return false;

    szYear[i] = pData[i];
  }
  iYear = FXSYS_atoi(szYear);
  iStyle = 0;
  if (pData.size() == 4)
    return true;

  iStyle = pData[4] == '-' ? 1 : 0;

  size_t iPosOff = iStyle == 0 ? 4 : 5;
  if (!std::isdigit(pData[iPosOff]) || !std::isdigit(pData[iPosOff + 1]))
    return false;

  char szBuffer[3] = {};
  szBuffer[0] = pData[iPosOff];
  szBuffer[1] = pData[iPosOff + 1];
  iMonth = FXSYS_atoi(szBuffer);
  if (iMonth > 12 || iMonth < 1)
    return false;

  if (iStyle == 0) {
    iPosOff += 2;
    if (pData.size() == 6)
      return true;
  } else {
    iPosOff += 3;
    if (pData.size() == 7)
      return true;
  }
  if (!std::isdigit(pData[iPosOff]) || !std::isdigit(pData[iPosOff + 1]))
    return false;

  szBuffer[0] = pData[iPosOff];
  szBuffer[1] = pData[iPosOff + 1];
  iDay = FXSYS_atoi(szBuffer);
  if (iPosOff + 2 < pData.size())
    return false;

  if (iMonth == 2) {
    bool bIsLeap = (!(iYear % 4) && (iYear % 100)) || !(iYear % 400);
    return iDay <= (bIsLeap ? 29 : 28);
  }

  if (iMonth < 8)
    return iDay <= (iMonth % 2 == 0 ? 30 : 31);
  return iDay <= (iMonth % 2 == 0 ? 31 : 30);
}

bool IsIsoTimeFormat(pdfium::span<const char> pData,
                     int32_t* pHour,
                     int32_t* pMinute,
                     int32_t* pSecond,
                     int32_t* pMilliSecond,
                     int32_t* pZoneHour,
                     int32_t* pZoneMinute) {
  int32_t& iHour = *pHour;
  int32_t& iMinute = *pMinute;
  int32_t& iSecond = *pSecond;
  int32_t& iMilliSecond = *pMilliSecond;
  int32_t& iZoneHour = *pZoneHour;
  int32_t& iZoneMinute = *pZoneMinute;

  iHour = 0;
  iMinute = 0;
  iSecond = 0;
  iMilliSecond = 0;
  iZoneHour = 0;
  iZoneMinute = 0;

  if (pData.empty())
    return false;

  size_t iZone = 0;
  size_t i = 0;
  while (i < pData.size()) {
    if (!std::isdigit(pData[i]) && pData[i] != ':') {
      iZone = i;
      break;
    }
    ++i;
  }
  if (i == pData.size())
    iZone = pData.size();

  char szBuffer[3] = {};
  size_t iPos = 0;
  size_t iIndex = 0;
  while (iIndex < iZone) {
    if (!std::isdigit(pData[iIndex]))
      return false;

    szBuffer[0] = pData[iIndex];
    if (!std::isdigit(pData[iIndex + 1]))
      return false;

    szBuffer[1] = pData[iIndex + 1];
    if (FXSYS_atoi(szBuffer) > 60)
      return false;

    if (pData[2] == ':') {
      if (iPos == 0) {
        iHour = FXSYS_atoi(szBuffer);
        ++iPos;
      } else if (iPos == 1) {
        iMinute = FXSYS_atoi(szBuffer);
        ++iPos;
      } else {
        iSecond = FXSYS_atoi(szBuffer);
      }
      iIndex += 3;
    } else {
      if (iPos == 0) {
        iHour = FXSYS_atoi(szBuffer);
        ++iPos;
      } else if (iPos == 1) {
        iMinute = FXSYS_atoi(szBuffer);
        ++iPos;
      } else if (iPos == 2) {
        iSecond = FXSYS_atoi(szBuffer);
        ++iPos;
      }
      iIndex += 2;
    }
  }

  if (iIndex < pData.size() && pData[iIndex] == '.') {
    constexpr int kSubSecondLength = 3;
    if (iIndex + kSubSecondLength >= pData.size())
      return false;

    ++iIndex;
    char szMilliSeconds[kSubSecondLength + 1];
    for (int j = 0; j < kSubSecondLength; ++j) {
      char c = pData[iIndex + j];
      if (!std::isdigit(c))
        return false;
      szMilliSeconds[j] = c;
    }
    szMilliSeconds[kSubSecondLength] = '\0';

    iMilliSecond = FXSYS_atoi(szMilliSeconds);
    if (iMilliSecond > 100) {
      iMilliSecond = 0;
      return false;
    }
    iIndex += kSubSecondLength;
  }

  if (iIndex < pData.size() && FXSYS_towlower(pData[iIndex]) == 'z')
    return true;

  int32_t iSign = 1;
  if (iIndex < pData.size()) {
    if (pData[iIndex] == '+') {
      ++iIndex;
    } else if (pData[iIndex] == '-') {
      iSign = -1;
      ++iIndex;
    }
  }
  iPos = 0;
  while (iIndex < pData.size()) {
    if (!std::isdigit(pData[iIndex]))
      return false;

    szBuffer[0] = pData[iIndex];
    if (!std::isdigit(pData[iIndex + 1]))
      return false;

    szBuffer[1] = pData[iIndex + 1];
    if (FXSYS_atoi(szBuffer) > 60)
      return false;

    if (pData[2] == ':') {
      if (iPos == 0) {
        iZoneHour = FXSYS_atoi(szBuffer);
      } else if (iPos == 1) {
        iZoneMinute = FXSYS_atoi(szBuffer);
      }
      iIndex += 3;
    } else {
      if (!iPos) {
        iZoneHour = FXSYS_atoi(szBuffer);
        ++iPos;
      } else if (iPos == 1) {
        iZoneMinute = FXSYS_atoi(szBuffer);
        ++iPos;
      }
      iIndex += 2;
    }
  }
  if (iIndex < pData.size())
    return false;

  iZoneHour *= iSign;
  return true;
}

bool IsIsoDateTimeFormat(pdfium::span<const char> pData,
                         int32_t* pYear,
                         int32_t* pMonth,
                         int32_t* pDay,
                         int32_t* pHour,
                         int32_t* pMinute,
                         int32_t* pSecond,
                         int32_t* pMilliSecond,
                         int32_t* pZoneHour,
                         int32_t* pZoneMinute) {
  int32_t& iYear = *pYear;
  int32_t& iMonth = *pMonth;
  int32_t& iDay = *pDay;
  int32_t& iHour = *pHour;
  int32_t& iMinute = *pMinute;
  int32_t& iSecond = *pSecond;
  int32_t& iMilliSecond = *pMilliSecond;
  int32_t& iZoneHour = *pZoneHour;
  int32_t& iZoneMinute = *pZoneMinute;

  iYear = 0;
  iMonth = 0;
  iDay = 0;
  iHour = 0;
  iMinute = 0;
  iSecond = 0;

  if (pData.empty())
    return false;

  size_t iIndex = 0;
  while (pData[iIndex] != 'T' && pData[iIndex] != 't') {
    if (iIndex >= pData.size())
      return false;
    ++iIndex;
  }
  if (iIndex != 8 && iIndex != 10)
    return false;

  int32_t iStyle = -1;
  if (!IsIsoDateFormat(pData.subspan(0, iIndex), &iStyle, &iYear, &iMonth,
                       &iDay)) {
    return false;
  }
  if (pData[iIndex] != 'T' && pData[iIndex] != 't')
    return true;

  return IsIsoTimeFormat(pData.subspan(iIndex + 1), &iHour, &iMinute, &iSecond,
                         &iMilliSecond, &iZoneHour, &iZoneMinute);
}

int32_t DateString2Num(ByteStringView bsDate) {
  int32_t iLength = bsDate.GetLength();
  int32_t iYear = 0;
  int32_t iMonth = 0;
  int32_t iDay = 0;
  if (iLength <= 10) {
    int32_t iStyle = -1;
    if (!IsIsoDateFormat(bsDate.span(), &iStyle, &iYear, &iMonth, &iDay))
      return 0;
  } else {
    int32_t iHour = 0;
    int32_t iMinute = 0;
    int32_t iSecond = 0;
    int32_t iMilliSecond = 0;
    int32_t iZoneHour = 0;
    int32_t iZoneMinute = 0;
    if (!IsIsoDateTimeFormat(bsDate.span(), &iYear, &iMonth, &iDay, &iHour,
                             &iMinute, &iSecond, &iMilliSecond, &iZoneHour,
                             &iZoneMinute)) {
      return 0;
    }
  }

  float dDays = 0;
  int32_t i = 1;
  if (iYear < 1900)
    return 0;

  while (iYear - i >= 1900) {
    dDays +=
        ((!((iYear - i) % 4) && ((iYear - i) % 100)) || !((iYear - i) % 400))
            ? 366
            : 365;
    ++i;
  }
  i = 1;
  while (i < iMonth) {
    if (i == 2)
      dDays += ((!(iYear % 4) && (iYear % 100)) || !(iYear % 400)) ? 29 : 28;
    else if (i <= 7)
      dDays += (i % 2 == 0) ? 30 : 31;
    else
      dDays += (i % 2 == 0) ? 31 : 30;

    ++i;
  }
  i = 0;
  while (iDay - i > 0) {
    ++dDays;
    ++i;
  }
  return (int32_t)dDays;
}

void GetLocalTimeZone(int32_t* pHour, int32_t* pMin, int32_t* pSec) {
  time_t now;
  FXSYS_time(&now);

  struct tm* pGmt = gmtime(&now);
  struct tm* pLocal = FXSYS_localtime(&now);
  *pHour = pLocal->tm_hour - pGmt->tm_hour;
  *pMin = pLocal->tm_min - pGmt->tm_min;
  *pSec = pLocal->tm_sec - pGmt->tm_sec;
}

bool HTMLSTR2Code(const WideString& pData, uint32_t* iCode) {
  auto cmpFunc = [](const XFA_FMHtmlReserveCode& iter, ByteStringView val) {
    return strcmp(val.unterminated_c_str(), iter.m_htmlReserve) > 0;
  };
  if (!pData.IsASCII())
    return false;
  ByteString temp = pData.ToASCII();
  const XFA_FMHtmlReserveCode* result = std::lower_bound(
      std::begin(kReservesForDecode), std::end(kReservesForDecode),
      temp.AsStringView(), cmpFunc);
  if (result != std::end(kReservesForDecode) &&
      !strcmp(temp.c_str(), result->m_htmlReserve)) {
    *iCode = result->m_uCode;
    return true;
  }
  return false;
}

bool HTMLCode2STR(uint32_t iCode, WideString* wsHTMLReserve) {
  auto cmpFunc = [](const XFA_FMHtmlReserveCode iter, const uint32_t& val) {
    return iter.m_uCode < val;
  };
  const XFA_FMHtmlReserveCode* result =
      std::lower_bound(std::begin(kReservesForEncode),
                       std::end(kReservesForEncode), iCode, cmpFunc);
  if (result != std::end(kReservesForEncode) && result->m_uCode == iCode) {
    *wsHTMLReserve = WideString::FromASCII(result->m_htmlReserve);
    return true;
  }
  return false;
}

WideString DecodeURL(const WideString& wsURL) {
  const wchar_t* pData = wsURL.c_str();
  size_t iLen = wsURL.GetLength();
  CFX_WideTextBuf wsResultBuf;
  for (size_t i = 0; i < iLen; ++i) {
    wchar_t ch = pData[i];
    if ('%' != ch) {
      wsResultBuf.AppendChar(ch);
      continue;
    }

    wchar_t chTemp = 0;
    int32_t iCount = 0;
    while (iCount < 2) {
      if (++i >= iLen)
        break;
      chTemp *= 16;
      ch = pData[i];
      if (!FXSYS_IsWideHexDigit(ch))
        return WideString();
      chTemp += FXSYS_WideHexCharToInt(ch);
      ++iCount;
    }
    wsResultBuf.AppendChar(chTemp);
  }
  wsResultBuf.AppendChar(0);
  return wsResultBuf.MakeString();
}

WideString DecodeMLInternal(const WideString& wsHTML, bool bIsHTML) {
  const wchar_t* pData = wsHTML.c_str();
  size_t iLen = wsHTML.GetLength();
  CFX_WideTextBuf wsResultBuf;
  for (size_t i = 0; i < iLen; ++i) {
    wchar_t ch = pData[i];
    if (ch != '&') {
      wsResultBuf.AppendChar(ch);
      continue;
    }

    if (++i >= iLen)
      break;
    ch = pData[i];
    if (ch == '#') {
      if (++i >= iLen)
        break;
      ch = pData[i];
      if (ch != 'x' && ch != 'X')
        return WideString();
      if (++i >= iLen)
        break;
      ch = pData[i];
      uint32_t iCode = 0;
      while (ch != ';' && i < iLen) {
        iCode *= 16;
        if (!FXSYS_IsWideHexDigit(ch))
          return WideString();
        iCode += FXSYS_WideHexCharToInt(ch);
        if (++i >= iLen)
          break;
        ch = pData[i];
      }
      wsResultBuf.AppendChar(iCode);
      continue;
    }

    wchar_t szBuffer[9];
    size_t iStrIndex = 0;
    while (ch != ';' && i < iLen) {
      if (iStrIndex < 8)
        szBuffer[iStrIndex++] = ch;
      if (++i >= iLen)
        break;
      ch = pData[i];
    }
    szBuffer[iStrIndex] = 0;
    if (bIsHTML) {
      uint32_t iData = 0;
      if (HTMLSTR2Code(szBuffer, &iData))
        wsResultBuf.AppendChar((wchar_t)iData);
    } else {
      if (wcscmp(szBuffer, L"quot") == 0)
        wsResultBuf.AppendChar('"');
      else if (wcscmp(szBuffer, L"amp") == 0)
        wsResultBuf.AppendChar('&');
      else if (wcscmp(szBuffer, L"apos") == 0)
        wsResultBuf.AppendChar('\'');
      else if (wcscmp(szBuffer, L"lt") == 0)
        wsResultBuf.AppendChar('<');
      else if (wcscmp(szBuffer, L"gt") == 0)
        wsResultBuf.AppendChar('>');
    }
  }

  wsResultBuf.AppendChar(0);
  return wsResultBuf.MakeString();
}

WideString DecodeHTML(const WideString& wsHTML) {
  return DecodeMLInternal(wsHTML, true);
}

WideString DecodeXML(const WideString& wsXML) {
  return DecodeMLInternal(wsXML, false);
}

WideString EncodeURL(const ByteString& bsURL) {
  static const wchar_t kStrUnsafe[] = {' ', '<',  '>', '"', '#', '%', '{', '}',
                                       '|', '\\', '^', '~', '[', ']', '`'};
  static const wchar_t kStrReserved[] = {';', '/', '?', ':', '@', '=', '&'};
  static const wchar_t kStrSpecial[] = {'$',  '-', '+', '!', '*',
                                        '\'', '(', ')', ','};

  WideString wsURL = WideString::FromUTF8(bsURL.AsStringView());
  CFX_WideTextBuf wsResultBuf;
  wchar_t szEncode[4];
  szEncode[0] = '%';
  szEncode[3] = 0;
  for (wchar_t ch : wsURL) {
    size_t i = 0;
    size_t iCount = FX_ArraySize(kStrUnsafe);
    while (i < iCount) {
      if (ch == kStrUnsafe[i]) {
        int32_t iIndex = ch / 16;
        szEncode[1] = kStrCode[iIndex];
        szEncode[2] = kStrCode[ch - iIndex * 16];
        wsResultBuf << szEncode;
        break;
      }
      ++i;
    }
    if (i < iCount)
      continue;

    i = 0;
    iCount = FX_ArraySize(kStrReserved);
    while (i < iCount) {
      if (ch == kStrReserved[i]) {
        int32_t iIndex = ch / 16;
        szEncode[1] = kStrCode[iIndex];
        szEncode[2] = kStrCode[ch - iIndex * 16];
        wsResultBuf << szEncode;
        break;
      }
      ++i;
    }
    if (i < iCount)
      continue;

    i = 0;
    iCount = FX_ArraySize(kStrSpecial);
    while (i < iCount) {
      if (ch == kStrSpecial[i]) {
        wsResultBuf.AppendChar(ch);
        break;
      }
      ++i;
    }
    if (i < iCount)
      continue;

    if ((ch >= 0x80 && ch <= 0xff) || ch <= 0x1f || ch == 0x7f) {
      int32_t iIndex = ch / 16;
      szEncode[1] = kStrCode[iIndex];
      szEncode[2] = kStrCode[ch - iIndex * 16];
      wsResultBuf << szEncode;
    } else if (ch >= 0x20 && ch <= 0x7e) {
      wsResultBuf.AppendChar(ch);
    } else {
      const wchar_t iRadix = 16;
      WideString wsBuffer;
      while (ch >= iRadix) {
        wchar_t tmp = kStrCode[ch % iRadix];
        ch /= iRadix;
        wsBuffer += tmp;
      }
      wsBuffer += kStrCode[ch];
      int32_t iLen = wsBuffer.GetLength();
      if (iLen < 2)
        break;

      int32_t iIndex = 0;
      if (iLen % 2 != 0) {
        szEncode[1] = '0';
        szEncode[2] = wsBuffer[iLen - 1];
        iIndex = iLen - 2;
      } else {
        szEncode[1] = wsBuffer[iLen - 1];
        szEncode[2] = wsBuffer[iLen - 2];
        iIndex = iLen - 3;
      }
      wsResultBuf << szEncode;
      while (iIndex > 0) {
        szEncode[1] = wsBuffer[iIndex];
        szEncode[2] = wsBuffer[iIndex - 1];
        iIndex -= 2;
        wsResultBuf << szEncode;
      }
    }
  }
  wsResultBuf.AppendChar(0);
  return wsResultBuf.MakeString();
}

WideString EncodeHTML(const ByteString& bsHTML) {
  WideString wsHTML = WideString::FromUTF8(bsHTML.AsStringView());
  wchar_t szEncode[9];
  szEncode[0] = '&';
  szEncode[1] = '#';
  szEncode[2] = 'x';
  CFX_WideTextBuf wsResultBuf;
  for (uint32_t ch : wsHTML) {
    WideString htmlReserve;
    if (HTMLCode2STR(ch, &htmlReserve)) {
      wsResultBuf.AppendChar(L'&');
      wsResultBuf << htmlReserve;
      wsResultBuf.AppendChar(L';');
    } else if (ch >= 32 && ch <= 126) {
      wsResultBuf.AppendChar(static_cast<wchar_t>(ch));
    } else if (ch < 256) {
      int32_t iIndex = ch / 16;
      szEncode[3] = kStrCode[iIndex];
      szEncode[4] = kStrCode[ch - iIndex * 16];
      szEncode[5] = ';';
      szEncode[6] = 0;
      wsResultBuf << szEncode;
    } else if (ch < 65536) {
      int32_t iBigByte = ch / 256;
      int32_t iLittleByte = ch % 256;
      szEncode[3] = kStrCode[iBigByte / 16];
      szEncode[4] = kStrCode[iBigByte % 16];
      szEncode[5] = kStrCode[iLittleByte / 16];
      szEncode[6] = kStrCode[iLittleByte % 16];
      szEncode[7] = ';';
      szEncode[8] = 0;
      wsResultBuf << szEncode;
    } else {
      // TODO(tsepez): Handle codepoint not in BMP.
    }
  }
  wsResultBuf.AppendChar(0);
  return wsResultBuf.MakeString();
}

WideString EncodeXML(const ByteString& bsXML) {
  WideString wsXML = WideString::FromUTF8(bsXML.AsStringView());
  CFX_WideTextBuf wsResultBuf;
  wchar_t szEncode[9];
  szEncode[0] = '&';
  szEncode[1] = '#';
  szEncode[2] = 'x';
  for (uint32_t ch : wsXML) {
    switch (ch) {
      case '"':
        wsResultBuf.AppendChar('&');
        wsResultBuf << WideStringView(L"quot");
        wsResultBuf.AppendChar(';');
        break;
      case '&':
        wsResultBuf.AppendChar('&');
        wsResultBuf << WideStringView(L"amp");
        wsResultBuf.AppendChar(';');
        break;
      case '\'':
        wsResultBuf.AppendChar('&');
        wsResultBuf << WideStringView(L"apos");
        wsResultBuf.AppendChar(';');
        break;
      case '<':
        wsResultBuf.AppendChar('&');
        wsResultBuf << WideStringView(L"lt");
        wsResultBuf.AppendChar(';');
        break;
      case '>':
        wsResultBuf.AppendChar('&');
        wsResultBuf << WideStringView(L"gt");
        wsResultBuf.AppendChar(';');
        break;
      default: {
        if (ch >= 32 && ch <= 126) {
          wsResultBuf.AppendChar(static_cast<wchar_t>(ch));
        } else if (ch < 256) {
          int32_t iIndex = ch / 16;
          szEncode[3] = kStrCode[iIndex];
          szEncode[4] = kStrCode[ch - iIndex * 16];
          szEncode[5] = ';';
          szEncode[6] = 0;
          wsResultBuf << szEncode;
        } else if (ch < 65536) {
          int32_t iBigByte = ch / 256;
          int32_t iLittleByte = ch % 256;
          szEncode[3] = kStrCode[iBigByte / 16];
          szEncode[4] = kStrCode[iBigByte % 16];
          szEncode[5] = kStrCode[iLittleByte / 16];
          szEncode[6] = kStrCode[iLittleByte % 16];
          szEncode[7] = ';';
          szEncode[8] = 0;
          wsResultBuf << szEncode;
        } else {
          // TODO(tsepez): Handle codepoint not in BMP.
        }
        break;
      }
    }
  }
  wsResultBuf.AppendChar(0);
  return wsResultBuf.MakeString();
}

ByteString TrillionUS(ByteStringView bsData) {
  static const ByteStringView pUnits[] = {"zero",  "one",  "two", "three",
                                          "four",  "five", "six", "seven",
                                          "eight", "nine"};
  static const ByteStringView pCapUnits[] = {"Zero",  "One",  "Two", "Three",
                                             "Four",  "Five", "Six", "Seven",
                                             "Eight", "Nine"};
  static const ByteStringView pTens[] = {
      "Ten",     "Eleven",  "Twelve",    "Thirteen", "Fourteen",
      "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen"};
  static const ByteStringView pLastTens[] = {"Twenty", "Thirty", "Forty",
                                             "Fifty",  "Sixty",  "Seventy",
                                             "Eighty", "Ninety"};
  static const ByteStringView pComm[] = {" Hundred ", " Thousand ", " Million ",
                                         " Billion ", "Trillion"};
  const char* pData = bsData.unterminated_c_str();
  int32_t iLength = bsData.GetLength();
  int32_t iComm = 0;
  if (iLength > 12)
    iComm = 4;
  else if (iLength > 9)
    iComm = 3;
  else if (iLength > 6)
    iComm = 2;
  else if (iLength > 3)
    iComm = 1;

  int32_t iFirstCount = iLength % 3;
  if (iFirstCount == 0)
    iFirstCount = 3;

  std::ostringstream strBuf;
  int32_t iIndex = 0;
  if (iFirstCount == 3) {
    if (pData[iIndex] != '0') {
      strBuf << pCapUnits[pData[iIndex] - '0'];
      strBuf << pComm[0];
    }
    if (pData[iIndex + 1] == '0') {
      strBuf << pCapUnits[pData[iIndex + 2] - '0'];
    } else {
      if (pData[iIndex + 1] > '1') {
        strBuf << pLastTens[pData[iIndex + 1] - '2'];
        strBuf << "-";
        strBuf << pUnits[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '1') {
        strBuf << pTens[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '0') {
        strBuf << pCapUnits[pData[iIndex + 2] - '0'];
      }
    }
    iIndex += 3;
  } else if (iFirstCount == 2) {
    if (pData[iIndex] == '0') {
      strBuf << pCapUnits[pData[iIndex + 1] - '0'];
    } else {
      if (pData[iIndex] > '1') {
        strBuf << pLastTens[pData[iIndex] - '2'];
        strBuf << "-";
        strBuf << pUnits[pData[iIndex + 1] - '0'];
      } else if (pData[iIndex] == '1') {
        strBuf << pTens[pData[iIndex + 1] - '0'];
      } else if (pData[iIndex] == '0') {
        strBuf << pCapUnits[pData[iIndex + 1] - '0'];
      }
    }
    iIndex += 2;
  } else if (iFirstCount == 1) {
    strBuf << pCapUnits[pData[iIndex] - '0'];
    ++iIndex;
  }
  if (iLength > 3 && iFirstCount > 0) {
    strBuf << pComm[iComm];
    --iComm;
  }
  while (iIndex < iLength) {
    if (pData[iIndex] != '0') {
      strBuf << pCapUnits[pData[iIndex] - '0'];
      strBuf << pComm[0];
    }
    if (pData[iIndex + 1] == '0') {
      strBuf << pCapUnits[pData[iIndex + 2] - '0'];
    } else {
      if (pData[iIndex + 1] > '1') {
        strBuf << pLastTens[pData[iIndex + 1] - '2'];
        strBuf << "-";
        strBuf << pUnits[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '1') {
        strBuf << pTens[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '0') {
        strBuf << pCapUnits[pData[iIndex + 2] - '0'];
      }
    }
    if (iIndex < iLength - 3) {
      strBuf << pComm[iComm];
      --iComm;
    }
    iIndex += 3;
  }
  return ByteString(strBuf);
}

ByteString WordUS(const ByteString& bsData, int32_t iStyle) {
  const char* pData = bsData.c_str();
  int32_t iLength = bsData.GetLength();
  if (iStyle < 0 || iStyle > 2) {
    return ByteString();
  }

  std::ostringstream strBuf;

  int32_t iIndex = 0;
  while (iIndex < iLength) {
    if (pData[iIndex] == '.')
      break;
    ++iIndex;
  }
  int32_t iInteger = iIndex;
  iIndex = 0;
  while (iIndex < iInteger) {
    int32_t iCount = (iInteger - iIndex) % 12;
    if (!iCount && iInteger - iIndex > 0)
      iCount = 12;

    strBuf << TrillionUS(ByteStringView(pData + iIndex, iCount));
    iIndex += iCount;
    if (iIndex < iInteger)
      strBuf << " Trillion ";
  }

  if (iStyle > 0)
    strBuf << " Dollars";

  if (iStyle > 1 && iInteger < iLength) {
    strBuf << " And ";
    iIndex = iInteger + 1;
    while (iIndex < iLength) {
      int32_t iCount = (iLength - iIndex) % 12;
      if (!iCount && iLength - iIndex > 0)
        iCount = 12;

      strBuf << TrillionUS(ByteStringView(pData + iIndex, iCount));
      iIndex += iCount;
      if (iIndex < iLength)
        strBuf << " Trillion ";
    }
    strBuf << " Cents";
  }
  return ByteString(strBuf);
}

void GetObjectDefaultValue(CFXJSE_Value* pValue, CFXJSE_Value* pDefaultValue) {
  CXFA_Node* pNode = ToNode(CFXJSE_Engine::ToObject(pValue));
  if (!pNode) {
    pDefaultValue->SetNull();
    return;
  }
  pNode->JSObject()->ScriptSomDefaultValue(pDefaultValue, false,
                                           XFA_Attribute::Unknown);
}

bool SetObjectDefaultValue(CFXJSE_Value* pValue, CFXJSE_Value* hNewValue) {
  CXFA_Node* pNode = ToNode(CFXJSE_Engine::ToObject(pValue));
  if (!pNode)
    return false;

  pNode->JSObject()->ScriptSomDefaultValue(hNewValue, true,
                                           XFA_Attribute::Unknown);
  return true;
}

}  // namespace

const FXJSE_CLASS_DESCRIPTOR kFormCalcFM2JSDescriptor = {
    kClassTag,                              // tag
    "XFA_FM2JS_FormCalcClass",              // name
    kFormCalcFM2JSFunctions,                // methods
    FX_ArraySize(kFormCalcFM2JSFunctions),  // number of methods
    nullptr,                                // dynamic prop type
    nullptr,                                // dynamic prop getter
    nullptr,                                // dynamic prop setter
    nullptr,                                // dynamic prop method call
};

// static
void CFXJSE_FormCalcContext::Abs(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Abs");
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }
  double dValue = ValueToDouble(pThis, argOne.get());
  if (dValue < 0)
    dValue = -dValue;

  info.GetReturnValue().Set(dValue);
}

// static
void CFXJSE_FormCalcContext::Avg(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1) {
    info.GetReturnValue().SetNull();
    return;
  }

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  uint32_t uCount = 0;
  double dSum = 0.0;
  for (int32_t i = 0; i < argc; i++) {
    auto argValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[i]);
    if (argValue->IsNull())
      continue;

    if (!argValue->IsArray()) {
      dSum += ValueToDouble(pThis, argValue.get());
      uCount++;
      continue;
    }

    auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argValue->GetObjectProperty("length", lengthValue.get());
    int32_t iLength = lengthValue->ToInteger();

    if (iLength > 2) {
      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectPropertyByIdx(1, propertyValue.get());

      auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      if (propertyValue->IsNull()) {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          auto defaultPropValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
          GetObjectDefaultValue(jsObjectValue.get(), defaultPropValue.get());
          if (defaultPropValue->IsNull())
            continue;

          dSum += ValueToDouble(pThis, defaultPropValue.get());
          uCount++;
        }
      } else {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          dSum += ValueToDouble(pThis, newPropertyValue.get());
          uCount++;
        }
      }
    }
  }
  if (uCount == 0) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(dSum / uCount);
}

// static
void CFXJSE_FormCalcContext::Ceil(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Ceil");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argValue = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argValue.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(ceil(ValueToFloat(pThis, argValue.get())));
}

// static
void CFXJSE_FormCalcContext::Count(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  int32_t iCount = 0;
  for (int32_t i = 0; i < info.Length(); i++) {
    auto argValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[i]);
    if (argValue->IsNull())
      continue;

    if (argValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectProperty("length", lengthValue.get());

      int32_t iLength = lengthValue->ToInteger();
      if (iLength <= 2) {
        pContext->ThrowArgumentMismatchException();
        return;
      }

      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectPropertyByIdx(1, propertyValue.get());
      argValue->GetObjectPropertyByIdx(2, jsObjectValue.get());
      if (propertyValue->IsNull()) {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
          if (!newPropertyValue->IsNull())
            iCount++;
        }
      } else {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
          iCount += newPropertyValue->IsNull() ? 0 : 1;
        }
      }
    } else if (argValue->IsObject()) {
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      GetObjectDefaultValue(argValue.get(), newPropertyValue.get());
      if (!newPropertyValue->IsNull())
        iCount++;
    } else {
      iCount++;
    }
  }
  info.GetReturnValue().Set(iCount);
}

// static
void CFXJSE_FormCalcContext::Floor(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Floor");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argValue = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argValue.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(floor(ValueToFloat(pThis, argValue.get())));
}

// static
void CFXJSE_FormCalcContext::Max(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  uint32_t uCount = 0;
  double dMaxValue = 0.0;
  for (int32_t i = 0; i < info.Length(); i++) {
    auto argValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[i]);
    if (argValue->IsNull())
      continue;

    if (argValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectProperty("length", lengthValue.get());
      int32_t iLength = lengthValue->ToInteger();
      if (iLength <= 2) {
        pContext->ThrowArgumentMismatchException();
        return;
      }

      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectPropertyByIdx(1, propertyValue.get());
      argValue->GetObjectPropertyByIdx(2, jsObjectValue.get());
      if (propertyValue->IsNull()) {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          uCount++;
          double dValue = ValueToDouble(pThis, newPropertyValue.get());
          dMaxValue = (uCount == 1) ? dValue : std::max(dMaxValue, dValue);
        }
      } else {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          uCount++;
          double dValue = ValueToDouble(pThis, newPropertyValue.get());
          dMaxValue = (uCount == 1) ? dValue : std::max(dMaxValue, dValue);
        }
      }
    } else if (argValue->IsObject()) {
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      GetObjectDefaultValue(argValue.get(), newPropertyValue.get());
      if (newPropertyValue->IsNull())
        continue;

      uCount++;
      double dValue = ValueToDouble(pThis, newPropertyValue.get());
      dMaxValue = (uCount == 1) ? dValue : std::max(dMaxValue, dValue);
    } else {
      uCount++;
      double dValue = ValueToDouble(pThis, argValue.get());
      dMaxValue = (uCount == 1) ? dValue : std::max(dMaxValue, dValue);
    }
  }
  if (uCount == 0) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(dMaxValue);
}

// static
void CFXJSE_FormCalcContext::Min(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  uint32_t uCount = 0;
  double dMinValue = 0.0;
  for (int32_t i = 0; i < info.Length(); i++) {
    auto argValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[i]);
    if (argValue->IsNull())
      continue;

    if (argValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectProperty("length", lengthValue.get());
      int32_t iLength = lengthValue->ToInteger();
      if (iLength <= 2) {
        pContext->ThrowArgumentMismatchException();
        return;
      }

      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectPropertyByIdx(1, propertyValue.get());
      argValue->GetObjectPropertyByIdx(2, jsObjectValue.get());
      if (propertyValue->IsNull()) {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          uCount++;
          double dValue = ValueToDouble(pThis, newPropertyValue.get());
          dMinValue = uCount == 1 ? dValue : std::min(dMinValue, dValue);
        }
      } else {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          uCount++;
          double dValue = ValueToDouble(pThis, newPropertyValue.get());
          dMinValue = uCount == 1 ? dValue : std::min(dMinValue, dValue);
        }
      }
    } else if (argValue->IsObject()) {
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      GetObjectDefaultValue(argValue.get(), newPropertyValue.get());
      if (newPropertyValue->IsNull())
        continue;

      uCount++;
      double dValue = ValueToDouble(pThis, newPropertyValue.get());
      dMinValue = uCount == 1 ? dValue : std::min(dMinValue, dValue);
    } else {
      uCount++;
      double dValue = ValueToDouble(pThis, argValue.get());
      dMinValue = uCount == 1 ? dValue : std::min(dMinValue, dValue);
    }
  }
  if (uCount == 0) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(dMinValue);
}

// static
void CFXJSE_FormCalcContext::Mod(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 2) {
    pContext->ThrowParamCountMismatchException(L"Mod");
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  auto argTwo = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[1]);
  if (argOne->IsNull() || argTwo->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  bool argOneResult;
  double dDividend = ExtractDouble(pThis, argOne.get(), &argOneResult);
  bool argTwoResult;
  double dDivisor = ExtractDouble(pThis, argTwo.get(), &argTwoResult);
  if (!argOneResult || !argTwoResult) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  if (dDivisor == 0.0) {
    pContext->ThrowDivideByZeroException();
    return;
  }

  info.GetReturnValue().Set(dDividend -
                            dDivisor * (int32_t)(dDividend / dDivisor));
}

// static
void CFXJSE_FormCalcContext::Round(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    pContext->ThrowParamCountMismatchException(L"Round");
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  bool dValueRet;
  double dValue = ExtractDouble(pThis, argOne.get(), &dValueRet);
  if (!dValueRet) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  uint8_t uPrecision = 0;
  if (argc > 1) {
    auto argTwo = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[1]);
    if (argTwo->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }

    bool dPrecisionRet;
    double dPrecision = ExtractDouble(pThis, argTwo.get(), &dPrecisionRet);
    if (!dPrecisionRet) {
      pContext->ThrowArgumentMismatchException();
      return;
    }

    uPrecision = static_cast<uint8_t>(pdfium::clamp(dPrecision, 0.0, 12.0));
  }

  CFGAS_Decimal decimalValue(static_cast<float>(dValue), uPrecision);
  info.GetReturnValue().Set(decimalValue.ToDouble());
}

// static
void CFXJSE_FormCalcContext::Sum(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc == 0) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  uint32_t uCount = 0;
  double dSum = 0.0;
  for (int32_t i = 0; i < argc; i++) {
    auto argValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[i]);
    if (argValue->IsNull())
      continue;

    if (argValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectProperty("length", lengthValue.get());
      int32_t iLength = lengthValue->ToInteger();
      if (iLength <= 2) {
        pContext->ThrowArgumentMismatchException();
        return;
      }

      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectPropertyByIdx(1, propertyValue.get());
      auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      if (propertyValue->IsNull()) {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          dSum += ValueToDouble(pThis, jsObjectValue.get());
          uCount++;
        }
      } else {
        for (int32_t j = 2; j < iLength; j++) {
          argValue->GetObjectPropertyByIdx(j, jsObjectValue.get());
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
          if (newPropertyValue->IsNull())
            continue;

          dSum += ValueToDouble(pThis, newPropertyValue.get());
          uCount++;
        }
      }
    } else if (argValue->IsObject()) {
      auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      GetObjectDefaultValue(argValue.get(), newPropertyValue.get());
      if (newPropertyValue->IsNull())
        continue;

      dSum += ValueToDouble(pThis, argValue.get());
      uCount++;
    } else {
      dSum += ValueToDouble(pThis, argValue.get());
      uCount++;
    }
  }
  if (uCount == 0) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(dSum);
}

// static
void CFXJSE_FormCalcContext::Date(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 0) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Date");
    return;
  }

  time_t currentTime;
  FXSYS_time(&currentTime);
  struct tm* pTmStruct = gmtime(&currentTime);

  info.GetReturnValue().Set(DateString2Num(
      ByteString::Format("%d%02d%02d", pTmStruct->tm_year + 1900,
                         pTmStruct->tm_mon + 1, pTmStruct->tm_mday)
          .AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Date2Num(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Date2Num");
    return;
  }

  std::unique_ptr<CFXJSE_Value> dateValue = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, dateValue.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsDate = ValueToUTF8String(dateValue.get());
  ByteString bsFormat;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> formatValue = GetSimpleValue(pThis, info, 1);
    if (ValueIsNull(pThis, formatValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(formatValue.get());
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (ValueIsNull(pThis, localeValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  ByteString bsIsoDate =
      Local2IsoDate(pThis, bsDate.AsStringView(), bsFormat.AsStringView(),
                    bsLocale.AsStringView());
  info.GetReturnValue().Set(DateString2Num(bsIsoDate.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::DateFmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Date2Num");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    std::unique_ptr<CFXJSE_Value> infotyle = GetSimpleValue(pThis, info, 0);
    if (infotyle->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }

    iStyle = (int32_t)ValueToFloat(pThis, infotyle.get());
    if (iStyle < 0 || iStyle > 4)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> argLocale = GetSimpleValue(pThis, info, 1);
    if (argLocale->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(argLocale.get());
  }

  ByteString bsFormat =
      GetStandardDateFormat(pThis, iStyle, bsLocale.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsFormat.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::IsoDate2Num(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"IsoDate2Num");
    return;
  }
  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  ByteString bsArg = ValueToUTF8String(argOne.get());
  info.GetReturnValue().Set(DateString2Num(bsArg.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::IsoTime2Num(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException(L"IsoTime2Num");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  ByteString bsArg = ValueToUTF8String(argOne.get());
  auto pos = bsArg.Find('T', 0);
  if (!pos.has_value() || pos.value() == bsArg.GetLength() - 1) {
    info.GetReturnValue().Set(0);
    return;
  }
  bsArg = bsArg.Last(bsArg.GetLength() - (pos.value() + 1));

  CXFA_LocaleValue timeValue(XFA_VT_TIME,
                             WideString::FromUTF8(bsArg.AsStringView()), pMgr);
  if (!timeValue.IsValid()) {
    info.GetReturnValue().Set(0);
    return;
  }

  CFX_DateTime uniTime = timeValue.GetTime();
  int32_t hour = uniTime.GetHour();
  int32_t min = uniTime.GetMinute();
  int32_t second = uniTime.GetSecond();
  int32_t milSecond = uniTime.GetMillisecond();

  // TODO(dsinclair): See if there is other time conversion code in pdfium and
  //   consolidate.
  int32_t mins = hour * 60 + min;
  mins -= (pMgr->GetDefLocale()->GetTimeZone().tzHour * 60);
  while (mins > 1440)
    mins -= 1440;
  while (mins < 0)
    mins += 1440;
  hour = mins / 60;
  min = mins % 60;

  info.GetReturnValue().Set(hour * 3600000 + min * 60000 + second * 1000 +
                            milSecond + 1);
}

// static
void CFXJSE_FormCalcContext::LocalDateFmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"LocalDateFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    std::unique_ptr<CFXJSE_Value> infotyle = GetSimpleValue(pThis, info, 0);
    if (infotyle->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = (int32_t)ValueToFloat(pThis, infotyle.get());
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> argLocale = GetSimpleValue(pThis, info, 1);
    if (argLocale->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(argLocale.get());
  }

  ByteString bsFormat =
      GetLocalDateFormat(pThis, iStyle, bsLocale.AsStringView(), false);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsFormat.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::LocalTimeFmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"LocalTimeFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    std::unique_ptr<CFXJSE_Value> infotyle = GetSimpleValue(pThis, info, 0);
    if (infotyle->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = (int32_t)ValueToFloat(pThis, infotyle.get());
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> argLocale = GetSimpleValue(pThis, info, 1);
    if (argLocale->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(argLocale.get());
  }

  ByteString bsFormat =
      GetLocalTimeFormat(pThis, iStyle, bsLocale.AsStringView(), false);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsFormat.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Num2Date(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Num2Date");
    return;
  }

  std::unique_ptr<CFXJSE_Value> dateValue = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, dateValue.get())) {
    info.GetReturnValue().SetNull();
    return;
  }
  int32_t dDate = (int32_t)ValueToFloat(pThis, dateValue.get());
  if (dDate < 1) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> formatValue = GetSimpleValue(pThis, info, 1);
    if (ValueIsNull(pThis, formatValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(formatValue.get());
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (ValueIsNull(pThis, localeValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  int32_t iYear = 1900;
  int32_t iMonth = 1;
  int32_t iDay = 1;
  int32_t i = 0;
  while (dDate > 0) {
    if (iMonth == 2) {
      if ((!((iYear + i) % 4) && ((iYear + i) % 100)) || !((iYear + i) % 400)) {
        if (dDate > 29) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 29;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      } else {
        if (dDate > 28) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 28;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      }
    } else if (iMonth < 8) {
      if ((iMonth % 2 == 0)) {
        if (dDate > 30) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 30;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      } else {
        if (dDate > 31) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 31;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      }
    } else {
      if (iMonth % 2 != 0) {
        if (dDate > 30) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 30;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      } else {
        if (dDate > 31) {
          ++iMonth;
          if (iMonth > 12) {
            iMonth = 1;
            ++i;
          }
          iDay = 1;
          dDate -= 31;
        } else {
          iDay += static_cast<int32_t>(dDate) - 1;
          dDate = 0;
        }
      }
    }
  }

  ByteString bsLocalDate = IsoDate2Local(
      pThis,
      ByteString::Format("%d%02d%02d", iYear + i, iMonth, iDay).AsStringView(),
      bsFormat.AsStringView(), bsLocale.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsLocalDate.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Num2GMTime(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Num2GMTime");
    return;
  }

  std::unique_ptr<CFXJSE_Value> timeValue = GetSimpleValue(pThis, info, 0);
  if (timeValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  int32_t iTime = (int32_t)ValueToFloat(pThis, timeValue.get());
  if (abs(iTime) < 1.0) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> formatValue = GetSimpleValue(pThis, info, 1);
    if (formatValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(formatValue.get());
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (localeValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  ByteString bsGMTTime = Num2AllTime(pThis, iTime, bsFormat.AsStringView(),
                                     bsLocale.AsStringView(), true);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsGMTTime.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Num2Time(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Num2Time");
    return;
  }

  std::unique_ptr<CFXJSE_Value> timeValue = GetSimpleValue(pThis, info, 0);
  if (timeValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fTime = ValueToFloat(pThis, timeValue.get());
  if (fabs(fTime) < 1.0) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> formatValue = GetSimpleValue(pThis, info, 1);
    if (formatValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(formatValue.get());
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (localeValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  ByteString bsLocalTime =
      Num2AllTime(pThis, static_cast<int32_t>(fTime), bsFormat.AsStringView(),
                  bsLocale.AsStringView(), false);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsLocalTime.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Time(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 0) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Time");
    return;
  }

  time_t now;
  FXSYS_time(&now);
  struct tm* pGmt = gmtime(&now);
  info.GetReturnValue().Set(
      (pGmt->tm_hour * 3600 + pGmt->tm_min * 60 + pGmt->tm_sec) * 1000);
}

// static
void CFXJSE_FormCalcContext::Time2Num(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Time2Num");
    return;
  }

  ByteString bsTime;
  std::unique_ptr<CFXJSE_Value> timeValue = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, timeValue.get())) {
    info.GetReturnValue().SetNull();
    return;
  }
  bsTime = ValueToUTF8String(timeValue.get());

  ByteString bsFormat;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> formatValue = GetSimpleValue(pThis, info, 1);
    if (ValueIsNull(pThis, formatValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(formatValue.get());
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (ValueIsNull(pThis, localeValue.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  LocaleIface* pLocale = nullptr;
  if (bsLocale.IsEmpty()) {
    CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
    pLocale = pThisNode->GetLocale();
  } else {
    pLocale =
        pMgr->GetLocaleByName(WideString::FromUTF8(bsLocale.AsStringView()));
  }

  WideString wsFormat;
  if (bsFormat.IsEmpty())
    wsFormat = pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Default);
  else
    wsFormat = WideString::FromUTF8(bsFormat.AsStringView());

  wsFormat = L"time{" + wsFormat + L"}";
  CXFA_LocaleValue localeValue(XFA_VT_TIME,
                               WideString::FromUTF8(bsTime.AsStringView()),
                               wsFormat, pLocale, pMgr);
  if (!localeValue.IsValid()) {
    info.GetReturnValue().Set(0);
    return;
  }

  CFX_DateTime uniTime = localeValue.GetTime();
  int32_t hour = uniTime.GetHour();
  int32_t min = uniTime.GetMinute();
  int32_t second = uniTime.GetSecond();
  int32_t milSecond = uniTime.GetMillisecond();
  int32_t mins = hour * 60 + min;

  mins -= (CXFA_TimeZoneProvider().GetTimeZone().tzHour * 60);
  while (mins > 1440)
    mins -= 1440;

  while (mins < 0)
    mins += 1440;

  hour = mins / 60;
  min = mins % 60;
  info.GetReturnValue().Set(hour * 3600000 + min * 60000 + second * 1000 +
                            milSecond + 1);
}

// static
void CFXJSE_FormCalcContext::TimeFmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"TimeFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    std::unique_ptr<CFXJSE_Value> infotyle = GetSimpleValue(pThis, info, 0);
    if (infotyle->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = (int32_t)ValueToFloat(pThis, infotyle.get());
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> argLocale = GetSimpleValue(pThis, info, 1);
    if (argLocale->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(argLocale.get());
  }

  ByteString bsFormat =
      GetStandardTimeFormat(pThis, iStyle, bsLocale.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsFormat.AsStringView()));
}

// static
ByteString CFXJSE_FormCalcContext::Local2IsoDate(CFXJSE_HostObject* pThis,
                                                 ByteStringView bsDate,
                                                 ByteStringView bsFormat,
                                                 ByteStringView bsLocale) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return ByteString();

  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  LocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = FormatFromString(pLocale, bsFormat);
  CFX_DateTime dt = CXFA_LocaleValue(XFA_VT_DATE, WideString::FromUTF8(bsDate),
                                     wsFormat, pLocale, pMgr)
                        .GetDate();

  return ByteString::Format("%4d-%02d-%02d", dt.GetYear(), dt.GetMonth(),
                            dt.GetDay());
}

// static
ByteString CFXJSE_FormCalcContext::IsoDate2Local(CFXJSE_HostObject* pThis,
                                                 ByteStringView bsDate,
                                                 ByteStringView bsFormat,
                                                 ByteStringView bsLocale) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return ByteString();

  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  LocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = FormatFromString(pLocale, bsFormat);
  WideString wsRet;
  CXFA_LocaleValue(XFA_VT_DATE, WideString::FromUTF8(bsDate), pMgr)
      .FormatPatterns(wsRet, wsFormat, pLocale, XFA_VALUEPICTURE_Display);
  return wsRet.ToUTF8();
}

// static
ByteString CFXJSE_FormCalcContext::IsoTime2Local(CFXJSE_HostObject* pThis,
                                                 ByteStringView bsTime,
                                                 ByteStringView bsFormat,
                                                 ByteStringView bsLocale) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return ByteString();

  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  LocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = {
      L"time{", FormatFromString(pLocale, bsFormat).AsStringView(), L"}"};
  CXFA_LocaleValue widgetValue(XFA_VT_TIME, WideString::FromUTF8(bsTime), pMgr);
  WideString wsRet;
  widgetValue.FormatPatterns(wsRet, wsFormat, pLocale,
                             XFA_VALUEPICTURE_Display);
  return wsRet.ToUTF8();
}

// static
ByteString CFXJSE_FormCalcContext::GetLocalDateFormat(CFXJSE_HostObject* pThis,
                                                      int32_t iStyle,
                                                      ByteStringView bsLocale,
                                                      bool bStandard) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return ByteString();

  return GetLocalDateTimeFormat(pDoc, iStyle, bsLocale, bStandard,
                                /*bIsDate=*/true);
}

// static
ByteString CFXJSE_FormCalcContext::GetLocalTimeFormat(CFXJSE_HostObject* pThis,
                                                      int32_t iStyle,
                                                      ByteStringView bsLocale,
                                                      bool bStandard) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return ByteString();

  return GetLocalDateTimeFormat(pDoc, iStyle, bsLocale, bStandard,
                                /*bIsDate=*/false);
}

// static
ByteString CFXJSE_FormCalcContext::GetStandardDateFormat(
    CFXJSE_HostObject* pThis,
    int32_t iStyle,
    ByteStringView bsLocale) {
  return GetLocalDateFormat(pThis, iStyle, bsLocale, true);
}

// static
ByteString CFXJSE_FormCalcContext::GetStandardTimeFormat(
    CFXJSE_HostObject* pThis,
    int32_t iStyle,
    ByteStringView bsLocale) {
  return GetLocalTimeFormat(pThis, iStyle, bsLocale, true);
}

// static
ByteString CFXJSE_FormCalcContext::Num2AllTime(CFXJSE_HostObject* pThis,
                                               int32_t iTime,
                                               ByteStringView bsFormat,
                                               ByteStringView bsLocale,
                                               bool bGM) {
  int32_t iHour = 0;
  int32_t iMin = 0;
  int32_t iSec = 0;
  iHour = static_cast<int>(iTime) / 3600000;
  iMin = (static_cast<int>(iTime) - iHour * 3600000) / 60000;
  iSec = (static_cast<int>(iTime) - iHour * 3600000 - iMin * 60000) / 1000;

  if (!bGM) {
    int32_t iZoneHour;
    int32_t iZoneMin;
    int32_t iZoneSec;
    GetLocalTimeZone(&iZoneHour, &iZoneMin, &iZoneSec);
    iHour += iZoneHour;
    iMin += iZoneMin;
    iSec += iZoneSec;
  }

  return IsoTime2Local(
      pThis,
      ByteString::Format("%02d:%02d:%02d", iHour, iMin, iSec).AsStringView(),
      bsFormat, bsLocale);
}

// static
void CFXJSE_FormCalcContext::Apr(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"Apr");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nPrincipal = ValueToDouble(pThis, argOne.get());
  double nPayment = ValueToDouble(pThis, argTwo.get());
  double nPeriods = ValueToDouble(pThis, argThree.get());
  if (nPrincipal <= 0 || nPayment <= 0 || nPeriods <= 0) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double r = 2 * (nPeriods * nPayment - nPrincipal) / (nPeriods * nPrincipal);
  double nTemp = 1;
  for (int32_t i = 0; i < nPeriods; ++i)
    nTemp *= (1 + r);

  double nRet = r * nTemp / (nTemp - 1) - nPayment / nPrincipal;
  while (fabs(nRet) > kFinancialPrecision) {
    double nDerivative =
        ((nTemp + r * nPeriods * (nTemp / (1 + r))) * (nTemp - 1) -
         (r * nTemp * nPeriods * (nTemp / (1 + r)))) /
        ((nTemp - 1) * (nTemp - 1));
    if (nDerivative == 0) {
      info.GetReturnValue().SetNull();
      return;
    }

    r = r - nRet / nDerivative;
    nTemp = 1;
    for (int32_t i = 0; i < nPeriods; ++i) {
      nTemp *= (1 + r);
    }
    nRet = r * nTemp / (nTemp - 1) - nPayment / nPrincipal;
  }
  info.GetReturnValue().Set(r * 12);
}

// static
void CFXJSE_FormCalcContext::CTerm(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"CTerm");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nRate = ValueToFloat(pThis, argOne.get());
  float nFutureValue = ValueToFloat(pThis, argTwo.get());
  float nInitAmount = ValueToFloat(pThis, argThree.get());
  if ((nRate <= 0) || (nFutureValue <= 0) || (nInitAmount <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  info.GetReturnValue().Set(log((float)(nFutureValue / nInitAmount)) /
                            log((float)(1 + nRate)));
}

// static
void CFXJSE_FormCalcContext::FV(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"FV");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nAmount = ValueToDouble(pThis, argOne.get());
  double nRate = ValueToDouble(pThis, argTwo.get());
  double nPeriod = ValueToDouble(pThis, argThree.get());
  if ((nRate < 0) || (nPeriod <= 0) || (nAmount <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double dResult = 0;
  if (nRate) {
    double nTemp = 1;
    for (int i = 0; i < nPeriod; ++i) {
      nTemp *= 1 + nRate;
    }
    dResult = nAmount * (nTemp - 1) / nRate;
  } else {
    dResult = nAmount * nPeriod;
  }

  info.GetReturnValue().Set(dResult);
}

// static
void CFXJSE_FormCalcContext::IPmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 5) {
    pContext->ThrowParamCountMismatchException(L"IPmt");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  std::unique_ptr<CFXJSE_Value> argFour = GetSimpleValue(pThis, info, 3);
  std::unique_ptr<CFXJSE_Value> argFive = GetSimpleValue(pThis, info, 4);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get()) || ValueIsNull(pThis, argFour.get()) ||
      ValueIsNull(pThis, argFive.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nPrincipalAmount = ValueToFloat(pThis, argOne.get());
  float nRate = ValueToFloat(pThis, argTwo.get());
  float nPayment = ValueToFloat(pThis, argThree.get());
  float nFirstMonth = ValueToFloat(pThis, argFour.get());
  float nNumberOfMonths = ValueToFloat(pThis, argFive.get());
  if ((nPrincipalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
      (nFirstMonth < 0) || (nNumberOfMonths < 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  float nRateOfMonth = nRate / 12;
  int32_t iNums =
      (int32_t)((log10((float)(nPayment / nPrincipalAmount)) -
                 log10((float)(nPayment / nPrincipalAmount - nRateOfMonth))) /
                log10((float)(1 + nRateOfMonth)));
  int32_t iEnd = std::min((int32_t)(nFirstMonth + nNumberOfMonths - 1), iNums);

  if (nPayment < nPrincipalAmount * nRateOfMonth) {
    info.GetReturnValue().Set(0);
    return;
  }

  int32_t i = 0;
  for (i = 0; i < nFirstMonth - 1; ++i)
    nPrincipalAmount -= nPayment - nPrincipalAmount * nRateOfMonth;

  float nSum = 0;
  for (; i < iEnd; ++i) {
    nSum += nPrincipalAmount * nRateOfMonth;
    nPrincipalAmount -= nPayment - nPrincipalAmount * nRateOfMonth;
  }
  info.GetReturnValue().Set(nSum);
}

// static
void CFXJSE_FormCalcContext::NPV(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 3) {
    pContext->ThrowParamCountMismatchException(L"NPV");
    return;
  }

  std::vector<std::unique_ptr<CFXJSE_Value>> argValues;
  for (int32_t i = 0; i < argc; i++) {
    argValues.push_back(GetSimpleValue(pThis, info, i));
    if (ValueIsNull(pThis, argValues[i].get())) {
      info.GetReturnValue().SetNull();
      return;
    }
  }

  double nRate = ValueToDouble(pThis, argValues[0].get());
  if (nRate <= 0) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  std::vector<double> data(argc - 1);
  for (int32_t i = 1; i < argc; i++)
    data.push_back(ValueToDouble(pThis, argValues[i].get()));

  double nSum = 0;
  int32_t iIndex = 0;
  for (int32_t i = 0; i < argc - 1; i++) {
    double nTemp = 1;
    for (int32_t j = 0; j <= i; j++)
      nTemp *= 1 + nRate;

    double nNum = data[iIndex++];
    nSum += nNum / nTemp;
  }
  info.GetReturnValue().Set(nSum);
}

// static
void CFXJSE_FormCalcContext::Pmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"Pmt");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nPrincipal = ValueToFloat(pThis, argOne.get());
  float nRate = ValueToFloat(pThis, argTwo.get());
  float nPeriods = ValueToFloat(pThis, argThree.get());
  if ((nPrincipal <= 0) || (nRate <= 0) || (nPeriods <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  float nTmp = 1 + nRate;
  float nSum = nTmp;
  for (int32_t i = 0; i < nPeriods - 1; ++i)
    nSum *= nTmp;

  info.GetReturnValue().Set((nPrincipal * nRate * nSum) / (nSum - 1));
}

// static
void CFXJSE_FormCalcContext::PPmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 5) {
    pContext->ThrowParamCountMismatchException(L"PPmt");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  std::unique_ptr<CFXJSE_Value> argFour = GetSimpleValue(pThis, info, 3);
  std::unique_ptr<CFXJSE_Value> argFive = GetSimpleValue(pThis, info, 4);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get()) || ValueIsNull(pThis, argFour.get()) ||
      ValueIsNull(pThis, argFive.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nPrincipalAmount = ValueToFloat(pThis, argOne.get());
  float nRate = ValueToFloat(pThis, argTwo.get());
  float nPayment = ValueToFloat(pThis, argThree.get());
  float nFirstMonth = ValueToFloat(pThis, argFour.get());
  float nNumberOfMonths = ValueToFloat(pThis, argFive.get());
  if ((nPrincipalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
      (nFirstMonth < 0) || (nNumberOfMonths < 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  float nRateOfMonth = nRate / 12;
  int32_t iNums =
      (int32_t)((log10((float)(nPayment / nPrincipalAmount)) -
                 log10((float)(nPayment / nPrincipalAmount - nRateOfMonth))) /
                log10((float)(1 + nRateOfMonth)));
  int32_t iEnd = std::min((int32_t)(nFirstMonth + nNumberOfMonths - 1), iNums);
  if (nPayment < nPrincipalAmount * nRateOfMonth) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  int32_t i = 0;
  for (i = 0; i < nFirstMonth - 1; ++i)
    nPrincipalAmount -= nPayment - nPrincipalAmount * nRateOfMonth;

  float nTemp = 0;
  float nSum = 0;
  for (; i < iEnd; ++i) {
    nTemp = nPayment - nPrincipalAmount * nRateOfMonth;
    nSum += nTemp;
    nPrincipalAmount -= nTemp;
  }
  info.GetReturnValue().Set(nSum);
}

// static
void CFXJSE_FormCalcContext::PV(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"PV");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nAmount = ValueToDouble(pThis, argOne.get());
  double nRate = ValueToDouble(pThis, argTwo.get());
  double nPeriod = ValueToDouble(pThis, argThree.get());
  if ((nAmount <= 0) || (nRate < 0) || (nPeriod <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double nTemp = 1;
  for (int32_t i = 0; i < nPeriod; ++i)
    nTemp *= 1 + nRate;

  nTemp = 1 / nTemp;
  info.GetReturnValue().Set(nAmount * ((1 - nTemp) / nRate));
}

// static
void CFXJSE_FormCalcContext::Rate(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"Rate");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nFuture = ValueToFloat(pThis, argOne.get());
  float nPresent = ValueToFloat(pThis, argTwo.get());
  float nTotalNumber = ValueToFloat(pThis, argThree.get());
  if ((nFuture <= 0) || (nPresent < 0) || (nTotalNumber <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  info.GetReturnValue().Set(
      FXSYS_pow((float)(nFuture / nPresent), (float)(1 / nTotalNumber)) - 1);
}

// static
void CFXJSE_FormCalcContext::Term(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException(L"Term");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get()) ||
      ValueIsNull(pThis, argThree.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nMount = ValueToFloat(pThis, argOne.get());
  float nRate = ValueToFloat(pThis, argTwo.get());
  float nFuture = ValueToFloat(pThis, argThree.get());
  if ((nMount <= 0) || (nRate <= 0) || (nFuture <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  info.GetReturnValue().Set(log((float)(nFuture / nMount * nRate) + 1) /
                            log((float)(1 + nRate)));
}

// static
void CFXJSE_FormCalcContext::Choose(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 2) {
    pContext->ThrowParamCountMismatchException(L"Choose");
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  int32_t iIndex = (int32_t)ValueToFloat(pThis, argOne.get());
  if (iIndex < 1) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  bool bFound = false;
  bool bStopCounterFlags = false;
  int32_t iArgIndex = 1;
  int32_t iValueIndex = 0;
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  while (!bFound && !bStopCounterFlags && (iArgIndex < argc)) {
    auto argIndexValue =
        pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[iArgIndex]);
    if (argIndexValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argIndexValue->GetObjectProperty("length", lengthValue.get());
      int32_t iLength = lengthValue->ToInteger();
      if (iLength > 3)
        bStopCounterFlags = true;

      iValueIndex += (iLength - 2);
      if (iValueIndex >= iIndex) {
        auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
        auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
        auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
        argIndexValue->GetObjectPropertyByIdx(1, propertyValue.get());
        argIndexValue->GetObjectPropertyByIdx(
            (iLength - 1) - (iValueIndex - iIndex), jsObjectValue.get());
        if (propertyValue->IsNull()) {
          GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
        } else {
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), newPropertyValue.get());
        }
        ByteString bsChosen = ValueToUTF8String(newPropertyValue.get());
        info.GetReturnValue().Set(
            fxv8::NewStringHelper(info.GetIsolate(), bsChosen.AsStringView()));
        bFound = true;
      }
    } else {
      iValueIndex++;
      if (iValueIndex == iIndex) {
        ByteString bsChosen = ValueToUTF8String(argIndexValue.get());
        info.GetReturnValue().Set(
            fxv8::NewStringHelper(info.GetIsolate(), bsChosen.AsStringView()));
        bFound = true;
      }
    }
    iArgIndex++;
  }
  if (!bFound)
    info.GetReturnValue().SetEmptyString();
}

// static
void CFXJSE_FormCalcContext::Exists(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Exists");
    return;
  }
  auto temp = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  info.GetReturnValue().Set(static_cast<int>(temp->IsObject()));
}

// static
void CFXJSE_FormCalcContext::HasValue(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"HasValue");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (!argOne->IsString()) {
    info.GetReturnValue().Set(
        static_cast<int>(argOne->IsNumber() || argOne->IsBoolean()));
    return;
  }

  ByteString bsValue = argOne->ToString();
  bsValue.TrimLeft();
  info.GetReturnValue().Set(static_cast<int>(!bsValue.IsEmpty()));
}

// static
void CFXJSE_FormCalcContext::Oneof(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Oneof");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  for (const auto& value : unfoldArgs(pThis, info)) {
    if (simpleValueCompare(pThis, argOne.get(), value.get())) {
      info.GetReturnValue().Set(1);
      return;
    }
  }
  info.GetReturnValue().Set(0);
}

// static
void CFXJSE_FormCalcContext::Within(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Within");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetUndefined();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argLow = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> argHigh = GetSimpleValue(pThis, info, 2);
  if (argOne->IsNumber()) {
    float oneNumber = ValueToFloat(pThis, argOne.get());
    float lowNumber = ValueToFloat(pThis, argLow.get());
    float heightNumber = ValueToFloat(pThis, argHigh.get());
    info.GetReturnValue().Set(static_cast<int>((oneNumber >= lowNumber) &&
                                               (oneNumber <= heightNumber)));
    return;
  }

  ByteString bsOne = ValueToUTF8String(argOne.get());
  ByteString bsLow = ValueToUTF8String(argLow.get());
  ByteString bsHeight = ValueToUTF8String(argHigh.get());
  info.GetReturnValue().Set(
      static_cast<int>((bsOne.Compare(bsLow.AsStringView()) >= 0) &&
                       (bsOne.Compare(bsHeight.AsStringView()) <= 0)));
}

// static
void CFXJSE_FormCalcContext::If(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"If");
    return;
  }

  std::unique_ptr<CFXJSE_Value> value =
      GetSimpleValue(pThis, info, 0)->ToBoolean()
          ? GetSimpleValue(pThis, info, 1)
          : GetSimpleValue(pThis, info, 2);

  info.GetReturnValue().Set(value->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::Eval(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException(L"Eval");
    return;
  }

  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  std::unique_ptr<CFXJSE_Value> scriptValue = GetSimpleValue(pThis, info, 0);
  ByteString bsUtf8Script = ValueToUTF8String(scriptValue.get());
  if (bsUtf8Script.IsEmpty()) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFX_WideTextBuf wsJavaScriptBuf;
  if (!CFXJSE_FormCalcContext::Translate(
          WideString::FromUTF8(bsUtf8Script.AsStringView()).AsStringView(),
          &wsJavaScriptBuf)) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Context> pNewContext(
      CFXJSE_Context::Create(pIsolate, nullptr, nullptr));

  auto returnValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  pNewContext->ExecuteScript(
      FX_UTF8Encode(wsJavaScriptBuf.AsStringView()).c_str(), returnValue.get(),
      nullptr);

  info.GetReturnValue().Set(returnValue->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::Ref(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException(L"Ref");
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (!argOne->IsArray() && !argOne->IsObject() && !argOne->IsBoolean() &&
      !argOne->IsString() && !argOne->IsNull() && !argOne->IsNumber()) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  if (argOne->IsBoolean() || argOne->IsString() || argOne->IsNumber()) {
    info.GetReturnValue().Set(argOne->DirectGetValue());
    return;
  }

  std::vector<std::unique_ptr<CFXJSE_Value>> values;
  for (int32_t i = 0; i < 3; i++)
    values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));

  int intVal = 3;
  if (argOne->IsNull()) {
    // TODO(dsinclair): Why is this 4 when the others are all 3?
    intVal = 4;
    values[2]->SetNull();
  } else if (argOne->IsArray()) {
#ifndef NDEBUG
    auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argOne->GetObjectProperty("length", lengthValue.get());
    ASSERT(lengthValue->ToInteger() >= 3);
#endif

    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argOne->GetObjectPropertyByIdx(1, propertyValue.get());
    argOne->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (!propertyValue->IsNull() || jsObjectValue->IsNull()) {
      pContext->ThrowArgumentMismatchException();
      return;
    }

    values[2]->Assign(jsObjectValue.get());
  } else if (argOne->IsObject()) {
    values[2]->Assign(argOne.get());
  }

  values[0]->SetInteger(intVal);
  values[1]->SetNull();

  auto temp = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
  temp->SetArray(values);
  info.GetReturnValue().Set(temp->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::UnitType(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"UnitType");
    return;
  }

  std::unique_ptr<CFXJSE_Value> unitspanValue = GetSimpleValue(pThis, info, 0);
  if (unitspanValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsUnitspan = ValueToUTF8String(unitspanValue.get());
  if (bsUnitspan.IsEmpty()) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  enum XFA_FM2JS_VALUETYPE_ParserStatus {
    VALUETYPE_START,
    VALUETYPE_HAVEINVALIDCHAR,
    VALUETYPE_HAVEDIGIT,
    VALUETYPE_HAVEDIGITWHITE,
    VALUETYPE_ISCM,
    VALUETYPE_ISMM,
    VALUETYPE_ISPT,
    VALUETYPE_ISMP,
    VALUETYPE_ISIN,
  };
  bsUnitspan.MakeLower();
  WideString wsType = WideString::FromUTF8(bsUnitspan.AsStringView());
  const wchar_t* pData = wsType.c_str();
  int32_t u = 0;
  int32_t uLen = wsType.GetLength();
  while (IsWhitespace(pData[u]))
    u++;

  XFA_FM2JS_VALUETYPE_ParserStatus eParserStatus = VALUETYPE_START;
  wchar_t typeChar;
  // TODO(dsinclair): Cleanup this parser, figure out what the various checks
  //    are for.
  while (u < uLen) {
    typeChar = pData[u];
    if (IsWhitespace(typeChar)) {
      if (eParserStatus != VALUETYPE_HAVEDIGIT &&
          eParserStatus != VALUETYPE_HAVEDIGITWHITE) {
        eParserStatus = VALUETYPE_ISIN;
        break;
      }
      eParserStatus = VALUETYPE_HAVEDIGITWHITE;
    } else if (IsPartOfNumberW(typeChar)) {
      if (eParserStatus == VALUETYPE_HAVEDIGITWHITE) {
        eParserStatus = VALUETYPE_ISIN;
        break;
      }
      eParserStatus = VALUETYPE_HAVEDIGIT;
    } else if ((typeChar == 'c' || typeChar == 'p') && (u + 1 < uLen)) {
      wchar_t nextChar = pData[u + 1];
      if ((eParserStatus == VALUETYPE_START ||
           eParserStatus == VALUETYPE_HAVEDIGIT ||
           eParserStatus == VALUETYPE_HAVEDIGITWHITE) &&
          !IsPartOfNumberW(nextChar)) {
        eParserStatus = (typeChar == 'c') ? VALUETYPE_ISCM : VALUETYPE_ISPT;
        break;
      }
      eParserStatus = VALUETYPE_HAVEINVALIDCHAR;
    } else if (typeChar == 'm' && (u + 1 < uLen)) {
      wchar_t nextChar = pData[u + 1];
      if ((eParserStatus == VALUETYPE_START ||
           eParserStatus == VALUETYPE_HAVEDIGIT ||
           eParserStatus == VALUETYPE_HAVEDIGITWHITE) &&
          !IsPartOfNumberW(nextChar)) {
        eParserStatus = VALUETYPE_ISMM;
        if (nextChar == 'p' || ((u + 5 < uLen) && pData[u + 1] == 'i' &&
                                pData[u + 2] == 'l' && pData[u + 3] == 'l' &&
                                pData[u + 4] == 'i' && pData[u + 5] == 'p')) {
          eParserStatus = VALUETYPE_ISMP;
        }
        break;
      }
    } else {
      eParserStatus = VALUETYPE_HAVEINVALIDCHAR;
    }
    u++;
  }
  switch (eParserStatus) {
    case VALUETYPE_ISCM:
      info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "cm"));
      break;
    case VALUETYPE_ISMM:
      info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "mm"));
      break;
    case VALUETYPE_ISPT:
      info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "pt"));
      break;
    case VALUETYPE_ISMP:
      info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "mp"));
      break;
    default:
      info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "in"));
      break;
  }
}

// static
void CFXJSE_FormCalcContext::UnitValue(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"UnitValue");
    return;
  }

  std::unique_ptr<CFXJSE_Value> unitspanValue = GetSimpleValue(pThis, info, 0);
  if (unitspanValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsUnitspan = ValueToUTF8String(unitspanValue.get());
  const char* pData = bsUnitspan.c_str();
  if (!pData) {
    info.GetReturnValue().Set(0);
    return;
  }

  size_t u = 0;
  while (IsWhitespace(pData[u]))
    ++u;

  while (u < bsUnitspan.GetLength()) {
    if (!IsPartOfNumber(pData[u]))
      break;
    ++u;
  }

  char* pTemp = nullptr;
  double dFirstNumber = strtod(pData, &pTemp);
  while (IsWhitespace(pData[u]))
    ++u;

  size_t uLen = bsUnitspan.GetLength();
  ByteString bsFirstUnit;
  while (u < uLen) {
    if (pData[u] == ' ')
      break;

    bsFirstUnit += pData[u];
    ++u;
  }
  bsFirstUnit.MakeLower();

  ByteString bsUnit;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> unitValue = GetSimpleValue(pThis, info, 1);
    ByteString bsUnitTemp = ValueToUTF8String(unitValue.get());
    const char* pChar = bsUnitTemp.c_str();
    size_t uVal = 0;
    while (IsWhitespace(pChar[uVal]))
      ++uVal;

    while (uVal < bsUnitTemp.GetLength()) {
      if (!std::isdigit(pChar[uVal]) && pChar[uVal] != '.')
        break;
      ++uVal;
    }
    while (IsWhitespace(pChar[uVal]))
      ++uVal;

    size_t uValLen = bsUnitTemp.GetLength();
    while (uVal < uValLen) {
      if (pChar[uVal] == ' ')
        break;

      bsUnit += pChar[uVal];
      ++uVal;
    }
    bsUnit.MakeLower();
  } else {
    bsUnit = bsFirstUnit;
  }

  double dResult = 0;
  if (bsFirstUnit == "in" || bsFirstUnit == "inches") {
    if (bsUnit == "mm" || bsUnit == "millimeters")
      dResult = dFirstNumber * 25.4;
    else if (bsUnit == "cm" || bsUnit == "centimeters")
      dResult = dFirstNumber * 2.54;
    else if (bsUnit == "pt" || bsUnit == "points")
      dResult = dFirstNumber / 72;
    else if (bsUnit == "mp" || bsUnit == "millipoints")
      dResult = dFirstNumber / 72000;
    else
      dResult = dFirstNumber;
  } else if (bsFirstUnit == "mm" || bsFirstUnit == "millimeters") {
    if (bsUnit == "mm" || bsUnit == "millimeters")
      dResult = dFirstNumber;
    else if (bsUnit == "cm" || bsUnit == "centimeters")
      dResult = dFirstNumber / 10;
    else if (bsUnit == "pt" || bsUnit == "points")
      dResult = dFirstNumber / 25.4 / 72;
    else if (bsUnit == "mp" || bsUnit == "millipoints")
      dResult = dFirstNumber / 25.4 / 72000;
    else
      dResult = dFirstNumber / 25.4;
  } else if (bsFirstUnit == "cm" || bsFirstUnit == "centimeters") {
    if (bsUnit == "mm" || bsUnit == "millimeters")
      dResult = dFirstNumber * 10;
    else if (bsUnit == "cm" || bsUnit == "centimeters")
      dResult = dFirstNumber;
    else if (bsUnit == "pt" || bsUnit == "points")
      dResult = dFirstNumber / 2.54 / 72;
    else if (bsUnit == "mp" || bsUnit == "millipoints")
      dResult = dFirstNumber / 2.54 / 72000;
    else
      dResult = dFirstNumber / 2.54;
  } else if (bsFirstUnit == "pt" || bsFirstUnit == "points") {
    if (bsUnit == "mm" || bsUnit == "millimeters")
      dResult = dFirstNumber / 72 * 25.4;
    else if (bsUnit == "cm" || bsUnit == "centimeters")
      dResult = dFirstNumber / 72 * 2.54;
    else if (bsUnit == "pt" || bsUnit == "points")
      dResult = dFirstNumber;
    else if (bsUnit == "mp" || bsUnit == "millipoints")
      dResult = dFirstNumber * 1000;
    else
      dResult = dFirstNumber / 72;
  } else if (bsFirstUnit == "mp" || bsFirstUnit == "millipoints") {
    if (bsUnit == "mm" || bsUnit == "millimeters")
      dResult = dFirstNumber / 72000 * 25.4;
    else if (bsUnit == "cm" || bsUnit == "centimeters")
      dResult = dFirstNumber / 72000 * 2.54;
    else if (bsUnit == "pt" || bsUnit == "points")
      dResult = dFirstNumber / 1000;
    else if (bsUnit == "mp" || bsUnit == "millipoints")
      dResult = dFirstNumber;
    else
      dResult = dFirstNumber / 72000;
  }
  info.GetReturnValue().Set(dResult);
}

// static
void CFXJSE_FormCalcContext::At(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"At");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString stringTwo = ValueToUTF8String(argTwo.get());
  if (stringTwo.IsEmpty()) {
    info.GetReturnValue().Set(1);
    return;
  }

  ByteString stringOne = ValueToUTF8String(argOne.get());
  auto pos = stringOne.Find(stringTwo.AsStringView());
  info.GetReturnValue().Set(
      static_cast<int>(pos.has_value() ? pos.value() + 1 : 0));
}

// static
void CFXJSE_FormCalcContext::Concat(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Concat");
    return;
  }

  ByteString bsResult;
  bool bAllNull = true;
  for (int32_t i = 0; i < argc; i++) {
    std::unique_ptr<CFXJSE_Value> value = GetSimpleValue(pThis, info, i);
    if (ValueIsNull(pThis, value.get()))
      continue;

    bAllNull = false;
    bsResult += ValueToUTF8String(value.get());
  }

  if (bAllNull) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsResult.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Decode(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Decode");
    return;
  }

  if (argc == 1) {
    std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
    if (ValueIsNull(pThis, argOne.get())) {
      info.GetReturnValue().SetNull();
      return;
    }

    WideString decoded = DecodeURL(
        WideString::FromUTF8(ValueToUTF8String(argOne.get()).AsStringView()));
    auto result = FX_UTF8Encode(decoded.AsStringView());
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsToDecode = ValueToUTF8String(argOne.get());
  ByteString bsIdentify = ValueToUTF8String(argTwo.get());
  WideString decoded;

  WideString wsToDecode = WideString::FromUTF8(bsToDecode.AsStringView());

  if (bsIdentify.EqualNoCase("html"))
    decoded = DecodeHTML(wsToDecode);
  else if (bsIdentify.EqualNoCase("xml"))
    decoded = DecodeXML(wsToDecode);
  else
    decoded = DecodeURL(wsToDecode);

  auto result = FX_UTF8Encode(decoded.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Encode(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Encode");
    return;
  }

  if (argc == 1) {
    std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
    if (ValueIsNull(pThis, argOne.get())) {
      info.GetReturnValue().SetNull();
      return;
    }
    WideString encoded = EncodeURL(ValueToUTF8String(argOne.get()));
    auto result = FX_UTF8Encode(encoded.AsStringView());
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if (ValueIsNull(pThis, argOne.get()) || ValueIsNull(pThis, argTwo.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsToEncode = ValueToUTF8String(argOne.get());
  ByteString bsIdentify = ValueToUTF8String(argTwo.get());
  WideString encoded;
  if (bsIdentify.EqualNoCase("html"))
    encoded = EncodeHTML(bsToEncode);
  else if (bsIdentify.EqualNoCase("xml"))
    encoded = EncodeXML(bsToEncode);
  else
    encoded = EncodeURL(bsToEncode);

  auto result = FX_UTF8Encode(encoded.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Format(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() < 2) {
    pContext->ThrowParamCountMismatchException(L"Format");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  ByteString bsPattern = ValueToUTF8String(argOne.get());

  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  ByteString bsValue = ValueToUTF8String(argTwo.get());

  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
  LocaleIface* pLocale = pThisNode->GetLocale();
  WideString wsPattern = WideString::FromUTF8(bsPattern.AsStringView());
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  bool bPatternIsString;
  uint32_t dwPatternType;
  std::tie(bPatternIsString, dwPatternType) =
      PatternStringType(bsPattern.AsStringView());
  if (!bPatternIsString) {
    switch (dwPatternType) {
      case XFA_VT_DATETIME: {
        auto iTChar = wsPattern.Find(L'T');
        if (!iTChar.has_value()) {
          info.GetReturnValue().SetEmptyString();
          return;
        }
        WideString wsDatePattern(L"date{");
        wsDatePattern += wsPattern.First(iTChar.value()) + L"} ";

        WideString wsTimePattern(L"time{");
        wsTimePattern +=
            wsPattern.Last(wsPattern.GetLength() - (iTChar.value() + 1)) + L"}";
        wsPattern = wsDatePattern + wsTimePattern;
      } break;
      case XFA_VT_DATE: {
        wsPattern = L"date{" + wsPattern + L"}";
      } break;
      case XFA_VT_TIME: {
        wsPattern = L"time{" + wsPattern + L"}";
      } break;
      case XFA_VT_TEXT: {
        wsPattern = L"text{" + wsPattern + L"}";
      } break;
      case XFA_VT_FLOAT: {
        wsPattern = L"num{" + wsPattern + L"}";
      } break;
      default: {
        WideString wsTestPattern = L"num{" + wsPattern + L"}";
        CXFA_LocaleValue tempLocaleValue(XFA_VT_FLOAT, wsValue, wsTestPattern,
                                         pLocale, pMgr);
        if (tempLocaleValue.IsValid()) {
          wsPattern = std::move(wsTestPattern);
          dwPatternType = XFA_VT_FLOAT;
        } else {
          wsPattern = L"text{" + wsPattern + L"}";
          dwPatternType = XFA_VT_TEXT;
        }
      } break;
    }
  }
  CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                               pMgr);
  WideString wsRet;
  if (!localeValue.FormatPatterns(wsRet, wsPattern, pLocale,
                                  XFA_VALUEPICTURE_Display)) {
    info.GetReturnValue().SetEmptyString();
    return;
  }
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), wsRet.ToUTF8().AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Left(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Left");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if ((ValueIsNull(pThis, argOne.get())) ||
      (ValueIsNull(pThis, argTwo.get()))) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(argOne.get());
  int32_t count = std::max(0, ValueToInteger(pThis, argTwo.get()));
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), bsSource.First(count).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Len(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Len");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(argOne.get());
  info.GetReturnValue().Set(static_cast<int>(bsSource.GetLength()));
}

// static
void CFXJSE_FormCalcContext::Lower(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Lower");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFX_WideTextBuf szLowBuf;
  ByteString bsArg = ValueToUTF8String(argOne.get());
  WideString wsArg = WideString::FromUTF8(bsArg.AsStringView());
  for (wchar_t ch : wsArg) {
    if ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0xC0 && ch <= 0xDE))
      ch += 32;
    else if (ch == 0x100 || ch == 0x102 || ch == 0x104)
      ch += 1;
    szLowBuf.AppendChar(ch);
  }
  szLowBuf.AppendChar(0);

  auto result = FX_UTF8Encode(szLowBuf.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Ltrim(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Ltrim");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(argOne.get());
  bsSource.TrimLeft();
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsSource.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Parse(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 2) {
    pContext->ThrowParamCountMismatchException(L"Parse");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if (ValueIsNull(pThis, argTwo.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsPattern = ValueToUTF8String(argOne.get());
  ByteString bsValue = ValueToUTF8String(argTwo.get());
  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
  LocaleIface* pLocale = pThisNode->GetLocale();
  WideString wsPattern = WideString::FromUTF8(bsPattern.AsStringView());
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  bool bPatternIsString;
  uint32_t dwPatternType;
  std::tie(bPatternIsString, dwPatternType) =
      PatternStringType(bsPattern.AsStringView());
  if (bPatternIsString) {
    CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                                 pMgr);
    if (!localeValue.IsValid()) {
      info.GetReturnValue().SetEmptyString();
      return;
    }
    auto result = localeValue.GetValue().ToUTF8();
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
    return;
  }

  switch (dwPatternType) {
    case XFA_VT_DATETIME: {
      auto iTChar = wsPattern.Find(L'T');
      if (!iTChar.has_value()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      WideString wsDatePattern(L"date{" + wsPattern.First(iTChar.value()) +
                               L"} ");
      WideString wsTimePattern(
          L"time{" +
          wsPattern.Last(wsPattern.GetLength() - (iTChar.value() + 1)) + L"}");
      wsPattern = wsDatePattern + wsTimePattern;
      CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                                   pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      auto result = localeValue.GetValue().ToUTF8();
      info.GetReturnValue().Set(
          fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
      return;
    }
    case XFA_VT_DATE: {
      wsPattern = L"date{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                                   pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      auto result = localeValue.GetValue().ToUTF8();
      info.GetReturnValue().Set(
          fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
      return;
    }
    case XFA_VT_TIME: {
      wsPattern = L"time{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                                   pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      auto result = localeValue.GetValue().ToUTF8();
      info.GetReturnValue().Set(
          fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
      return;
    }
    case XFA_VT_TEXT: {
      wsPattern = L"text{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(XFA_VT_TEXT, wsValue, wsPattern, pLocale,
                                   pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      auto result = localeValue.GetValue().ToUTF8();
      info.GetReturnValue().Set(
          fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
      return;
    }
    case XFA_VT_FLOAT: {
      wsPattern = L"num{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(XFA_VT_FLOAT, wsValue, wsPattern, pLocale,
                                   pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      info.GetReturnValue().Set(localeValue.GetDoubleNum());
      return;
    }
    default: {
      {
        WideString wsTestPattern = L"num{" + wsPattern + L"}";
        CXFA_LocaleValue localeValue(XFA_VT_FLOAT, wsValue, wsTestPattern,
                                     pLocale, pMgr);
        if (localeValue.IsValid()) {
          info.GetReturnValue().Set(localeValue.GetDoubleNum());
          return;
        }
      }

      {
        WideString wsTestPattern = L"text{" + wsPattern + L"}";
        CXFA_LocaleValue localeValue(XFA_VT_TEXT, wsValue, wsTestPattern,
                                     pLocale, pMgr);
        if (localeValue.IsValid()) {
          auto result = localeValue.GetValue().ToUTF8();
          info.GetReturnValue().Set(
              fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
          return;
        }
      }
      info.GetReturnValue().SetEmptyString();
      return;
    }
  }
}

// static
void CFXJSE_FormCalcContext::Replace(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 2 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Replace");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  ByteString bsOne;
  ByteString bsTwo;
  if (!ValueIsNull(pThis, argOne.get()) && !ValueIsNull(pThis, argTwo.get())) {
    bsOne = ValueToUTF8String(argOne.get());
    bsTwo = ValueToUTF8String(argTwo.get());
  }

  ByteString bsThree;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
    bsThree = ValueToUTF8String(argThree.get());
  }

  bsOne.Replace(bsTwo.AsStringView(), bsThree.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsOne.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Right(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Right");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  if ((ValueIsNull(pThis, argOne.get())) ||
      (ValueIsNull(pThis, argTwo.get()))) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(argOne.get());
  int32_t count = std::max(0, ValueToInteger(pThis, argTwo.get()));
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), bsSource.Last(count).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Rtrim(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Rtrim");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(argOne.get());
  bsSource.TrimRight();
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsSource.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Space(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Space");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  int32_t count = std::max(0, ValueToInteger(pThis, argOne.get()));
  std::ostringstream spaceString;
  int32_t index = 0;
  while (index < count) {
    spaceString << ' ';
    index++;
  }
  spaceString << '\0';
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), ByteStringView(spaceString.str().c_str())));
}

// static
void CFXJSE_FormCalcContext::Str(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Str");
    return;
  }

  std::unique_ptr<CFXJSE_Value> numberValue = GetSimpleValue(pThis, info, 0);
  if (numberValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fNumber = ValueToFloat(pThis, numberValue.get());

  int32_t iWidth = 10;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> widthValue = GetSimpleValue(pThis, info, 1);
    iWidth = static_cast<int32_t>(ValueToFloat(pThis, widthValue.get()));
  }

  int32_t iPrecision = 0;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> precisionValue =
        GetSimpleValue(pThis, info, 2);
    iPrecision = std::max(
        0, static_cast<int32_t>(ValueToFloat(pThis, precisionValue.get())));
  }

  ByteString bsFormat = "%";
  if (iPrecision) {
    bsFormat += ".";
    bsFormat += ByteString::FormatInteger(iPrecision);
  }
  bsFormat += "f";
  ByteString bsNumber = ByteString::Format(bsFormat.c_str(), fNumber);

  const char* pData = bsNumber.c_str();
  int32_t iLength = bsNumber.GetLength();
  int32_t u = 0;
  while (u < iLength) {
    if (pData[u] == '.')
      break;

    ++u;
  }

  std::ostringstream resultBuf;
  if (u > iWidth || (iPrecision + u) >= iWidth) {
    int32_t i = 0;
    while (i < iWidth) {
      resultBuf << '*';
      ++i;
    }
    resultBuf << '\0';
    info.GetReturnValue().Set(fxv8::NewStringHelper(
        info.GetIsolate(), ByteStringView(resultBuf.str().c_str())));
    return;
  }

  if (u == iLength) {
    if (iLength > iWidth) {
      int32_t i = 0;
      while (i < iWidth) {
        resultBuf << '*';
        ++i;
      }
    } else {
      int32_t i = 0;
      while (i < iWidth - iLength) {
        resultBuf << ' ';
        ++i;
      }
      resultBuf << pData;
    }
    info.GetReturnValue().Set(fxv8::NewStringHelper(
        info.GetIsolate(), ByteStringView(resultBuf.str().c_str())));
    return;
  }

  int32_t iLeavingSpace = iWidth - u - iPrecision;
  if (iPrecision != 0)
    iLeavingSpace--;

  int32_t i = 0;
  while (i < iLeavingSpace) {
    resultBuf << ' ';
    ++i;
  }
  i = 0;
  while (i < u) {
    resultBuf << pData[i];
    ++i;
  }
  if (iPrecision != 0)
    resultBuf << '.';

  u++;
  i = 0;
  while (u < iLength) {
    if (i >= iPrecision)
      break;

    resultBuf << pData[u];
    ++i;
    ++u;
  }
  while (i < iPrecision) {
    resultBuf << '0';
    ++i;
  }
  resultBuf << '\0';
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), ByteStringView(resultBuf.str().c_str())));
}

// static
void CFXJSE_FormCalcContext::Stuff(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 3 || argc > 4) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Stuff");
    return;
  }

  ByteString bsSource;
  ByteString bsInsert;
  int32_t iLength = 0;
  int32_t iStart = 0;
  int32_t iDelete = 0;
  std::unique_ptr<CFXJSE_Value> sourceValue = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> startValue = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> deleteValue = GetSimpleValue(pThis, info, 2);
  if (!sourceValue->IsNull() && !startValue->IsNull() &&
      !deleteValue->IsNull()) {
    bsSource = ValueToUTF8String(sourceValue.get());
    iLength = bsSource.GetLength();
    iStart = pdfium::clamp(
        static_cast<int32_t>(ValueToFloat(pThis, startValue.get())), 1,
        iLength);
    iDelete = std::max(
        0, static_cast<int32_t>(ValueToFloat(pThis, deleteValue.get())));
  }

  if (argc > 3) {
    std::unique_ptr<CFXJSE_Value> insertValue = GetSimpleValue(pThis, info, 3);
    bsInsert = ValueToUTF8String(insertValue.get());
  }

  --iStart;
  std::ostringstream szResult;
  int32_t i = 0;
  while (i < iStart) {
    szResult << static_cast<char>(bsSource[i]);
    ++i;
  }
  szResult << bsInsert.AsStringView();
  i = iStart + iDelete;
  while (i < iLength) {
    szResult << static_cast<char>(bsSource[i]);
    ++i;
  }
  szResult << '\0';
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), ByteStringView(szResult.str().c_str())));
  ;
}

// static
void CFXJSE_FormCalcContext::Substr(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Substr");
    return;
  }

  std::unique_ptr<CFXJSE_Value> string_value = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> start_value = GetSimpleValue(pThis, info, 1);
  std::unique_ptr<CFXJSE_Value> end_value = GetSimpleValue(pThis, info, 2);
  if (ValueIsNull(pThis, string_value.get()) ||
      ValueIsNull(pThis, start_value.get()) ||
      ValueIsNull(pThis, end_value.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(string_value.get());
  size_t iLength = bsSource.GetLength();
  if (iLength == 0) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  // |start_value| is 1-based. Assume first character if |start_value| is less
  // than 1, per spec. Subtract 1 since |iStart| is 0-based.
  size_t iStart = std::max(ValueToInteger(pThis, start_value.get()), 1) - 1;
  if (iStart >= iLength) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  // Negative values are treated as 0. Can't clamp() due to sign mismatches.
  size_t iCount = std::max(ValueToInteger(pThis, end_value.get()), 0);
  iCount = std::min(iCount, iLength - iStart);
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), bsSource.Substr(iStart, iCount).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Uuid(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 0 || argc > 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Uuid");
    return;
  }

  int32_t iNum = 0;
  if (argc > 0) {
    std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
    iNum = static_cast<int32_t>(ValueToFloat(pThis, argOne.get()));
  }
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), GUIDString(!!iNum).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Upper(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"Upper");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (ValueIsNull(pThis, argOne.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFX_WideTextBuf upperStringBuf;
  ByteString bsArg = ValueToUTF8String(argOne.get());
  WideString wsArg = WideString::FromUTF8(bsArg.AsStringView());
  const wchar_t* pData = wsArg.c_str();
  size_t i = 0;
  while (i < wsArg.GetLength()) {
    int32_t ch = pData[i];
    if ((ch >= 0x61 && ch <= 0x7A) || (ch >= 0xE0 && ch <= 0xFE))
      ch -= 32;
    else if (ch == 0x101 || ch == 0x103 || ch == 0x105)
      ch -= 1;

    upperStringBuf.AppendChar(ch);
    ++i;
  }
  upperStringBuf.AppendChar(0);

  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(),
      FX_UTF8Encode(upperStringBuf.AsStringView()).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::WordNum(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException(L"WordNum");
    return;
  }

  std::unique_ptr<CFXJSE_Value> numberValue = GetSimpleValue(pThis, info, 0);
  if (numberValue->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fNumber = ValueToFloat(pThis, numberValue.get());

  int32_t iIdentifier = 0;
  if (argc > 1) {
    std::unique_ptr<CFXJSE_Value> identifierValue =
        GetSimpleValue(pThis, info, 1);
    if (identifierValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    iIdentifier =
        static_cast<int32_t>(ValueToFloat(pThis, identifierValue.get()));
  }

  ByteString bsLocale;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> localeValue = GetSimpleValue(pThis, info, 2);
    if (localeValue->IsNull()) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(localeValue.get());
  }

  if (std::isnan(fNumber) || fNumber < 0.0f ||
      fNumber > 922337203685477550.0f) {
    info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "*"));
    return;
  }

  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(),
      WordUS(ByteString::Format("%.2f", fNumber), iIdentifier).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Get(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException(L"Get");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  IXFA_AppProvider* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  ByteString bsUrl = ValueToUTF8String(argOne.get());
  RetainPtr<IFX_SeekableReadStream> pFile =
      pAppProvider->DownloadURL(WideString::FromUTF8(bsUrl.AsStringView()));
  if (!pFile)
    return;

  int32_t size = pFile->GetSize();
  std::vector<uint8_t> dataBuf(size);
  pFile->ReadBlock(dataBuf.data(), size);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), ByteStringView(dataBuf)));
}

// static
void CFXJSE_FormCalcContext::Post(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 2 || argc > 5) {
    pContext->ThrowParamCountMismatchException(L"Post");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  IXFA_AppProvider* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  ByteString bsURL = ValueToUTF8String(argOne.get());

  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  ByteString bsData = ValueToUTF8String(argTwo.get());

  ByteString bsContentType;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
    bsContentType = ValueToUTF8String(argThree.get());
  }

  ByteString bsEncode;
  if (argc > 3) {
    std::unique_ptr<CFXJSE_Value> argFour = GetSimpleValue(pThis, info, 3);
    bsEncode = ValueToUTF8String(argFour.get());
  }

  ByteString bsHeader;
  if (argc > 4) {
    std::unique_ptr<CFXJSE_Value> argFive = GetSimpleValue(pThis, info, 4);
    bsHeader = ValueToUTF8String(argFive.get());
  }

  WideString decodedResponse;
  if (!pAppProvider->PostRequestURL(
          WideString::FromUTF8(bsURL.AsStringView()),
          WideString::FromUTF8(bsData.AsStringView()),
          WideString::FromUTF8(bsContentType.AsStringView()),
          WideString::FromUTF8(bsEncode.AsStringView()),
          WideString::FromUTF8(bsHeader.AsStringView()), decodedResponse)) {
    pContext->ThrowServerDeniedException();
    return;
  }
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), decodedResponse.ToUTF8().AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Put(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 2 || argc > 3) {
    pContext->ThrowParamCountMismatchException(L"Put");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  IXFA_AppProvider* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  ByteString bsURL = ValueToUTF8String(argOne.get());

  std::unique_ptr<CFXJSE_Value> argTwo = GetSimpleValue(pThis, info, 1);
  ByteString bsData = ValueToUTF8String(argTwo.get());

  ByteString bsEncode;
  if (argc > 2) {
    std::unique_ptr<CFXJSE_Value> argThree = GetSimpleValue(pThis, info, 2);
    bsEncode = ValueToUTF8String(argThree.get());
  }
  if (!pAppProvider->PutRequestURL(
          WideString::FromUTF8(bsURL.AsStringView()),
          WideString::FromUTF8(bsData.AsStringView()),
          WideString::FromUTF8(bsEncode.AsStringView()))) {
    pContext->ThrowServerDeniedException();
    return;
  }
  info.GetReturnValue().SetEmptyString();
}

// static
void CFXJSE_FormCalcContext::assign_value_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 2) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  ByteStringView bsFuncName("asgn_val_op");
  auto lValue = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  std::unique_ptr<CFXJSE_Value> rValue = GetSimpleValue(pThis, info, 1);
  if (lValue->IsArray()) {
    v8::Isolate* pIsolate = pContext->GetScriptRuntime();
    auto leftLengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    lValue->GetObjectProperty("length", leftLengthValue.get());
    int32_t iLeftLength = leftLengthValue->ToInteger();
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    lValue->GetObjectPropertyByIdx(1, propertyValue.get());
    if (propertyValue->IsNull()) {
      for (int32_t i = 2; i < iLeftLength; i++) {
        lValue->GetObjectPropertyByIdx(i, jsObjectValue.get());
        if (!SetObjectDefaultValue(jsObjectValue.get(), rValue.get())) {
          pContext->ThrowNoDefaultPropertyException(bsFuncName);
          return;
        }
      }
    } else {
      for (int32_t i = 2; i < iLeftLength; i++) {
        lValue->GetObjectPropertyByIdx(i, jsObjectValue.get());
        jsObjectValue->SetObjectProperty(
            propertyValue->ToString().AsStringView(), rValue.get());
      }
    }
  } else if (lValue->IsObject()) {
    if (!SetObjectDefaultValue(lValue.get(), rValue.get())) {
      pContext->ThrowNoDefaultPropertyException(bsFuncName);
      return;
    }
  }
  info.GetReturnValue().Set(rValue->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::logical_or_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> infoecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() && infoecond->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  float first = ValueToFloat(pThis, argFirst.get());
  float second = ValueToFloat(pThis, infoecond.get());
  info.GetReturnValue().Set(static_cast<int>(first || second));
}

// static
void CFXJSE_FormCalcContext::logical_and_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> infoecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() && infoecond->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  float first = ValueToFloat(pThis, argFirst.get());
  float second = ValueToFloat(pThis, infoecond.get());
  info.GetReturnValue().Set(static_cast<int>(first && second));
}

// static
void CFXJSE_FormCalcContext::equality_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  if (fm_ref_equal(pThis, info)) {
    info.GetReturnValue().Set(1);
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> infoecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || infoecond->IsNull()) {
    info.GetReturnValue().Set(
        static_cast<int>(argFirst->IsNull() && infoecond->IsNull()));
    return;
  }

  if (argFirst->IsString() && infoecond->IsString()) {
    info.GetReturnValue().Set(
        static_cast<int>(argFirst->ToString() == infoecond->ToString()));
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, infoecond.get());
  info.GetReturnValue().Set(static_cast<int>(first == second));
}

// static
void CFXJSE_FormCalcContext::notequality_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  if (fm_ref_equal(pThis, info)) {
    info.GetReturnValue().Set(0);
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> infoecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || infoecond->IsNull()) {
    info.GetReturnValue().Set(
        static_cast<int>(!argFirst->IsNull() || !infoecond->IsNull()));
    return;
  }

  if (argFirst->IsString() && infoecond->IsString()) {
    info.GetReturnValue().Set(
        static_cast<int>(argFirst->ToString() != infoecond->ToString()));
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, infoecond.get());
  info.GetReturnValue().Set(static_cast<int>(first != second));
}

// static
bool CFXJSE_FormCalcContext::fm_ref_equal(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto argFirst = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  auto argSecond = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[1]);
  if (!argFirst->IsArray() || !argSecond->IsArray())
    return false;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  auto firstFlagValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  auto secondFlagValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argFirst->GetObjectPropertyByIdx(0, firstFlagValue.get());
  argSecond->GetObjectPropertyByIdx(0, secondFlagValue.get());
  if (firstFlagValue->ToInteger() != 3 || secondFlagValue->ToInteger() != 3)
    return false;

  auto firstJSObject = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  auto secondJSObject = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argFirst->GetObjectPropertyByIdx(2, firstJSObject.get());
  argSecond->GetObjectPropertyByIdx(2, secondJSObject.get());
  if (firstJSObject->IsNull() || secondJSObject->IsNull())
    return false;

  return firstJSObject->ToHostObject() == secondJSObject->ToHostObject();
}

// static
void CFXJSE_FormCalcContext::less_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || argSecond->IsNull()) {
    info.GetReturnValue().Set(0);
    return;
  }

  if (argFirst->IsString() && argSecond->IsString()) {
    int result =
        argFirst->ToString().Compare(argSecond->ToString().AsStringView()) < 0;
    info.GetReturnValue().Set(result);
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(static_cast<int>(first < second));
}

// static
void CFXJSE_FormCalcContext::lessequal_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || argSecond->IsNull()) {
    info.GetReturnValue().Set(
        static_cast<int>(argFirst->IsNull() && argSecond->IsNull()));
    return;
  }

  if (argFirst->IsString() && argSecond->IsString()) {
    int result =
        argFirst->ToString().Compare(argSecond->ToString().AsStringView()) <= 0;
    info.GetReturnValue().Set(result);
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(static_cast<int>(first <= second));
}

// static
void CFXJSE_FormCalcContext::greater_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || argSecond->IsNull()) {
    info.GetReturnValue().Set(0);
    return;
  }

  if (argFirst->IsString() && argSecond->IsString()) {
    int result =
        argFirst->ToString().Compare(argSecond->ToString().AsStringView()) > 0;
    info.GetReturnValue().Set(result);
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(static_cast<int>(first > second));
}

// static
void CFXJSE_FormCalcContext::greaterequal_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() || argSecond->IsNull()) {
    info.GetReturnValue().Set(
        static_cast<int>(argFirst->IsNull() && argSecond->IsNull()));
    return;
  }

  if (argFirst->IsString() && argSecond->IsString()) {
    int result =
        argFirst->ToString().Compare(argSecond->ToString().AsStringView()) >= 0;
    info.GetReturnValue().Set(result);
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(static_cast<int>(first >= second));
}

// static
void CFXJSE_FormCalcContext::plus_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  auto argFirst = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  auto argSecond = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[1]);
  if (ValueIsNull(pThis, argFirst.get()) &&
      ValueIsNull(pThis, argSecond.get())) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(first + second);
}

// static
void CFXJSE_FormCalcContext::minus_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() && argSecond->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(first - second);
}

// static
void CFXJSE_FormCalcContext::multiple_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() && argSecond->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  double second = ValueToDouble(pThis, argSecond.get());
  info.GetReturnValue().Set(first * second);
}

// static
void CFXJSE_FormCalcContext::divide_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 2) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argFirst = GetSimpleValue(pThis, info, 0);
  std::unique_ptr<CFXJSE_Value> argSecond = GetSimpleValue(pThis, info, 1);
  if (argFirst->IsNull() && argSecond->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  double second = ValueToDouble(pThis, argSecond.get());
  if (second == 0.0) {
    pContext->ThrowDivideByZeroException();
    return;
  }

  double first = ValueToDouble(pThis, argFirst.get());
  info.GetReturnValue().Set(first / second);
}

// static
void CFXJSE_FormCalcContext::positive_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(0.0 + ValueToDouble(pThis, argOne.get()));
}

// static
void CFXJSE_FormCalcContext::negative_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(0.0 - ValueToDouble(pThis, argOne.get()));
}

// static
void CFXJSE_FormCalcContext::logical_not_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  if (argOne->IsNull()) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(pThis, argOne.get());
  info.GetReturnValue().Set((first == 0.0) ? 1 : 0);
}

// static
void CFXJSE_FormCalcContext::dot_accessor(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  DotAccessorCommon(pThis, info, /*bDotAccessor=*/true);
}

// static
void CFXJSE_FormCalcContext::dotdot_accessor(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  DotAccessorCommon(pThis, info, /*bDotAccessor=*/false);
}

// static
void CFXJSE_FormCalcContext::eval_translation(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException(L"Eval");
    return;
  }

  std::unique_ptr<CFXJSE_Value> argOne = GetSimpleValue(pThis, info, 0);
  ByteString bsArg = ValueToUTF8String(argOne.get());
  if (bsArg.IsEmpty()) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  WideString wsScript = WideString::FromUTF8(bsArg.AsStringView());
  CFX_WideTextBuf wsJavaScriptBuf;
  if (!CFXJSE_FormCalcContext::Translate(wsScript.AsStringView(),
                                         &wsJavaScriptBuf)) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(),
      FX_UTF8Encode(wsJavaScriptBuf.AsStringView()).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::is_fm_object(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    info.GetReturnValue().Set(false);
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  info.GetReturnValue().Set(argOne->IsObject());
}

// static
void CFXJSE_FormCalcContext::is_fm_array(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    info.GetReturnValue().Set(false);
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  info.GetReturnValue().Set(argOne->IsArray());
}

// static
void CFXJSE_FormCalcContext::get_fm_value(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (argOne->IsArray()) {
    v8::Isolate* pIsolate = pContext->GetScriptRuntime();
    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argOne->GetObjectPropertyByIdx(1, propertyValue.get());
    argOne->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
      GetObjectDefaultValue(jsObjectValue.get(), pReturn.get());
      info.GetReturnValue().Set(pReturn->DirectGetValue());
      return;
    }

    auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     pReturn.get());
    info.GetReturnValue().Set(pReturn->DirectGetValue());
    return;
  }

  if (argOne->IsObject()) {
    auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
    GetObjectDefaultValue(argOne.get(), pReturn.get());
    info.GetReturnValue().Set(pReturn->DirectGetValue());
    return;
  }

  info.GetReturnValue().Set(argOne->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::get_fm_jsobj(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (!argOne->IsArray()) {
    info.GetReturnValue().Set(argOne->DirectGetValue());
    return;
  }

#ifndef NDEBUG
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argOne->GetObjectProperty("length", lengthValue.get());
  ASSERT(lengthValue->ToInteger() >= 3);
#endif

  auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
  argOne->GetObjectPropertyByIdx(2, pReturn.get());
  info.GetReturnValue().Set(pReturn->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::fm_var_filter(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  auto argOne = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (!argOne->IsArray()) {
    std::unique_ptr<CFXJSE_Value> simpleValue = GetSimpleValue(pThis, info, 0);
    info.GetReturnValue().Set(simpleValue->DirectGetValue());
    return;
  }

#ifndef NDEBUG
  auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argOne->GetObjectProperty("length", lengthValue.get());
  ASSERT(lengthValue->ToInteger() >= 3);
#endif

  auto flagsValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argOne->GetObjectPropertyByIdx(0, flagsValue.get());
  int32_t iFlags = flagsValue->ToInteger();
  if (iFlags != 3 && iFlags != 4) {
    std::unique_ptr<CFXJSE_Value> simpleValue = GetSimpleValue(pThis, info, 0);
    info.GetReturnValue().Set(simpleValue->DirectGetValue());
    return;
  }

  if (iFlags == 4) {
    std::vector<std::unique_ptr<CFXJSE_Value>> values;
    for (int32_t i = 0; i < 3; i++)
      values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));

    values[0]->SetInteger(3);
    values[1]->SetNull();
    values[2]->SetNull();
    auto pResult = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
    pResult->SetArray(values);
    info.GetReturnValue().Set(pResult->DirectGetValue());
    return;
  }

  auto objectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  argOne->GetObjectPropertyByIdx(2, objectValue.get());
  if (objectValue->IsNull()) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  info.GetReturnValue().Set(argOne->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::concat_fm_object(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  std::vector<std::unique_ptr<CFXJSE_Value>> returnValues;
  for (int32_t i = 0; i < info.Length(); ++i) {
    auto argValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate, info[i]);
    if (argValue->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      argValue->GetObjectProperty("length", lengthValue.get());
      int32_t length = lengthValue->ToInteger();
      for (int32_t j = 2; j < length; j++) {
        returnValues.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
        argValue->GetObjectPropertyByIdx(j, returnValues.back().get());
      }
    }
    returnValues.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
    returnValues.back()->Assign(argValue.get());
  }
  auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
  pReturn->SetArray(returnValues);
  info.GetReturnValue().Set(pReturn->DirectGetValue());
}

// static
std::unique_ptr<CFXJSE_Value> CFXJSE_FormCalcContext::GetSimpleValue(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info,
    uint32_t index) {
  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  ASSERT(index < (uint32_t)info.Length());

  auto argIndex =
      pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[index]);
  if (!argIndex->IsArray() && !argIndex->IsObject())
    return argIndex;

  if (argIndex->IsArray()) {
    auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argIndex->GetObjectProperty("length", lengthValue.get());
    int32_t iLength = lengthValue->ToInteger();
    auto simpleValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    if (iLength < 3) {
      simpleValue.get()->SetUndefined();
      return simpleValue;
    }

    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argIndex->GetObjectPropertyByIdx(1, propertyValue.get());
    argIndex->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      GetObjectDefaultValue(jsObjectValue.get(), simpleValue.get());
      return simpleValue;
    }

    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     simpleValue.get());
    return simpleValue;
  }

  auto defaultValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  GetObjectDefaultValue(argIndex.get(), defaultValue.get());
  return defaultValue;
}

// static
bool CFXJSE_FormCalcContext::ValueIsNull(CFXJSE_HostObject* pThis,
                                         CFXJSE_Value* arg) {
  if (!arg || arg->IsNull())
    return true;

  if (!arg->IsArray() && !arg->IsObject())
    return false;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  if (arg->IsArray()) {
    int32_t iLength = hvalue_get_array_length(pThis, arg);
    if (iLength < 3)
      return true;

    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    arg->GetObjectPropertyByIdx(1, propertyValue.get());
    arg->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      auto defaultValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      GetObjectDefaultValue(jsObjectValue.get(), defaultValue.get());
      return defaultValue->IsNull();
    }

    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     newPropertyValue.get());
    return newPropertyValue->IsNull();
  }

  auto defaultValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  GetObjectDefaultValue(arg, defaultValue.get());
  return defaultValue->IsNull();
}

// static
int32_t CFXJSE_FormCalcContext::hvalue_get_array_length(
    CFXJSE_HostObject* pThis,
    CFXJSE_Value* arg) {
  if (!arg || !arg->IsArray())
    return 0;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  arg->GetObjectProperty("length", lengthValue.get());
  return lengthValue->ToInteger();
}

// static
bool CFXJSE_FormCalcContext::simpleValueCompare(CFXJSE_HostObject* pThis,
                                                CFXJSE_Value* firstValue,
                                                CFXJSE_Value* secondValue) {
  if (!firstValue)
    return false;

  if (firstValue->IsString()) {
    ByteString bsFirst = ValueToUTF8String(firstValue);
    ByteString bsSecond = ValueToUTF8String(secondValue);
    return bsFirst == bsSecond;
  }
  if (firstValue->IsNumber()) {
    float first = ValueToFloat(pThis, firstValue);
    float second = ValueToFloat(pThis, secondValue);
    return first == second;
  }
  if (firstValue->IsBoolean())
    return firstValue->ToBoolean() == secondValue->ToBoolean();

  return firstValue->IsNull() && secondValue && secondValue->IsNull();
}

// static
std::vector<std::unique_ptr<CFXJSE_Value>> CFXJSE_FormCalcContext::unfoldArgs(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  std::vector<std::unique_ptr<CFXJSE_Value>> results;
  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  for (int32_t i = 1; i < info.Length(); ++i) {
    auto arg = pdfium::MakeUnique<CFXJSE_Value>(pIsolate, info[i]);
    if (arg->IsArray()) {
      auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      arg->GetObjectProperty("length", lengthValue.get());
      int32_t iLength = lengthValue->ToInteger();
      if (iLength < 3)
        continue;

      auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      arg->GetObjectPropertyByIdx(1, propertyValue.get());

      for (int32_t j = 2; j < iLength; j++) {
        auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
        arg->GetObjectPropertyByIdx(j, jsObjectValue.get());
        results.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
        if (propertyValue->IsNull()) {
          GetObjectDefaultValue(jsObjectValue.get(), results.back().get());
        } else {
          jsObjectValue->GetObjectProperty(
              propertyValue->ToString().AsStringView(), results.back().get());
        }
      }
    } else if (arg->IsObject()) {
      results.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
      GetObjectDefaultValue(arg.get(), results.back().get());
    } else {
      results.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
      results.back()->Assign(arg.get());
    }
  }
  return results;
}

// static

// static
ByteString CFXJSE_FormCalcContext::GenerateSomExpression(ByteStringView bsName,
                                                         int32_t iIndexFlags,
                                                         int32_t iIndexValue,
                                                         bool bIsStar) {
  if (bIsStar)
    return ByteString(bsName, "[*]");

  if (iIndexFlags == 0)
    return ByteString(bsName);

  if (iIndexFlags == 1 || iIndexValue == 0) {
    return ByteString(bsName, "[") + ByteString::FormatInteger(iIndexValue) +
           "]";
  }

  const bool bNegative = iIndexValue < 0;
  ByteString bsSomExp(bsName);
  if (iIndexFlags == 2)
    bsSomExp += bNegative ? "[-" : "[+";
  else
    bsSomExp += bNegative ? "[" : "[-";
  iIndexValue = bNegative ? 0 - iIndexValue : iIndexValue;
  bsSomExp += ByteString::FormatInteger(iIndexValue);
  bsSomExp += "]";
  return bsSomExp;
}

// static
bool CFXJSE_FormCalcContext::GetObjectForName(CFXJSE_HostObject* pThis,
                                              CFXJSE_Value* accessorValue,
                                              ByteStringView bsAccessorName) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return false;

  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  XFA_RESOLVENODE_RS resolveNodeRS;
  uint32_t dwFlags = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                     XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent;
  bool bRet = pScriptContext->ResolveObjects(
      pScriptContext->GetThisObject(),
      WideString::FromUTF8(bsAccessorName).AsStringView(), &resolveNodeRS,
      dwFlags, nullptr);
  if (bRet && resolveNodeRS.dwFlags == XFA_ResolveNode_RSType_Nodes) {
    accessorValue->Assign(pScriptContext->GetOrCreateJSBindingFromMap(
        resolveNodeRS.objects.front().Get()));
    return true;
  }
  return false;
}

// static
bool CFXJSE_FormCalcContext::ResolveObjects(CFXJSE_HostObject* pThis,
                                            CFXJSE_Value* pRefValue,
                                            ByteStringView bsSomExp,
                                            XFA_RESOLVENODE_RS* resolveNodeRS,
                                            bool bDotAccessor,
                                            bool bHasNoResolveName) {
  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  if (!pDoc)
    return false;

  WideString wsSomExpression = WideString::FromUTF8(bsSomExp);
  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  CXFA_Object* pNode = nullptr;
  uint32_t dFlags = 0UL;
  if (bDotAccessor) {
    if (pRefValue && pRefValue->IsNull()) {
      pNode = pScriptContext->GetThisObject();
      dFlags = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent;
    } else {
      pNode = CFXJSE_Engine::ToObject(pRefValue);
      if (!pNode)
        return false;

      if (bHasNoResolveName) {
        WideString wsName;
        if (CXFA_Node* pXFANode = pNode->AsNode()) {
          Optional<WideString> ret =
              pXFANode->JSObject()->TryAttribute(XFA_Attribute::Name, false);
          if (ret)
            wsName = *ret;
        }
        if (wsName.IsEmpty())
          wsName = L"#" + WideString::FromASCII(pNode->GetClassName());

        wsSomExpression = wsName + wsSomExpression;
        dFlags = XFA_RESOLVENODE_Siblings;
      } else {
        dFlags = (bsSomExp == "*")
                     ? (XFA_RESOLVENODE_Children)
                     : (XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                        XFA_RESOLVENODE_Properties);
      }
    }
  } else {
    pNode = CFXJSE_Engine::ToObject(pRefValue);
    dFlags = XFA_RESOLVENODE_AnyChild;
  }
  return pScriptContext->ResolveObjects(pNode, wsSomExpression.AsStringView(),
                                        resolveNodeRS, dFlags, nullptr);
}

// static
void CFXJSE_FormCalcContext::ParseResolveResult(
    CFXJSE_HostObject* pThis,
    const XFA_RESOLVENODE_RS& resolveNodeRS,
    CFXJSE_Value* pParentValue,
    std::vector<std::unique_ptr<CFXJSE_Value>>* resultValues,
    bool* bAttribute) {
  ASSERT(bAttribute);

  resultValues->clear();

  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();

  if (resolveNodeRS.dwFlags == XFA_ResolveNode_RSType_Nodes) {
    *bAttribute = false;
    CFXJSE_Engine* pScriptContext = pContext->GetDocument()->GetScriptContext();
    for (auto& pObject : resolveNodeRS.objects) {
      resultValues->push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
      resultValues->back()->Assign(
          pScriptContext->GetOrCreateJSBindingFromMap(pObject.Get()));
    }
    return;
  }

  *bAttribute = true;
  if (resolveNodeRS.script_attribute.callback &&
      resolveNodeRS.script_attribute.eValueType == XFA_ScriptType::Object) {
    for (auto& pObject : resolveNodeRS.objects) {
      auto pValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
      CJX_Object* jsObject = pObject->JSObject();
      (*resolveNodeRS.script_attribute.callback)(
          jsObject, pValue.get(), false,
          resolveNodeRS.script_attribute.attribute);
      resultValues->push_back(std::move(pValue));
      *bAttribute = false;
    }
  }
  if (!*bAttribute)
    return;
  if (!pParentValue || !pParentValue->IsObject())
    return;

  resultValues->push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
  resultValues->back()->Assign(pParentValue);
}

// static
int32_t CFXJSE_FormCalcContext::ValueToInteger(CFXJSE_HostObject* pThis,
                                               CFXJSE_Value* pValue) {
  if (!pValue || pValue->IsEmpty())
    return 0;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  if (pValue->IsArray()) {
    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    pValue->GetObjectPropertyByIdx(1, propertyValue.get());
    pValue->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
      return ValueToInteger(pThis, newPropertyValue.get());
    }

    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     newPropertyValue.get());
    return ValueToInteger(pThis, newPropertyValue.get());
  }
  if (pValue->IsObject()) {
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    GetObjectDefaultValue(pValue, newPropertyValue.get());
    return ValueToInteger(pThis, newPropertyValue.get());
  }
  if (pValue->IsString())
    return FXSYS_atoi(pValue->ToString().c_str());
  return pValue->ToInteger();
}

// static
float CFXJSE_FormCalcContext::ValueToFloat(CFXJSE_HostObject* pThis,
                                           CFXJSE_Value* arg) {
  if (!arg)
    return 0.0f;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  if (arg->IsArray()) {
    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    arg->GetObjectPropertyByIdx(1, propertyValue.get());
    arg->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
      return ValueToFloat(pThis, newPropertyValue.get());
    }
    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     newPropertyValue.get());
    return ValueToFloat(pThis, newPropertyValue.get());
  }
  if (arg->IsObject()) {
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    GetObjectDefaultValue(arg, newPropertyValue.get());
    return ValueToFloat(pThis, newPropertyValue.get());
  }
  if (arg->IsString())
    return strtof(arg->ToString().c_str(), nullptr);
  if (arg->IsUndefined() || arg->IsEmpty())
    return 0.0f;
  return arg->ToFloat();
}

// static
double CFXJSE_FormCalcContext::ValueToDouble(CFXJSE_HostObject* pThis,
                                             CFXJSE_Value* arg) {
  if (!arg)
    return 0;

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  if (arg->IsArray()) {
    auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    arg->GetObjectPropertyByIdx(1, propertyValue.get());
    arg->GetObjectPropertyByIdx(2, jsObjectValue.get());
    if (propertyValue->IsNull()) {
      GetObjectDefaultValue(jsObjectValue.get(), newPropertyValue.get());
      return ValueToDouble(pThis, newPropertyValue.get());
    }
    jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                     newPropertyValue.get());
    return ValueToDouble(pThis, newPropertyValue.get());
  }
  if (arg->IsObject()) {
    auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    GetObjectDefaultValue(arg, newPropertyValue.get());
    return ValueToDouble(pThis, newPropertyValue.get());
  }
  if (arg->IsString())
    return strtod(arg->ToString().c_str(), nullptr);
  if (arg->IsUndefined() || arg->IsEmpty())
    return 0;
  return arg->ToDouble();
}

// static.
double CFXJSE_FormCalcContext::ExtractDouble(CFXJSE_HostObject* pThis,
                                             CFXJSE_Value* src,
                                             bool* ret) {
  ASSERT(ret);
  *ret = true;

  if (!src)
    return 0;

  if (!src->IsArray())
    return ValueToDouble(pThis, src);

  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetScriptRuntime();
  auto lengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  src->GetObjectProperty("length", lengthValue.get());
  int32_t iLength = lengthValue->ToInteger();
  if (iLength <= 2) {
    *ret = false;
    return 0.0;
  }

  auto propertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  auto jsObjectValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  src->GetObjectPropertyByIdx(1, propertyValue.get());
  src->GetObjectPropertyByIdx(2, jsObjectValue.get());
  if (propertyValue->IsNull())
    return ValueToDouble(pThis, jsObjectValue.get());

  auto newPropertyValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
  jsObjectValue->GetObjectProperty(propertyValue->ToString().AsStringView(),
                                   newPropertyValue.get());
  return ValueToDouble(pThis, newPropertyValue.get());
}

// static
ByteString CFXJSE_FormCalcContext::ValueToUTF8String(CFXJSE_Value* arg) {
  if (!arg || arg->IsNull() || arg->IsUndefined() || arg->IsEmpty())
    return ByteString();
  if (arg->IsBoolean())
    return arg->ToBoolean() ? "1" : "0";
  return arg->ToString();
}

// static.
bool CFXJSE_FormCalcContext::Translate(WideStringView wsFormcalc,
                                       CFX_WideTextBuf* wsJavascript) {
  if (wsFormcalc.IsEmpty()) {
    wsJavascript->Clear();
    return true;
  }

  CXFA_FMParser parser(wsFormcalc);
  std::unique_ptr<CXFA_FMAST> ast = parser.Parse();
  if (!ast || parser.HasError())
    return false;

  CXFA_FMToJavaScriptDepth::Reset();
  if (!ast->ToJavaScript(wsJavascript))
    return false;

  wsJavascript->AppendChar(0);
  return !CXFA_IsTooBig(wsJavascript);
}

CFXJSE_FormCalcContext::CFXJSE_FormCalcContext(v8::Isolate* pScriptIsolate,
                                               CFXJSE_Context* pScriptContext,
                                               CXFA_Document* pDoc)
    : m_pIsolate(pScriptIsolate),
      m_pValue(pdfium::MakeUnique<CFXJSE_Value>(pScriptIsolate)),
      m_pDocument(pDoc) {
  m_pValue->SetHostObject(
      this,
      CFXJSE_Class::Create(pScriptContext, &kFormCalcFM2JSDescriptor, false));
}

CFXJSE_FormCalcContext::~CFXJSE_FormCalcContext() = default;

CFXJSE_FormCalcContext* CFXJSE_FormCalcContext::AsFormCalcContext() {
  return this;
}

void CFXJSE_FormCalcContext::GlobalPropertyGetter(CFXJSE_Value* pValue) {
  pValue->Assign(m_pValue.get());
}

// static
void CFXJSE_FormCalcContext::DotAccessorCommon(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info,
    bool bDotAccessor) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetScriptRuntime();
  int32_t argc = info.Length();
  if (argc < 4 || argc > 5) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  bool bIsStar = true;
  int32_t iIndexValue = 0;
  if (argc > 4) {
    bIsStar = false;
    auto temp = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[4]);
    iIndexValue = ValueToInteger(pThis, temp.get());
  }

  const ByteString bsName =
      fxv8::ReentrantToByteStringHelper(info.GetIsolate(), info[2]);
  const bool bHasNoResolveName = bDotAccessor && bsName.IsEmpty();
  ByteString bsSomExp = GenerateSomExpression(
      bsName.AsStringView(),
      fxv8::ReentrantToInt32Helper(info.GetIsolate(), info[3]), iIndexValue,
      bIsStar);

  auto argAccessor =
      pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate(), info[0]);
  if (argAccessor->IsArray()) {
    auto pLengthValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    argAccessor->GetObjectProperty("length", pLengthValue.get());
    int32_t iLength = pLengthValue->ToInteger();
    if (iLength < 3) {
      pContext->ThrowArgumentMismatchException();
      return;
    }

    auto hJSObjValue = pdfium::MakeUnique<CFXJSE_Value>(pIsolate);
    std::vector<std::vector<std::unique_ptr<CFXJSE_Value>>> resolveValues(
        iLength - 2);
    bool bAttribute = false;
    bool bAllEmpty = true;
    for (int32_t i = 2; i < iLength; i++) {
      argAccessor->GetObjectPropertyByIdx(i, hJSObjValue.get());
      XFA_RESOLVENODE_RS resolveNodeRS;
      if (ResolveObjects(pThis, hJSObjValue.get(), bsSomExp.AsStringView(),
                         &resolveNodeRS, bDotAccessor, bHasNoResolveName)) {
        ParseResolveResult(pThis, resolveNodeRS, hJSObjValue.get(),
                           &resolveValues[i - 2], &bAttribute);
        bAllEmpty = bAllEmpty && resolveValues[i - 2].empty();
      }
    }
    if (bAllEmpty) {
      pContext->ThrowPropertyNotInObjectException(
          WideString::FromUTF8(bsName.AsStringView()),
          WideString::FromUTF8(bsSomExp.AsStringView()));
      return;
    }

    std::vector<std::unique_ptr<CFXJSE_Value>> values;
    values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
    values.back()->SetInteger(1);
    values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
    if (bAttribute)
      values.back()->SetString(bsName.AsStringView());
    else
      values.back()->SetNull();

    for (int32_t i = 0; i < iLength - 2; i++) {
      for (size_t j = 0; j < resolveValues[i].size(); j++) {
        values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));
        values.back()->Assign(resolveValues[i][j].get());
      }
    }
    auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
    pReturn->SetArray(values);
    info.GetReturnValue().Set(pReturn->DirectGetValue());
    return;
  }

  XFA_RESOLVENODE_RS resolveNodeRS;
  bool bRet = false;
  ByteString bsAccessorName =
      fxv8::ReentrantToByteStringHelper(info.GetIsolate(), info[1]);
  if (argAccessor->IsObject() ||
      (argAccessor->IsNull() && bsAccessorName.IsEmpty())) {
    bRet = ResolveObjects(pThis, argAccessor.get(), bsSomExp.AsStringView(),
                          &resolveNodeRS, bDotAccessor, bHasNoResolveName);
  } else if (!argAccessor->IsObject() && !bsAccessorName.IsEmpty() &&
             GetObjectForName(pThis, argAccessor.get(),
                              bsAccessorName.AsStringView())) {
    bRet = ResolveObjects(pThis, argAccessor.get(), bsSomExp.AsStringView(),
                          &resolveNodeRS, bDotAccessor, bHasNoResolveName);
  }
  if (!bRet) {
    pContext->ThrowPropertyNotInObjectException(
        WideString::FromUTF8(bsName.AsStringView()),
        WideString::FromUTF8(bsSomExp.AsStringView()));
    return;
  }

  std::vector<std::unique_ptr<CFXJSE_Value>> resolveValues;
  bool bAttribute = false;
  ParseResolveResult(pThis, resolveNodeRS, argAccessor.get(), &resolveValues,
                     &bAttribute);

  std::vector<std::unique_ptr<CFXJSE_Value>> values;
  for (size_t i = 0; i < resolveValues.size() + 2; i++)
    values.push_back(pdfium::MakeUnique<CFXJSE_Value>(pIsolate));

  values[0]->SetInteger(1);
  if (bAttribute)
    values[1]->SetString(bsName.AsStringView());
  else
    values[1]->SetNull();

  for (size_t i = 0; i < resolveValues.size(); i++)
    values[i + 2]->Assign(resolveValues[i].get());

  auto pReturn = pdfium::MakeUnique<CFXJSE_Value>(info.GetIsolate());
  pReturn->SetArray(values);
  info.GetReturnValue().Set(pReturn->DirectGetValue());
}

void CFXJSE_FormCalcContext::ThrowNoDefaultPropertyException(
    ByteStringView name) const {
  ThrowException(WideString::FromUTF8(name) +
                 WideString::FromASCII(" doesn't have a default property."));
}

void CFXJSE_FormCalcContext::ThrowCompilerErrorException() const {
  ThrowException(WideString::FromASCII("Compiler error."));
}

void CFXJSE_FormCalcContext::ThrowDivideByZeroException() const {
  ThrowException(WideString::FromASCII("Divide by zero."));
}

void CFXJSE_FormCalcContext::ThrowServerDeniedException() const {
  ThrowException(WideString::FromASCII("Server does not permit operation."));
}

void CFXJSE_FormCalcContext::ThrowPropertyNotInObjectException(
    const WideString& name,
    const WideString& exp) const {
  ThrowException(
      WideString::FromASCII("An attempt was made to reference property '") +
      name + WideString::FromASCII("' of a non-object in SOM expression ") +
      exp + L".");
}

void CFXJSE_FormCalcContext::ThrowParamCountMismatchException(
    const WideString& method) const {
  ThrowException(
      WideString::FromASCII("Incorrect number of parameters calling method '") +
      method + L"'.");
}

void CFXJSE_FormCalcContext::ThrowArgumentMismatchException() const {
  ThrowException(WideString::FromASCII(
      "Argument mismatch in property or function argument."));
}

void CFXJSE_FormCalcContext::ThrowException(const WideString& str) const {
  ASSERT(!str.IsEmpty());
  FXJSE_ThrowMessage(str.ToUTF8().AsStringView());
}
