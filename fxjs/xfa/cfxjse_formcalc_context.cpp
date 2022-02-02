// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_formcalc_context.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_datetime.h"
#include "core/fxcrt/cfx_widetextbuf.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_random.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/check.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "v8/include/v8-container.h"
#include "v8/include/v8-function-callback.h"
#include "v8/include/v8-object.h"
#include "v8/include/v8-primitive.h"
#include "xfa/fgas/crt/cfgas_decimal.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/fm2js/cxfa_fmparser.h"
#include "xfa/fxfa/fm2js/cxfa_fmtojavascriptdepth.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"
#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"
#include "xfa/fxfa/parser/gced_locale_iface.h"
#include "xfa/fxfa/parser/xfa_utils.h"

using pdfium::fxjse::kClassTag;
using pdfium::fxjse::kFuncTag;

namespace {

// Maximum number of characters Acrobat can fit in a text box.
constexpr int kMaxCharCount = 15654908;

const double kFinancialPrecision = 0.00000001;

const wchar_t kStrCode[] = L"0123456789abcdef";

struct XFA_FMHtmlReserveCode {
  uint16_t m_uCode;
  // Inline string data reduces size for small strings.
  const char m_htmlReserve[10];
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
static_assert(pdfium::size(kAltTableDate) == L'a' - L'A' + 1,
              "Invalid kAltTableDate size.");

const uint8_t kAltTableTime[] = {
    14,  255, 255, 3,   9,   255, 255, 15,  255, 255, 255,
    255, 6,   255, 255, 255, 255, 255, 7,   255, 255, 255,
    255, 255, 1,   17,  255, 255, 255, 255, 255, 255, 255,
};
static_assert(pdfium::size(kAltTableTime) == L'a' - L'A' + 1,
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

std::pair<bool, CXFA_LocaleValue::ValueType> PatternStringType(
    ByteStringView bsPattern) {
  WideString wsPattern = WideString::FromUTF8(bsPattern);
  if (L"datetime" == wsPattern.First(8))
    return {true, CXFA_LocaleValue::ValueType::kDateTime};
  if (L"date" == wsPattern.First(4)) {
    auto pos = wsPattern.Find(L"time");
    if (pos.has_value() && pos.value() != 0)
      return {true, CXFA_LocaleValue::ValueType::kDateTime};
    return {true, CXFA_LocaleValue::ValueType::kDate};
  }
  if (L"time" == wsPattern.First(4))
    return {true, CXFA_LocaleValue::ValueType::kTime};
  if (L"text" == wsPattern.First(4))
    return {true, CXFA_LocaleValue::ValueType::kText};
  if (L"num" == wsPattern.First(3)) {
    if (L"integer" == wsPattern.Substr(4, 7))
      return {true, CXFA_LocaleValue::ValueType::kInteger};
    if (L"decimal" == wsPattern.Substr(4, 7))
      return {true, CXFA_LocaleValue::ValueType::kDecimal};
    if (L"currency" == wsPattern.Substr(4, 8))
      return {true, CXFA_LocaleValue::ValueType::kFloat};
    if (L"percent" == wsPattern.Substr(4, 7))
      return {true, CXFA_LocaleValue::ValueType::kFloat};
    return {true, CXFA_LocaleValue::ValueType::kFloat};
  }

  CXFA_LocaleValue::ValueType type = CXFA_LocaleValue::ValueType::kNull;
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
      return {false, CXFA_LocaleValue::ValueType::kTime};
    if (wsPatternChar == 'x' || wsPatternChar == 'o' || wsPatternChar == '0')
      return {false, CXFA_LocaleValue::ValueType::kText};
    if (wsPatternChar == 'v' || wsPatternChar == '8' || wsPatternChar == '$')
      return {false, CXFA_LocaleValue::ValueType::kFloat};
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
          return {false, CXFA_LocaleValue::ValueType::kDateTime};
        iIndex++;
      }
      return {false, CXFA_LocaleValue::ValueType::kDate};
    }

    if (wsPatternChar == 'a') {
      type = CXFA_LocaleValue::ValueType::kText;
    } else if (wsPatternChar == 'z' || wsPatternChar == 's' ||
               wsPatternChar == 'e' || wsPatternChar == ',' ||
               wsPatternChar == '.') {
      type = CXFA_LocaleValue::ValueType::kFloat;
    }
    iIndex++;
  }
  return {false, type};
}

CFXJSE_FormCalcContext* ToFormCalcContext(CFXJSE_HostObject* pHostObj) {
  return pHostObj ? pHostObj->AsFormCalcContext() : nullptr;
}

GCedLocaleIface* LocaleFromString(CXFA_Document* pDoc,
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

  return pLocale->GetDatePattern(LocaleIface::DateTimeSubcategory::kDefault);
}

LocaleIface::DateTimeSubcategory SubCategoryFromInt(int32_t iStyle) {
  switch (iStyle) {
    case 1:
      return LocaleIface::DateTimeSubcategory::kShort;
    case 3:
      return LocaleIface::DateTimeSubcategory::kLong;
    case 4:
      return LocaleIface::DateTimeSubcategory::kFull;
    case 0:
    case 2:
    default:
      return LocaleIface::DateTimeSubcategory::kMedium;
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

  LocaleIface::DateTimeSubcategory category = SubCategoryFromInt(iStyle);
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
  return isdigit(ch) || ch == '-' || ch == '.';
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
    if (!isdigit(pData[i]))
      return false;

    szYear[i] = pData[i];
  }
  iYear = FXSYS_atoi(szYear);
  iStyle = 0;
  if (pData.size() == 4)
    return true;

  iStyle = pData[4] == '-' ? 1 : 0;

  size_t iPosOff = iStyle == 0 ? 4 : 5;
  if (!isdigit(pData[iPosOff]) || !isdigit(pData[iPosOff + 1]))
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
  if (!isdigit(pData[iPosOff]) || !isdigit(pData[iPosOff + 1]))
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
    if (!isdigit(pData[i]) && pData[i] != ':') {
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
    if (!isdigit(pData[iIndex]))
      return false;

    szBuffer[0] = pData[iIndex];
    if (!isdigit(pData[iIndex + 1]))
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
      if (!isdigit(c))
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
    if (!isdigit(pData[iIndex]))
      return false;

    szBuffer[0] = pData[iIndex];
    if (!isdigit(pData[iIndex + 1]))
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
  *pYear = 0;
  *pMonth = 0;
  *pDay = 0;
  *pHour = 0;
  *pMinute = 0;
  *pSecond = 0;

  size_t iIndex = 0;
  while (iIndex < pData.size()) {
    if (pData[iIndex] == 'T' || pData[iIndex] == 't')
      break;
    ++iIndex;
  }
  if (iIndex == pData.size() || (iIndex != 8 && iIndex != 10))
    return false;

  pdfium::span<const char> pDateSpan = pData.subspan(0, iIndex);
  pdfium::span<const char> pTimeSpan = pData.subspan(iIndex + 1);

  int32_t iStyle = -1;
  return IsIsoDateFormat(pDateSpan, &iStyle, pYear, pMonth, pDay) &&
         IsIsoTimeFormat(pTimeSpan, pHour, pMinute, pSecond, pMilliSecond,
                         pZoneHour, pZoneMinute);
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
  return static_cast<int32_t>(dDays);
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
    size_t iCount = pdfium::size(kStrUnsafe);
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
    iCount = pdfium::size(kStrReserved);
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
    iCount = pdfium::size(kStrSpecial);
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
  return wsResultBuf.MakeString();
}

ByteString TrillionUS(ByteStringView bsData) {
  static const char kUnits[][6] = {"zero", "one", "two",   "three", "four",
                                   "five", "six", "seven", "eight", "nine"};
  static const char kCapUnits[][6] = {"Zero", "One", "Two",   "Three", "Four",
                                      "Five", "Six", "Seven", "Eight", "Nine"};
  static const char kTens[][10] = {
      "Ten",     "Eleven",  "Twelve",    "Thirteen", "Fourteen",
      "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen"};
  static const char kLastTens[][8] = {"Twenty", "Thirty",  "Forty",  "Fifty",
                                      "Sixty",  "Seventy", "Eighty", "Ninety"};
  static const char kComm[][11] = {" Hundred ", " Thousand ", " Million ",
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

  ByteString strBuf;
  int32_t iIndex = 0;
  if (iFirstCount == 3) {
    if (pData[iIndex] != '0') {
      strBuf += kCapUnits[pData[iIndex] - '0'];
      strBuf += kComm[0];
    }
    if (pData[iIndex + 1] == '0') {
      strBuf += kCapUnits[pData[iIndex + 2] - '0'];
    } else {
      if (pData[iIndex + 1] > '1') {
        strBuf += kLastTens[pData[iIndex + 1] - '2'];
        strBuf += "-";
        strBuf += kUnits[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '1') {
        strBuf += kTens[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '0') {
        strBuf += kCapUnits[pData[iIndex + 2] - '0'];
      }
    }
    iIndex += 3;
  } else if (iFirstCount == 2) {
    if (pData[iIndex] == '0') {
      strBuf += kCapUnits[pData[iIndex + 1] - '0'];
    } else {
      if (pData[iIndex] > '1') {
        strBuf += kLastTens[pData[iIndex] - '2'];
        strBuf += "-";
        strBuf += kUnits[pData[iIndex + 1] - '0'];
      } else if (pData[iIndex] == '1') {
        strBuf += kTens[pData[iIndex + 1] - '0'];
      } else if (pData[iIndex] == '0') {
        strBuf += kCapUnits[pData[iIndex + 1] - '0'];
      }
    }
    iIndex += 2;
  } else if (iFirstCount == 1) {
    strBuf += kCapUnits[pData[iIndex] - '0'];
    ++iIndex;
  }
  if (iLength > 3 && iFirstCount > 0) {
    strBuf += kComm[iComm];
    --iComm;
  }
  while (iIndex < iLength) {
    if (pData[iIndex] != '0') {
      strBuf += kCapUnits[pData[iIndex] - '0'];
      strBuf += kComm[0];
    }
    if (pData[iIndex + 1] == '0') {
      strBuf += kCapUnits[pData[iIndex + 2] - '0'];
    } else {
      if (pData[iIndex + 1] > '1') {
        strBuf += kLastTens[pData[iIndex + 1] - '2'];
        strBuf += "-";
        strBuf += kUnits[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '1') {
        strBuf += kTens[pData[iIndex + 2] - '0'];
      } else if (pData[iIndex + 1] == '0') {
        strBuf += kCapUnits[pData[iIndex + 2] - '0'];
      }
    }
    if (iIndex < iLength - 3) {
      strBuf += kComm[iComm];
      --iComm;
    }
    iIndex += 3;
  }
  return strBuf;
}

ByteString WordUS(ByteStringView bsData, int32_t iStyle) {
  if (iStyle < 0 || iStyle > 2)
    return ByteString();

  int32_t iLength = bsData.GetLength();
  ByteString strBuf;
  int32_t iIndex = 0;
  while (iIndex < iLength) {
    if (bsData[iIndex] == '.')
      break;
    ++iIndex;
  }
  int32_t iInteger = iIndex;
  iIndex = 0;
  while (iIndex < iInteger) {
    int32_t iCount = (iInteger - iIndex) % 12;
    if (!iCount && iInteger - iIndex > 0)
      iCount = 12;

    strBuf += TrillionUS(bsData.Substr(iIndex, iCount));
    iIndex += iCount;
    if (iIndex < iInteger)
      strBuf += " Trillion ";
  }

  if (iStyle > 0)
    strBuf += " Dollars";

  if (iStyle > 1 && iInteger < iLength) {
    strBuf += " And ";
    iIndex = iInteger + 1;
    while (iIndex < iLength) {
      int32_t iCount = (iLength - iIndex) % 12;
      if (!iCount && iLength - iIndex > 0)
        iCount = 12;

      strBuf += TrillionUS(bsData.Substr(iIndex, iCount));
      iIndex += iCount;
      if (iIndex < iLength)
        strBuf += " Trillion ";
    }
    strBuf += " Cents";
  }
  return strBuf;
}

v8::Local<v8::Value> GetObjectDefaultValue(v8::Isolate* pIsolate,
                                           v8::Local<v8::Object> pObject) {
  CXFA_Node* pNode = ToNode(CFXJSE_Engine::ToObject(pIsolate, pObject));
  if (!pNode)
    return fxv8::NewNullHelper(pIsolate);

  v8::Local<v8::Value> value;
  pNode->JSObject()->ScriptSomDefaultValue(pIsolate, &value, false,
                                           XFA_Attribute::Unknown);
  return value;
}

bool SetObjectDefaultValue(v8::Isolate* pIsolate,
                           v8::Local<v8::Object> pObject,
                           v8::Local<v8::Value> hNewValue) {
  CXFA_Node* pNode = ToNode(CFXJSE_Engine::ToObject(pIsolate, pObject));
  if (!pNode)
    return false;

  pNode->JSObject()->ScriptSomDefaultValue(pIsolate, &hNewValue, true,
                                           XFA_Attribute::Unknown);
  return true;
}

v8::Local<v8::Value> GetExtractedValue(v8::Isolate* pIsolate,
                                       v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return v8::Local<v8::Value>();

  if (fxv8::IsArray(pValue)) {
    v8::Local<v8::Array> arr = pValue.As<v8::Array>();
    uint32_t iLength = fxv8::GetArrayLengthHelper(arr);
    if (iLength < 3)
      return fxv8::NewUndefinedHelper(pIsolate);

    v8::Local<v8::Value> propertyValue =
        fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 1);
    v8::Local<v8::Value> jsValue =
        fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 2);
    if (!fxv8::IsObject(jsValue))
      return fxv8::NewUndefinedHelper(pIsolate);

    v8::Local<v8::Object> jsObjectValue = jsValue.As<v8::Object>();
    if (fxv8::IsNull(propertyValue))
      return GetObjectDefaultValue(pIsolate, jsObjectValue);

    ByteString bsName =
        fxv8::ReentrantToByteStringHelper(pIsolate, propertyValue);
    return fxv8::ReentrantGetObjectPropertyHelper(pIsolate, jsObjectValue,
                                                  bsName.AsStringView());
  }

  if (fxv8::IsObject(pValue))
    return GetObjectDefaultValue(pIsolate, pValue.As<v8::Object>());

  return pValue;
}

v8::Local<v8::Value> GetSimpleValue(
    const v8::FunctionCallbackInfo<v8::Value>& info,
    uint32_t index) {
  DCHECK(index < static_cast<uint32_t>(info.Length()));
  return GetExtractedValue(info.GetIsolate(), info[index]);
}

bool ValueIsNull(v8::Isolate* pIsolate, v8::Local<v8::Value> arg) {
  v8::Local<v8::Value> extracted = GetExtractedValue(pIsolate, arg);
  return extracted.IsEmpty() || fxv8::IsNull(extracted);
}

int32_t ValueToInteger(v8::Isolate* pIsolate, v8::Local<v8::Value> arg) {
  v8::Local<v8::Value> extracted = GetExtractedValue(pIsolate, arg);
  if (extracted.IsEmpty())
    return 0;

  if (fxv8::IsObject(extracted) || fxv8::IsArray(extracted))
    return ValueToInteger(pIsolate, extracted);

  if (fxv8::IsString(extracted)) {
    ByteString bsValue = fxv8::ReentrantToByteStringHelper(pIsolate, extracted);
    return FXSYS_atoi(bsValue.c_str());
  }

  return fxv8::ReentrantToInt32Helper(pIsolate, extracted);
}

float ValueToFloat(v8::Isolate* pIsolate, v8::Local<v8::Value> arg) {
  v8::Local<v8::Value> extracted = GetExtractedValue(pIsolate, arg);
  if (extracted.IsEmpty())
    return 0.0f;

  if (fxv8::IsUndefined(extracted))
    return 0.0f;

  if (fxv8::IsObject(extracted) || fxv8::IsArray(extracted))
    return ValueToFloat(pIsolate, extracted);

  if (fxv8::IsString(extracted)) {
    ByteString bsValue = fxv8::ReentrantToByteStringHelper(pIsolate, extracted);
    return strtof(bsValue.c_str(), nullptr);
  }

  return fxv8::ReentrantToFloatHelper(pIsolate, extracted);
}

double ValueToDouble(v8::Isolate* pIsolate, v8::Local<v8::Value> arg) {
  v8::Local<v8::Value> extracted = GetExtractedValue(pIsolate, arg);
  if (extracted.IsEmpty())
    return 0.0;

  if (fxv8::IsUndefined(extracted))
    return 0.0;

  if (fxv8::IsObject(extracted) || fxv8::IsArray(extracted))
    return ValueToDouble(pIsolate, extracted);

  if (fxv8::IsString(extracted)) {
    ByteString bsValue = fxv8::ReentrantToByteStringHelper(pIsolate, extracted);
    return strtod(bsValue.c_str(), nullptr);
  }

  return fxv8::ReentrantToDoubleHelper(pIsolate, extracted);
}

absl::optional<double> ExtractDouble(v8::Isolate* pIsolate,
                                     v8::Local<v8::Value> src) {
  if (src.IsEmpty())
    return 0.0;

  if (!fxv8::IsArray(src))
    return ValueToDouble(pIsolate, src);

  v8::Local<v8::Array> arr = src.As<v8::Array>();
  uint32_t iLength = fxv8::GetArrayLengthHelper(arr);
  if (iLength < 3)
    return absl::nullopt;

  v8::Local<v8::Value> propertyValue =
      fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 1);
  v8::Local<v8::Value> jsValue =
      fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 2);
  if (fxv8::IsNull(propertyValue) || !fxv8::IsObject(jsValue))
    return ValueToDouble(pIsolate, jsValue);

  ByteString bsName =
      fxv8::ReentrantToByteStringHelper(pIsolate, propertyValue);
  return ValueToDouble(
      pIsolate, fxv8::ReentrantGetObjectPropertyHelper(
                    pIsolate, jsValue.As<v8::Object>(), bsName.AsStringView()));
}

ByteString ValueToUTF8String(v8::Isolate* pIsolate, v8::Local<v8::Value> arg) {
  if (arg.IsEmpty())
    return ByteString();

  if (fxv8::IsNull(arg) || fxv8::IsUndefined(arg))
    return ByteString();

  if (fxv8::IsBoolean(arg))
    return fxv8::ReentrantToBooleanHelper(pIsolate, arg) ? "1" : "0";

  return fxv8::ReentrantToByteStringHelper(pIsolate, arg);
}

bool SimpleValueCompare(v8::Isolate* pIsolate,
                        v8::Local<v8::Value> firstValue,
                        v8::Local<v8::Value> secondValue) {
  if (firstValue.IsEmpty())
    return false;

  if (fxv8::IsString(firstValue)) {
    const ByteString first = ValueToUTF8String(pIsolate, firstValue);
    const ByteString second = ValueToUTF8String(pIsolate, secondValue);
    return first == second;
  }
  if (fxv8::IsNumber(firstValue)) {
    const float first = ValueToFloat(pIsolate, firstValue);
    const float second = ValueToFloat(pIsolate, secondValue);
    return first == second;
  }
  if (fxv8::IsBoolean(firstValue)) {
    const bool first = fxv8::ReentrantToBooleanHelper(pIsolate, firstValue);
    const bool second = fxv8::ReentrantToBooleanHelper(pIsolate, secondValue);
    return first == second;
  }
  return fxv8::IsNull(firstValue) && fxv8::IsNull(secondValue);
}

std::vector<v8::Local<v8::Value>> UnfoldArgs(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  std::vector<v8::Local<v8::Value>> results;
  v8::Isolate* pIsolate = info.GetIsolate();
  for (int i = 1; i < info.Length(); ++i) {
    v8::Local<v8::Value> arg = info[i];
    if (fxv8::IsArray(arg)) {
      v8::Local<v8::Array> arr = arg.As<v8::Array>();
      uint32_t iLength = fxv8::GetArrayLengthHelper(arr);
      if (iLength < 3)
        continue;

      v8::Local<v8::Value> propertyValue =
          fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 1);

      for (uint32_t j = 2; j < iLength; j++) {
        v8::Local<v8::Value> jsValue =
            fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, j);

        if (!fxv8::IsObject(jsValue)) {
          results.push_back(fxv8::NewUndefinedHelper(pIsolate));
        } else if (fxv8::IsNull(propertyValue)) {
          results.push_back(
              GetObjectDefaultValue(pIsolate, jsValue.As<v8::Object>()));
        } else {
          ByteString bsName =
              fxv8::ReentrantToByteStringHelper(pIsolate, propertyValue);
          results.push_back(fxv8::ReentrantGetObjectPropertyHelper(
              pIsolate, jsValue.As<v8::Object>(), bsName.AsStringView()));
        }
      }
    } else if (fxv8::IsObject(arg)) {
      results.push_back(GetObjectDefaultValue(pIsolate, arg.As<v8::Object>()));
    } else {
      results.push_back(arg);
    }
  }
  return results;
}

// Returns empty value on failure.
v8::Local<v8::Value> GetObjectForName(CFXJSE_HostObject* pHostObject,
                                      ByteStringView bsAccessorName) {
  CXFA_Document* pDoc = ToFormCalcContext(pHostObject)->GetDocument();
  if (!pDoc)
    return v8::Local<v8::Value>();

  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  absl::optional<CFXJSE_Engine::ResolveResult> maybeResult =
      pScriptContext->ResolveObjects(
          pScriptContext->GetThisObject(),
          WideString::FromUTF8(bsAccessorName).AsStringView(),
          Mask<XFA_ResolveFlag>{
              XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kProperties,
              XFA_ResolveFlag::kSiblings, XFA_ResolveFlag::kParent});
  if (!maybeResult.has_value() ||
      maybeResult.value().type != CFXJSE_Engine::ResolveResult::Type::kNodes ||
      maybeResult.value().objects.empty()) {
    return v8::Local<v8::Value>();
  }
  return pScriptContext->GetOrCreateJSBindingFromMap(
      maybeResult.value().objects.front().Get());
}

absl::optional<CFXJSE_Engine::ResolveResult> ResolveObjects(
    CFXJSE_HostObject* pHostObject,
    v8::Local<v8::Value> pRefValue,
    ByteStringView bsSomExp,
    bool bDotAccessor,
    bool bHasNoResolveName) {
  CXFA_Document* pDoc = ToFormCalcContext(pHostObject)->GetDocument();
  if (!pDoc)
    return absl::nullopt;

  v8::Isolate* pIsolate = ToFormCalcContext(pHostObject)->GetIsolate();
  WideString wsSomExpression = WideString::FromUTF8(bsSomExp);
  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  CXFA_Object* pNode = nullptr;
  Mask<XFA_ResolveFlag> dwFlags;
  if (bDotAccessor) {
    if (fxv8::IsNull(pRefValue)) {
      pNode = pScriptContext->GetThisObject();
      dwFlags = {XFA_ResolveFlag::kSiblings, XFA_ResolveFlag::kParent};
    } else {
      pNode = CFXJSE_Engine::ToObject(pIsolate, pRefValue);
      if (!pNode)
        return absl::nullopt;

      if (bHasNoResolveName) {
        WideString wsName;
        if (CXFA_Node* pXFANode = pNode->AsNode()) {
          absl::optional<WideString> ret =
              pXFANode->JSObject()->TryAttribute(XFA_Attribute::Name, false);
          if (ret.has_value())
            wsName = ret.value();
        }
        if (wsName.IsEmpty())
          wsName = L"#" + WideString::FromASCII(pNode->GetClassName());

        wsSomExpression = wsName + wsSomExpression;
        dwFlags = XFA_ResolveFlag::kSiblings;
      } else {
        dwFlags = (bsSomExp == "*")
                      ? Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kChildren}
                      : Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kChildren,
                                              XFA_ResolveFlag::kAttributes,
                                              XFA_ResolveFlag::kProperties};
      }
    }
  } else {
    pNode = CFXJSE_Engine::ToObject(pIsolate, pRefValue);
    dwFlags = XFA_ResolveFlag::kAnyChild;
  }
  return pScriptContext->ResolveObjects(pNode, wsSomExpression.AsStringView(),
                                        dwFlags);
}

std::vector<v8::Local<v8::Value>> ParseResolveResult(
    CFXJSE_HostObject* pHostObject,
    const CFXJSE_Engine::ResolveResult& resolveNodeRS,
    v8::Local<v8::Value> pParentValue,
    bool* bAttribute) {
  std::vector<v8::Local<v8::Value>> resultValues;
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pHostObject);
  v8::Isolate* pIsolate = pContext->GetIsolate();

  if (resolveNodeRS.type == CFXJSE_Engine::ResolveResult::Type::kNodes) {
    *bAttribute = false;
    CFXJSE_Engine* pScriptContext = pContext->GetDocument()->GetScriptContext();
    for (auto& pObject : resolveNodeRS.objects) {
      resultValues.push_back(
          pScriptContext->GetOrCreateJSBindingFromMap(pObject.Get()));
    }
    return resultValues;
  }

  *bAttribute = true;
  if (resolveNodeRS.script_attribute.callback &&
      resolveNodeRS.script_attribute.eValueType == XFA_ScriptType::Object) {
    for (auto& pObject : resolveNodeRS.objects) {
      v8::Local<v8::Value> pValue;
      CJX_Object* jsObject = pObject->JSObject();
      (*resolveNodeRS.script_attribute.callback)(
          pIsolate, jsObject, &pValue, false,
          resolveNodeRS.script_attribute.attribute);
      resultValues.push_back(pValue);
      *bAttribute = false;
    }
  }
  if (*bAttribute && fxv8::IsObject(pParentValue))
    resultValues.push_back(pParentValue);

  return resultValues;
}

}  // namespace

const FXJSE_CLASS_DESCRIPTOR kFormCalcFM2JSDescriptor = {
    kClassTag,                              // tag
    "XFA_FM2JS_FormCalcClass",              // name
    kFormCalcFM2JSFunctions,                // methods
    pdfium::size(kFormCalcFM2JSFunctions),  // number of methods
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Abs");
    return;
  }

  if (ValueIsNull(info.GetIsolate(), info[0])) {
    info.GetReturnValue().SetNull();
    return;
  }
  double dValue = ValueToDouble(info.GetIsolate(), info[0]);
  if (dValue < 0)
    dValue = -dValue;

  info.GetReturnValue().Set(dValue);
}

// static
void CFXJSE_FormCalcContext::Avg(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  uint32_t uCount = 0;
  double dSum = 0.0;
  auto fn = [&uCount, &dSum](v8::Isolate* pIsolate,
                             v8::Local<v8::Value> pValue) {
    dSum += ValueToDouble(pIsolate, pValue);
    uCount++;
  };
  if (!ToFormCalcContext(pThis)->ApplyToExpansion(fn, info, /*bStrict=*/false))
    return;

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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Ceil");
    return;
  }

  v8::Local<v8::Value> argValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(ceil(ValueToFloat(info.GetIsolate(), argValue)));
}

// static
void CFXJSE_FormCalcContext::Count(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  uint32_t iCount = 0;
  auto fn = [&iCount](v8::Isolate* pIsolate, v8::Local<v8::Value> pvalue) {
    ++iCount;
  };
  if (!ToFormCalcContext(pThis)->ApplyToExpansion(fn, info, /*bStrict=*/true))
    return;

  info.GetReturnValue().Set(iCount);
}

// static
void CFXJSE_FormCalcContext::Floor(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Floor");
    return;
  }

  v8::Local<v8::Value> argValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  info.GetReturnValue().Set(floor(ValueToFloat(info.GetIsolate(), argValue)));
}

// static
void CFXJSE_FormCalcContext::Max(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  uint32_t uCount = 0;
  double dMaxValue = 0.0;
  auto fn = [&uCount, &dMaxValue](v8::Isolate* pIsolate,
                                  v8::Local<v8::Value> pValue) {
    ++uCount;
    double dValue = ValueToDouble(pIsolate, pValue);
    dMaxValue = uCount == 1 ? dValue : std::max(dMaxValue, dValue);
  };
  if (!ToFormCalcContext(pThis)->ApplyToExpansion(fn, info, /*bStrict=*/true))
    return;

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
  uint32_t uCount = 0;
  double dMinValue = 0.0;
  auto fn = [&uCount, &dMinValue](v8::Isolate* pIsolate,
                                  v8::Local<v8::Value> pValue) {
    ++uCount;
    double dValue = ValueToDouble(pIsolate, pValue);
    dMinValue = uCount == 1 ? dValue : std::min(dMinValue, dValue);
  };
  if (!ToFormCalcContext(pThis)->ApplyToExpansion(fn, info, /*bStrict=*/true))
    return;

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
    pContext->ThrowParamCountMismatchException("Mod");
    return;
  }

  if (fxv8::IsNull(info[0]) || fxv8::IsNull(info[1])) {
    info.GetReturnValue().SetNull();
    return;
  }

  absl::optional<double> maybe_dividend =
      ExtractDouble(info.GetIsolate(), info[0]);
  absl::optional<double> maybe_divisor =
      ExtractDouble(info.GetIsolate(), info[1]);
  if (!maybe_dividend.has_value() || !maybe_divisor.has_value()) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double dividend = maybe_dividend.value();
  double divisor = maybe_divisor.value();
  if (divisor == 0.0) {
    pContext->ThrowDivideByZeroException();
    return;
  }

  info.GetReturnValue().Set(dividend -
                            divisor * static_cast<int32_t>(dividend / divisor));
}

// static
void CFXJSE_FormCalcContext::Round(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    pContext->ThrowParamCountMismatchException("Round");
    return;
  }

  if (fxv8::IsNull(info[0])) {
    info.GetReturnValue().SetNull();
    return;
  }

  absl::optional<double> maybe_value =
      ExtractDouble(info.GetIsolate(), info[0]);
  if (!maybe_value.has_value()) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double dValue = maybe_value.value();
  uint8_t uPrecision = 0;
  if (argc > 1) {
    if (fxv8::IsNull(info[1])) {
      info.GetReturnValue().SetNull();
      return;
    }
    absl::optional<double> maybe_precision =
        ExtractDouble(info.GetIsolate(), info[1]);
    if (!maybe_precision.has_value()) {
      pContext->ThrowArgumentMismatchException();
      return;
    }
    double dPrecision = maybe_precision.value();
    uPrecision = static_cast<uint8_t>(pdfium::clamp(dPrecision, 0.0, 12.0));
  }

  CFGAS_Decimal decimalValue(static_cast<float>(dValue), uPrecision);
  info.GetReturnValue().Set(decimalValue.ToDouble());
}

// static
void CFXJSE_FormCalcContext::Sum(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  uint32_t uCount = 0;
  double dSum = 0.0;
  auto fn = [&uCount, &dSum](v8::Isolate* pIsolate,
                             v8::Local<v8::Value> pValue) {
    ++uCount;
    dSum += ValueToDouble(pIsolate, pValue);
  };
  if (!ToFormCalcContext(pThis)->ApplyToExpansion(fn, info, /*bStrict=*/true))
    return;

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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Date");
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Date2Num");
    return;
  }

  v8::Local<v8::Value> dateValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), dateValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsDate = ValueToUTF8String(info.GetIsolate(), dateValue);
  ByteString bsFormat;
  if (argc > 1) {
    v8::Local<v8::Value> formatValue = GetSimpleValue(info, 1);
    if (ValueIsNull(info.GetIsolate(), formatValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(info.GetIsolate(), formatValue);
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (ValueIsNull(info.GetIsolate(), localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Date2Num");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    v8::Local<v8::Value> infotyle = GetSimpleValue(info, 0);
    if (fxv8::IsNull(infotyle)) {
      info.GetReturnValue().SetNull();
      return;
    }

    iStyle = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), infotyle));
    if (iStyle < 0 || iStyle > 4)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    v8::Local<v8::Value> argLocale = GetSimpleValue(info, 1);
    if (fxv8::IsNull(argLocale)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), argLocale);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("IsoDate2Num");
    return;
  }
  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }
  ByteString bsArg = ValueToUTF8String(info.GetIsolate(), argOne);
  info.GetReturnValue().Set(DateString2Num(bsArg.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::IsoTime2Num(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException("IsoTime2Num");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  ByteString bsArg = ValueToUTF8String(info.GetIsolate(), argOne);
  auto pos = bsArg.Find('T', 0);
  if (!pos.has_value() || pos.value() == bsArg.GetLength() - 1) {
    info.GetReturnValue().Set(0);
    return;
  }
  bsArg = bsArg.Last(bsArg.GetLength() - (pos.value() + 1));

  CXFA_LocaleValue timeValue(CXFA_LocaleValue::ValueType::kTime,
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
  mins -= pMgr->GetDefLocale()->GetTimeZoneInMinutes();
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("LocalDateFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    v8::Local<v8::Value> infotyle = GetSimpleValue(info, 0);
    if (fxv8::IsNull(infotyle)) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), infotyle));
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    v8::Local<v8::Value> argLocale = GetSimpleValue(info, 1);
    if (fxv8::IsNull(argLocale)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), argLocale);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("LocalTimeFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    v8::Local<v8::Value> infotyle = GetSimpleValue(info, 0);
    if (fxv8::IsNull(infotyle)) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), infotyle));
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    v8::Local<v8::Value> argLocale = GetSimpleValue(info, 1);
    if (fxv8::IsNull(argLocale)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), argLocale);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Num2Date");
    return;
  }

  v8::Local<v8::Value> dateValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), dateValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  int32_t dDate =
      static_cast<int32_t>(ValueToFloat(info.GetIsolate(), dateValue));
  if (dDate < 1) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    v8::Local<v8::Value> formatValue = GetSimpleValue(info, 1);
    if (ValueIsNull(info.GetIsolate(), formatValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(info.GetIsolate(), formatValue);
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (ValueIsNull(info.GetIsolate(), localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Num2GMTime");
    return;
  }

  v8::Local<v8::Value> timeValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(timeValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  int32_t iTime =
      static_cast<int32_t>(ValueToFloat(info.GetIsolate(), timeValue));
  if (abs(iTime) < 1.0) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    v8::Local<v8::Value> formatValue = GetSimpleValue(info, 1);
    if (fxv8::IsNull(formatValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(info.GetIsolate(), formatValue);
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (fxv8::IsNull(localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Num2Time");
    return;
  }

  v8::Local<v8::Value> timeValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(timeValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fTime = ValueToFloat(info.GetIsolate(), timeValue);
  if (fabs(fTime) < 1.0) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsFormat;
  if (argc > 1) {
    v8::Local<v8::Value> formatValue = GetSimpleValue(info, 1);
    if (fxv8::IsNull(formatValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(info.GetIsolate(), formatValue);
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (fxv8::IsNull(localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Time");
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Time2Num");
    return;
  }

  ByteString bsTime;
  v8::Local<v8::Value> timeValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), timeValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  bsTime = ValueToUTF8String(info.GetIsolate(), timeValue);

  ByteString bsFormat;
  if (argc > 1) {
    v8::Local<v8::Value> formatValue = GetSimpleValue(info, 1);
    if (ValueIsNull(info.GetIsolate(), formatValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsFormat = ValueToUTF8String(info.GetIsolate(), formatValue);
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (ValueIsNull(info.GetIsolate(), localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
  }

  CXFA_Document* pDoc = ToFormCalcContext(pThis)->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  GCedLocaleIface* pLocale = nullptr;
  if (!bsLocale.IsEmpty()) {
    pLocale =
        pMgr->GetLocaleByName(WideString::FromUTF8(bsLocale.AsStringView()));
  }
  if (!pLocale) {
    CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
    pLocale = pThisNode->GetLocale();
  }

  WideString wsFormat;
  if (bsFormat.IsEmpty()) {
    wsFormat =
        pLocale->GetTimePattern(LocaleIface::DateTimeSubcategory::kDefault);
  } else {
    wsFormat = WideString::FromUTF8(bsFormat.AsStringView());
  }
  wsFormat = L"time{" + wsFormat + L"}";
  CXFA_LocaleValue localeValue(CXFA_LocaleValue::ValueType::kTime,
                               WideString::FromUTF8(bsTime.AsStringView()),
                               wsFormat, pLocale, pMgr);
  if (!localeValue.IsValid()) {
    info.GetReturnValue().Set(0);
    return;
  }

  CFX_DateTime uniTime = localeValue.GetTime();
  int32_t hour = uniTime.GetHour();
  int32_t minute = uniTime.GetMinute();
  const int32_t second = uniTime.GetSecond();
  const int32_t millisecond = uniTime.GetMillisecond();

  constexpr int kMinutesInDay = 24 * 60;
  int32_t minutes_with_tz =
      hour * 60 + minute - CXFA_TimeZoneProvider().GetTimeZoneInMinutes();
  minutes_with_tz %= kMinutesInDay;
  if (minutes_with_tz < 0)
    minutes_with_tz += kMinutesInDay;

  hour = minutes_with_tz / 60;
  minute = minutes_with_tz % 60;
  info.GetReturnValue().Set(hour * 3600000 + minute * 60000 + second * 1000 +
                            millisecond + 1);
}

// static
void CFXJSE_FormCalcContext::TimeFmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("TimeFmt");
    return;
  }

  int32_t iStyle = 0;
  if (argc > 0) {
    v8::Local<v8::Value> infotyle = GetSimpleValue(info, 0);
    if (fxv8::IsNull(infotyle)) {
      info.GetReturnValue().SetNull();
      return;
    }
    iStyle = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), infotyle));
    if (iStyle > 4 || iStyle < 0)
      iStyle = 0;
  }

  ByteString bsLocale;
  if (argc > 1) {
    v8::Local<v8::Value> argLocale = GetSimpleValue(info, 1);
    if (fxv8::IsNull(argLocale)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), argLocale);
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
  GCedLocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = FormatFromString(pLocale, bsFormat);
  CFX_DateTime dt =
      CXFA_LocaleValue(CXFA_LocaleValue::ValueType::kDate,
                       WideString::FromUTF8(bsDate), wsFormat, pLocale, pMgr)
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
  GCedLocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = FormatFromString(pLocale, bsFormat);
  WideString wsRet;
  CXFA_LocaleValue(CXFA_LocaleValue::ValueType::kDate,
                   WideString::FromUTF8(bsDate), pMgr)
      .FormatPatterns(wsRet, wsFormat, pLocale, XFA_ValuePicture::kDisplay);
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
  GCedLocaleIface* pLocale = LocaleFromString(pDoc, pMgr, bsLocale);
  if (!pLocale)
    return ByteString();

  WideString wsFormat = {
      L"time{", FormatFromString(pLocale, bsFormat).AsStringView(), L"}"};
  CXFA_LocaleValue widgetValue(CXFA_LocaleValue::ValueType::kTime,
                               WideString::FromUTF8(bsTime), pMgr);
  WideString wsRet;
  widgetValue.FormatPatterns(wsRet, wsFormat, pLocale,
                             XFA_ValuePicture::kDisplay);
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
    pContext->ThrowParamCountMismatchException("Apr");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nPrincipal = ValueToDouble(info.GetIsolate(), argOne);
  double nPayment = ValueToDouble(info.GetIsolate(), argTwo);
  double nPeriods = ValueToDouble(info.GetIsolate(), argThree);
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
    pContext->ThrowParamCountMismatchException("CTerm");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nRate = ValueToFloat(info.GetIsolate(), argOne);
  float nFutureValue = ValueToFloat(info.GetIsolate(), argTwo);
  float nInitAmount = ValueToFloat(info.GetIsolate(), argThree);
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
    pContext->ThrowParamCountMismatchException("FV");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nAmount = ValueToDouble(info.GetIsolate(), argOne);
  double nRate = ValueToDouble(info.GetIsolate(), argTwo);
  double nPeriod = ValueToDouble(info.GetIsolate(), argThree);
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
    pContext->ThrowParamCountMismatchException("IPmt");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  v8::Local<v8::Value> argFour = GetSimpleValue(info, 3);
  v8::Local<v8::Value> argFive = GetSimpleValue(info, 4);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree) ||
      ValueIsNull(info.GetIsolate(), argFour) ||
      ValueIsNull(info.GetIsolate(), argFive)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nPrincipalAmount = ValueToFloat(info.GetIsolate(), argOne);
  float nRate = ValueToFloat(info.GetIsolate(), argTwo);
  float nPayment = ValueToFloat(info.GetIsolate(), argThree);
  float nFirstMonth = ValueToFloat(info.GetIsolate(), argFour);
  float nNumberOfMonths = ValueToFloat(info.GetIsolate(), argFive);
  if ((nPrincipalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
      (nFirstMonth < 0) || (nNumberOfMonths < 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  float nRateOfMonth = nRate / 12;
  int32_t iNums = static_cast<int32_t>(
      (log10((float)(nPayment / nPrincipalAmount)) -
       log10((float)(nPayment / nPrincipalAmount - nRateOfMonth))) /
      log10((float)(1 + nRateOfMonth)));
  int32_t iEnd =
      std::min(static_cast<int32_t>(nFirstMonth + nNumberOfMonths - 1), iNums);

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
  if (argc < 2) {
    pContext->ThrowParamCountMismatchException("NPV");
    return;
  }

  v8::Local<v8::Value> argValue = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nRate = ValueToDouble(info.GetIsolate(), argValue);
  if (nRate <= 0) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  std::vector<double> data;
  for (int32_t i = 1; i < argc; i++) {
    argValue = GetSimpleValue(info, i);
    if (ValueIsNull(info.GetIsolate(), argValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    data.push_back(ValueToDouble(info.GetIsolate(), argValue));
  }

  double nSum = 0;
  double nDivisor = 1.0 + nRate;
  while (!data.empty()) {
    nSum += data.back();
    nSum /= nDivisor;
    data.pop_back();
  }
  info.GetReturnValue().Set(nSum);
}

// static
void CFXJSE_FormCalcContext::Pmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException("Pmt");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nPrincipal = ValueToDouble(info.GetIsolate(), argOne);
  double nRate = ValueToDouble(info.GetIsolate(), argTwo);
  double nPeriods = ValueToDouble(info.GetIsolate(), argThree);
  if (nPrincipal <= 0 || nRate <= 0 || nPeriods < 1 ||
      nPeriods > static_cast<double>(std::numeric_limits<int32_t>::max())) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  double nTmp = 1 + nRate;
  double nSum = nTmp;
  for (int32_t i = 0; i < static_cast<int>(nPeriods) - 1; ++i)
    nSum *= nTmp;

  info.GetReturnValue().Set((nPrincipal * nRate * nSum) / (nSum - 1));
}

// static
void CFXJSE_FormCalcContext::PPmt(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 5) {
    pContext->ThrowParamCountMismatchException("PPmt");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  v8::Local<v8::Value> argFour = GetSimpleValue(info, 3);
  v8::Local<v8::Value> argFive = GetSimpleValue(info, 4);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree) ||
      ValueIsNull(info.GetIsolate(), argFour) ||
      ValueIsNull(info.GetIsolate(), argFive)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nPrincipalAmount = ValueToFloat(info.GetIsolate(), argOne);
  float nRate = ValueToFloat(info.GetIsolate(), argTwo);
  float nPayment = ValueToFloat(info.GetIsolate(), argThree);
  float nFirstMonth = ValueToFloat(info.GetIsolate(), argFour);
  float nNumberOfMonths = ValueToFloat(info.GetIsolate(), argFive);
  if ((nPrincipalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
      (nFirstMonth < 0) || (nNumberOfMonths < 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  float nRateOfMonth = nRate / 12;
  int32_t iNums = static_cast<int32_t>(
      (log10((float)(nPayment / nPrincipalAmount)) -
       log10((float)(nPayment / nPrincipalAmount - nRateOfMonth))) /
      log10((float)(1 + nRateOfMonth)));
  int32_t iEnd =
      std::min(static_cast<int32_t>(nFirstMonth + nNumberOfMonths - 1), iNums);
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
    pContext->ThrowParamCountMismatchException("PV");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double nAmount = ValueToDouble(info.GetIsolate(), argOne);
  double nRate = ValueToDouble(info.GetIsolate(), argTwo);
  double nPeriod = ValueToDouble(info.GetIsolate(), argThree);
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
    pContext->ThrowParamCountMismatchException("Rate");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nFuture = ValueToFloat(info.GetIsolate(), argOne);
  float nPresent = ValueToFloat(info.GetIsolate(), argTwo);
  float nTotalNumber = ValueToFloat(info.GetIsolate(), argThree);
  if ((nFuture <= 0) || (nPresent < 0) || (nTotalNumber <= 0)) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  info.GetReturnValue().Set(powf(nFuture / nPresent, 1.0f / nTotalNumber) -
                            1.0f);
}

// static
void CFXJSE_FormCalcContext::Term(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 3) {
    pContext->ThrowParamCountMismatchException("Term");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo) ||
      ValueIsNull(info.GetIsolate(), argThree)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float nMount = ValueToFloat(info.GetIsolate(), argOne);
  float nRate = ValueToFloat(info.GetIsolate(), argTwo);
  float nFuture = ValueToFloat(info.GetIsolate(), argThree);
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
    pContext->ThrowParamCountMismatchException("Choose");
    return;
  }

  if (ValueIsNull(info.GetIsolate(), info[0])) {
    info.GetReturnValue().SetNull();
    return;
  }

  int32_t iIndex =
      static_cast<int32_t>(ValueToFloat(info.GetIsolate(), info[0]));
  if (iIndex < 1) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  bool bFound = false;
  bool bStopCounterFlags = false;
  int32_t iArgIndex = 1;
  int32_t iValueIndex = 0;
  while (!bFound && !bStopCounterFlags && (iArgIndex < argc)) {
    v8::Local<v8::Value> argIndexValue = info[iArgIndex];
    if (fxv8::IsArray(argIndexValue)) {
      v8::Local<v8::Array> arr = argIndexValue.As<v8::Array>();
      uint32_t iLength = fxv8::GetArrayLengthHelper(arr);
      if (iLength > 3)
        bStopCounterFlags = true;

      iValueIndex += (iLength - 2);
      if (iValueIndex >= iIndex) {
        v8::Local<v8::Value> propertyValue =
            fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 1);
        v8::Local<v8::Value> jsValue = fxv8::ReentrantGetArrayElementHelper(
            info.GetIsolate(), arr, (iLength - 1) - (iValueIndex - iIndex));
        v8::Local<v8::Value> newPropertyValue;
        if (fxv8::IsObject(jsValue)) {
          v8::Local<v8::Object> jsObjectValue = jsValue.As<v8::Object>();
          if (fxv8::IsNull(propertyValue)) {
            newPropertyValue =
                GetObjectDefaultValue(info.GetIsolate(), jsObjectValue);
          } else {
            ByteString bsName = fxv8::ReentrantToByteStringHelper(
                info.GetIsolate(), propertyValue);
            newPropertyValue = fxv8::ReentrantGetObjectPropertyHelper(
                info.GetIsolate(), jsObjectValue, bsName.AsStringView());
          }
        }
        ByteString bsChosen =
            ValueToUTF8String(info.GetIsolate(), newPropertyValue);
        info.GetReturnValue().Set(
            fxv8::NewStringHelper(info.GetIsolate(), bsChosen.AsStringView()));
        bFound = true;
      }
    } else {
      iValueIndex++;
      if (iValueIndex == iIndex) {
        ByteString bsChosen =
            ValueToUTF8String(info.GetIsolate(), argIndexValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Exists");
    return;
  }
  info.GetReturnValue().Set(fxv8::IsObject(info[0]));
}

// static
void CFXJSE_FormCalcContext::HasValue(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("HasValue");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (!fxv8::IsString(argOne)) {
    info.GetReturnValue().Set(
        static_cast<int>(fxv8::IsNumber(argOne) || fxv8::IsBoolean(argOne)));
    return;
  }

  ByteString bsValue =
      fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argOne);
  bsValue.TrimLeft();
  info.GetReturnValue().Set(static_cast<int>(!bsValue.IsEmpty()));
}

// static
void CFXJSE_FormCalcContext::Oneof(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Oneof");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  for (const auto& value : UnfoldArgs(info)) {
    if (SimpleValueCompare(info.GetIsolate(), argOne, value)) {
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Within");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetUndefined();
    return;
  }

  v8::Local<v8::Value> argLow = GetSimpleValue(info, 1);
  v8::Local<v8::Value> argHigh = GetSimpleValue(info, 2);
  if (fxv8::IsNumber(argOne)) {
    float oneNumber = ValueToFloat(info.GetIsolate(), argOne);
    float lowNumber = ValueToFloat(info.GetIsolate(), argLow);
    float heightNumber = ValueToFloat(info.GetIsolate(), argHigh);
    info.GetReturnValue().Set(static_cast<int>((oneNumber >= lowNumber) &&
                                               (oneNumber <= heightNumber)));
    return;
  }

  ByteString bsOne = ValueToUTF8String(info.GetIsolate(), argOne);
  ByteString bsLow = ValueToUTF8String(info.GetIsolate(), argLow);
  ByteString bsHeight = ValueToUTF8String(info.GetIsolate(), argHigh);
  info.GetReturnValue().Set(
      static_cast<int>((bsOne.Compare(bsLow.AsStringView()) >= 0) &&
                       (bsOne.Compare(bsHeight.AsStringView()) <= 0)));
}

// static
void CFXJSE_FormCalcContext::If(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("If");
    return;
  }

  const bool condition = fxv8::ReentrantToBooleanHelper(
      info.GetIsolate(), GetSimpleValue(info, 0));

  info.GetReturnValue().Set(GetSimpleValue(info, condition ? 1 : 2));
}

// static
void CFXJSE_FormCalcContext::Eval(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException("Eval");
    return;
  }

  v8::Isolate* pIsolate = pContext->GetIsolate();
  v8::Local<v8::Value> scriptValue = GetSimpleValue(info, 0);
  ByteString bsUtf8Script = ValueToUTF8String(info.GetIsolate(), scriptValue);
  if (bsUtf8Script.IsEmpty()) {
    info.GetReturnValue().SetNull();
    return;
  }

  WideString wsCalcScript = WideString::FromUTF8(bsUtf8Script.AsStringView());
  absl::optional<CFX_WideTextBuf> wsJavaScriptBuf =
      CFXJSE_FormCalcContext::Translate(pContext->GetDocument()->GetHeap(),
                                        wsCalcScript.AsStringView());
  if (!wsJavaScriptBuf.has_value()) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  std::unique_ptr<CFXJSE_Context> pNewContext =
      CFXJSE_Context::Create(pIsolate, nullptr, nullptr, nullptr);

  auto returnValue = std::make_unique<CFXJSE_Value>();
  ByteString bsScript = FX_UTF8Encode(wsJavaScriptBuf.value().AsStringView());
  pNewContext->ExecuteScript(bsScript.AsStringView(), returnValue.get(),
                             v8::Local<v8::Object>());

  info.GetReturnValue().Set(returnValue->DirectGetValue());
}

// static
void CFXJSE_FormCalcContext::Ref(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException("Ref");
    return;
  }

  v8::Local<v8::Value> argOne = info[0];
  if (fxv8::IsBoolean(argOne) || fxv8::IsString(argOne) ||
      fxv8::IsNumber(argOne)) {
    info.GetReturnValue().Set(argOne);
    return;
  }

  std::vector<v8::Local<v8::Value>> values(3);
  int intVal = 3;
  if (fxv8::IsNull(argOne)) {
    // TODO(dsinclair): Why is this 4 when the others are all 3?
    intVal = 4;
    values[2] = fxv8::NewNullHelper(info.GetIsolate());
  } else if (fxv8::IsArray(argOne)) {
    v8::Local<v8::Array> arr = argOne.As<v8::Array>();
    v8::Local<v8::Value> propertyValue =
        fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 1);
    v8::Local<v8::Value> jsObjectValue =
        fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 2);
    if (!fxv8::IsNull(propertyValue) || fxv8::IsNull(jsObjectValue)) {
      pContext->ThrowArgumentMismatchException();
      return;
    }
    values[2] = jsObjectValue;
  } else if (fxv8::IsObject(argOne)) {
    values[2] = argOne;
  } else {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  values[0] = fxv8::NewNumberHelper(info.GetIsolate(), intVal);
  values[1] = fxv8::NewNullHelper(info.GetIsolate());
  info.GetReturnValue().Set(fxv8::NewArrayHelper(info.GetIsolate(), values));
}

// static
void CFXJSE_FormCalcContext::UnitType(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("UnitType");
    return;
  }

  v8::Local<v8::Value> unitspanValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(unitspanValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsUnitspan = ValueToUTF8String(info.GetIsolate(), unitspanValue);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("UnitValue");
    return;
  }

  v8::Local<v8::Value> unitspanValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(unitspanValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsUnitspan = ValueToUTF8String(info.GetIsolate(), unitspanValue);
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
    v8::Local<v8::Value> unitValue = GetSimpleValue(info, 1);
    ByteString bsUnitTemp = ValueToUTF8String(info.GetIsolate(), unitValue);
    const char* pChar = bsUnitTemp.c_str();
    size_t uVal = 0;
    while (IsWhitespace(pChar[uVal]))
      ++uVal;

    while (uVal < bsUnitTemp.GetLength()) {
      if (!isdigit(pChar[uVal]) && pChar[uVal] != '.')
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("At");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString stringTwo = ValueToUTF8String(info.GetIsolate(), argTwo);
  if (stringTwo.IsEmpty()) {
    info.GetReturnValue().Set(1);
    return;
  }

  ByteString stringOne = ValueToUTF8String(info.GetIsolate(), argOne);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Concat");
    return;
  }

  ByteString bsResult;
  bool bAllNull = true;
  for (int32_t i = 0; i < argc; i++) {
    v8::Local<v8::Value> value = GetSimpleValue(info, i);
    if (ValueIsNull(info.GetIsolate(), value))
      continue;

    bAllNull = false;
    bsResult += ValueToUTF8String(info.GetIsolate(), value);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Decode");
    return;
  }

  if (argc == 1) {
    v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
    if (ValueIsNull(info.GetIsolate(), argOne)) {
      info.GetReturnValue().SetNull();
      return;
    }

    WideString decoded = DecodeURL(WideString::FromUTF8(
        ValueToUTF8String(info.GetIsolate(), argOne).AsStringView()));
    auto result = FX_UTF8Encode(decoded.AsStringView());
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsToDecode = ValueToUTF8String(info.GetIsolate(), argOne);
  ByteString bsIdentify = ValueToUTF8String(info.GetIsolate(), argTwo);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Encode");
    return;
  }

  if (argc == 1) {
    v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
    if (ValueIsNull(info.GetIsolate(), argOne)) {
      info.GetReturnValue().SetNull();
      return;
    }
    WideString encoded =
        EncodeURL(ValueToUTF8String(info.GetIsolate(), argOne));
    auto result = FX_UTF8Encode(encoded.AsStringView());
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if (ValueIsNull(info.GetIsolate(), argOne) ||
      ValueIsNull(info.GetIsolate(), argTwo)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsToEncode = ValueToUTF8String(info.GetIsolate(), argOne);
  ByteString bsIdentify = ValueToUTF8String(info.GetIsolate(), argTwo);
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
    pContext->ThrowParamCountMismatchException("Format");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  ByteString bsPattern = ValueToUTF8String(info.GetIsolate(), argOne);

  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  ByteString bsValue = ValueToUTF8String(info.GetIsolate(), argTwo);

  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
  GCedLocaleIface* pLocale = pThisNode->GetLocale();
  WideString wsPattern = WideString::FromUTF8(bsPattern.AsStringView());
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  bool bPatternIsString;
  CXFA_LocaleValue::ValueType dwPatternType;
  std::tie(bPatternIsString, dwPatternType) =
      PatternStringType(bsPattern.AsStringView());
  if (!bPatternIsString) {
    switch (dwPatternType) {
      case CXFA_LocaleValue::ValueType::kDateTime: {
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
      case CXFA_LocaleValue::ValueType::kDate: {
        wsPattern = L"date{" + wsPattern + L"}";
      } break;
      case CXFA_LocaleValue::ValueType::kTime: {
        wsPattern = L"time{" + wsPattern + L"}";
      } break;
      case CXFA_LocaleValue::ValueType::kText: {
        wsPattern = L"text{" + wsPattern + L"}";
      } break;
      case CXFA_LocaleValue::ValueType::kFloat: {
        wsPattern = L"num{" + wsPattern + L"}";
      } break;
      default: {
        WideString wsTestPattern = L"num{" + wsPattern + L"}";
        CXFA_LocaleValue tempLocaleValue(CXFA_LocaleValue::ValueType::kFloat,
                                         wsValue, wsTestPattern, pLocale, pMgr);
        if (tempLocaleValue.IsValid()) {
          wsPattern = std::move(wsTestPattern);
          dwPatternType = CXFA_LocaleValue::ValueType::kFloat;
        } else {
          wsPattern = L"text{" + wsPattern + L"}";
          dwPatternType = CXFA_LocaleValue::ValueType::kText;
        }
      } break;
    }
  }
  CXFA_LocaleValue localeValue(dwPatternType, wsValue, wsPattern, pLocale,
                               pMgr);
  WideString wsRet;
  if (!localeValue.FormatPatterns(wsRet, wsPattern, pLocale,
                                  XFA_ValuePicture::kDisplay)) {
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Left");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if ((ValueIsNull(info.GetIsolate(), argOne)) ||
      (ValueIsNull(info.GetIsolate(), argTwo))) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), argOne);
  int32_t count = std::max(0, ValueToInteger(info.GetIsolate(), argTwo));
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), bsSource.First(count).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Len(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Len");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), argOne);
  info.GetReturnValue().Set(static_cast<int>(bsSource.GetLength()));
}

// static
void CFXJSE_FormCalcContext::Lower(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 2) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Lower");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFX_WideTextBuf szLowBuf;
  ByteString bsArg = ValueToUTF8String(info.GetIsolate(), argOne);
  WideString wsArg = WideString::FromUTF8(bsArg.AsStringView());
  for (wchar_t ch : wsArg) {
    if ((ch >= 0x41 && ch <= 0x5A) || (ch >= 0xC0 && ch <= 0xDE))
      ch += 32;
    else if (ch == 0x100 || ch == 0x102 || ch == 0x104)
      ch += 1;
    szLowBuf.AppendChar(ch);
  }
  auto result = FX_UTF8Encode(szLowBuf.AsStringView());
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Ltrim(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Ltrim");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), argOne);
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
    pContext->ThrowParamCountMismatchException("Parse");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if (ValueIsNull(info.GetIsolate(), argTwo)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsPattern = ValueToUTF8String(info.GetIsolate(), argOne);
  ByteString bsValue = ValueToUTF8String(info.GetIsolate(), argTwo);
  CXFA_Document* pDoc = pContext->GetDocument();
  CXFA_LocaleMgr* pMgr = pDoc->GetLocaleMgr();
  CXFA_Node* pThisNode = ToNode(pDoc->GetScriptContext()->GetThisObject());
  GCedLocaleIface* pLocale = pThisNode->GetLocale();
  WideString wsPattern = WideString::FromUTF8(bsPattern.AsStringView());
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  bool bPatternIsString;
  CXFA_LocaleValue::ValueType dwPatternType;
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
    case CXFA_LocaleValue::ValueType::kDateTime: {
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
    case CXFA_LocaleValue::ValueType::kDate: {
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
    case CXFA_LocaleValue::ValueType::kTime: {
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
    case CXFA_LocaleValue::ValueType::kText: {
      wsPattern = L"text{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(CXFA_LocaleValue::ValueType::kText, wsValue,
                                   wsPattern, pLocale, pMgr);
      if (!localeValue.IsValid()) {
        info.GetReturnValue().SetEmptyString();
        return;
      }
      auto result = localeValue.GetValue().ToUTF8();
      info.GetReturnValue().Set(
          fxv8::NewStringHelper(info.GetIsolate(), result.AsStringView()));
      return;
    }
    case CXFA_LocaleValue::ValueType::kFloat: {
      wsPattern = L"num{" + wsPattern + L"}";
      CXFA_LocaleValue localeValue(CXFA_LocaleValue::ValueType::kFloat, wsValue,
                                   wsPattern, pLocale, pMgr);
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
        CXFA_LocaleValue localeValue(CXFA_LocaleValue::ValueType::kFloat,
                                     wsValue, wsTestPattern, pLocale, pMgr);
        if (localeValue.IsValid()) {
          info.GetReturnValue().Set(localeValue.GetDoubleNum());
          return;
        }
      }

      {
        WideString wsTestPattern = L"text{" + wsPattern + L"}";
        CXFA_LocaleValue localeValue(CXFA_LocaleValue::ValueType::kText,
                                     wsValue, wsTestPattern, pLocale, pMgr);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Replace");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  ByteString bsOne;
  ByteString bsTwo;
  if (!ValueIsNull(info.GetIsolate(), argOne) &&
      !ValueIsNull(info.GetIsolate(), argTwo)) {
    bsOne = ValueToUTF8String(info.GetIsolate(), argOne);
    bsTwo = ValueToUTF8String(info.GetIsolate(), argTwo);
  }

  ByteString bsThree;
  if (argc > 2) {
    v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
    bsThree = ValueToUTF8String(info.GetIsolate(), argThree);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Right");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  if ((ValueIsNull(info.GetIsolate(), argOne)) ||
      (ValueIsNull(info.GetIsolate(), argTwo))) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), argOne);
  int32_t count = std::max(0, ValueToInteger(info.GetIsolate(), argTwo));
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(), bsSource.Last(count).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Rtrim(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Rtrim");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), argOne);
  bsSource.TrimRight();
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsSource.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Space(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Space");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  int count = std::max(0, ValueToInteger(info.GetIsolate(), argOne));
  if (count > kMaxCharCount) {
    ToFormCalcContext(pThis)->ThrowException("String too long.");
    return;
  }
  std::vector<char, FxAllocAllocator<char>> space_string(count, ' ');
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), ByteStringView(space_string)));
}

// static
void CFXJSE_FormCalcContext::Str(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 1 || argc > 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Str");
    return;
  }

  v8::Local<v8::Value> numberValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(numberValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fNumber = ValueToFloat(info.GetIsolate(), numberValue);

  int32_t iWidth = 10;
  if (argc > 1) {
    v8::Local<v8::Value> widthValue = GetSimpleValue(info, 1);
    iWidth = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), widthValue));
    if (iWidth > kMaxCharCount) {
      ToFormCalcContext(pThis)->ThrowException("String too long.");
      return;
    }
  }

  int32_t iPrecision = 0;
  if (argc > 2) {
    v8::Local<v8::Value> precisionValue = GetSimpleValue(info, 2);
    iPrecision = std::max(0, static_cast<int32_t>(ValueToFloat(
                                 info.GetIsolate(), precisionValue)));
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

  if (u > iWidth || (iPrecision + u) >= iWidth) {
    std::vector<char, FxAllocAllocator<char>> stars(std::max(iWidth, 0), '*');
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), ByteStringView(stars)));
    return;
  }

  ByteString resultBuf;
  if (u == iLength) {
    if (iLength > iWidth) {
      int32_t i = 0;
      while (i < iWidth) {
        resultBuf += '*';
        ++i;
      }
    } else {
      int32_t i = 0;
      while (i < iWidth - iLength) {
        resultBuf += ' ';
        ++i;
      }
      resultBuf += pData;
    }
    info.GetReturnValue().Set(
        fxv8::NewStringHelper(info.GetIsolate(), resultBuf.AsStringView()));
    return;
  }

  int32_t iLeavingSpace = iWidth - u - iPrecision;
  if (iPrecision != 0)
    iLeavingSpace--;

  int32_t i = 0;
  while (i < iLeavingSpace) {
    resultBuf += ' ';
    ++i;
  }
  i = 0;
  while (i < u) {
    resultBuf += pData[i];
    ++i;
  }
  if (iPrecision != 0)
    resultBuf += '.';

  u++;
  i = 0;
  while (u < iLength) {
    if (i >= iPrecision)
      break;

    resultBuf += pData[u];
    ++i;
    ++u;
  }
  while (i < iPrecision) {
    resultBuf += '0';
    ++i;
  }
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), resultBuf.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Stuff(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  int32_t argc = info.Length();
  if (argc < 3 || argc > 4) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Stuff");
    return;
  }

  v8::Local<v8::Value> sourceValue = GetSimpleValue(info, 0);
  v8::Local<v8::Value> startValue = GetSimpleValue(info, 1);
  v8::Local<v8::Value> deleteValue = GetSimpleValue(info, 2);
  if (fxv8::IsNull(sourceValue) || fxv8::IsNull(startValue) ||
      fxv8::IsNull(deleteValue)) {
    info.GetReturnValue().SetNull();
    return;
  }

  int32_t iStart = 1;  // one-based character indexing.
  int32_t iDelete = 0;
  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), sourceValue);
  int32_t iLength = pdfium::base::checked_cast<int32_t>(bsSource.GetLength());
  if (iLength) {
    iStart = pdfium::clamp(
        static_cast<int32_t>(ValueToFloat(info.GetIsolate(), startValue)), 1,
        iLength);
    iDelete = pdfium::clamp(
        static_cast<int32_t>(ValueToFloat(info.GetIsolate(), deleteValue)), 0,
        iLength - iStart + 1);
  }

  ByteString bsInsert;
  if (argc > 3) {
    v8::Local<v8::Value> insertValue = GetSimpleValue(info, 3);
    bsInsert = ValueToUTF8String(info.GetIsolate(), insertValue);
  }

  --iStart;  // now zero-based.
  ByteString bsResult = {bsSource.AsStringView().First(iStart),
                         bsInsert.AsStringView(),
                         bsSource.AsStringView().Substr(iStart + iDelete)};
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsResult.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Substr(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3) {
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Substr");
    return;
  }

  v8::Local<v8::Value> string_value = GetSimpleValue(info, 0);
  v8::Local<v8::Value> start_value = GetSimpleValue(info, 1);
  v8::Local<v8::Value> end_value = GetSimpleValue(info, 2);
  if (ValueIsNull(info.GetIsolate(), string_value) ||
      ValueIsNull(info.GetIsolate(), start_value) ||
      ValueIsNull(info.GetIsolate(), end_value)) {
    info.GetReturnValue().SetNull();
    return;
  }

  ByteString bsSource = ValueToUTF8String(info.GetIsolate(), string_value);
  size_t iLength = bsSource.GetLength();
  if (iLength == 0) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  // |start_value| is 1-based. Assume first character if |start_value| is less
  // than 1, per spec. Subtract 1 since |iStart| is 0-based.
  size_t iStart =
      std::max(ValueToInteger(info.GetIsolate(), start_value), 1) - 1;
  if (iStart >= iLength) {
    info.GetReturnValue().SetEmptyString();
    return;
  }

  // Negative values are treated as 0. Can't clamp() due to sign mismatches.
  size_t iCount = std::max(ValueToInteger(info.GetIsolate(), end_value), 0);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Uuid");
    return;
  }

  int32_t iNum = 0;
  if (argc > 0) {
    v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
    iNum = static_cast<int32_t>(ValueToFloat(info.GetIsolate(), argOne));
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("Upper");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (ValueIsNull(info.GetIsolate(), argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  CFX_WideTextBuf upperStringBuf;
  ByteString bsArg = ValueToUTF8String(info.GetIsolate(), argOne);
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
    ToFormCalcContext(pThis)->ThrowParamCountMismatchException("WordNum");
    return;
  }

  v8::Local<v8::Value> numberValue = GetSimpleValue(info, 0);
  if (fxv8::IsNull(numberValue)) {
    info.GetReturnValue().SetNull();
    return;
  }
  float fNumber = ValueToFloat(info.GetIsolate(), numberValue);

  int32_t iIdentifier = 0;
  if (argc > 1) {
    v8::Local<v8::Value> identifierValue = GetSimpleValue(info, 1);
    if (fxv8::IsNull(identifierValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    iIdentifier =
        static_cast<int32_t>(ValueToFloat(info.GetIsolate(), identifierValue));
  }

  ByteString bsLocale;
  if (argc > 2) {
    v8::Local<v8::Value> localeValue = GetSimpleValue(info, 2);
    if (fxv8::IsNull(localeValue)) {
      info.GetReturnValue().SetNull();
      return;
    }
    bsLocale = ValueToUTF8String(info.GetIsolate(), localeValue);
  }

  if (isnan(fNumber) || fNumber < 0.0f || fNumber > 922337203685477550.0f) {
    info.GetReturnValue().Set(fxv8::NewStringHelper(info.GetIsolate(), "*"));
    return;
  }
  ByteString bsFormatted = ByteString::Format("%.2f", fNumber);
  ByteString bsWorded = WordUS(bsFormatted.AsStringView(), iIdentifier);
  info.GetReturnValue().Set(
      fxv8::NewStringHelper(info.GetIsolate(), bsWorded.AsStringView()));
}

// static
void CFXJSE_FormCalcContext::Get(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 1) {
    pContext->ThrowParamCountMismatchException("Get");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  CXFA_FFApp::CallbackIface* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  ByteString bsUrl = ValueToUTF8String(info.GetIsolate(), argOne);
  RetainPtr<IFX_SeekableReadStream> pFile =
      pAppProvider->DownloadURL(WideString::FromUTF8(bsUrl.AsStringView()));
  if (!pFile)
    return;

  int32_t size = pFile->GetSize();
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> dataBuf(size);
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
    pContext->ThrowParamCountMismatchException("Post");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  CXFA_FFApp::CallbackIface* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  ByteString bsURL = ValueToUTF8String(info.GetIsolate(), argOne);

  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  ByteString bsData = ValueToUTF8String(info.GetIsolate(), argTwo);

  ByteString bsContentType;
  if (argc > 2) {
    v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
    bsContentType = ValueToUTF8String(info.GetIsolate(), argThree);
  }

  ByteString bsEncode;
  if (argc > 3) {
    v8::Local<v8::Value> argFour = GetSimpleValue(info, 3);
    bsEncode = ValueToUTF8String(info.GetIsolate(), argFour);
  }

  ByteString bsHeader;
  if (argc > 4) {
    v8::Local<v8::Value> argFive = GetSimpleValue(info, 4);
    bsHeader = ValueToUTF8String(info.GetIsolate(), argFive);
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
    pContext->ThrowParamCountMismatchException("Put");
    return;
  }

  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc)
    return;

  CXFA_FFApp::CallbackIface* pAppProvider = pDoc->GetNotify()->GetAppProvider();
  if (!pAppProvider)
    return;

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  ByteString bsURL = ValueToUTF8String(info.GetIsolate(), argOne);

  v8::Local<v8::Value> argTwo = GetSimpleValue(info, 1);
  ByteString bsData = ValueToUTF8String(info.GetIsolate(), argTwo);

  ByteString bsEncode;
  if (argc > 2) {
    v8::Local<v8::Value> argThree = GetSimpleValue(info, 2);
    bsEncode = ValueToUTF8String(info.GetIsolate(), argThree);
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
  v8::Isolate* pIsolate = info.GetIsolate();
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  if (info.Length() != 2) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  ByteStringView bsFuncName("asgn_val_op");
  v8::Local<v8::Value> lValue = info[0];
  v8::Local<v8::Value> rValue = GetSimpleValue(info, 1);
  if (fxv8::IsArray(lValue)) {
    v8::Local<v8::Array> arr = lValue.As<v8::Array>();
    uint32_t iLeftLength = fxv8::GetArrayLengthHelper(arr);
    v8::Local<v8::Value> propertyValue =
        fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, 1);
    for (uint32_t i = 2; i < iLeftLength; i++) {
      v8::Local<v8::Value> jsValue =
          fxv8::ReentrantGetArrayElementHelper(pIsolate, arr, i);
      if (!fxv8::IsObject(jsValue)) {
        pContext->ThrowNoDefaultPropertyException(bsFuncName);
        return;
      }
      v8::Local<v8::Object> jsObjectValue = jsValue.As<v8::Object>();
      if (fxv8::IsNull(propertyValue)) {
        if (!SetObjectDefaultValue(pIsolate, jsObjectValue, rValue)) {
          pContext->ThrowNoDefaultPropertyException(bsFuncName);
          return;
        }
      } else {
        fxv8::ReentrantPutObjectPropertyHelper(
            pIsolate, jsObjectValue,
            fxv8::ReentrantToByteStringHelper(pIsolate, propertyValue)
                .AsStringView(),
            rValue);
      }
    }
  } else if (fxv8::IsObject(lValue)) {
    if (!SetObjectDefaultValue(pIsolate, lValue.As<v8::Object>(), rValue)) {
      pContext->ThrowNoDefaultPropertyException(bsFuncName);
      return;
    }
  }
  info.GetReturnValue().Set(rValue);
}

// static
void CFXJSE_FormCalcContext::logical_or_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float first = ValueToFloat(info.GetIsolate(), argFirst);
  float second = ValueToFloat(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)) {
    info.GetReturnValue().SetNull();
    return;
  }

  float first = ValueToFloat(info.GetIsolate(), argFirst);
  float second = ValueToFloat(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(
        static_cast<int>(fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)));
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    info.GetReturnValue().Set(static_cast<int>(
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst) ==
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond)));
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(
        static_cast<int>(!fxv8::IsNull(argFirst) || !fxv8::IsNull(argSecond)));
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    info.GetReturnValue().Set(static_cast<int>(
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst) !=
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond)));
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
  info.GetReturnValue().Set(static_cast<int>(first != second));
}

// static
bool CFXJSE_FormCalcContext::fm_ref_equal(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Local<v8::Value> argFirst = info[0];
  v8::Local<v8::Value> argSecond = info[1];
  if (!fxv8::IsArray(argFirst) || !fxv8::IsArray(argSecond))
    return false;

  v8::Local<v8::Array> firstArr = argFirst.As<v8::Array>();
  v8::Local<v8::Array> secondArr = argSecond.As<v8::Array>();
  v8::Local<v8::Value> firstFlag =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), firstArr, 0);
  v8::Local<v8::Value> secondFlag =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), secondArr, 0);
  if (fxv8::ReentrantToInt32Helper(info.GetIsolate(), firstFlag) != 3 ||
      fxv8::ReentrantToInt32Helper(info.GetIsolate(), secondFlag) != 3) {
    return false;
  }

  v8::Local<v8::Value> firstValue =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), firstArr, 2);
  v8::Local<v8::Value> secondValue =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), secondArr, 2);

  if (fxv8::IsNull(firstValue) || fxv8::IsNull(secondValue))
    return false;

  return FXJSE_RetrieveObjectBinding(firstValue) ==
         FXJSE_RetrieveObjectBinding(secondValue);
}

// static
void CFXJSE_FormCalcContext::less_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 2) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(0);
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    ByteString bs1 =
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst);
    ByteString bs2 =
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond);
    info.GetReturnValue().Set(bs1.Compare(bs2.AsStringView()) < 0);
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(
        static_cast<int>(fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)));
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    auto bs1 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst);
    auto bs2 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond);
    info.GetReturnValue().Set(bs1.Compare(bs2.AsStringView()) <= 0);
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(0);
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    auto bs1 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst);
    auto bs2 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond);
    info.GetReturnValue().Set(bs1.Compare(bs2.AsStringView()) > 0);
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) || fxv8::IsNull(argSecond)) {
    info.GetReturnValue().Set(
        static_cast<int>(fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)));
    return;
  }

  if (fxv8::IsString(argFirst) && fxv8::IsString(argSecond)) {
    auto bs1 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argFirst);
    auto bs2 = fxv8::ReentrantToByteStringHelper(info.GetIsolate(), argSecond);
    info.GetReturnValue().Set(bs1.Compare(bs2.AsStringView()) >= 0);
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  if (ValueIsNull(info.GetIsolate(), info[0]) &&
      ValueIsNull(info.GetIsolate(), info[1])) {
    info.GetReturnValue().SetNull();
    return;
  }

  const double first = ValueToDouble(info.GetIsolate(), info[0]);
  const double second = ValueToDouble(info.GetIsolate(), info[1]);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
  double second = ValueToDouble(info.GetIsolate(), argSecond);
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

  v8::Local<v8::Value> argFirst = GetSimpleValue(info, 0);
  v8::Local<v8::Value> argSecond = GetSimpleValue(info, 1);
  if (fxv8::IsNull(argFirst) && fxv8::IsNull(argSecond)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double second = ValueToDouble(info.GetIsolate(), argSecond);
  if (second == 0.0) {
    pContext->ThrowDivideByZeroException();
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argFirst);
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

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(0.0 + ValueToDouble(info.GetIsolate(), argOne));
}

// static
void CFXJSE_FormCalcContext::negative_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }
  info.GetReturnValue().Set(0.0 - ValueToDouble(info.GetIsolate(), argOne));
}

// static
void CFXJSE_FormCalcContext::logical_not_operator(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  if (fxv8::IsNull(argOne)) {
    info.GetReturnValue().SetNull();
    return;
  }

  double first = ValueToDouble(info.GetIsolate(), argOne);
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
    pContext->ThrowParamCountMismatchException("Eval");
    return;
  }

  v8::Local<v8::Value> argOne = GetSimpleValue(info, 0);
  ByteString bsArg = ValueToUTF8String(info.GetIsolate(), argOne);
  if (bsArg.IsEmpty()) {
    pContext->ThrowArgumentMismatchException();
    return;
  }

  WideString wsCalcScript = WideString::FromUTF8(bsArg.AsStringView());
  absl::optional<CFX_WideTextBuf> wsJavaScriptBuf =
      CFXJSE_FormCalcContext::Translate(pContext->GetDocument()->GetHeap(),
                                        wsCalcScript.AsStringView());
  if (!wsJavaScriptBuf.has_value()) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  info.GetReturnValue().Set(fxv8::NewStringHelper(
      info.GetIsolate(),
      FX_UTF8Encode(wsJavaScriptBuf.value().AsStringView()).AsStringView()));
}

// static
void CFXJSE_FormCalcContext::is_fm_object(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const bool result = info.Length() == 1 && fxv8::IsObject(info[0]);
  info.GetReturnValue().Set(result);
}

// static
void CFXJSE_FormCalcContext::is_fm_array(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  const bool result = info.Length() == 1 && fxv8::IsArray(info[0]);
  info.GetReturnValue().Set(result);
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

  v8::Local<v8::Value> argOne = info[0];
  if (fxv8::IsArray(argOne)) {
    v8::Local<v8::Array> arr = argOne.As<v8::Array>();
    v8::Local<v8::Value> propertyValue =
        fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 1);
    v8::Local<v8::Value> jsValue =
        fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 2);
    if (!fxv8::IsObject(jsValue)) {
      info.GetReturnValue().Set(fxv8::NewUndefinedHelper(info.GetIsolate()));
      return;
    }
    v8::Local<v8::Object> jsObjectValue = jsValue.As<v8::Object>();
    if (fxv8::IsNull(propertyValue)) {
      info.GetReturnValue().Set(
          GetObjectDefaultValue(info.GetIsolate(), jsObjectValue));
      return;
    }
    ByteString bsName =
        fxv8::ReentrantToByteStringHelper(info.GetIsolate(), propertyValue);
    info.GetReturnValue().Set(fxv8::ReentrantGetObjectPropertyHelper(
        info.GetIsolate(), jsObjectValue, bsName.AsStringView()));
    return;
  }

  if (fxv8::IsObject(argOne)) {
    v8::Local<v8::Object> obj = argOne.As<v8::Object>();
    info.GetReturnValue().Set(GetObjectDefaultValue(info.GetIsolate(), obj));
    return;
  }

  info.GetReturnValue().Set(argOne);
}

// static
void CFXJSE_FormCalcContext::get_fm_jsobj(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    ToFormCalcContext(pThis)->ThrowCompilerErrorException();
    return;
  }

  v8::Local<v8::Value> argOne = info[0];
  if (!fxv8::IsArray(argOne)) {
    info.GetReturnValue().Set(argOne);
    return;
  }

  v8::Local<v8::Array> arr = argOne.As<v8::Array>();
  info.GetReturnValue().Set(
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 2));
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

  v8::Local<v8::Value> argOne = info[0];
  if (!fxv8::IsArray(argOne)) {
    info.GetReturnValue().Set(GetSimpleValue(info, 0));
    return;
  }

  v8::Local<v8::Array> arr = argOne.As<v8::Array>();
  v8::Local<v8::Value> flagsValue =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 0);
  int32_t iFlags = fxv8::ReentrantToInt32Helper(info.GetIsolate(), flagsValue);
  if (iFlags != 3 && iFlags != 4) {
    info.GetReturnValue().Set(GetSimpleValue(info, 0));
    return;
  }

  if (iFlags == 4) {
    std::vector<v8::Local<v8::Value>> values(3);
    values[0] = fxv8::NewNumberHelper(info.GetIsolate(), 3);
    values[1] = fxv8::NewNullHelper(info.GetIsolate());
    values[2] = fxv8::NewNullHelper(info.GetIsolate());
    info.GetReturnValue().Set(fxv8::NewArrayHelper(info.GetIsolate(), values));
    return;
  }

  v8::Local<v8::Value> objectValue =
      fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, 2);
  if (fxv8::IsNull(objectValue)) {
    pContext->ThrowCompilerErrorException();
    return;
  }
  info.GetReturnValue().Set(argOne);
}

// static
void CFXJSE_FormCalcContext::concat_fm_object(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* pIsolate = ToFormCalcContext(pThis)->GetIsolate();
  std::vector<v8::Local<v8::Value>> returnValues;
  for (int i = 0; i < info.Length(); ++i) {
    if (fxv8::IsArray(info[i])) {
      v8::Local<v8::Array> arr = info[i].As<v8::Array>();
      uint32_t length = fxv8::GetArrayLengthHelper(arr);
      for (uint32_t j = 2; j < length; j++) {
        returnValues.push_back(
            fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, j));
      }
    }
    returnValues.push_back(info[i]);
  }
  info.GetReturnValue().Set(fxv8::NewArrayHelper(pIsolate, returnValues));
}

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

absl::optional<CFX_WideTextBuf> CFXJSE_FormCalcContext::Translate(
    cppgc::Heap* pHeap,
    WideStringView wsFormcalc) {
  if (wsFormcalc.IsEmpty())
    return CFX_WideTextBuf();

  CXFA_FMLexer lexer(wsFormcalc);
  CXFA_FMParser parser(pHeap, &lexer);
  CXFA_FMAST* ast = parser.Parse();
  if (!ast || parser.HasError())
    return absl::nullopt;

  CXFA_FMToJavaScriptDepth::Reset();
  absl::optional<CFX_WideTextBuf> wsJavaScript = ast->ToJavaScript();
  if (!wsJavaScript.has_value())
    return absl::nullopt;

  if (CXFA_IsTooBig(wsJavaScript.value()))
    return absl::nullopt;

  return wsJavaScript;
}

CFXJSE_FormCalcContext::CFXJSE_FormCalcContext(v8::Isolate* pIsolate,
                                               CFXJSE_Context* pScriptContext,
                                               CXFA_Document* pDoc)
    : m_pIsolate(pIsolate), m_pDocument(pDoc) {
  m_Value.Reset(
      m_pIsolate,
      NewBoundV8Object(
          m_pIsolate,
          CFXJSE_Class::Create(pScriptContext, &kFormCalcFM2JSDescriptor, false)
              ->GetTemplate(m_pIsolate)));
}

CFXJSE_FormCalcContext::~CFXJSE_FormCalcContext() = default;

CFXJSE_FormCalcContext* CFXJSE_FormCalcContext::AsFormCalcContext() {
  return this;
}

v8::Local<v8::Value> CFXJSE_FormCalcContext::GlobalPropertyGetter() {
  return v8::Local<v8::Value>::New(m_pIsolate, m_Value);
}

// static
void CFXJSE_FormCalcContext::DotAccessorCommon(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info,
    bool bDotAccessor) {
  CFXJSE_FormCalcContext* pContext = ToFormCalcContext(pThis);
  v8::Isolate* pIsolate = pContext->GetIsolate();
  int32_t argc = info.Length();
  if (argc < 4 || argc > 5) {
    pContext->ThrowCompilerErrorException();
    return;
  }

  bool bIsStar = true;
  int32_t iIndexValue = 0;
  if (argc > 4) {
    bIsStar = false;
    iIndexValue = ValueToInteger(info.GetIsolate(), info[4]);
  }

  const ByteString bsName =
      fxv8::ReentrantToByteStringHelper(info.GetIsolate(), info[2]);
  const bool bHasNoResolveName = bDotAccessor && bsName.IsEmpty();
  ByteString bsSomExp = GenerateSomExpression(
      bsName.AsStringView(),
      fxv8::ReentrantToInt32Helper(info.GetIsolate(), info[3]), iIndexValue,
      bIsStar);

  v8::Local<v8::Value> argAccessor = info[0];
  if (fxv8::IsArray(argAccessor)) {
    v8::Local<v8::Array> arr = argAccessor.As<v8::Array>();
    uint32_t iLength = fxv8::GetArrayLengthHelper(arr);
    if (iLength < 3) {
      pContext->ThrowArgumentMismatchException();
      return;
    }

    std::vector<std::vector<v8::Local<v8::Value>>> resolveValues(iLength - 2);
    bool bAttribute = false;
    bool bAllEmpty = true;
    for (uint32_t i = 2; i < iLength; i++) {
      v8::Local<v8::Value> hJSObjValue =
          fxv8::ReentrantGetArrayElementHelper(info.GetIsolate(), arr, i);
      absl::optional<CFXJSE_Engine::ResolveResult> maybeResult =
          ResolveObjects(pThis, hJSObjValue, bsSomExp.AsStringView(),
                         bDotAccessor, bHasNoResolveName);
      if (maybeResult.has_value()) {
        resolveValues[i - 2] = ParseResolveResult(pThis, maybeResult.value(),
                                                  hJSObjValue, &bAttribute);
        bAllEmpty = bAllEmpty && resolveValues[i - 2].empty();
      }
    }
    if (bAllEmpty) {
      pContext->ThrowPropertyNotInObjectException(bsName.AsStringView(),
                                                  bsSomExp.AsStringView());
      return;
    }

    std::vector<v8::Local<v8::Value>> values;
    values.push_back(fxv8::NewNumberHelper(pIsolate, 1));
    values.push_back(
        bAttribute ? fxv8::NewStringHelper(pIsolate, bsName.AsStringView())
                         .As<v8::Value>()
                   : fxv8::NewNullHelper(pIsolate).As<v8::Value>());
    for (uint32_t i = 0; i < iLength - 2; i++) {
      for (size_t j = 0; j < resolveValues[i].size(); j++)
        values.push_back(resolveValues[i][j]);
    }
    info.GetReturnValue().Set(fxv8::NewArrayHelper(pIsolate, values));
    return;
  }

  absl::optional<CFXJSE_Engine::ResolveResult> maybeResult;
  ByteString bsAccessorName =
      fxv8::ReentrantToByteStringHelper(info.GetIsolate(), info[1]);
  if (fxv8::IsObject(argAccessor) ||
      (fxv8::IsNull(argAccessor) && bsAccessorName.IsEmpty())) {
    maybeResult = ResolveObjects(pThis, argAccessor, bsSomExp.AsStringView(),
                                 bDotAccessor, bHasNoResolveName);
  } else if (!fxv8::IsObject(argAccessor) && !bsAccessorName.IsEmpty()) {
    v8::Local<v8::Value> obj =
        GetObjectForName(pThis, bsAccessorName.AsStringView());
    if (!obj.IsEmpty()) {
      argAccessor = obj;
      maybeResult = ResolveObjects(pThis, argAccessor, bsSomExp.AsStringView(),
                                   bDotAccessor, bHasNoResolveName);
    }
  }
  if (!maybeResult.has_value()) {
    pContext->ThrowPropertyNotInObjectException(bsName.AsStringView(),
                                                bsSomExp.AsStringView());
    return;
  }

  bool bAttribute = false;
  std::vector<v8::Local<v8::Value>> resolveValues =
      ParseResolveResult(pThis, maybeResult.value(), argAccessor, &bAttribute);

  std::vector<v8::Local<v8::Value>> values(resolveValues.size() + 2);
  values[0] = fxv8::NewNumberHelper(pIsolate, 1);
  values[1] = bAttribute
                  ? fxv8::NewStringHelper(pIsolate, bsName.AsStringView())
                        .As<v8::Value>()
                  : fxv8::NewNullHelper(pIsolate).As<v8::Value>();

  for (size_t i = 0; i < resolveValues.size(); i++)
    values[i + 2] = resolveValues[i];

  info.GetReturnValue().Set(fxv8::NewArrayHelper(pIsolate, values));
}

bool CFXJSE_FormCalcContext::ApplyToExpansion(
    std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
    const v8::FunctionCallbackInfo<v8::Value>& info,
    bool bStrict) {
  v8::Isolate* pIsolate = info.GetIsolate();
  for (int32_t i = 0; i < info.Length(); i++) {
    v8::Local<v8::Value> argValue = info[i];
    if (fxv8::IsArray(argValue)) {
      if (!ApplyToArray(pIsolate, fn, argValue.As<v8::Array>()) && bStrict) {
        ThrowArgumentMismatchException();
        return false;
      }
    } else if (fxv8::IsObject(argValue)) {
      ApplyToObject(pIsolate, fn, argValue.As<v8::Object>());
    } else if (!fxv8::IsNull(argValue)) {
      fn(pIsolate, argValue);
    }
  }
  return true;
}

bool CFXJSE_FormCalcContext::ApplyToArray(
    v8::Isolate* pIsolate,
    std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
    v8::Local<v8::Array> pArray) {
  uint32_t iLength = fxv8::GetArrayLengthHelper(pArray);
  if (iLength < 3)
    return false;

  v8::Local<v8::Value> propertyValue =
      fxv8::ReentrantGetArrayElementHelper(pIsolate, pArray, 1);

  ByteString bsName;
  const bool nullprop = fxv8::IsNull(propertyValue);
  if (!nullprop)
    bsName = fxv8::ReentrantToByteStringHelper(pIsolate, propertyValue);

  for (uint32_t j = 2; j < iLength; j++) {
    v8::Local<v8::Value> jsValue =
        fxv8::ReentrantGetArrayElementHelper(pIsolate, pArray, j);
    if (!fxv8::IsObject(jsValue))
      continue;

    v8::Local<v8::Object> jsObjectValue = jsValue.As<v8::Object>();
    v8::Local<v8::Value> newPropertyValue =
        nullprop ? GetObjectDefaultValue(pIsolate, jsObjectValue)
                 : fxv8::ReentrantGetObjectPropertyHelper(
                       pIsolate, jsObjectValue, bsName.AsStringView());
    if (!fxv8::IsNull(newPropertyValue))
      fn(pIsolate, newPropertyValue);
  }
  return true;
}

void CFXJSE_FormCalcContext::ApplyToObject(
    v8::Isolate* pIsolate,
    std::function<void(v8::Isolate*, v8::Local<v8::Value>)> fn,
    v8::Local<v8::Object> pObject) {
  v8::Local<v8::Value> newPropertyValue =
      GetObjectDefaultValue(pIsolate, pObject);
  if (!fxv8::IsNull(newPropertyValue))
    fn(pIsolate, newPropertyValue);
}

void CFXJSE_FormCalcContext::ThrowNoDefaultPropertyException(
    ByteStringView name) const {
  ByteString msg(name);
  msg += " doesn't have a default property.";
  ThrowException(msg.AsStringView());
}

void CFXJSE_FormCalcContext::ThrowCompilerErrorException() const {
  ThrowException("Compiler error.");
}

void CFXJSE_FormCalcContext::ThrowDivideByZeroException() const {
  ThrowException("Divide by zero.");
}

void CFXJSE_FormCalcContext::ThrowServerDeniedException() const {
  ThrowException("Server does not permit operation.");
}

void CFXJSE_FormCalcContext::ThrowPropertyNotInObjectException(
    ByteStringView name,
    ByteStringView exp) const {
  ByteString msg("An attempt was made to reference property '");
  msg += name;
  msg += "' of a non-object in SOM expression ";
  msg += exp;
  msg += ".";
  ThrowException(msg.AsStringView());
}

void CFXJSE_FormCalcContext::ThrowParamCountMismatchException(
    ByteStringView method) const {
  ByteString msg("Incorrect number of parameters calling method '");
  msg += method;
  msg += "'.";
  ThrowException(msg.AsStringView());
}

void CFXJSE_FormCalcContext::ThrowArgumentMismatchException() const {
  ThrowException("Argument mismatch in property or function argument.");
}

void CFXJSE_FormCalcContext::ThrowException(ByteStringView str) const {
  DCHECK(!str.IsEmpty());
  FXJSE_ThrowMessage(GetIsolate(), str);
}
