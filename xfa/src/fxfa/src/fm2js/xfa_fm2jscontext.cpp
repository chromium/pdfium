// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
#include <time.h>
#define FINANCIAL_PRECISION 0.00000001
struct XFA_FMHtmlReserveCode {
  uint32_t m_uCode;
  const FX_WCHAR* m_htmlReserve;
};
struct XFA_FMHtmlHashedReserveCode {
  uint32_t m_uHash;
  const FX_WCHAR* m_htmlReserve;
  uint32_t m_uCode;
};
static XFA_FMHtmlHashedReserveCode reservesForDecode[] = {
    {0x00018b62, L"Mu", 924},       {0x00019083, L"Nu", 925},
    {0x00019ab9, L"Pi", 928},       {0x0001c3c1, L"Xi", 926},
    {0x000210ac, L"ge", 8805},      {0x000210bb, L"gt", 62},
    {0x00022a51, L"le", 8804},      {0x00022a60, L"lt", 60},
    {0x00022f82, L"mu", 956},       {0x00023493, L"ne", 8800},
    {0x00023497, L"ni", 8715},      {0x000234a3, L"nu", 957},
    {0x000239c1, L"or", 8744},      {0x00023ed9, L"pi", 960},
    {0x000267e1, L"xi", 958},       {0x00c41789, L"lceil", 8968},
    {0x00eef34f, L"thetasym", 977}, {0x012d7ead, L"lcirc", 206},
    {0x01637b56, L"agrave", 224},   {0x020856da, L"crarr", 8629},
    {0x022188c3, L"gamma", 947},    {0x033586d3, L"nbsp", 160},
    {0x04f4c358, L"nsub", 8836},    {0x0581466a, L"dagger", 8224},
    {0x06b1f790, L"oelig", 339},    {0x06e490d4, L"Chi", 935},
    {0x0718c6a1, L"ETH", 208},      {0x07196ada, L"Eta", 919},
    {0x07f667ca, L"Ugrave", 217},   {0x083a8a21, L"Phi", 934},
    {0x083ac28c, L"Psi", 936},      {0x086f26a9, L"Rho", 929},
    {0x089b5b51, L"aring", 229},    {0x08a39f4a, L"Tau", 932},
    {0x08b6188b, L"THORN", 222},    {0x09ce792a, L"icirc", 238},
    {0x09f9d61e, L"amp", 38},       {0x09f9db33, L"and", 8743},
    {0x09f9db36, L"ang", 8736},     {0x0a2e3514, L"cap", 8745},
    {0x0a2e58f4, L"chi", 967},      {0x0a2e9ba8, L"cup", 8746},
    {0x0a4897d0, L"deg", 176},      {0x0a6332fa, L"eta", 951},
    {0x0a633301, L"eth", 240},      {0x0acc4d4b, L"int", 8747},
    {0x0b1b3d35, L"loz", 9674},     {0x0b1b4c8b, L"lrm", 8206},
    {0x0b4fd9b1, L"not", 172},      {0x0b845241, L"phi", 966},
    {0x0b84576f, L"piv", 982},      {0x0b848aac, L"psi", 968},
    {0x0bb8df5e, L"reg", 174},      {0x0bb8eec9, L"rho", 961},
    {0x0bb9034b, L"rlm", 8207},     {0x0bd33d14, L"shy", 173},
    {0x0bd34229, L"sim", 8764},     {0x0bd37faa, L"sub", 8834},
    {0x0bd37fb5, L"sum", 8721},     {0x0bd37fb8, L"sup", 8835},
    {0x0bed676a, L"tau", 964},      {0x0c07f32e, L"uml", 168},
    {0x0c71032c, L"yen", 165},      {0x0c7f2889, L"szlig", 223},
    {0x0c8badbb, L"zwj", 8205},     {0x10ba4dba, L"Egrave", 200},
    {0x10f1ea24, L"para", 182},     {0x10f1ea37, L"part", 8706},
    {0x115b2337, L"perp", 8869},    {0x12b10d15, L"prod", 8719},
    {0x12b10d21, L"prop", 8733},    {0x12dfa9f4, L"rfloor", 8971},
    {0x12eb4736, L"Agrave", 192},   {0x12fff2b7, L"pund", 163},
    {0x13fda9f2, L"tilde", 732},    {0x1417fd62, L"times", 215},
    {0x154fc726, L"ecirc", 234},    {0x165aa451, L"sigma", 963},
    {0x1709124a, L"Dagger", 8225},  {0x192f78d5, L"iexcl", 161},
    {0x1b7ed8d7, L"rArr", 8658},    {0x1ec88c68, L"rang", 9002},
    {0x1ec8a0f7, L"rarr", 8594},    {0x1eda07f3, L"atilde", 227},
    {0x1f3182c4, L"real", 8476},    {0x1fc34f8b, L"yacute", 253},
    {0x20d11522, L"acirc", 226},    {0x21933a9b, L"rsaquo", 8250},
    {0x21f44907, L"uacute", 250},   {0x220cca72, L"acute", 180},
    {0x242cded1, L"alefsym", 8501}, {0x2655c66a, L"delta", 948},
    {0x269e4b4d, L"exist", 8707},   {0x273379fa, L"micro", 181},
    {0x27a37440, L"forall", 8704},  {0x2854e62c, L"minus", 8722},
    {0x28636f81, L"cedil", 184},    {0x2887357b, L"iacute", 237},
    {0x2994d5ff, L"frac12", 189},   {0x2994d601, L"frac14", 188},
    {0x2994e043, L"frac34", 190},   {0x2a1feb41, L"lambda", 955},
    {0x2ab215f3, L"apos", 39},      {0x2ab82ef7, L"eacute", 233},
    {0x2b3592ef, L"auml", 228},     {0x2ce92873, L"aacute", 225},
    {0x2daff48a, L"oslash", 248},   {0x2ef68882, L"aelig", 230},
    {0x3061d3d3, L"Atilde", 195},   {0x314b1b6b, L"Yacute", 221},
    {0x337c14e7, L"Uacute", 218},   {0x37676aca, L"cent", 162},
    {0x37d0b841, L"circ", 710},     {0x386e7947, L"cong", 8773},
    {0x386e839b, L"copy", 169},     {0x3a0e225a, L"Epsilon", 917},
    {0x3ba7b721, L"Lambda", 923},   {0x3bd9abe6, L"Alpha", 913},
    {0x3c3ffad7, L"Eacute", 201},   {0x3cfaf69f, L"brvbar", 166},
    {0x3d54a489, L"omega", 969},    {0x3e70f453, L"Aacute", 193},
    {0x3f37c06a, L"Oslash", 216},   {0x40e1b34e, L"diams", 9830},
    {0x416596df, L"plusmn", 177},   {0x4354ff16, L"Ucirc", 219},
    {0x454fce6a, L"Upsilon", 933},  {0x4610ad35, L"emsp", 8195},
    {0x462afb76, L"ensp", 8194},    {0x46e30073, L"euml", 235},
    {0x46e31a1b, L"euro", 8364},    {0x46f2eada, L"lowast", 8727},
    {0x4dca26cf, L"Auml", 196},     {0x4e2d6083, L"image", 8465},
    {0x4f964ee8, L"notin", 8713},   {0x50917a7a, L"epsilon", 949},
    {0x52f9a4cd, L"Kappa", 922},    {0x5496f410, L"Ocirc", 212},
    {0x568cbf34, L"zeta", 950},     {0x57badd20, L"ntilde", 241},
    {0x58662109, L"zwnj", 8204},    {0x5b39870f, L"empty", 8709},
    {0x5bd3268a, L"upsilon", 965},  {0x5e2bf8a3, L"Gamma", 915},
    {0x5f73c13a, L"rsquo", 8217},   {0x61f2bc4d, L"iota", 953},
    {0x625bbcf3, L"isin", 8712},    {0x62906df7, L"iuml", 239},
    {0x64a5cb31, L"Aring", 197},    {0x66f25c4a, L"sbquo", 8218},
    {0x6851ab60, L"spades", 9824},  {0x6942a900, L"Ntilde", 209},
    {0x69779453, L"Euml", 203},     {0x6cda6e23, L"current", 164},
    {0x70b5b634, L"lsquo", 8216},   {0x715a3706, L"Ecirc", 202},
    {0x71e8bf8d, L"tdquo", 8221},   {0x72651431, L"Sigma", 931},
    {0x7569813b, L"iquest", 191},   {0x776a436a, L"equiv", 8801},
    {0x79215314, L"Zeta", 918},     {0x79b81224, L"ograve", 242},
    {0x7c2f8b23, L"macr", 175},     {0x7cdb8502, L"Acirc", 194},
    {0x8185c62e, L"ndash", 8211},   {0x8260364a, L"Delta", 916},
    {0x846619ad, L"mdash", 8212},   {0x8550fb50, L"OElig", 338},
    {0x88eb5b85, L"ldquo", 8220},   {0x8b3fde04, L"Ograve", 210},
    {0x8bc5794b, L"ordf", 170},     {0x8bc57952, L"ordm", 186},
    {0x8c14923d, L"ouml", 246},     {0x8c5a7cd6, L"theta", 952},
    {0x8d61812b, L"thorn", 254},    {0x912b95aa, L"asymp", 8776},
    {0x947faf81, L"middot", 183},   {0x9629202e, L"lfloor", 8970},
    {0x972e9ec1, L"otilde", 245},   {0x9748f231, L"otimes", 8855},
    {0x995f1469, L"Omega", 937},    {0x99eb5349, L"quot", 34},
    {0x9aeb639e, L"hellip", 8230},  {0xa0ae2f86, L"Scaron", 352},
    {0xa4dcb0d5, L"lsaquo", 8249},  {0xa53dbf41, L"oacute", 243},
    {0xa5ae9e7b, L"bdquo", 8222},   {0xa602d7ba, L"sdot", 8901},
    {0xa61ce86f, L"sect", 167},     {0xa6e4c3d7, L"sigmaf", 962},
    {0xa7c1c74f, L"sube", 8838},    {0xa7c20ee9, L"sup1", 185},
    {0xa7c20eea, L"sup2", 178},     {0xa7c20eeb, L"sup3", 179},
    {0xa7c20f1d, L"supe", 8839},    {0xa8b66aa1, L"Otilde", 213},
    {0xad958c42, L"AElig", 198},    {0xaea9261d, L"Ouml", 214},
    {0xb040eafa, L"uArr", 8657},    {0xb07c2e1c, L"beta", 946},
    {0xb220e92f, L"bull", 8226},    {0xb22750c4, L"ccedil", 231},
    {0xb38ab31a, L"uarr", 8593},    {0xb598b683, L"uuml", 252},
    {0xb6c58b21, L"Oacute", 211},   {0xb6d2a617, L"oline", 8254},
    {0xba9fd989, L"dArr", 8659},    {0xbb5ccd41, L"lgrave", 204},
    {0xbd39b44c, L"weierp", 8472},  {0xbde9a1a9, L"darr", 8595},
    {0xc027e329, L"permil", 8240},  {0xc2451389, L"upsih", 978},
    {0xc3af1ca4, L"Ccedil", 199},   {0xcd164249, L"fnof", 402},
    {0xcf6c8467, L"hearts", 9829},  {0xd1228390, L"trade", 8482},
    {0xd1462407, L"yuml", 255},     {0xd2cf2253, L"oplus", 8853},
    {0xd310c1fc, L"Beta", 914},     {0xd59c4d74, L"infin", 8734},
    {0xd64d470d, L"hArr", 8660},    {0xd67d9c75, L"divide", 247},
    {0xd698dd37, L"Omicron", 927},  {0xd82d4a63, L"Uuml", 220},
    {0xd9970f2d, L"harr", 8596},    {0xda91fd99, L"clubs", 9827},
    {0xdbe5bdcc, L"there4", 8756},  {0xdd7671bd, L"prime", 8242},
    {0xdfcf3c06, L"alpha", 945},    {0xe0213063, L"saron", 353},
    {0xe1911d83, L"radic", 8730},   {0xe2e75468, L"raquo", 187},
    {0xe6e27a5e, L"lacute", 205},   {0xe74a8f36, L"ucirc", 251},
    {0xe864ecb6, L"Theta", 920},    {0xecddde5e, L"nabla", 8711},
    {0xed1c3557, L"omicron", 959},  {0xef82228f, L"rceil", 8969},
    {0xf1fab491, L"lArr", 8656},    {0xf3dab7e7, L"Yuml", 376},
    {0xf4294962, L"laquo", 171},    {0xf5446822, L"lang", 9001},
    {0xf5447cb1, L"larr", 8592},    {0xf66e9bea, L"ugrave", 249},
    {0xf6b4ce70, L"lota", 921},     {0xf6ef34ed, L"kappa", 954},
    {0xf72a3a56, L"thinsp", 8201},  {0xf752801a, L"luml", 207},
    {0xf88c8430, L"ocirc", 244},    {0xf9676178, L"frasl", 8260},
    {0xfd01885e, L"igrave", 236},   {0xff3281da, L"egrave", 232},
};
static XFA_FMHtmlReserveCode reservesForEncode[] = {
    {34, L"quot"},     {38, L"amp"},      {39, L"apos"},
    {60, L"lt"},       {62, L"gt"},       {160, L"nbsp"},
    {161, L"iexcl"},   {162, L"cent"},    {163, L"pund"},
    {164, L"current"}, {165, L"yen"},     {166, L"brvbar"},
    {167, L"sect"},    {168, L"uml"},     {169, L"copy"},
    {170, L"ordf"},    {171, L"laquo"},   {172, L"not"},
    {173, L"shy"},     {174, L"reg"},     {175, L"macr"},
    {176, L"deg"},     {177, L"plusmn"},  {178, L"sup2"},
    {179, L"sup3"},    {180, L"acute"},   {181, L"micro"},
    {182, L"para"},    {183, L"middot"},  {184, L"cedil"},
    {185, L"sup1"},    {186, L"ordm"},    {187, L"raquo"},
    {188, L"frac14"},  {189, L"frac12"},  {190, L"frac34"},
    {191, L"iquest"},  {192, L"Agrave"},  {193, L"Aacute"},
    {194, L"Acirc"},   {195, L"Atilde"},  {196, L"Auml"},
    {197, L"Aring"},   {198, L"AElig"},   {199, L"Ccedil"},
    {200, L"Egrave"},  {201, L"Eacute"},  {202, L"Ecirc"},
    {203, L"Euml"},    {204, L"lgrave"},  {205, L"lacute"},
    {206, L"lcirc"},   {207, L"luml"},    {208, L"ETH"},
    {209, L"Ntilde"},  {210, L"Ograve"},  {211, L"Oacute"},
    {212, L"Ocirc"},   {213, L"Otilde"},  {214, L"Ouml"},
    {215, L"times"},   {216, L"Oslash"},  {217, L"Ugrave"},
    {218, L"Uacute"},  {219, L"Ucirc"},   {220, L"Uuml"},
    {221, L"Yacute"},  {222, L"THORN"},   {223, L"szlig"},
    {224, L"agrave"},  {225, L"aacute"},  {226, L"acirc"},
    {227, L"atilde"},  {228, L"auml"},    {229, L"aring"},
    {230, L"aelig"},   {231, L"ccedil"},  {232, L"egrave"},
    {233, L"eacute"},  {234, L"ecirc"},   {235, L"euml"},
    {236, L"igrave"},  {237, L"iacute"},  {238, L"icirc"},
    {239, L"iuml"},    {240, L"eth"},     {241, L"ntilde"},
    {242, L"ograve"},  {243, L"oacute"},  {244, L"ocirc"},
    {245, L"otilde"},  {246, L"ouml"},    {247, L"divide"},
    {248, L"oslash"},  {249, L"ugrave"},  {250, L"uacute"},
    {251, L"ucirc"},   {252, L"uuml"},    {253, L"yacute"},
    {254, L"thorn"},   {255, L"yuml"},    {338, L"OElig"},
    {339, L"oelig"},   {352, L"Scaron"},  {353, L"saron"},
    {376, L"Yuml"},    {402, L"fnof"},    {710, L"circ"},
    {732, L"tilde"},   {913, L"Alpha"},   {914, L"Beta"},
    {915, L"Gamma"},   {916, L"Delta"},   {917, L"Epsilon"},
    {918, L"Zeta"},    {919, L"Eta"},     {920, L"Theta"},
    {921, L"lota"},    {922, L"Kappa"},   {923, L"Lambda"},
    {924, L"Mu"},      {925, L"Nu"},      {926, L"Xi"},
    {927, L"Omicron"}, {928, L"Pi"},      {929, L"Rho"},
    {931, L"Sigma"},   {932, L"Tau"},     {933, L"Upsilon"},
    {934, L"Phi"},     {935, L"Chi"},     {936, L"Psi"},
    {937, L"Omega"},   {945, L"alpha"},   {946, L"beta"},
    {947, L"gamma"},   {948, L"delta"},   {949, L"epsilon"},
    {950, L"zeta"},    {951, L"eta"},     {952, L"theta"},
    {953, L"iota"},    {954, L"kappa"},   {955, L"lambda"},
    {956, L"mu"},      {957, L"nu"},      {958, L"xi"},
    {959, L"omicron"}, {960, L"pi"},      {961, L"rho"},
    {962, L"sigmaf"},  {963, L"sigma"},   {964, L"tau"},
    {965, L"upsilon"}, {966, L"phi"},     {967, L"chi"},
    {968, L"psi"},     {969, L"omega"},   {977, L"thetasym"},
    {978, L"upsih"},   {982, L"piv"},     {8194, L"ensp"},
    {8195, L"emsp"},   {8201, L"thinsp"}, {8204, L"zwnj"},
    {8205, L"zwj"},    {8206, L"lrm"},    {8207, L"rlm"},
    {8211, L"ndash"},  {8212, L"mdash"},  {8216, L"lsquo"},
    {8217, L"rsquo"},  {8218, L"sbquo"},  {8220, L"ldquo"},
    {8221, L"tdquo"},  {8222, L"bdquo"},  {8224, L"dagger"},
    {8225, L"Dagger"}, {8226, L"bull"},   {8230, L"hellip"},
    {8240, L"permil"}, {8242, L"prime"},  {8249, L"lsaquo"},
    {8250, L"rsaquo"}, {8254, L"oline"},  {8260, L"frasl"},
    {8364, L"euro"},   {8465, L"image"},  {8472, L"weierp"},
    {8476, L"real"},   {8482, L"trade"},  {8501, L"alefsym"},
    {8592, L"larr"},   {8593, L"uarr"},   {8594, L"rarr"},
    {8595, L"darr"},   {8596, L"harr"},   {8629, L"crarr"},
    {8656, L"lArr"},   {8657, L"uArr"},   {8658, L"rArr"},
    {8659, L"dArr"},   {8660, L"hArr"},   {8704, L"forall"},
    {8706, L"part"},   {8707, L"exist"},  {8709, L"empty"},
    {8711, L"nabla"},  {8712, L"isin"},   {8713, L"notin"},
    {8715, L"ni"},     {8719, L"prod"},   {8721, L"sum"},
    {8722, L"minus"},  {8727, L"lowast"}, {8730, L"radic"},
    {8733, L"prop"},   {8734, L"infin"},  {8736, L"ang"},
    {8743, L"and"},    {8744, L"or"},     {8745, L"cap"},
    {8746, L"cup"},    {8747, L"int"},    {8756, L"there4"},
    {8764, L"sim"},    {8773, L"cong"},   {8776, L"asymp"},
    {8800, L"ne"},     {8801, L"equiv"},  {8804, L"le"},
    {8805, L"ge"},     {8834, L"sub"},    {8835, L"sup"},
    {8836, L"nsub"},   {8838, L"sube"},   {8839, L"supe"},
    {8853, L"oplus"},  {8855, L"otimes"}, {8869, L"perp"},
    {8901, L"sdot"},   {8968, L"lceil"},  {8969, L"rceil"},
    {8970, L"lfloor"}, {8971, L"rfloor"}, {9001, L"lang"},
    {9002, L"rang"},   {9674, L"loz"},    {9824, L"spades"},
    {9827, L"clubs"},  {9829, L"hearts"}, {9830, L"diams"},
};
void CXFA_FM2JSContext::Abs(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE dValue = HValueToDouble(hThis, argOne);
      if (dValue < 0) {
        dValue = -dValue;
      }
      FXJSE_Value_SetDouble(args.GetReturnValue(), dValue);
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Abs");
  }
}
void CXFA_FM2JSContext::Avg(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint32_t uCount = 0;
  FX_DOUBLE dSum = 0.0;
  if (argc >= 1) {
    FXJSE_HVALUE argValue = 0;
    for (int32_t i = 0; i < argc; i++) {
      argValue = args.GetValue(i);
      if (FXJSE_Value_IsNull(argValue)) {
        FXJSE_Value_Release(argValue);
        continue;
      } else if (FXJSE_Value_IsArray(argValue)) {
        FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectProp(argValue, "length", lengthValue);
        int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
        FXJSE_Value_Release(lengthValue);
        if (iLength > 2) {
          FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectPropByIdx(argValue, 1, propertyValue);
          FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
          if (FXJSE_Value_IsNull(propertyValue)) {
            for (int32_t j = 2; j < iLength; j++) {
              FXJSE_Value_GetObjectPropByIdx(argValue, j, jsObjectValue);
              FXJSE_HVALUE defaultPropValue = FXJSE_Value_Create(hruntime);
              GetObjectDefaultValue(jsObjectValue, defaultPropValue);
              if (!FXJSE_Value_IsNull(defaultPropValue)) {
                dSum += HValueToDouble(hThis, defaultPropValue);
                uCount++;
              }
              FXJSE_Value_Release(defaultPropValue);
            }
          } else {
            CFX_ByteString propertyStr;
            FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
            FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
            for (int32_t j = 2; j < iLength; j++) {
              FXJSE_Value_GetObjectPropByIdx(argValue, j, jsObjectValue);
              FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                        newPropertyValue);
              if (!FXJSE_Value_IsNull(newPropertyValue)) {
                dSum += HValueToDouble(hThis, newPropertyValue);
                uCount++;
              }
            }
            FXJSE_Value_Release(newPropertyValue);
          }
          FXJSE_Value_Release(jsObjectValue);
          FXJSE_Value_Release(propertyValue);
        }
      } else {
        dSum += HValueToDouble(hThis, argValue);
        uCount++;
      }
      FXJSE_Value_Release(argValue);
    }
    argValue = 0;
  }
  if (0 == uCount) {
    FXJSE_Value_SetNull(args.GetReturnValue());
  } else {
    FXJSE_Value_SetDouble(args.GetReturnValue(), dSum / uCount);
  }
}
void CXFA_FM2JSContext::Ceil(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argValue = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argValue)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FXJSE_Value_SetFloat(args.GetReturnValue(),
                           FXSYS_ceil(HValueToFloat(hThis, argValue)));
    }
    FXJSE_Value_Release(argValue);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Ceil");
  }
}
void CXFA_FM2JSContext::Count(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint32_t uCount = 0;
  FXJSE_HVALUE argValue = 0;
  for (int32_t i = 0; i < argc; i++) {
    argValue = args.GetValue(i);
    if (FXJSE_Value_IsNull(argValue)) {
      FXJSE_Value_Release(argValue);
      continue;
    } else if (FXJSE_Value_IsArray(argValue)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argValue, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      if (iLength > 2) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argValue, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argValue, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            GetObjectDefaultValue(jsObjectValue, newPropertyValue);
            if (!FXJSE_Value_IsNull(newPropertyValue)) {
              uCount++;
            }
          }
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                      newPropertyValue);
            uCount += (FXJSE_Value_IsNull(newPropertyValue) ? 0 : 1);
          }
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
        FXJSE_Value_Release(newPropertyValue);
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      }
    } else if (FXJSE_Value_IsObject(argValue)) {
      FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
      GetObjectDefaultValue(argValue, newPropertyValue);
      if (!FXJSE_Value_IsNull(newPropertyValue)) {
        uCount++;
      }
      FXJSE_Value_Release(newPropertyValue);
    } else {
      uCount++;
    }
    FXJSE_Value_Release(argValue);
  }
  argValue = 0;
  FXJSE_Value_SetInteger(args.GetReturnValue(), (int32_t)uCount);
}
void CXFA_FM2JSContext::Floor(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argValue = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argValue)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FXJSE_Value_SetFloat(args.GetReturnValue(),
                           FXSYS_floor(HValueToFloat(hThis, argValue)));
    }
    FXJSE_Value_Release(argValue);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Floor");
  }
}
void CXFA_FM2JSContext::Max(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint32_t uCount = 0;
  FX_DOUBLE dMaxValue = 0.0;
  FXJSE_HVALUE argValue = 0;
  for (int32_t i = 0; i < argc; i++) {
    argValue = args.GetValue(i);
    if (FXJSE_Value_IsNull(argValue)) {
      FXJSE_Value_Release(argValue);
      continue;
    } else if (FXJSE_Value_IsArray(argValue)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argValue, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      if (iLength > 2) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argValue, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argValue, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            GetObjectDefaultValue(jsObjectValue, newPropertyValue);
            if (!FXJSE_Value_IsNull(newPropertyValue)) {
              uCount++;
              if (uCount == 1) {
                dMaxValue = HValueToDouble(hThis, newPropertyValue);
              } else {
                FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
                if (dMaxValue < dValue) {
                  dMaxValue = dValue;
                }
              }
            }
          }
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                      newPropertyValue);
            if (!FXJSE_Value_IsNull(newPropertyValue)) {
              uCount++;
              if (uCount == 1) {
                dMaxValue = HValueToDouble(hThis, newPropertyValue);
              } else {
                FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
                if (dMaxValue < dValue) {
                  dMaxValue = dValue;
                }
              }
            }
          }
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
        FXJSE_Value_Release(newPropertyValue);
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      }
    } else if (FXJSE_Value_IsObject(argValue)) {
      FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
      GetObjectDefaultValue(argValue, newPropertyValue);
      if (!FXJSE_Value_IsNull(newPropertyValue)) {
        uCount++;
        if (uCount == 1) {
          dMaxValue = HValueToDouble(hThis, newPropertyValue);
        } else {
          FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
          if (dMaxValue < dValue) {
            dMaxValue = dValue;
          }
        }
      }
      FXJSE_Value_Release(newPropertyValue);
    } else {
      uCount++;
      if (uCount == 1) {
        dMaxValue = HValueToDouble(hThis, argValue);
      } else {
        FX_DOUBLE dValue = HValueToDouble(hThis, argValue);
        if (dMaxValue < dValue) {
          dMaxValue = dValue;
        }
      }
    }
    FXJSE_Value_Release(argValue);
  }
  argValue = 0;
  if (uCount) {
    FXJSE_Value_SetDouble(args.GetReturnValue(), dMaxValue);
  } else {
    FXJSE_Value_SetNull(args.GetReturnValue());
  }
}
void CXFA_FM2JSContext::Min(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint32_t uCount = 0;
  FX_DOUBLE dMinValue = 0.0;
  FXJSE_HVALUE argValue = 0;
  for (int32_t i = 0; i < argc; i++) {
    argValue = args.GetValue(i);
    if (FXJSE_Value_IsNull(argValue)) {
      FXJSE_Value_Release(argValue);
      continue;
    } else if (FXJSE_Value_IsArray(argValue)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argValue, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      if (iLength > 2) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argValue, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argValue, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            GetObjectDefaultValue(jsObjectValue, newPropertyValue);
            if (!FXJSE_Value_IsNull(newPropertyValue)) {
              uCount++;
              if (uCount == 1) {
                dMinValue = HValueToDouble(hThis, newPropertyValue);
              } else {
                FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
                if (dMinValue > dValue) {
                  dMinValue = dValue;
                }
              }
            }
          }
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          for (int32_t i = 2; i < iLength; i++) {
            FXJSE_Value_GetObjectPropByIdx(argValue, i, jsObjectValue);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                      newPropertyValue);
            if (!FXJSE_Value_IsNull(newPropertyValue)) {
              uCount++;
              if (uCount == 1) {
                dMinValue = HValueToDouble(hThis, newPropertyValue);
              } else {
                FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
                if (dMinValue > dValue) {
                  dMinValue = dValue;
                }
              }
            }
          }
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
        FXJSE_Value_Release(newPropertyValue);
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      }
    } else if (FXJSE_Value_IsObject(argValue)) {
      FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
      GetObjectDefaultValue(argValue, newPropertyValue);
      if (!FXJSE_Value_IsNull(newPropertyValue)) {
        uCount++;
        if (uCount == 1) {
          dMinValue = HValueToDouble(hThis, newPropertyValue);
        } else {
          FX_DOUBLE dValue = HValueToDouble(hThis, newPropertyValue);
          if (dMinValue > dValue) {
            dMinValue = dValue;
          }
        }
      }
      FXJSE_Value_Release(newPropertyValue);
    } else {
      uCount++;
      if (uCount == 1) {
        dMinValue = HValueToDouble(hThis, argValue);
      } else {
        FX_DOUBLE dValue = HValueToDouble(hThis, argValue);
        if (dMinValue > dValue) {
          dMinValue = dValue;
        }
      }
    }
    FXJSE_Value_Release(argValue);
  }
  argValue = 0;
  if (uCount) {
    FXJSE_Value_SetDouble(args.GetReturnValue(), dMinValue);
  } else {
    FXJSE_Value_SetNull(args.GetReturnValue());
  }
}
void CXFA_FM2JSContext::Mod(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FXJSE_HVALUE argTwo = args.GetValue(1);
    if (FXJSE_Value_IsNull(argOne) || FXJSE_Value_IsNull(argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE dDividend = 0.0;
      FX_DOUBLE dDividor = 0.0;
      if (FXJSE_Value_IsArray(argOne)) {
        FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectProp(argOne, "length", lengthValue);
        int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
        FXJSE_Value_Release(lengthValue);
        if (iLength > 2) {
          FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectPropByIdx(argOne, 1, propertyValue);
          FXJSE_Value_GetObjectPropByIdx(argOne, 2, jsObjectValue);
          if (FXJSE_Value_IsNull(propertyValue)) {
            dDividend = HValueToDouble(hThis, jsObjectValue);
          } else {
            CFX_ByteString propertyStr;
            FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
            FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                      newPropertyValue);
            dDividend = HValueToDouble(hThis, newPropertyValue);
            FXJSE_Value_Release(newPropertyValue);
          }
          FXJSE_Value_Release(propertyValue);
          FXJSE_Value_Release(jsObjectValue);
        } else {
          pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
        }
      } else {
        dDividend = HValueToDouble(hThis, argOne);
      }
      if (FXJSE_Value_IsArray(argTwo)) {
        FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectProp(argTwo, "length", lengthValue);
        int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
        FXJSE_Value_Release(lengthValue);
        if (iLength > 2) {
          FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectPropByIdx(argTwo, 1, propertyValue);
          FXJSE_Value_GetObjectPropByIdx(argTwo, 2, jsObjectValue);
          if (FXJSE_Value_IsNull(propertyValue)) {
            dDividor = HValueToDouble(hThis, jsObjectValue);
          } else {
            CFX_ByteString propertyStr;
            FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
            FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                      newPropertyValue);
            dDividor = HValueToDouble(hThis, newPropertyValue);
            FXJSE_Value_Release(newPropertyValue);
          }
          FXJSE_Value_Release(propertyValue);
          FXJSE_Value_Release(jsObjectValue);
        } else {
          pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
        }
      } else {
        dDividor = HValueToDouble(hThis, argTwo);
      }
      if (dDividor) {
        FXJSE_Value_SetDouble(
            args.GetReturnValue(),
            dDividend - dDividor * (int32_t)(dDividend / dDividor));
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_DIVIDE_ZERO);
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Mod");
  }
}
void CXFA_FM2JSContext::Round(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint8_t uPrecision = 0;
  if (argc == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE dValue = 0.0;
      if (FXJSE_Value_IsArray(argOne)) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argOne, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argOne, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          dValue = HValueToDouble(hThis, jsObjectValue);
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                    newPropertyValue);
          dValue = HValueToDouble(hThis, newPropertyValue);
          FXJSE_Value_Release(newPropertyValue);
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
      } else {
        dValue = HValueToDouble(hThis, argOne);
      }
      CFX_Decimal decimalValue((FX_FLOAT)dValue, uPrecision);
      CFX_WideString wsValue = decimalValue;
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), wsValue.UTF8Encode());
    }
    FXJSE_Value_Release(argOne);
  } else if (argc == 2) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FXJSE_HVALUE argTwo = args.GetValue(1);
    if (FXJSE_Value_IsNull(argOne) || FXJSE_Value_IsNull(argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE dValue = 0.0;
      if (FXJSE_Value_IsArray(argOne)) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argOne, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argOne, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          dValue = HValueToDouble(hThis, jsObjectValue);
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                    newPropertyValue);
          dValue = HValueToDouble(hThis, newPropertyValue);
          FXJSE_Value_Release(newPropertyValue);
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
      } else {
        dValue = HValueToDouble(hThis, argOne);
      }
      FX_DOUBLE dPrecision = 0.0;
      if (FXJSE_Value_IsArray(argTwo)) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argTwo, 1, propertyValue);
        FXJSE_Value_GetObjectPropByIdx(argTwo, 2, jsObjectValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          dPrecision = HValueToDouble(hThis, jsObjectValue);
        } else {
          CFX_ByteString propertyStr;
          FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
          FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                    newPropertyValue);
          dPrecision = HValueToDouble(hThis, newPropertyValue);
          FXJSE_Value_Release(newPropertyValue);
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
      } else {
        dPrecision = HValueToDouble(hThis, argTwo);
      }
      if (dPrecision < 0) {
        uPrecision = 0;
      } else if (dPrecision > 12.0) {
        uPrecision = 12;
      } else {
        uPrecision = (uint8_t)dPrecision;
      }
      CFX_Decimal decimalValue((FX_FLOAT)dValue, uPrecision);
      CFX_WideString wsValue = decimalValue;
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), wsValue.UTF8Encode());
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Round");
  }
}
void CXFA_FM2JSContext::Sum(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  uint32_t uCount = 0;
  FX_DOUBLE dSum = 0.0;
  if (argc) {
    FXJSE_HVALUE argValue = 0;
    for (int32_t i = 0; i < argc; i++) {
      argValue = args.GetValue(i);
      if (FXJSE_Value_IsNull(argValue)) {
        FXJSE_Value_Release(argValue);
        continue;
      } else if (FXJSE_Value_IsArray(argValue)) {
        FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectProp(argValue, "length", lengthValue);
        int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
        FXJSE_Value_Release(lengthValue);
        if (iLength > 2) {
          FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectPropByIdx(argValue, 1, propertyValue);
          FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
          FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
          if (FXJSE_Value_IsNull(propertyValue)) {
            for (int32_t j = 2; j < iLength; j++) {
              FXJSE_Value_GetObjectPropByIdx(argValue, j, jsObjectValue);
              GetObjectDefaultValue(jsObjectValue, newPropertyValue);
              if (!FXJSE_Value_IsNull(newPropertyValue)) {
                dSum += HValueToDouble(hThis, jsObjectValue);
                uCount++;
              }
            }
          } else {
            CFX_ByteString propertyStr;
            FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
            for (int32_t j = 2; j < iLength; j++) {
              FXJSE_Value_GetObjectPropByIdx(argValue, j, jsObjectValue);
              FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr,
                                        newPropertyValue);
              if (!FXJSE_Value_IsNull(newPropertyValue)) {
                dSum += HValueToDouble(hThis, newPropertyValue);
                uCount++;
              }
            }
          }
          FXJSE_Value_Release(newPropertyValue);
          FXJSE_Value_Release(jsObjectValue);
          FXJSE_Value_Release(propertyValue);
        } else {
          pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
        }
      } else if (FXJSE_Value_IsObject(argValue)) {
        FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
        GetObjectDefaultValue(argValue, newPropertyValue);
        if (!FXJSE_Value_IsNull(newPropertyValue)) {
          dSum += HValueToDouble(hThis, argValue);
          uCount++;
        }
        FXJSE_Value_Release(newPropertyValue);
      } else {
        dSum += HValueToDouble(hThis, argValue);
        uCount++;
      }
      FXJSE_Value_Release(argValue);
    }
    argValue = 0;
  }
  if (uCount < 1) {
    FXJSE_Value_SetNull(args.GetReturnValue());
  } else {
    FXJSE_Value_SetDouble(args.GetReturnValue(), dSum);
  }
}
void CXFA_FM2JSContext::Date(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  if (args.GetLength() == 0) {
    struct tm* pTmStruct = 0;
    time_t currentTime;
    time(&currentTime);
    pTmStruct = gmtime(&currentTime);
    CFX_ByteString bufferYear;
    CFX_ByteString bufferMon;
    CFX_ByteString bufferDay;
    bufferYear.Format("%d", pTmStruct->tm_year + 1900);
    bufferMon.Format("%02d", pTmStruct->tm_mon + 1);
    bufferDay.Format("%02d", pTmStruct->tm_mday);
    CFX_ByteString bufferCurrent = bufferYear + bufferMon + bufferDay;
    int32_t dDays = DateString2Num(bufferCurrent);
    FXJSE_Value_SetInteger(args.GetReturnValue(), dDays);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Date");
  }
}
void CXFA_FM2JSContext::Date2Num(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    CFX_ByteString dateString;
    CFX_ByteString formatString;
    CFX_ByteString localString;
    FXJSE_HVALUE dateValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE formatValue = 0;
    FXJSE_HVALUE localValue = 0;
    if (HValueIsNull(hThis, dateValue)) {
      bFlags = TRUE;
    } else {
      HValueToUTF8String(dateValue, dateString);
    }
    if (argc > 1) {
      formatValue = GetSimpleHValue(hThis, args, 1);
      if (HValueIsNull(hThis, formatValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(formatValue, formatString);
      }
    }
    if (argc == 3) {
      localValue = GetSimpleHValue(hThis, args, 2);
      if (HValueIsNull(hThis, localValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localValue, localString);
      }
    }
    if (!bFlags) {
      CFX_ByteString szIsoDateString;
      FX_BOOL bRet = Local2IsoDate(hThis, dateString, formatString, localString,
                                   szIsoDateString);
      if (bRet) {
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               DateString2Num(szIsoDateString));
      } else {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
      }
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(dateValue);
    if (argc > 1) {
      FXJSE_Value_Release(formatValue);
      if (argc == 3) {
        FXJSE_Value_Release(localValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Date2Num");
  }
}
void CXFA_FM2JSContext::DateFmt(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc < 3) {
    FX_BOOL bFlags = FALSE;
    int32_t iStyle = 0;
    CFX_ByteString szLocal;
    FXJSE_HVALUE argStyle = 0;
    FXJSE_HVALUE argLocal = 0;
    if (argc > 0) {
      argStyle = GetSimpleHValue(hThis, args, 0);
      if (FXJSE_Value_IsNull(argStyle)) {
        bFlags = TRUE;
      }
      iStyle = (int32_t)HValueToFloat(hThis, argStyle);
      if (iStyle > 4 || iStyle < 0) {
        iStyle = 0;
      }
    }
    if (argc == 2) {
      argLocal = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argLocal)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(argLocal, szLocal);
      }
    }
    if (!bFlags) {
      CFX_ByteString formatStr;
      GetStandardDateFormat(hThis, iStyle, szLocal, formatStr);
      if (formatStr.IsEmpty()) {
        formatStr = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), formatStr);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    if (argc > 0) {
      FXJSE_Value_Release(argStyle);
      if (argc == 2) {
        FXJSE_Value_Release(argLocal);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Date2Num");
  }
}
void CXFA_FM2JSContext::IsoDate2Num(FXJSE_HOBJECT hThis,
                                    const CFX_ByteStringC& szFuncName,
                                    CFXJSE_Arguments& args) {
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString szArgString;
      HValueToUTF8String(argOne, szArgString);
      int32_t dDays = DateString2Num(szArgString);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (int32_t)dDays);
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"IsoDate2Num");
  }
}
void CXFA_FM2JSContext::IsoTime2Num(FXJSE_HOBJECT hThis,
                                    const CFX_ByteStringC& szFuncName,
                                    CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CXFA_Document* pDoc = pContext->GetDocument();
      FXSYS_assert(pDoc);
      IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
      CFX_ByteString szArgString;
      HValueToUTF8String(argOne, szArgString);
      szArgString = szArgString.Mid(szArgString.Find('T', 0) + 1);
      if (szArgString.IsEmpty()) {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
        FXJSE_Value_Release(argOne);
        return;
      }
      CXFA_LocaleValue timeValue(
          XFA_VT_TIME,
          CFX_WideString::FromUTF8(szArgString, szArgString.GetLength()),
          (CXFA_LocaleMgr*)pMgr);
      if (timeValue.IsValid()) {
        CFX_Unitime uniTime = timeValue.GetTime();
        int32_t hour = uniTime.GetHour();
        int32_t min = uniTime.GetMinute();
        int32_t second = uniTime.GetSecond();
        int32_t milSecond = uniTime.GetMillisecond();
        IFX_Locale* pDefLocale = pMgr->GetDefLocale();
        FXSYS_assert(pDefLocale);
        FX_TIMEZONE tzLocale;
        pDefLocale->GetTimeZone(tzLocale);
        int32_t mins = hour * 60 + min;
        mins -= (tzLocale.tzHour * 60);
        while (mins > 1440) {
          mins -= 1440;
        }
        while (mins < 0) {
          mins += 1440;
        }
        hour = mins / 60;
        min = mins % 60;
        int32_t iResult =
            hour * 3600000 + min * 60000 + second * 1000 + milSecond + 1;
        FXJSE_Value_SetInteger(args.GetReturnValue(), iResult);
      } else {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
      }
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"IsoTime2Num");
  }
}
void CXFA_FM2JSContext::LocalDateFmt(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc < 3) {
    FX_BOOL bFlags = FALSE;
    int32_t iStyle = 0;
    CFX_ByteString szLocal;
    FXJSE_HVALUE argStyle = 0;
    FXJSE_HVALUE argLocal = 0;
    if (argc > 0) {
      argStyle = GetSimpleHValue(hThis, args, 0);
      if (FXJSE_Value_IsNull(argStyle)) {
        bFlags = TRUE;
      }
      iStyle = (int32_t)HValueToFloat(hThis, argStyle);
      if (iStyle > 4 || iStyle < 0) {
        iStyle = 0;
      }
    }
    if (argc == 2) {
      argLocal = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argLocal)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(argLocal, szLocal);
      }
    }
    if (!bFlags) {
      CFX_ByteString formatStr;
      GetLocalDateFormat(hThis, iStyle, szLocal, formatStr, FALSE);
      if (formatStr.IsEmpty()) {
        formatStr = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), formatStr);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    if (argc > 0) {
      FXJSE_Value_Release(argStyle);
      if (argc == 2) {
        FXJSE_Value_Release(argLocal);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"LocalDateFmt");
  }
}
void CXFA_FM2JSContext::LocalTimeFmt(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc < 3) {
    FX_BOOL bFlags = FALSE;
    int32_t iStyle = 0;
    CFX_ByteString szLocal;
    FXJSE_HVALUE argStyle = 0;
    FXJSE_HVALUE argLocal = 0;
    if (argc > 0) {
      argStyle = GetSimpleHValue(hThis, args, 0);
      if (FXJSE_Value_IsNull(argStyle)) {
        bFlags = TRUE;
      }
      iStyle = (int32_t)HValueToFloat(hThis, argStyle);
      if (iStyle > 4 || iStyle < 0) {
        iStyle = 0;
      }
    }
    if (argc == 2) {
      argLocal = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argLocal)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(argLocal, szLocal);
      }
    }
    if (!bFlags) {
      CFX_ByteString formatStr;
      GetLocalTimeFormat(hThis, iStyle, szLocal, formatStr, FALSE);
      if (formatStr.IsEmpty()) {
        formatStr = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), formatStr);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    if (argc > 0) {
      FXJSE_Value_Release(argStyle);
      if (argc == 2) {
        FXJSE_Value_Release(argLocal);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"LocalTimeFmt");
  }
}
void CXFA_FM2JSContext::Num2Date(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    int32_t dDate;
    CFX_ByteString formatString;
    CFX_ByteString localString;
    FXJSE_HVALUE dateValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE formatValue = 0;
    FXJSE_HVALUE localValue = 0;
    if (HValueIsNull(hThis, dateValue)) {
      bFlags = TRUE;
    } else {
      dDate = (int32_t)HValueToFloat(hThis, dateValue);
      bFlags = dDate < 1;
    }
    if (argc > 1) {
      formatValue = GetSimpleHValue(hThis, args, 1);
      if (HValueIsNull(hThis, formatValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(formatValue, formatString);
      }
    }
    if (argc == 3) {
      localValue = GetSimpleHValue(hThis, args, 2);
      if (HValueIsNull(hThis, localValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localValue, localString);
      }
    }
    if (!bFlags) {
      int32_t iYear = 1900;
      int32_t iMonth = 1;
      int32_t iDay = 1;
      int32_t i = 0;
      while (dDate > 0) {
        if (iMonth == 2) {
          if ((!((iYear + i) % 4) && ((iYear + i) % 100)) ||
              !((iYear + i) % 400)) {
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
      CFX_ByteString szIsoDateString;
      szIsoDateString.Format("%d%02d%02d", iYear + i, iMonth, iDay);
      CFX_ByteString szLocalDateString;
      IsoDate2Local(hThis, szIsoDateString, formatString,
                    localString, szLocalDateString);
      if (szLocalDateString.IsEmpty()) {
        szLocalDateString = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), szLocalDateString);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(dateValue);
    if (argc > 1) {
      FXJSE_Value_Release(formatValue);
      if (argc == 3) {
        FXJSE_Value_Release(localValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Num2Date");
  }
}
void CXFA_FM2JSContext::Num2GMTime(FXJSE_HOBJECT hThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    int32_t iTime;
    CFX_ByteString formatString;
    CFX_ByteString localString;
    FXJSE_HVALUE timeValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE formatValue = 0;
    FXJSE_HVALUE localValue = 0;
    if (FXJSE_Value_IsNull(timeValue)) {
      bFlags = TRUE;
    } else {
      iTime = (int32_t)HValueToFloat(hThis, timeValue);
      if (FXSYS_abs(iTime) < 1.0) {
        bFlags = TRUE;
      }
    }
    if (argc > 1) {
      formatValue = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(formatValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(formatValue, formatString);
      }
    }
    if (argc == 3) {
      localValue = GetSimpleHValue(hThis, args, 2);
      if (FXJSE_Value_IsNull(localValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localValue, localString);
      }
    }
    if (!bFlags) {
      CFX_ByteString szGMTTimeString;
      Num2AllTime(hThis, iTime, formatString, localString, TRUE,
                  szGMTTimeString);
      if (szGMTTimeString.IsEmpty()) {
        szGMTTimeString = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), szGMTTimeString);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(timeValue);
    if (argc > 1) {
      FXJSE_Value_Release(formatValue);
      if (argc == 3) {
        FXJSE_Value_Release(localValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Num2GMTime");
  }
}
void CXFA_FM2JSContext::Num2Time(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT fTime;
    CFX_ByteString formatString;
    CFX_ByteString localString;
    FXJSE_HVALUE timeValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE formatValue = 0;
    FXJSE_HVALUE localValue = 0;
    if (FXJSE_Value_IsNull(timeValue)) {
      bFlags = TRUE;
    } else {
      fTime = HValueToFloat(hThis, timeValue);
      if (FXSYS_fabs(fTime) < 1.0) {
        bFlags = TRUE;
      }
    }
    if (argc > 1) {
      formatValue = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(formatValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(formatValue, formatString);
      }
    }
    if (argc == 3) {
      localValue = GetSimpleHValue(hThis, args, 2);
      if (FXJSE_Value_IsNull(localValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localValue, localString);
      }
    }
    if (!bFlags) {
      CFX_ByteString szLocalTimeString;
      Num2AllTime(hThis, (int32_t)fTime, formatString, localString, FALSE,
                  szLocalTimeString);
      if (szLocalTimeString.IsEmpty()) {
        szLocalTimeString = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), szLocalTimeString);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(timeValue);
    if (argc > 1) {
      FXJSE_Value_Release(formatValue);
      if (argc == 3) {
        FXJSE_Value_Release(localValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Num2Time");
  }
}
void CXFA_FM2JSContext::Time(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  if (args.GetLength() == 0) {
    time_t now;
    time(&now);
    struct tm* pGmt = gmtime(&now);
    int32_t iGMHour = pGmt->tm_hour;
    int32_t iGMMin = pGmt->tm_min;
    int32_t iGMSec = pGmt->tm_sec;
    FXJSE_Value_SetInteger(args.GetReturnValue(),
                           ((iGMHour * 3600 + iGMMin * 60 + iGMSec) * 1000));
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Time");
  }
}
void CXFA_FM2JSContext::Time2Num(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    CFX_ByteString timeString;
    CFX_ByteString formatString;
    CFX_ByteString localString;
    FXJSE_HVALUE timeValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE formatValue = 0;
    FXJSE_HVALUE localValue = 0;
    if (HValueIsNull(hThis, timeValue)) {
      bFlags = TRUE;
    } else {
      HValueToUTF8String(timeValue, timeString);
    }
    if (argc > 1) {
      formatValue = GetSimpleHValue(hThis, args, 1);
      if (HValueIsNull(hThis, formatValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(formatValue, formatString);
      }
    }
    if (argc == 3) {
      localValue = GetSimpleHValue(hThis, args, 2);
      if (HValueIsNull(hThis, localValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localValue, localString);
      }
    }
    if (!bFlags) {
      CXFA_FM2JSContext* pContext =
          (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
      CXFA_Document* pDoc = pContext->GetDocument();
      IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
      IFX_Locale* pLocale = NULL;
      if (localString.IsEmpty()) {
        CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
        FXSYS_assert(pThisNode->IsNode());
        CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
        pLocale = widgetData.GetLocal();
      } else {
        pLocale = pMgr->GetLocaleByName(
            CFX_WideString::FromUTF8(localString, localString.GetLength()));
      }
      CFX_WideString wsFormat;
      if (formatString.IsEmpty()) {
        pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
      } else {
        wsFormat =
            CFX_WideString::FromUTF8(formatString, formatString.GetLength());
      }
      wsFormat = FX_WSTRC(L"time{") + wsFormat;
      wsFormat += FX_WSTRC(L"}");
      CXFA_LocaleValue timeValue(
          XFA_VT_TIME,
          CFX_WideString::FromUTF8(timeString, timeString.GetLength()),
          wsFormat, pLocale, (CXFA_LocaleMgr*)pMgr);
      if (timeValue.IsValid()) {
        CFX_Unitime uniTime = timeValue.GetTime();
        int32_t hour = uniTime.GetHour();
        int32_t min = uniTime.GetMinute();
        int32_t second = uniTime.GetSecond();
        int32_t milSecond = uniTime.GetMillisecond();
        int32_t mins = hour * 60 + min;
        IXFA_TimeZoneProvider* pProvider = IXFA_TimeZoneProvider::Get();
        if (pProvider != NULL) {
          FX_TIMEZONE tz;
          pProvider->GetTimeZone(tz);
          mins -= (tz.tzHour * 60);
          while (mins > 1440) {
            mins -= 1440;
          }
          while (mins < 0) {
            mins += 1440;
          }
          hour = mins / 60;
          min = mins % 60;
        }
        int32_t iResult =
            hour * 3600000 + min * 60000 + second * 1000 + milSecond + 1;
        FXJSE_Value_SetInteger(args.GetReturnValue(), iResult);
      } else {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
      }
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(timeValue);
    if (argc > 1) {
      FXJSE_Value_Release(formatValue);
      if (argc == 3) {
        FXJSE_Value_Release(localValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Time2Num");
  }
}
void CXFA_FM2JSContext::TimeFmt(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc < 3) {
    FX_BOOL bFlags = FALSE;
    int32_t iStyle = 0;
    CFX_ByteString szLocal;
    FXJSE_HVALUE argStyle = 0;
    FXJSE_HVALUE argLocal = 0;
    if (argc > 0) {
      argStyle = GetSimpleHValue(hThis, args, 0);
      if (FXJSE_Value_IsNull(argStyle)) {
        bFlags = TRUE;
      }
      iStyle = (int32_t)HValueToFloat(hThis, argStyle);
      if (iStyle > 4 || iStyle < 0) {
        iStyle = 0;
      }
    }
    if (argc == 2) {
      argLocal = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argLocal)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(argLocal, szLocal);
      }
    }
    if (!bFlags) {
      CFX_ByteString formatStr;
      GetStandardTimeFormat(hThis, iStyle, szLocal, formatStr);
      if (formatStr.IsEmpty()) {
        formatStr = "";
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), formatStr);
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    if (argc > 0) {
      FXJSE_Value_Release(argStyle);
      if (argc == 2) {
        FXJSE_Value_Release(argLocal);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"TimeFmt");
  }
}
FX_BOOL CXFA_FM2JSContext::IsIsoDateFormat(const FX_CHAR* pData,
                                           int32_t iLength,
                                           int32_t& iStyle,
                                           int32_t& iYear,
                                           int32_t& iMonth,
                                           int32_t& iDay) {
  iYear = 0;
  iMonth = 1;
  iDay = 1;
  FX_BOOL iRet = FALSE;
  if (iLength < 4) {
    return iRet;
  }
  FX_CHAR strYear[5];
  strYear[4] = '\0';
  for (int32_t i = 0; i < 4; ++i) {
    if (*(pData + i) <= '9' && *(pData + i) >= '0') {
      strYear[i] = *(pData + i);
    } else {
      return iRet;
    }
  }
  iYear = FXSYS_atoi(strYear);
  iStyle = 0;
  if (iLength > 4) {
    if (*(pData + 4) == '-') {
      iStyle = 1;
    } else {
      iStyle = 0;
    }
  } else {
    iRet = TRUE;
    return iRet;
  }
  FX_CHAR strTemp[3];
  strTemp[2] = '\0';
  int32_t iPosOff = 0;
  if (iStyle == 0) {
    iPosOff = 4;
    if (iLength == 4) {
      iRet = TRUE;
      return iRet;
    }
  } else {
    iPosOff = 5;
    if (iLength == 4) {
      iRet = TRUE;
      return iRet;
    }
  }
  if ((*(pData + iPosOff) > '9' || *(pData + iPosOff) < '0') ||
      (*(pData + iPosOff + 1) > '9' || *(pData + iPosOff + 1) < '0')) {
    return iRet;
  }
  strTemp[0] = *(pData + iPosOff);
  strTemp[1] = *(pData + iPosOff + 1);
  iMonth = FXSYS_atoi(strTemp);
  if (iMonth > 12 || iMonth < 1) {
    return iRet;
  }
  if (iStyle == 0) {
    iPosOff += 2;
    if (iLength == 6) {
      iRet = 1;
      return iRet;
    }
  } else {
    iPosOff += 3;
    if (iLength == 7) {
      iRet = 1;
      return iRet;
    }
  }
  if ((*(pData + iPosOff) > '9' || *(pData + iPosOff) < '0') ||
      (*(pData + iPosOff + 1) > '9' || *(pData + iPosOff + 1) < '0')) {
    return iRet;
  }
  strTemp[0] = *(pData + iPosOff);
  strTemp[1] = *(pData + iPosOff + 1);
  iDay = FXSYS_atoi(strTemp);
  if (iPosOff + 2 < iLength) {
    return iRet;
  }
  if ((!(iYear % 4) && (iYear % 100)) || !(iYear % 400)) {
    if (iMonth == 2) {
      if (iDay > 29) {
        return iRet;
      }
    } else {
      if (iMonth < 8) {
        if (iDay > (iMonth % 2 == 0 ? 30 : 31)) {
          return iRet;
        }
      } else {
        if (iDay > (iMonth % 2 == 0 ? 31 : 30)) {
          return iRet;
        }
      }
    }
  } else {
    if (iMonth == 2) {
      if (iDay > 28) {
        return iRet;
      }
    } else {
      if (iMonth < 8) {
        if (iDay > (iMonth % 2 == 0 ? 30 : 31)) {
          return iRet;
        }
      } else {
        if (iDay > (iMonth % 2 == 0 ? 31 : 30)) {
          return iRet;
        }
      }
    }
  }
  iRet = TRUE;
  return iRet;
}
FX_BOOL CXFA_FM2JSContext::IsIsoTimeFormat(const FX_CHAR* pData,
                                           int32_t iLength,
                                           int32_t& iHour,
                                           int32_t& iMinute,
                                           int32_t& iSecond,
                                           int32_t& iMilliSecond,
                                           int32_t& iZoneHour,
                                           int32_t& iZoneMinute) {
  iHour = 0;
  iMinute = 0;
  iSecond = 0;
  iMilliSecond = 0;
  iZoneHour = 0;
  iZoneMinute = 0;
  if (!pData) {
    return FALSE;
  }
  int32_t iRet = FALSE;
  FX_CHAR strTemp[3];
  strTemp[2] = '\0';
  int32_t iIndex = 0;
  int32_t iZone = 0;
  int32_t i = iIndex;
  while (i < iLength) {
    if ((*(pData + i) > '9' || *(pData + i) < '0') && *(pData + i) != ':') {
      iZone = i;
      break;
    }
    ++i;
  }
  if (i == iLength) {
    iZone = iLength;
  }
  int32_t iPos = 0;
  while (iIndex < iZone) {
    if (iIndex >= iZone) {
      break;
    }
    if (*(pData + iIndex) > '9' || *(pData + iIndex) < '0') {
      return iRet;
    }
    strTemp[0] = *(pData + iIndex);
    if (*(pData + iIndex + 1) > '9' || *(pData + iIndex + 1) < '0') {
      return iRet;
    }
    strTemp[1] = *(pData + iIndex + 1);
    if (FXSYS_atoi(strTemp) > 60) {
      return iRet;
    }
    if (*(pData + 2) == ':') {
      if (iPos == 0) {
        iHour = FXSYS_atoi(strTemp);
        ++iPos;
      } else if (iPos == 1) {
        iMinute = FXSYS_atoi(strTemp);
        ++iPos;
      } else {
        iSecond = FXSYS_atoi(strTemp);
      }
      iIndex += 3;
    } else {
      if (iPos == 0) {
        iHour = FXSYS_atoi(strTemp);
        ++iPos;
      } else if (iPos == 1) {
        iMinute = FXSYS_atoi(strTemp);
        ++iPos;
      } else if (iPos == 2) {
        iSecond = FXSYS_atoi(strTemp);
        ++iPos;
      }
      iIndex += 2;
    }
  }
  if (*(pData + iIndex) == '.') {
    ++iIndex;
    FX_CHAR strTemp[4];
    strTemp[3] = '\0';
    if (*(pData + iIndex) > '9' || *(pData + iIndex) < '0') {
      return iRet;
    }
    strTemp[0] = *(pData + iIndex);
    if (*(pData + iIndex + 1) > '9' || *(pData + iIndex + 1) < '0') {
      return iRet;
    }
    strTemp[1] = *(pData + iIndex + 1);
    if (*(pData + iIndex + 2) > '9' || *(pData + iIndex + 2) < '0') {
      return iRet;
    }
    strTemp[2] = *(pData + iIndex + 2);
    iMilliSecond = FXSYS_atoi(strTemp);
    if (iMilliSecond > 100) {
      iMilliSecond = 0;
      return iRet;
    }
    iIndex += 3;
  }
  int32_t iSign = 1;
  if (*(pData + iIndex) == 'z' || *(pData + iIndex) == 'Z') {
    iRet = 1;
    return iRet;
  } else if (*(pData + iIndex) == '+') {
    ++iIndex;
  } else if (*(pData + iIndex) == '-') {
    iSign = -1;
    ++iIndex;
  }
  iPos = 0;
  while (iIndex < iLength) {
    if (iIndex >= iLength) {
      return iRet;
    }
    if (*(pData + iIndex) > '9' || *(pData + iIndex) < '0') {
      return iRet;
    }
    strTemp[0] = *(pData + iIndex);
    if (*(pData + iIndex + 1) > '9' || *(pData + iIndex + 1) < '0') {
      return iRet;
    }
    strTemp[1] = *(pData + iIndex + 1);
    if (FXSYS_atoi(strTemp) > 60) {
      return iRet;
    }
    if (*(pData + 2) == ':') {
      if (iPos == 0) {
        iZoneHour = FXSYS_atoi(strTemp);
      } else if (iPos == 1) {
        iZoneMinute = FXSYS_atoi(strTemp);
      }
      iIndex += 3;
    } else {
      if (!iPos) {
        iZoneHour = FXSYS_atoi(strTemp);
        ++iPos;
      } else if (iPos == 1) {
        iZoneMinute = FXSYS_atoi(strTemp);
        ++iPos;
      }
      iIndex += 2;
    }
  }
  if (iIndex < iLength) {
    return iRet;
  }
  iZoneHour *= iSign;
  iRet = TRUE;
  return iRet;
}
FX_BOOL CXFA_FM2JSContext::IsIsoDateTimeFormat(const FX_CHAR* pData,
                                               int32_t iLength,
                                               int32_t& iYear,
                                               int32_t& iMonth,
                                               int32_t& iDay,
                                               int32_t& iHour,
                                               int32_t& iMinute,
                                               int32_t& iSecond,
                                               int32_t& iMillionSecond,
                                               int32_t& iZoneHour,
                                               int32_t& iZoneMinute) {
  iYear = 0;
  iMonth = 0;
  iDay = 0;
  iHour = 0;
  iMinute = 0;
  iSecond = 0;
  if (!pData) {
    return FALSE;
  }
  int32_t iRet = FALSE;
  int32_t iIndex = 0;
  while (*(pData + iIndex) != 'T' && *(pData + iIndex) != 't') {
    if (iIndex >= iLength) {
      return iRet;
    }
    ++iIndex;
  }
  if (iIndex != 8 && iIndex != 10) {
    return iRet;
  }
  int32_t iStyle = -1;
  iRet = IsIsoDateFormat(pData, iIndex, iStyle, iYear, iMonth, iDay);
  if (!iRet) {
    return iRet;
  }
  if (*(pData + iIndex) != 'T' && *(pData + iIndex) != 't') {
    return iRet;
  }
  ++iIndex;
  if (((iLength - iIndex > 13) && (iLength - iIndex < 6)) &&
      (iLength - iIndex != 15)) {
    return iRet;
  }
  iRet = IsIsoTimeFormat(pData + iIndex, iLength - iIndex, iHour, iMinute,
                         iSecond, iMillionSecond, iZoneHour, iZoneMinute);
  if (!iRet) {
    return iRet;
  }
  iRet = TRUE;
  return iRet;
}
FX_BOOL CXFA_FM2JSContext::Local2IsoDate(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szDate,
                                         const CFX_ByteStringC& szFormat,
                                         const CFX_ByteStringC& szLocale,
                                         CFX_ByteString& strIsoDate) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return FALSE;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocale.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocale.GetCStr(), szLocale.GetLength()));
  }
  if (!pLocale) {
    return FALSE;
  }
  CFX_WideString wsFormat;
  if (szFormat.IsEmpty()) {
    pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
  } else {
    wsFormat =
        CFX_WideString::FromUTF8(szFormat.GetCStr(), szFormat.GetLength());
  }
  CXFA_LocaleValue widgetValue(
      XFA_VT_DATE,
      CFX_WideString::FromUTF8(szDate.GetCStr(), szDate.GetLength()), wsFormat,
      pLocale, (CXFA_LocaleMgr*)pMgr);
  CFX_Unitime dt = widgetValue.GetDate();
  strIsoDate.Format("%4d-%02d-%02d", dt.GetYear(), dt.GetMonth(), dt.GetDay());
  return TRUE;
}
FX_BOOL CXFA_FM2JSContext::Local2IsoTime(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szTime,
                                         const CFX_ByteStringC& szFormat,
                                         const CFX_ByteStringC& szLocale,
                                         CFX_ByteString& strIsoTime) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return FALSE;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocale.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocale.GetCStr(), szLocale.GetLength()));
  }
  if (!pLocale) {
    return FALSE;
  }
  CFX_WideString wsFormat;
  if (szFormat.IsEmpty()) {
    pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
  } else {
    wsFormat =
        CFX_WideString::FromUTF8(szFormat.GetCStr(), szFormat.GetLength());
  }
  wsFormat = FX_WSTRC(L"time{") + wsFormat;
  wsFormat += FX_WSTRC(L"}");
  CXFA_LocaleValue widgetValue(
      XFA_VT_TIME,
      CFX_WideString::FromUTF8(szTime.GetCStr(), szTime.GetLength()), wsFormat,
      pLocale, (CXFA_LocaleMgr*)pMgr);
  CFX_Unitime utime = widgetValue.GetTime();
  strIsoTime.Format("%02d:%02d:%02d.%03d", utime.GetHour(), utime.GetMinute(),
                    utime.GetSecond(), utime.GetMillisecond());
  return TRUE;
}
FX_BOOL CXFA_FM2JSContext::IsoDate2Local(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szDate,
                                         const CFX_ByteStringC& szFormat,
                                         const CFX_ByteStringC& szLocale,
                                         CFX_ByteString& strLocalDate) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return FALSE;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocale.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocale.GetCStr(), szLocale.GetLength()));
  }
  if (!pLocale) {
    return FALSE;
  }
  CFX_WideString wsFormat;
  if (szFormat.IsEmpty()) {
    pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
  } else {
    wsFormat =
        CFX_WideString::FromUTF8(szFormat.GetCStr(), szFormat.GetLength());
  }
  CXFA_LocaleValue widgetValue(
      XFA_VT_DATE,
      CFX_WideString::FromUTF8(szDate.GetCStr(), szDate.GetLength()),
      (CXFA_LocaleMgr*)pMgr);
  CFX_WideString wsRet;
  widgetValue.FormatPatterns(wsRet, wsFormat, pLocale,
                             XFA_VALUEPICTURE_Display);
  strLocalDate = FX_UTF8Encode(wsRet, wsRet.GetLength());
  return TRUE;
}
FX_BOOL CXFA_FM2JSContext::IsoTime2Local(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szTime,
                                         const CFX_ByteStringC& szFormat,
                                         const CFX_ByteStringC& szLocale,
                                         CFX_ByteString& strLocalTime) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return FALSE;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocale.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocale.GetCStr(), szLocale.GetLength()));
  }
  if (!pLocale) {
    return FALSE;
  }
  CFX_WideString wsFormat;
  if (szFormat.IsEmpty()) {
    pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
  } else {
    wsFormat =
        CFX_WideString::FromUTF8(szFormat.GetCStr(), szFormat.GetLength());
  }
  wsFormat = FX_WSTRC(L"time{") + wsFormat;
  wsFormat += FX_WSTRC(L"}");
  CXFA_LocaleValue widgetValue(
      XFA_VT_TIME,
      CFX_WideString::FromUTF8(szTime.GetCStr(), szTime.GetLength()),
      (CXFA_LocaleMgr*)pMgr);
  CFX_WideString wsRet;
  widgetValue.FormatPatterns(wsRet, wsFormat, pLocale,
                             XFA_VALUEPICTURE_Display);
  strLocalTime = FX_UTF8Encode(wsRet, wsRet.GetLength());
  return TRUE;
}
FX_BOOL CXFA_FM2JSContext::GetGMTTime(FXJSE_HOBJECT hThis,
                                      const CFX_ByteStringC& szTime,
                                      const CFX_ByteStringC& szFormat,
                                      const CFX_ByteStringC& szLocale,
                                      CFX_ByteString& strGMTTime) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return FALSE;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocale.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocale.GetCStr(), szLocale.GetLength()));
  }
  if (!pLocale) {
    return FALSE;
  }
  CFX_WideString wsFormat;
  if (szFormat.IsEmpty()) {
    pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Default, wsFormat);
  } else {
    wsFormat =
        CFX_WideString::FromUTF8(szFormat.GetCStr(), szFormat.GetLength());
  }
  wsFormat = FX_WSTRC(L"time{") + wsFormat;
  wsFormat += FX_WSTRC(L"}");
  CXFA_LocaleValue widgetValue(
      XFA_VT_TIME,
      CFX_WideString::FromUTF8(szTime.GetCStr(), szTime.GetLength()),
      (CXFA_LocaleMgr*)pMgr);
  CFX_WideString wsRet;
  widgetValue.FormatPatterns(wsRet, wsFormat, pLocale,
                             XFA_VALUEPICTURE_Display);
  strGMTTime = FX_UTF8Encode(wsRet, wsRet.GetLength());
  return TRUE;
}
int32_t CXFA_FM2JSContext::DateString2Num(const CFX_ByteStringC& szDateString) {
  FX_BOOL bFlags = FALSE;
  int32_t iLength = szDateString.GetLength();
  FX_BOOL iRet = FALSE;
  int32_t iStyle = -1;
  int32_t iYear = 0;
  int32_t iMonth = 0;
  int32_t iDay = 0;
  int32_t iHour = 0;
  int32_t iMinute = 0;
  int32_t iSecond = 0;
  int32_t iMillionSecond = 0;
  int32_t iZoneHour = 0;
  int32_t iZoneMinute = 0;
  if (iLength <= 10) {
    iRet = IsIsoDateFormat(szDateString.GetCStr(), iLength, iStyle, iYear,
                           iMonth, iDay);
  } else {
    iRet = IsIsoDateTimeFormat(szDateString.GetCStr(), iLength, iYear, iMonth,
                               iDay, iHour, iMinute, iSecond, iMillionSecond,
                               iZoneHour, iZoneMinute);
  }
  if (!iRet) {
    bFlags = TRUE;
  }
  FX_FLOAT dDays = 0;
  int32_t i = 1;
  if (iYear < 1900) {
    bFlags = TRUE;
  }
  if (!bFlags) {
    while (iYear - i >= 1900) {
      if ((!((iYear - i) % 4) && ((iYear - i) % 100)) || !((iYear - i) % 400)) {
        dDays += 366;
      } else {
        dDays += 365;
      }
      ++i;
    }
    i = 1;
    while (i < iMonth) {
      if (i == 2) {
        if ((!(iYear % 4) && (iYear % 100)) || !(iYear % 400)) {
          dDays += 29;
        } else {
          dDays += 28;
        }
      } else if (i <= 7) {
        if (i % 2 == 0) {
          dDays += 30;
        } else {
          dDays += 31;
        }
      } else {
        if (i % 2 == 0) {
          dDays += 31;
        } else {
          dDays += 30;
        }
      }
      ++i;
    }
    i = 0;
    while (iDay - i > 0) {
      dDays += 1;
      ++i;
    }
  } else {
    dDays = 0;
  }
  return (int32_t)dDays;
}
#define XFA_N 19
static uint8_t g_sAltTable_Date[] = {
    XFA_N, XFA_N, XFA_N, 3,     9,     XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, 2,     XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, 1,     XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
};
static uint8_t g_sAltTable_Time[] = {
    14,    XFA_N, XFA_N, 3,     9,     XFA_N, XFA_N, 15,    XFA_N, XFA_N, XFA_N,
    XFA_N, 6,     XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, 7,     XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, 1,     17,    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, 15,    XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
    XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N, XFA_N,
};
static void XFA_FM_AlternateDateTimeSymbols(CFX_WideString& wsPattern,
                                            const CFX_WideString& wsAltSymbols,
                                            uint8_t* pAltTable) {
  int32_t nLength = wsPattern.GetLength();
  FX_BOOL bInConstRange = FALSE;
  FX_BOOL bEscape = FALSE;
  int32_t i = 0, n = 0;
  while (i < nLength) {
    FX_WCHAR wc = wsPattern[i];
    if (wc == L'\'') {
      bInConstRange = !bInConstRange;
      if (bEscape) {
        i++;
      } else {
        wsPattern.Delete(i);
        nLength--;
      }
      bEscape = !bEscape;
      continue;
    }
    if (!bInConstRange && (n = wc - L'A') >= 0 && n <= (L'a' - L'A')) {
      int32_t nAlt = (int32_t)pAltTable[n];
      if (nAlt != XFA_N) {
        wsPattern.SetAt(i, wsAltSymbols[nAlt]);
      }
    }
    i++;
    bEscape = FALSE;
  }
}
#undef XFA_N
void CXFA_FM2JSContext::GetLocalDateFormat(FXJSE_HOBJECT hThis,
                                           int32_t iStyle,
                                           const CFX_ByteStringC& szLocalStr,
                                           CFX_ByteString& strFormat,
                                           FX_BOOL bStandard) {
  FX_LOCALEDATETIMESUBCATEGORY strStyle;
  switch (iStyle) {
    case 0:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
    case 1:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Short;
      break;
    case 2:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
    case 3:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Long;
      break;
    case 4:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Full;
      break;
    default:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
  }
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocalStr.IsEmpty()) {
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocalStr.GetCStr(), szLocalStr.GetLength()));
  }
  if (!pLocale) {
    return;
  }
  CFX_WideString strRet;
  pLocale->GetDatePattern(strStyle, strRet);
  if (!bStandard) {
    CFX_WideString wsSymbols;
    pLocale->GetDateTimeSymbols(wsSymbols);
    XFA_FM_AlternateDateTimeSymbols(strRet, wsSymbols, g_sAltTable_Date);
  }
  strFormat = FX_UTF8Encode(strRet, strRet.GetLength());
}
void CXFA_FM2JSContext::GetLocalTimeFormat(FXJSE_HOBJECT hThis,
                                           int32_t iStyle,
                                           const CFX_ByteStringC& szLocalStr,
                                           CFX_ByteString& strFormat,
                                           FX_BOOL bStandard) {
  FX_LOCALEDATETIMESUBCATEGORY strStyle;
  switch (iStyle) {
    case 0:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
    case 1:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Short;
      break;
    case 2:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
    case 3:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Long;
      break;
    case 4:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Full;
      break;
    default:
      strStyle = FX_LOCALEDATETIMESUBCATEGORY_Medium;
      break;
  }
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return;
  }
  IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
  IFX_Locale* pLocale = NULL;
  if (szLocalStr.IsEmpty()) {
    CXFA_Object* pThisObject = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisObject->IsNode());
    CXFA_Node* pThisNode = (CXFA_Node*)pThisObject;
    CXFA_WidgetData widgetData(pThisNode);
    pLocale = widgetData.GetLocal();
  } else {
    pLocale = pMgr->GetLocaleByName(
        CFX_WideString::FromUTF8(szLocalStr.GetCStr(), szLocalStr.GetLength()));
  }
  if (!pLocale) {
    return;
  }
  CFX_WideString strRet;
  pLocale->GetTimePattern(strStyle, strRet);
  if (!bStandard) {
    CFX_WideString wsSymbols;
    pLocale->GetDateTimeSymbols(wsSymbols);
    XFA_FM_AlternateDateTimeSymbols(strRet, wsSymbols, g_sAltTable_Time);
  }
  strFormat = FX_UTF8Encode(strRet, strRet.GetLength());
}
void CXFA_FM2JSContext::GetStandardDateFormat(FXJSE_HOBJECT hThis,
                                              int32_t iStyle,
                                              const CFX_ByteStringC& szLocalStr,
                                              CFX_ByteString& strFormat) {
  GetLocalDateFormat(hThis, iStyle, szLocalStr, strFormat, TRUE);
}
void CXFA_FM2JSContext::GetStandardTimeFormat(FXJSE_HOBJECT hThis,
                                              int32_t iStyle,
                                              const CFX_ByteStringC& szLocalStr,
                                              CFX_ByteString& strFormat) {
  GetLocalTimeFormat(hThis, iStyle, szLocalStr, strFormat, TRUE);
}
void CXFA_FM2JSContext::Num2AllTime(FXJSE_HOBJECT hThis,
                                    int32_t iTime,
                                    const CFX_ByteStringC& szFormat,
                                    const CFX_ByteStringC& szLocale,
                                    FX_BOOL bGM,
                                    CFX_ByteString& strTime) {
  int32_t iHour = 0;
  int32_t iMin = 0;
  int32_t iSec = 0;
  int32_t iZoneHour = 0;
  int32_t iZoneMin = 0;
  int32_t iZoneSec = 0;
  iHour = static_cast<int>(iTime) / 3600000;
  iMin = (static_cast<int>(iTime) - iHour * 3600000) / 60000;
  iSec = (static_cast<int>(iTime) - iHour * 3600000 - iMin * 60000) / 1000;
  if (!bGM) {
    GetLocalTimeZone(iZoneHour, iZoneMin, iZoneSec);
    iHour += iZoneHour;
    iMin += iZoneMin;
    iSec += iZoneSec;
  }
  int32_t iRet = 0;
  CFX_ByteString strIsoTime;
  strIsoTime.Format("%02d:%02d:%02d", iHour, iMin, iSec);
  if (bGM) {
    iRet = GetGMTTime(hThis, strIsoTime, szFormat, szLocale, strTime);
  } else {
    iRet = IsoTime2Local(hThis, strIsoTime, szFormat, szLocale, strTime);
  }
  if (!iRet) {
    strTime = "";
  }
  return;
}
void CXFA_FM2JSContext::GetLocalTimeZone(int32_t& iHour,
                                         int32_t& iMin,
                                         int32_t& iSec) {
  time_t now;
  time(&now);
  struct tm* pGmt = gmtime(&now);
  int32_t iGMHour = pGmt->tm_hour;
  int32_t iGMMin = pGmt->tm_min;
  int32_t iGMSec = pGmt->tm_sec;
  struct tm* pLocal = localtime(&now);
  int32_t iLocalHour = pLocal->tm_hour;
  int32_t iLocalMin = pLocal->tm_min;
  int32_t iLocalSec = pLocal->tm_sec;
  iHour = iLocalHour - iGMHour;
  iMin = iLocalMin - iGMMin;
  iSec = iLocalSec - iGMSec;
}
void CXFA_FM2JSContext::Apr(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_DOUBLE nPrincipal = 0;
    FX_DOUBLE nPayment = 0;
    FX_DOUBLE nPeriods = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nPrincipal = HValueToDouble(hThis, argOne);
      nPayment = HValueToDouble(hThis, argTwo);
      nPeriods = HValueToDouble(hThis, argThree);
      bFlags = ((nPrincipal <= 0) || (nPayment <= 0) || (nPeriods <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_DOUBLE r =
            2 * (nPeriods * nPayment - nPrincipal) / (nPeriods * nPrincipal);
        FX_DOUBLE nTemp = 1;
        for (int32_t i = 0; i < nPeriods; ++i) {
          nTemp *= (1 + r);
        }
        FX_DOUBLE nRet = r * nTemp / (nTemp - 1) - nPayment / nPrincipal;
        while ((nRet > FINANCIAL_PRECISION || nRet < -FINANCIAL_PRECISION) &&
               (!bFlags)) {
          FX_DOUBLE nDerivative = 0;
          nDerivative =
              ((nTemp + r * nPeriods * (nTemp / (1 + r))) * (nTemp - 1) -
               (r * nTemp * nPeriods * (nTemp / (1 + r)))) /
              ((nTemp - 1) * (nTemp - 1));
          if (nDerivative == 0) {
            bFlags = TRUE;
            continue;
          }
          r = r - nRet / nDerivative;
          nTemp = 1;
          for (int32_t i = 0; i < nPeriods; ++i) {
            nTemp *= (1 + r);
          }
          nRet = r * nTemp / (nTemp - 1) - nPayment / nPrincipal;
        }
        if (bFlags) {
          FXJSE_Value_SetNull(args.GetReturnValue());
        } else {
          r = r * 12;
          FXJSE_Value_SetDouble(args.GetReturnValue(), r);
        }
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Apr");
  }
}
void CXFA_FM2JSContext::CTerm(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nRate = 0;
    FX_FLOAT nFutureValue = 0;
    FX_FLOAT nInitAmount = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nRate = HValueToFloat(hThis, argOne);
      nFutureValue = HValueToFloat(hThis, argTwo);
      nInitAmount = HValueToFloat(hThis, argThree);
      bFlags = ((nRate <= 0) || (nFutureValue <= 0) || (nInitAmount <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FXJSE_Value_SetFloat(args.GetReturnValue(),
                             FXSYS_log((FX_FLOAT)(nFutureValue / nInitAmount)) /
                                 FXSYS_log((FX_FLOAT)(1 + nRate)));
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"CTerm");
  }
}
void CXFA_FM2JSContext::FV(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_DOUBLE nAmount = 0;
    FX_DOUBLE nRate = 0;
    FX_DOUBLE nPeriod = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nAmount = HValueToDouble(hThis, argOne);
      nRate = HValueToDouble(hThis, argTwo);
      nPeriod = HValueToDouble(hThis, argThree);
      bFlags = ((nRate < 0) || (nPeriod <= 0) || (nAmount <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_DOUBLE dResult = 0;
        if (!nRate) {
          dResult = nAmount * nPeriod;
        } else {
          FX_DOUBLE nTemp = 1;
          for (int i = 0; i < nPeriod; ++i) {
            nTemp *= 1 + nRate;
          }
          dResult = nAmount * (nTemp - 1) / nRate;
        }
        FXJSE_Value_SetDouble(args.GetReturnValue(), dResult);
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"FV");
  }
}
void CXFA_FM2JSContext::IPmt(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 5) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nPrincpalAmount = 0;
    FX_FLOAT nRate = 0;
    FX_FLOAT nPayment = 0;
    FX_FLOAT nFirstMonth = 0;
    FX_FLOAT nNumberOfMonths = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    FXJSE_HVALUE argFour = GetSimpleHValue(hThis, args, 3);
    FXJSE_HVALUE argFive = GetSimpleHValue(hThis, args, 4);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree) || HValueIsNull(hThis, argFour) ||
              HValueIsNull(hThis, argFive));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nPrincpalAmount = HValueToFloat(hThis, argOne);
      nRate = HValueToFloat(hThis, argTwo);
      nPayment = HValueToFloat(hThis, argThree);
      nFirstMonth = HValueToFloat(hThis, argFour);
      nNumberOfMonths = HValueToFloat(hThis, argFive);
      bFlags = ((nPrincpalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
                (nFirstMonth < 0) || (nNumberOfMonths < 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_FLOAT fResult = 0;
        FX_FLOAT nRateOfMonth = nRate / 12;
        int32_t iNums =
            (int32_t)((FXSYS_log10((FX_FLOAT)(nPayment / nPrincpalAmount)) -
                       FXSYS_log10((FX_FLOAT)(nPayment / nPrincpalAmount -
                                              nRateOfMonth))) /
                      FXSYS_log10((FX_FLOAT)(1 + nRateOfMonth)));
        int32_t iEnd = (int32_t)(nFirstMonth + nNumberOfMonths - 1);
        if (iEnd > iNums) {
          iEnd = iNums;
        }
        FX_FLOAT nSum = 0;
        if (nPayment < nPrincpalAmount * nRateOfMonth) {
          bFlags = TRUE;
          fResult = 0;
        }
        if (!bFlags) {
          int32_t i = 0;
          for (i = 0; i < nFirstMonth - 1; ++i) {
            nPrincpalAmount -= nPayment - nPrincpalAmount * nRateOfMonth;
          }
          for (; i < iEnd; ++i) {
            nSum += nPrincpalAmount * nRateOfMonth;
            nPrincpalAmount -= nPayment - nPrincpalAmount * nRateOfMonth;
          }
          fResult = nSum;
        }
        FXJSE_Value_SetFloat(args.GetReturnValue(), fResult);
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
    FXJSE_Value_Release(argFour);
    FXJSE_Value_Release(argFive);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"IPmt");
  }
}
void CXFA_FM2JSContext::NPV(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc > 2) {
    FX_BOOL bFlags = FALSE;
    FXJSE_HVALUE* argValues = FX_Alloc(FXJSE_HVALUE, argc);
    for (int32_t i = 0; i < argc; i++) {
      argValues[i] = GetSimpleHValue(hThis, args, i);
      if (HValueIsNull(hThis, argValues[i])) {
        bFlags = TRUE;
      }
    }
    if (!bFlags) {
      FX_DOUBLE nRate = 0;
      nRate = HValueToDouble(hThis, argValues[0]);
      if (nRate <= 0) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_DOUBLE* pData = FX_Alloc(FX_DOUBLE, argc - 1);
        for (int32_t i = 1; i < argc; i++) {
          pData[i - 1] = HValueToDouble(hThis, argValues[i]);
        }
        FX_DOUBLE nSum = 0;
        int32_t iIndex = 0;
        for (int32_t i = 0; i < argc - 1; i++) {
          FX_DOUBLE nTemp = 1;
          for (int32_t j = 0; j <= i; j++) {
            nTemp *= 1 + nRate;
          }
          FX_DOUBLE nNum = *(pData + iIndex++);
          nSum += nNum / nTemp;
        }
        FXJSE_Value_SetDouble(args.GetReturnValue(), nSum);
        FX_Free(pData);
        pData = 0;
      }
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    for (int32_t i = 0; i < argc; i++) {
      FXJSE_Value_Release(argValues[i]);
    }
    FX_Free(argValues);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"NPV");
  }
}
void CXFA_FM2JSContext::Pmt(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nPrincipal = 0;
    FX_FLOAT nRate = 0;
    FX_FLOAT nPeriods = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nPrincipal = HValueToFloat(hThis, argOne);
      nRate = HValueToFloat(hThis, argTwo);
      nPeriods = HValueToFloat(hThis, argThree);
      bFlags = ((nPrincipal <= 0) || (nRate <= 0) || (nPeriods <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_FLOAT nSum = 0;
        FX_FLOAT nTmp = 1 + nRate;
        nSum = nTmp;
        for (int32_t i = 0; i < nPeriods - 1; ++i) {
          nSum *= nTmp;
        }
        FXJSE_Value_SetFloat(args.GetReturnValue(),
                             (nPrincipal * nRate * nSum) / (nSum - 1));
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Pmt");
  }
}
void CXFA_FM2JSContext::PPmt(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 5) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nPrincpalAmount = 0;
    FX_FLOAT nRate = 0;
    FX_FLOAT nPayment = 0;
    FX_FLOAT nFirstMonth = 0;
    FX_FLOAT nNumberOfMonths = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    FXJSE_HVALUE argFour = GetSimpleHValue(hThis, args, 3);
    FXJSE_HVALUE argFive = GetSimpleHValue(hThis, args, 4);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree) || HValueIsNull(hThis, argFour) ||
              HValueIsNull(hThis, argFive));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nPrincpalAmount = HValueToFloat(hThis, argOne);
      nRate = HValueToFloat(hThis, argTwo);
      nPayment = HValueToFloat(hThis, argThree);
      nFirstMonth = HValueToFloat(hThis, argFour);
      nNumberOfMonths = HValueToFloat(hThis, argFive);
      bFlags = ((nPrincpalAmount <= 0) || (nRate <= 0) || (nPayment <= 0) ||
                (nFirstMonth < 0) || (nNumberOfMonths < 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        int32_t iEnd = (int32_t)(nFirstMonth + nNumberOfMonths - 1);
        FX_FLOAT nSum = 0;
        FX_FLOAT nRateOfMonth = nRate / 12;
        int32_t iNums =
            (int32_t)((FXSYS_log10((FX_FLOAT)(nPayment / nPrincpalAmount)) -
                       FXSYS_log10((FX_FLOAT)(nPayment / nPrincpalAmount -
                                              nRateOfMonth))) /
                      FXSYS_log10((FX_FLOAT)(1 + nRateOfMonth)));
        if (iEnd > iNums) {
          iEnd = iNums;
        }
        if (nPayment < nPrincpalAmount * nRateOfMonth) {
          bFlags = TRUE;
        }
        if (!bFlags) {
          int32_t i = 0;
          for (i = 0; i < nFirstMonth - 1; ++i) {
            nPrincpalAmount -= nPayment - nPrincpalAmount * nRateOfMonth;
          }
          FX_FLOAT nTemp = 0;
          for (; i < iEnd; ++i) {
            nTemp = nPayment - nPrincpalAmount * nRateOfMonth;
            nSum += nTemp;
            nPrincpalAmount -= nTemp;
          }
          FXJSE_Value_SetFloat(args.GetReturnValue(), nSum);
        } else {
          pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
        }
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
    FXJSE_Value_Release(argFour);
    FXJSE_Value_Release(argFive);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"PPmt");
  }
}
void CXFA_FM2JSContext::PV(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_DOUBLE nAmount = 0;
    FX_DOUBLE nRate = 0;
    FX_DOUBLE nPeriod = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nAmount = HValueToDouble(hThis, argOne);
      nRate = HValueToDouble(hThis, argTwo);
      nPeriod = HValueToDouble(hThis, argThree);
      bFlags = ((nAmount <= 0) || (nRate < 0) || (nPeriod <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FX_DOUBLE nTemp = 1;
        for (int32_t i = 0; i < nPeriod; ++i) {
          nTemp *= 1 + nRate;
        }
        nTemp = 1 / nTemp;
        FXJSE_Value_SetDouble(args.GetReturnValue(),
                              nAmount * ((1 - nTemp) / nRate));
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"PV");
  }
}
void CXFA_FM2JSContext::Rate(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nFuture = 0;
    FX_FLOAT nPresent = 0;
    FX_FLOAT nTotalNumber = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo) ||
              HValueIsNull(hThis, argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nFuture = HValueToFloat(hThis, argOne);
      nPresent = HValueToFloat(hThis, argTwo);
      nTotalNumber = HValueToFloat(hThis, argThree);
      bFlags = ((nFuture <= 0) || (nPresent < 0) || (nTotalNumber <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FXJSE_Value_SetFloat(args.GetReturnValue(),
                             (FXSYS_pow((FX_FLOAT)(nFuture / nPresent),
                                        (FX_FLOAT)(1 / nTotalNumber)) -
                              1));
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Rate");
  }
}
void CXFA_FM2JSContext::Term(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 3) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT nMount = 0;
    FX_FLOAT nRate = 0;
    FX_FLOAT nFuture = 0;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = GetSimpleHValue(hThis, args, 2);
    bFlags = (FXJSE_Value_IsNull(argOne) || FXJSE_Value_IsNull(argTwo) ||
              FXJSE_Value_IsNull(argThree));
    if (bFlags) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      nMount = HValueToFloat(hThis, argOne);
      nRate = HValueToFloat(hThis, argTwo);
      nFuture = HValueToFloat(hThis, argThree);
      bFlags = ((nMount <= 0) || (nRate <= 0) || (nFuture <= 0));
      if (bFlags) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else {
        FXJSE_Value_SetFloat(
            args.GetReturnValue(),
            (FXSYS_log((FX_FLOAT)(nFuture / nMount * nRate) + 1) /
             FXSYS_log((FX_FLOAT)(1 + nRate))));
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    FXJSE_Value_Release(argThree);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Term");
  }
}
void CXFA_FM2JSContext::Choose(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  if (argc > 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FX_BOOL argOneIsNull = FALSE;
    int32_t iIndex = 0;
    argOneIsNull = HValueIsNull(hThis, argOne);
    if (!argOneIsNull) {
      iIndex = (int32_t)HValueToFloat(hThis, argOne);
    }
    FXJSE_Value_Release(argOne);
    if (argOneIsNull) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else if (iIndex < 1) {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
    } else {
      FX_BOOL bFound = FALSE;
      FX_BOOL bStopCounterFlags = FALSE;
      int32_t iArgIndex = 1;
      int32_t iValueIndex = 0;
      while (!bFound && !bStopCounterFlags && (iArgIndex < argc)) {
        FXJSE_HVALUE argIndexValue = args.GetValue(iArgIndex);
        if (FXJSE_Value_IsArray(argIndexValue)) {
          FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
          FXJSE_Value_GetObjectProp(argIndexValue, "length", lengthValue);
          int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
          FXJSE_Value_Release(lengthValue);
          if (iLength > 3) {
            bStopCounterFlags = TRUE;
          }
          iValueIndex += (iLength - 2);
          if (iValueIndex >= iIndex) {
            FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
            FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
            FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
            FXJSE_Value_GetObjectPropByIdx(argIndexValue, 1, propertyValue);
            FXJSE_Value_GetObjectPropByIdx(
                argIndexValue, ((iLength - 1) - (iValueIndex - iIndex)),
                jsobjectValue);
            if (FXJSE_Value_IsNull(propertyValue)) {
              GetObjectDefaultValue(jsobjectValue, newProperty);
            } else {
              CFX_ByteString propStr;
              FXJSE_Value_ToUTF8String(propertyValue, propStr);
              FXJSE_Value_GetObjectProp(jsobjectValue, propStr, newProperty);
            }
            CFX_ByteString bsChoosed;
            HValueToUTF8String(newProperty, bsChoosed);
            FXJSE_Value_SetUTF8String(args.GetReturnValue(), bsChoosed);
            FXJSE_Value_Release(newProperty);
            FXJSE_Value_Release(jsobjectValue);
            FXJSE_Value_Release(propertyValue);
            bFound = TRUE;
          }
        } else {
          iValueIndex++;
          if (iValueIndex == iIndex) {
            CFX_ByteString bsChoosed;
            HValueToUTF8String(argIndexValue, bsChoosed);
            FXJSE_Value_SetUTF8String(args.GetReturnValue(), bsChoosed);
            bFound = TRUE;
          }
        }
        FXJSE_Value_Release(argIndexValue);
        iArgIndex++;
      }
      if (!bFound) {
        FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Choose");
  }
}
void CXFA_FM2JSContext::Exists(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FXJSE_Value_SetInteger(args.GetReturnValue(), FXJSE_Value_IsObject(argOne));
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Exists");
  }
}
void CXFA_FM2JSContext::HasValue(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsUTF8String(argOne)) {
      CFX_ByteString valueStr;
      FXJSE_Value_ToUTF8String(argOne, valueStr);
      valueStr.TrimLeft();
      FXJSE_Value_SetInteger(args.GetReturnValue(), (!valueStr.IsEmpty()));
    } else if (FXJSE_Value_IsNumber(argOne) || FXJSE_Value_IsBoolean(argOne)) {
      FXJSE_Value_SetInteger(args.GetReturnValue(), TRUE);
    } else {
      FXJSE_Value_SetInteger(args.GetReturnValue(), FALSE);
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"HasValue");
  }
}
void CXFA_FM2JSContext::Oneof(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc > 1) {
    FX_BOOL bFlags = FALSE;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE* parametersValue = 0;
    int32_t iCount = 0;
    unfoldArgs(hThis, args, parametersValue, iCount, 1);
    for (int32_t i = 0; i < iCount; i++) {
      if (simpleValueCompare(hThis, argOne, parametersValue[i])) {
        bFlags = TRUE;
        break;
      }
    }
    FXJSE_Value_SetInteger(args.GetReturnValue(), bFlags);
    FXJSE_Value_Release(argOne);
    for (int32_t i = 0; i < iCount; i++) {
      FXJSE_Value_Release(parametersValue[i]);
    }
    FX_Free(parametersValue);
    parametersValue = 0;
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Oneof");
  }
}
void CXFA_FM2JSContext::Within(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc == 3) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetUndefined(args.GetReturnValue());
    } else {
      FXJSE_HVALUE argLow = GetSimpleHValue(hThis, args, 1);
      FXJSE_HVALUE argHeight = GetSimpleHValue(hThis, args, 2);
      if (FXJSE_Value_IsNumber(argOne)) {
        FX_FLOAT oneNumber = HValueToFloat(hThis, argOne);
        FX_FLOAT lowNumber = HValueToFloat(hThis, argLow);
        FX_FLOAT heightNumber = HValueToFloat(hThis, argHeight);
        FXJSE_Value_SetInteger(
            args.GetReturnValue(),
            ((oneNumber >= lowNumber) && (oneNumber <= heightNumber)));
      } else {
        CFX_ByteString oneString;
        CFX_ByteString lowString;
        CFX_ByteString heightString;
        HValueToUTF8String(argOne, oneString);
        HValueToUTF8String(argLow, lowString);
        HValueToUTF8String(argHeight, heightString);
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               ((oneString.Compare(lowString) >= 0) &&
                                (oneString.Compare(heightString) <= 0)));
      }
      FXJSE_Value_Release(argLow);
      FXJSE_Value_Release(argHeight);
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Within");
  }
}
void CXFA_FM2JSContext::If(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args) {
  if (args.GetLength() == 3) {
    FXJSE_HVALUE argCondition = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argFirstValue = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argSecondValue = GetSimpleHValue(hThis, args, 2);
    FX_BOOL bCondition = FXJSE_Value_ToBoolean(argCondition);
    FXJSE_Value_Set(args.GetReturnValue(),
                    bCondition ? argFirstValue : argSecondValue);
    FXJSE_Value_Release(argSecondValue);
    FXJSE_Value_Release(argFirstValue);
    FXJSE_Value_Release(argCondition);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"If");
  }
}
void CXFA_FM2JSContext::Eval(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  if (args.GetLength() == 1) {
    FXJSE_HVALUE scriptValue = GetSimpleHValue(hThis, args, 0);
    CFX_ByteString utf8ScriptString;
    HValueToUTF8String(scriptValue, utf8ScriptString);
    if (utf8ScriptString.IsEmpty()) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_WideTextBuf wsJavaScriptBuf;
      CFX_WideString javaScript;
      CFX_WideString wsError;
      XFA_FM2JS_Translate(CFX_WideString::FromUTF8(
                              utf8ScriptString, utf8ScriptString.GetLength()),
                          wsJavaScriptBuf, wsError);
      FXJSE_HCONTEXT hContext = FXJSE_Context_Create(hruntime);
      FXJSE_HVALUE returnValue = FXJSE_Value_Create(hruntime);
      javaScript = wsJavaScriptBuf.GetWideString();
      FXJSE_ExecuteScript(hContext,
                          FX_UTF8Encode(javaScript, javaScript.GetLength()),
                          returnValue);
      FXJSE_Value_Set(args.GetReturnValue(), returnValue);
      FXJSE_Value_Release(returnValue);
      FXJSE_Context_Release(hContext);
    }
    FXJSE_Value_Release(scriptValue);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Eval");
  }
}
void CXFA_FM2JSContext::Ref(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_HVALUE rgValues[3];
      for (int32_t i = 0; i < 3; i++) {
        rgValues[i] = FXJSE_Value_Create(hruntime);
      }
      FXJSE_Value_SetInteger(rgValues[0], 4);
      FXJSE_Value_SetNull(rgValues[1]);
      FXJSE_Value_SetNull(rgValues[2]);
      FXJSE_Value_SetArray(args.GetReturnValue(), 3, rgValues);
      for (int32_t i = 0; i < 3; i++) {
        FXJSE_Value_Release(rgValues[i]);
      }
    } else if (FXJSE_Value_IsArray(argOne)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argOne, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      FXSYS_assert(iLength >= 3);
      FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(argOne, 1, propertyValue);
      FXJSE_Value_GetObjectPropByIdx(argOne, 2, jsObjectValue);
      if (FXJSE_Value_IsNull(jsObjectValue)) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      } else if (FXJSE_Value_IsNull(propertyValue) &&
                 (!FXJSE_Value_IsNull(jsObjectValue))) {
        FXJSE_HVALUE rgValues[3];
        for (int32_t i = 0; i < 3; i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 3);
        FXJSE_Value_SetNull(rgValues[1]);
        FXJSE_Value_Set(rgValues[2], jsObjectValue);
        FXJSE_Value_SetArray(args.GetReturnValue(), 3, rgValues);
        for (int32_t i = 0; i < 3; i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
      }
      FXJSE_Value_Release(jsObjectValue);
      FXJSE_Value_Release(propertyValue);
    } else if (FXJSE_Value_IsObject(argOne)) {
      FXJSE_HVALUE rgValues[3];
      for (int32_t i = 0; i < 3; i++) {
        rgValues[i] = FXJSE_Value_Create(hruntime);
      }
      FXJSE_Value_SetInteger(rgValues[0], 3);
      FXJSE_Value_SetNull(rgValues[1]);
      FXJSE_Value_Set(rgValues[2], argOne);
      FXJSE_Value_SetArray(args.GetReturnValue(), 3, rgValues);
      for (int32_t i = 0; i < 3; i++) {
        FXJSE_Value_Release(rgValues[i]);
      }
    } else if (FXJSE_Value_IsBoolean(argOne) ||
               FXJSE_Value_IsUTF8String(argOne) ||
               FXJSE_Value_IsNumber(argOne)) {
      FXJSE_Value_Set(args.GetReturnValue(), argOne);
    } else {
      pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Ref");
  }
}
void CXFA_FM2JSContext::UnitType(FXJSE_HOBJECT hThis,
                                 const CFX_ByteStringC& szFuncName,
                                 CFXJSE_Arguments& args) {
  if (args.GetLength() == 1) {
    FXJSE_HVALUE unitspanValue = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(unitspanValue)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
      FXJSE_Value_Release(unitspanValue);
      return;
    }
    CFX_ByteString unitspanString;
    HValueToUTF8String(unitspanValue, unitspanString);
    if (unitspanString.IsEmpty()) {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), "in");
    } else {
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
      unitspanString.MakeLower();
      CFX_WideString wsTypeString =
          CFX_WideString::FromUTF8(unitspanString, unitspanString.GetLength());
      const FX_WCHAR* pData = wsTypeString;
      int32_t u = 0;
      int32_t uLen = wsTypeString.GetLength();
      while (*(pData + u) == 0x20 || *(pData + u) == 0x09 ||
             *(pData + u) == 0x0B || *(pData + u) == 0x0C ||
             *(pData + u) == 0x0A || *(pData + u) == 0x0D) {
        u++;
      }
      XFA_FM2JS_VALUETYPE_ParserStatus eParserStatus = VALUETYPE_START;
      FX_WCHAR typeChar;
      while (u < uLen) {
        typeChar = *(pData + u);
        if (typeChar == 0x20 || typeChar == 0x09 || typeChar == 0x0B ||
            typeChar == 0x0C || typeChar == 0x0A || typeChar == 0x0D) {
          if (eParserStatus == VALUETYPE_HAVEDIGIT ||
              eParserStatus == VALUETYPE_HAVEDIGITWHITE) {
            eParserStatus = VALUETYPE_HAVEDIGITWHITE;
          } else {
            eParserStatus = VALUETYPE_ISIN;
            break;
          }
        } else if ((typeChar >= '0' && typeChar <= '9') || typeChar == '-' ||
                   typeChar == '.') {
          if (eParserStatus == VALUETYPE_HAVEDIGITWHITE) {
            eParserStatus = VALUETYPE_ISIN;
            break;
          } else {
            eParserStatus = VALUETYPE_HAVEDIGIT;
          }
        } else if ((typeChar == 'c' || typeChar == 'p') && (u + 1 < uLen)) {
          FX_WCHAR nextChar = *(pData + u + 1);
          if ((eParserStatus == VALUETYPE_START ||
               eParserStatus == VALUETYPE_HAVEDIGIT ||
               eParserStatus == VALUETYPE_HAVEDIGITWHITE) &&
              (nextChar > '9' || nextChar < '0') && nextChar != '.' &&
              nextChar != '-') {
            eParserStatus = (typeChar == 'c') ? VALUETYPE_ISCM : VALUETYPE_ISPT;
            break;
          } else {
            eParserStatus = VALUETYPE_HAVEINVALIDCHAR;
          }
        } else if (typeChar == 'm' && (u + 1 < uLen)) {
          FX_WCHAR nextChar = *(pData + u + 1);
          if ((eParserStatus == VALUETYPE_START ||
               eParserStatus == VALUETYPE_HAVEDIGIT ||
               eParserStatus == VALUETYPE_HAVEDIGITWHITE) &&
              (nextChar > '9' || nextChar < '0') && nextChar != '.' &&
              nextChar != '-') {
            eParserStatus = VALUETYPE_ISMM;
            if (nextChar == 'p' ||
                ((u + 5 < uLen) && *(pData + u + 1) == 'i' &&
                 *(pData + u + 2) == 'l' && *(pData + u + 3) == 'l' &&
                 *(pData + u + 4) == 'i' && *(pData + u + 5) == 'p')) {
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
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "cm");
          break;
        case VALUETYPE_ISMM:
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "mm");
          break;
        case VALUETYPE_ISPT:
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "pt");
          break;
        case VALUETYPE_ISMP:
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "mp");
          break;
        default:
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "in");
          break;
      }
    }
    FXJSE_Value_Release(unitspanValue);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"UnitType");
  }
}
void CXFA_FM2JSContext::UnitValue(FXJSE_HOBJECT hThis,
                                  const CFX_ByteStringC& szFuncName,
                                  CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc == 1) || (argc == 2)) {
    FXJSE_HVALUE unitspanValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE unitValue = 0;
    CFX_ByteString unitspanString;
    FX_DOUBLE dFirstNumber = 0;
    CFX_ByteString strFirstUnit;
    CFX_ByteString strUnit;
    if (FXJSE_Value_IsNull(unitspanValue)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      HValueToUTF8String(unitspanValue, unitspanString);
      const FX_CHAR* pData = unitspanString;
      if (pData) {
        int32_t u = 0;
        while (*(pData + u) == 0x20 || *(pData + u) == 0x09 ||
               *(pData + u) == 0x0B || *(pData + u) == 0x0C ||
               *(pData + u) == 0x0A || *(pData + u) == 0x0D) {
          ++u;
        }
        while (u < unitspanString.GetLength()) {
          if ((*(pData + u) > '9' || *(pData + u) < '0') &&
              *(pData + u) != '.' && *(pData + u) != '-') {
            break;
          }
          ++u;
        }
        FX_CHAR* pTemp = NULL;
        dFirstNumber = strtod(pData, &pTemp);
        while (*(pData + u) == ' ' || *(pData + u) == 0x09 ||
               *(pData + u) == 0x0B || *(pData + u) == 0x0C ||
               *(pData + u) == 0x0A || *(pData + u) == 0x0D) {
          ++u;
        }
        int32_t uLen = unitspanString.GetLength();
        while (u < uLen) {
          if (*(pData + u) == ' ') {
            break;
          }
          strFirstUnit += (*(pData + u));
          ++u;
        }
        strFirstUnit.MakeLower();
        if (argc == 2) {
          unitValue = GetSimpleHValue(hThis, args, 1);
          CFX_ByteString unitTempString;
          HValueToUTF8String(unitValue, unitTempString);
          const FX_CHAR* pData = unitTempString;
          int32_t u = 0;
          while (*(pData + u) == ' ' || *(pData + u) == 0x09 ||
                 *(pData + u) == 0x0B || *(pData + u) == 0x0C ||
                 *(pData + u) == 0x0A || *(pData + u) == 0x0D) {
            ++u;
          }
          while (u < unitTempString.GetLength()) {
            if ((*(pData + u) > '9' || *(pData + u) < '0') &&
                *(pData + u) != '.') {
              break;
            }
            ++u;
          }
          while (*(pData + u) == ' ' || *(pData + u) == 0x09 ||
                 *(pData + u) == 0x0B || *(pData + u) == 0x0C ||
                 *(pData + u) == 0x0A || *(pData + u) == 0x0D) {
            ++u;
          }
          int32_t uLen = unitTempString.GetLength();
          while (u < uLen) {
            if (*(pData + u) == ' ') {
              break;
            }
            strUnit += (*(pData + u));
            ++u;
          }
          strUnit.MakeLower();
        } else {
          strUnit = strFirstUnit;
        }
        FX_DOUBLE dResult = 0;
        if (strFirstUnit.Equal("in") || strFirstUnit.Equal("inches")) {
          if (strUnit.Equal("mm") || strUnit.Equal("millimeters")) {
            dResult = dFirstNumber * 25.4;
          } else if (strUnit.Equal("cm") || strUnit.Equal("centimeters")) {
            dResult = dFirstNumber * 2.54;
          } else if (strUnit.Equal("pt") || strUnit.Equal("points")) {
            dResult = dFirstNumber / 72;
          } else if (strUnit.Equal("mp") || strUnit.Equal("millipoints")) {
            dResult = dFirstNumber / 72000;
          } else {
            dResult = dFirstNumber;
          }
        } else if (strFirstUnit.Equal("mm") ||
                   strFirstUnit.Equal("millimeters")) {
          if (strUnit.Equal("mm") || strUnit.Equal("millimeters")) {
            dResult = dFirstNumber;
          } else if (strUnit.Equal("cm") || strUnit.Equal("centimeters")) {
            dResult = dFirstNumber / 10;
          } else if (strUnit.Equal("pt") || strUnit.Equal("points")) {
            dResult = dFirstNumber / 25.4 / 72;
          } else if (strUnit.Equal("mp") || strUnit.Equal("millipoints")) {
            dResult = dFirstNumber / 25.4 / 72000;
          } else {
            dResult = dFirstNumber / 25.4;
          }
        } else if (strFirstUnit.Equal("cm") ||
                   strFirstUnit.Equal("centimeters")) {
          if (strUnit.Equal("mm") || strUnit.Equal("millimeters")) {
            dResult = dFirstNumber * 10;
          } else if (strUnit.Equal("cm") || strUnit.Equal("centimeters")) {
            dResult = dFirstNumber;
          } else if (strUnit.Equal("pt") || strUnit.Equal("points")) {
            dResult = dFirstNumber / 2.54 / 72;
          } else if (strUnit.Equal("mp") || strUnit.Equal("millipoints")) {
            dResult = dFirstNumber / 2.54 / 72000;
          } else {
            dResult = dFirstNumber / 2.54;
          }
        } else if (strFirstUnit.Equal("pt") || strFirstUnit.Equal("points")) {
          if (strUnit.Equal("mm") || strUnit.Equal("millimeters")) {
            dResult = dFirstNumber / 72 * 25.4;
          } else if (strUnit.Equal("cm") || strUnit.Equal("centimeters")) {
            dResult = dFirstNumber / 72 * 2.54;
          } else if (strUnit.Equal("pt") || strUnit.Equal("points")) {
            dResult = dFirstNumber;
          } else if (strUnit.Equal("mp") || strUnit.Equal("millipoints")) {
            dResult = dFirstNumber * 1000;
          } else {
            dResult = dFirstNumber / 72;
          }
        } else if (strFirstUnit.Equal("mp") ||
                   strFirstUnit.Equal("millipoints")) {
          if (strUnit.Equal("mm") || strUnit.Equal("millimeters")) {
            dResult = dFirstNumber / 72000 * 25.4;
          } else if (strUnit.Equal("cm") || strUnit.Equal("centimeters")) {
            dResult = dFirstNumber / 72000 * 2.54;
          } else if (strUnit.Equal("pt") || strUnit.Equal("points")) {
            dResult = dFirstNumber / 1000;
          } else if (strUnit.Equal("mp") || strUnit.Equal("millipoints")) {
            dResult = dFirstNumber;
          } else {
            dResult = dFirstNumber / 72000;
          }
        }
        FXJSE_Value_SetDouble(args.GetReturnValue(), dResult);
      } else {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
      }
    }
    FXJSE_Value_Release(unitspanValue);
    if (argc == 2) {
      FXJSE_Value_Release(unitValue);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"UnitValue");
  }
}
void CXFA_FM2JSContext::At(FXJSE_HOBJECT hThis,
                           const CFX_ByteStringC& szFuncName,
                           CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    if (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString stringTwo;
      HValueToUTF8String(argTwo, stringTwo);
      if (stringTwo.IsEmpty()) {
        FXJSE_Value_SetInteger(args.GetReturnValue(), 1);
      } else {
        CFX_ByteString stringOne;
        HValueToUTF8String(argOne, stringOne);
        FX_STRSIZE iPosition = stringOne.Find(stringTwo);
        FXJSE_Value_SetInteger(args.GetReturnValue(), iPosition + 1);
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"At");
  }
}
void CXFA_FM2JSContext::Concat(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc >= 1) {
    CFX_ByteString resultString;
    FX_BOOL bAllNull = TRUE;
    FXJSE_HVALUE* argValues = FX_Alloc(FXJSE_HVALUE, argc);
    for (int32_t i = 0; i < argc; i++) {
      argValues[i] = GetSimpleHValue(hThis, args, i);
      if (!HValueIsNull(hThis, argValues[i])) {
        CFX_ByteString valueStr;
        HValueToUTF8String(argValues[i], valueStr);
        resultString += valueStr;
        bAllNull = FALSE;
      }
    }
    for (int32_t i = 0; i < argc; i++) {
      FXJSE_Value_Release(argValues[i]);
    }
    FX_Free(argValues);
    if (bAllNull) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), resultString);
    }
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Concat");
  }
}
void CXFA_FM2JSContext::Decode(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString toDecodeString;
      HValueToUTF8String(argOne, toDecodeString);
      CFX_ByteTextBuf resultBuf;
      DecodeURL(toDecodeString, resultBuf);
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                resultBuf.GetByteString());
    }
    FXJSE_Value_Release(argOne);
  } else if (argc == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    if (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString toDecodeString;
      HValueToUTF8String(argOne, toDecodeString);
      CFX_ByteString identifyString;
      HValueToUTF8String(argTwo, identifyString);
      CFX_ByteTextBuf resultBuf;
      if (identifyString.EqualNoCase("html")) {
        DecodeHTML(toDecodeString, resultBuf);
      } else if (identifyString.EqualNoCase("xml")) {
        DecodeXML(toDecodeString, resultBuf);
      } else {
        DecodeURL(toDecodeString, resultBuf);
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                resultBuf.GetByteString());
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Decode");
  }
}
void CXFA_FM2JSContext::DecodeURL(const CFX_ByteStringC& szURLString,
                                  CFX_ByteTextBuf& szResultString) {
  CFX_WideString wsURLString =
      CFX_WideString::FromUTF8(szURLString.GetCStr(), szURLString.GetLength());
  const FX_WCHAR* pData = wsURLString;
  int32_t iLen = wsURLString.GetLength();
  int32_t i = 0;
  FX_WCHAR ch = 0;
  FX_WCHAR chTemp = 0;
  CFX_WideTextBuf wsResultBuf;
  while (i < iLen) {
    ch = *(pData + i);
    if ('%' == ch) {
      chTemp = 0;
      int32_t iCount = 0;
      while (iCount < 2) {
        ++i;
        ch = *(pData + i);
        if (ch <= '9' && ch >= '0') {
          if (!iCount) {
            chTemp += (ch - '0') * 16;
          } else {
            chTemp += (ch - '0');
          }
        } else {
          if (ch <= 'F' && ch >= 'A') {
            if (!iCount) {
              chTemp += (ch - 'A' + 10) * 16;
            } else {
              chTemp += (ch - 'A' + 10);
            }
          } else if (ch <= 'f' && ch >= 'a') {
            if (!iCount) {
              chTemp += (ch - 'a' + 10) * 16;
            } else {
              chTemp += (ch - 'a' + 10);
            }
          } else {
            wsResultBuf.Clear();
            return;
          }
        }
        ++iCount;
      }
      wsResultBuf.AppendChar(chTemp);
    } else {
      wsResultBuf.AppendChar(ch);
    }
    ++i;
  }
  wsResultBuf.AppendChar(0);
  szResultString =
      FX_UTF8Encode(wsResultBuf.GetBuffer(), wsResultBuf.GetLength());
}
void CXFA_FM2JSContext::DecodeHTML(const CFX_ByteStringC& szHTMLString,
                                   CFX_ByteTextBuf& szResultString) {
  CFX_WideString wsHTMLString = CFX_WideString::FromUTF8(
      szHTMLString.GetCStr(), szHTMLString.GetLength());
  FX_WCHAR strString[9];
  int32_t iStrIndex = 0;
  int32_t iLen = wsHTMLString.GetLength();
  int32_t i = 0;
  int32_t iCode = 0;
  FX_WCHAR ch = 0;
  const FX_WCHAR* pData = wsHTMLString;
  CFX_WideTextBuf wsResultBuf;
  while (i < iLen) {
    ch = *(pData + i);
    if (ch == '&') {
      ++i;
      ch = *(pData + i);
      if (ch == '#') {
        ++i;
        ch = *(pData + i);
        if (ch == 'x' || ch == 'X') {
          ++i;
          ch = *(pData + i);
          if ((ch >= '0' && ch <= '9') || (ch <= 'f' && ch >= 'a') ||
              (ch <= 'F' && ch >= 'A')) {
            while (ch != ';' && i < iLen) {
              if (ch >= '0' && ch <= '9') {
                iCode += ch - '0';
              } else if (ch <= 'f' && ch >= 'a') {
                iCode += ch - 'a' + 10;
              } else if (ch <= 'F' && ch >= 'A') {
                iCode += ch - 'A' + 10;
              } else {
                wsResultBuf.Clear();
                return;
              }
              ++i;
              iCode *= 16;
              ch = *(pData + i);
            }
            iCode /= 16;
          }
        } else {
          wsResultBuf.Clear();
          return;
        }
      } else {
        while (ch != ';' && i < iLen) {
          strString[iStrIndex++] = ch;
          ++i;
          ch = *(pData + i);
        }
        strString[iStrIndex] = 0;
      }
    } else {
      wsResultBuf.AppendChar(ch);
      ++i;
      continue;
    }
    uint32_t iData = 0;
    if (HTMLSTR2Code(strString, iData)) {
      wsResultBuf.AppendChar((FX_WCHAR)iData);
    } else {
      wsResultBuf.AppendChar(iCode);
    }
    iStrIndex = 0;
    strString[iStrIndex] = 0;
    ++i;
  }
  wsResultBuf.AppendChar(0);
  szResultString =
      FX_UTF8Encode(wsResultBuf.GetBuffer(), wsResultBuf.GetLength());
}
void CXFA_FM2JSContext::DecodeXML(const CFX_ByteStringC& szXMLString,
                                  CFX_ByteTextBuf& szResultString) {
  CFX_WideString wsXMLString =
      CFX_WideString::FromUTF8(szXMLString.GetCStr(), szXMLString.GetLength());
  FX_WCHAR strString[9];
  int32_t iStrIndex = 0;
  int32_t iLen = wsXMLString.GetLength();
  int32_t i = 0;
  int32_t iCode = 0;
  FX_WCHAR ch = 0;
  const FX_WCHAR* pData = wsXMLString;
  CFX_WideTextBuf wsXMLBuf;
  while (i < iLen) {
    ch = *(pData + i);
    if (ch == '&') {
      ++i;
      ch = *(pData + i);
      if (ch == '#') {
        ++i;
        ch = *(pData + i);
        if (ch == 'x' || ch == 'X') {
          ++i;
          ch = *(pData + i);
          if ((ch >= '0' && ch <= '9') || (ch <= 'f' && ch >= 'a') ||
              (ch <= 'F' && ch >= 'A')) {
            while (ch != ';') {
              if (ch >= '0' && ch <= '9') {
                iCode += ch - '0';
              } else if (ch <= 'f' && ch >= 'a') {
                iCode += ch - 'a' + 10;
              } else if (ch <= 'F' && ch >= 'A') {
                iCode += ch - 'A' + 10;
              } else {
                wsXMLBuf.Clear();
                return;
              }
              ++i;
              iCode *= 16;
              ch = *(pData + i);
            }
            iCode /= 16;
          }
        } else {
          wsXMLBuf.Clear();
          return;
        }
      } else {
        while (ch != ';' && i < iLen) {
          strString[iStrIndex++] = ch;
          ++i;
          ch = *(pData + i);
        }
        strString[iStrIndex] = 0;
      }
    } else {
      wsXMLBuf.AppendChar(ch);
      ++i;
      continue;
    }
    const FX_WCHAR* const strName[] = {L"quot", L"amp", L"apos", L"lt", L"gt"};
    int32_t iIndex = 0;
    while (iIndex < 5) {
      if (FXSYS_memcmp(strString, strName[iIndex],
                       FXSYS_wcslen(strName[iIndex])) == 0) {
        break;
      }
      ++iIndex;
    }
    switch (iIndex) {
      case 0:
        wsXMLBuf.AppendChar('"');
        break;
      case 1:
        wsXMLBuf.AppendChar('&');
        break;
      case 2:
        wsXMLBuf.AppendChar('\'');
        break;
      case 3:
        wsXMLBuf.AppendChar('<');
        break;
      case 4:
        wsXMLBuf.AppendChar('>');
        break;
      default:
        wsXMLBuf.AppendChar(iCode);
        break;
    }
    iStrIndex = 0;
    strString[iStrIndex] = 0;
    ++i;
    iCode = 0;
  }
  wsXMLBuf.AppendChar(0);
  szResultString = FX_UTF8Encode(wsXMLBuf.GetBuffer(), wsXMLBuf.GetLength());
}
void CXFA_FM2JSContext::Encode(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString toEncodeString;
      HValueToUTF8String(argOne, toEncodeString);
      CFX_ByteTextBuf resultBuf;
      EncodeURL(toEncodeString, resultBuf);
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                resultBuf.GetByteString());
    }
    FXJSE_Value_Release(argOne);
  } else if (argc == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    if (HValueIsNull(hThis, argOne) || HValueIsNull(hThis, argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString toEncodeString;
      HValueToUTF8String(argOne, toEncodeString);
      CFX_ByteString identifyString;
      HValueToUTF8String(argTwo, identifyString);
      CFX_ByteTextBuf resultBuf;
      if (identifyString.EqualNoCase("html")) {
        EncodeHTML(toEncodeString, resultBuf);
      } else if (identifyString.EqualNoCase("xml")) {
        EncodeXML(toEncodeString, resultBuf);
      } else {
        EncodeURL(toEncodeString, resultBuf);
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                resultBuf.GetByteString());
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Encode");
  }
}
void CXFA_FM2JSContext::EncodeURL(const CFX_ByteStringC& szURLString,
                                  CFX_ByteTextBuf& szResultBuf) {
  CFX_WideString wsURLString =
      CFX_WideString::FromUTF8(szURLString.GetCStr(), szURLString.GetLength());
  CFX_WideTextBuf wsResultBuf;
  FX_WCHAR ch = 0;
  int32_t iLength = wsURLString.GetLength();
  FX_WCHAR strEncode[4];
  strEncode[0] = '%';
  strEncode[3] = 0;
  FX_WCHAR strUnsafe[] = {' ', '<',  '>', '"', '#', '%', '{', '}',
                          '|', '\\', '^', '~', '[', ']', '`'};
  FX_WCHAR strReserved[] = {';', '/', '?', ':', '@', '=', '&'};
  FX_WCHAR strSpecial[] = {'$', '-', '+', '!', '*', '\'', '(', ')', ','};
  const FX_WCHAR* strCode = L"0123456789abcdef";
  for (int32_t u = 0; u < iLength; ++u) {
    ch = wsURLString.GetAt(u);
    int32_t i = 0;
    int32_t iCount = sizeof(strUnsafe) / sizeof(strUnsafe[0]);
    while (i < iCount) {
      if (ch == strUnsafe[i]) {
        int32_t iIndex = ch / 16;
        strEncode[1] = strCode[iIndex];
        strEncode[2] = strCode[ch - iIndex * 16];
        wsResultBuf << FX_WSTRC(strEncode);
        break;
      }
      ++i;
    }
    if (i < iCount) {
      continue;
    }
    i = 0;
    iCount = sizeof(strReserved) / sizeof(strReserved[0]);
    while (i < iCount) {
      if (ch == strReserved[i]) {
        int32_t iIndex = ch / 16;
        strEncode[1] = strCode[iIndex];
        strEncode[2] = strCode[ch - iIndex * 16];
        wsResultBuf << FX_WSTRC(strEncode);
        break;
      }
      ++i;
    }
    if (i < iCount) {
      continue;
    }
    i = 0;
    iCount = sizeof(strSpecial) / sizeof(strSpecial[0]);
    while (i < iCount) {
      if (ch == strSpecial[i]) {
        wsResultBuf.AppendChar(ch);
        break;
      }
      ++i;
    }
    if (i < iCount) {
      continue;
    }
    if (ch >= 0x80 && ch <= 0xff) {
      int32_t iIndex = ch / 16;
      strEncode[1] = strCode[iIndex];
      strEncode[2] = strCode[ch - iIndex * 16];
      wsResultBuf << FX_WSTRC(strEncode);
    } else if ((ch >= 0x0 && ch <= 0x1f) || ch == 0x7f) {
      int32_t iIndex = ch / 16;
      strEncode[1] = strCode[iIndex];
      strEncode[2] = strCode[ch - iIndex * 16];
      wsResultBuf << FX_WSTRC(strEncode);
    } else if (ch >= 0x20 && ch <= 0x7e) {
      wsResultBuf.AppendChar(ch);
    } else {
      int32_t iRadix = 16;
      CFX_WideString strTmp;
      while (ch >= iRadix) {
        FX_WCHAR tmp = strCode[ch % iRadix];
        ch /= iRadix;
        strTmp += tmp;
      }
      strTmp += strCode[ch];
      int32_t iLen = strTmp.GetLength();
      if (iLen < 2) {
        break;
      }
      int32_t iIndex = 0;
      if (iLen % 2 != 0) {
        strEncode[1] = '0';
        strEncode[2] = strTmp.GetAt(iLen - 1);
        iIndex = iLen - 2;
      } else {
        strEncode[1] = strTmp.GetAt(iLen - 1);
        strEncode[2] = strTmp.GetAt(iLen - 2);
        iIndex = iLen - 3;
      }
      wsResultBuf << FX_WSTRC(strEncode);
      while (iIndex > 0) {
        strEncode[1] = strTmp.GetAt(iIndex);
        strEncode[2] = strTmp.GetAt(iIndex - 1);
        iIndex -= 2;
        wsResultBuf << FX_WSTRC(strEncode);
      }
    }
  }
  wsResultBuf.AppendChar(0);
  szResultBuf = FX_UTF8Encode(wsResultBuf.GetBuffer(), wsResultBuf.GetLength());
}
void CXFA_FM2JSContext::EncodeHTML(const CFX_ByteStringC& szHTMLString,
                                   CFX_ByteTextBuf& szResultBuf) {
  CFX_ByteString str = szHTMLString.GetCStr();
  CFX_WideString wsHTMLString = CFX_WideString::FromUTF8(str, str.GetLength());
  const FX_WCHAR* strCode = L"0123456789abcdef";
  FX_WCHAR strEncode[9];
  strEncode[0] = '&';
  strEncode[1] = '#';
  strEncode[2] = 'x';
  strEncode[5] = ';';
  strEncode[6] = 0;
  strEncode[7] = ';';
  strEncode[8] = 0;
  CFX_WideTextBuf wsResultBuf;
  uint32_t ch = 0;
  int32_t iLen = wsHTMLString.GetLength();
  int32_t i = 0;
  const FX_WCHAR* pData = wsHTMLString;
  int32_t iIndex = 0;
  CFX_WideString htmlReserve;
  while (i < iLen) {
    ch = *(pData + i);
    htmlReserve.Empty();
    if (HTMLCode2STR(ch, htmlReserve)) {
      wsResultBuf.AppendChar(L'&');
      wsResultBuf << htmlReserve;
      wsResultBuf.AppendChar(L';');
    } else {
      if (ch >= 32 && ch <= 126) {
        wsResultBuf.AppendChar((FX_WCHAR)ch);
      } else if (ch < 256) {
        iIndex = ch / 16;
        strEncode[3] = strCode[iIndex];
        strEncode[4] = strCode[ch - iIndex * 16];
        strEncode[5] = ';';
        strEncode[6] = 0;
        wsResultBuf << FX_WSTRC(strEncode);
      } else {
        int32_t iBigByte = ch / 256;
        int32_t iLittleByte = ch % 256;
        strEncode[3] = strCode[iBigByte / 16];
        strEncode[4] = strCode[iBigByte % 16];
        strEncode[5] = strCode[iLittleByte / 16];
        strEncode[6] = strCode[iLittleByte % 16];
        wsResultBuf << FX_WSTRC(strEncode);
      }
    }
    ++i;
  }
  wsResultBuf.AppendChar(0);
  szResultBuf = FX_UTF8Encode(wsResultBuf.GetBuffer(), wsResultBuf.GetLength());
}
void CXFA_FM2JSContext::EncodeXML(const CFX_ByteStringC& szXMLString,
                                  CFX_ByteTextBuf& szResultBuf) {
  CFX_WideString wsXMLString =
      CFX_WideString::FromUTF8(szXMLString.GetCStr(), szXMLString.GetLength());
  CFX_WideTextBuf wsResultBuf;
  enum {
    QUOT,
    AMP,
    APOS,
    LT,
    GT,
  };
  FX_WCHAR strEncode[9];
  strEncode[0] = '&';
  strEncode[1] = '#';
  strEncode[2] = 'x';
  strEncode[5] = ';';
  strEncode[6] = 0;
  strEncode[7] = ';';
  strEncode[8] = 0;
  const FX_WCHAR* const strName[] = {L"quot", L"amp", L"apos", L"lt", L"gt"};
  const FX_WCHAR* strCode = L"0123456789abcdef";
  FX_WCHAR ch = 0;
  int32_t iLength = wsXMLString.GetLength();
  int32_t iIndex = 0;
  int32_t u = 0;
  const FX_WCHAR* pData = wsXMLString;
  for (u = 0; u < iLength; ++u) {
    ch = *(pData + u);
    switch (ch) {
      case '"':
        wsResultBuf.AppendChar('&');
        wsResultBuf << CFX_WideStringC(strName[QUOT]);
        wsResultBuf.AppendChar(';');
        break;
      case '&':
        wsResultBuf.AppendChar('&');
        wsResultBuf << CFX_WideStringC(strName[AMP]);
        wsResultBuf.AppendChar(';');
        break;
      case '\'':
        wsResultBuf.AppendChar('&');
        wsResultBuf << CFX_WideStringC(strName[APOS]);
        wsResultBuf.AppendChar(';');
        break;
      case '<':
        wsResultBuf.AppendChar('&');
        wsResultBuf << CFX_WideStringC(strName[LT]);
        wsResultBuf.AppendChar(';');
        break;
      case '>':
        wsResultBuf.AppendChar('&');
        wsResultBuf << CFX_WideStringC(strName[GT]);
        wsResultBuf.AppendChar(';');
        break;
      default: {
        if (ch >= 32 && ch <= 126) {
          wsResultBuf.AppendChar(ch);
        } else if (ch < 256) {
          iIndex = ch / 16;
          strEncode[3] = strCode[iIndex];
          strEncode[4] = strCode[ch - iIndex * 16];
          strEncode[5] = ';';
          strEncode[6] = 0;
          wsResultBuf << FX_WSTRC(strEncode);
        } else {
          int32_t iBigByte = ch / 256;
          int32_t iLittleByte = ch % 256;
          strEncode[3] = strCode[iBigByte / 16];
          strEncode[4] = strCode[iBigByte % 16];
          strEncode[5] = strCode[iLittleByte / 16];
          strEncode[6] = strCode[iLittleByte % 16];
          wsResultBuf << FX_WSTRC(strEncode);
        }
      } break;
    }
  }
  wsResultBuf.AppendChar(0);
  szResultBuf = FX_UTF8Encode(wsResultBuf.GetBuffer(), wsResultBuf.GetLength());
}
FX_BOOL CXFA_FM2JSContext::HTMLSTR2Code(const CFX_WideStringC& pData,
                                        uint32_t& iCode) {
  int32_t iLength = pData.GetLength();
  uint32_t uHash = FX_HashCode_String_GetW(pData.GetPtr(), iLength);
  XFA_FMHtmlHashedReserveCode htmlhashedreservecode;
  int32_t iStart = 0,
          iEnd = (sizeof(reservesForDecode) / sizeof(reservesForDecode[0])) - 1;
  int32_t iMid = (iStart + iEnd) / 2;
  do {
    iMid = (iStart + iEnd) / 2;
    htmlhashedreservecode = reservesForDecode[iMid];
    if (uHash == htmlhashedreservecode.m_uHash) {
      iCode = htmlhashedreservecode.m_uCode;
      return TRUE;
    } else if (uHash < htmlhashedreservecode.m_uHash) {
      iEnd = iMid - 1;
    } else {
      iStart = iMid + 1;
    }
  } while (iStart <= iEnd);
  return FALSE;
}
FX_BOOL CXFA_FM2JSContext::HTMLCode2STR(uint32_t iCode,
                                        CFX_WideString& wsHTMLReserve) {
  XFA_FMHtmlReserveCode htmlreservecode;
  int32_t iStart = 0,
          iEnd = (sizeof(reservesForEncode) / sizeof(reservesForEncode[0])) - 1;
  int32_t iMid = (iStart + iEnd) / 2;
  do {
    iMid = (iStart + iEnd) / 2;
    htmlreservecode = reservesForEncode[iMid];
    if (iCode == htmlreservecode.m_uCode) {
      wsHTMLReserve = htmlreservecode.m_htmlReserve;
      return TRUE;
    } else if (iCode < htmlreservecode.m_uCode) {
      iEnd = iMid - 1;
    } else {
      iStart = iMid + 1;
    }
  } while (iStart <= iEnd);
  return FALSE;
}
static FX_BOOL XFA_PATTERN_STRING_Type(const CFX_ByteStringC& szPattern,
                                       FX_DWORD& patternType) {
  CFX_WideString wsPattern =
      CFX_WideString::FromUTF8(szPattern.GetCStr(), szPattern.GetLength());
  if (FX_WSTRC(L"datetime") == wsPattern.Left(8)) {
    patternType = XFA_VT_DATETIME;
    return TRUE;
  } else if (FX_WSTRC(L"date") == wsPattern.Left(4)) {
    patternType = wsPattern.Find(L"time") > 0 ? XFA_VT_DATETIME : XFA_VT_DATE;
    return TRUE;
  } else if (FX_WSTRC(L"time") == wsPattern.Left(4)) {
    patternType = XFA_VT_TIME;
    return TRUE;
  } else if (FX_WSTRC(L"text") == wsPattern.Left(4)) {
    patternType = XFA_VT_TEXT;
    return TRUE;
  } else if (FX_WSTRC(L"num") == wsPattern.Left(3)) {
    if (FX_WSTRC(L"integer") == wsPattern.Mid(4, 7)) {
      patternType = XFA_VT_INTEGER;
    } else if (FX_WSTRC(L"decimal") == wsPattern.Mid(4, 7)) {
      patternType = XFA_VT_DECIMAL;
    } else if (FX_WSTRC(L"currency") == wsPattern.Mid(4, 8)) {
      patternType = XFA_VT_FLOAT;
    } else if (FX_WSTRC(L"percent") == wsPattern.Mid(4, 7)) {
      patternType = XFA_VT_FLOAT;
    } else {
      patternType = XFA_VT_FLOAT;
    }
    return TRUE;
  }
  patternType = XFA_VT_NULL;
  wsPattern.MakeLower();
  const FX_WCHAR* pData = wsPattern;
  int32_t iLength = wsPattern.GetLength();
  int32_t iIndex = 0;
  FX_BOOL bSingleQuotation = FALSE;
  FX_WCHAR patternChar;
  while (iIndex < iLength) {
    patternChar = *(pData + iIndex);
    if (patternChar == 0x27) {
      bSingleQuotation = !bSingleQuotation;
    } else if (!bSingleQuotation &&
               (patternChar == 'y' || patternChar == 'j')) {
      patternType = XFA_VT_DATE;
      iIndex++;
      FX_WCHAR timePatternChar;
      while (iIndex < iLength) {
        timePatternChar = *(pData + iIndex);
        if (timePatternChar == 0x27) {
          bSingleQuotation = !bSingleQuotation;
        } else if (!bSingleQuotation && timePatternChar == 't') {
          patternType = XFA_VT_DATETIME;
          break;
        }
        iIndex++;
      }
      break;
    } else if (!bSingleQuotation &&
               (patternChar == 'h' || patternChar == 'k')) {
      patternType = XFA_VT_TIME;
      break;
    } else if (!bSingleQuotation &&
               (patternChar == 'a' || patternChar == 'x' ||
                patternChar == 'o' || patternChar == '0')) {
      patternType = XFA_VT_TEXT;
      if (patternChar == 'x' || patternChar == 'o' || patternChar == '0') {
        break;
      }
    } else if (!bSingleQuotation &&
               (patternChar == 'z' || patternChar == 's' ||
                patternChar == 'e' || patternChar == 'v' ||
                patternChar == '8' || patternChar == ',' ||
                patternChar == '.' || patternChar == '$')) {
      patternType = XFA_VT_FLOAT;
      if (patternChar == 'v' || patternChar == '8' || patternChar == '$') {
        break;
      }
    }
    iIndex++;
  }
  if (patternType == XFA_VT_NULL) {
    patternType = XFA_VT_TEXT | XFA_VT_FLOAT;
  }
  return FALSE;
}
void CXFA_FM2JSContext::Format(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc >= 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    CFX_ByteString szPattern;
    HValueToUTF8String(argOne, szPattern);
    CFX_ByteString szValue;
    HValueToUTF8String(argTwo, szValue);
    CXFA_Document* pDoc = pContext->GetDocument();
    IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
    CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
    FXSYS_assert(pThisNode->IsNode());
    CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
    IFX_Locale* pLocale = widgetData.GetLocal();
    FX_DWORD patternType;
    FX_BOOL bCompelte = XFA_PATTERN_STRING_Type(szPattern, patternType);
    CFX_WideString wsPattern =
        CFX_WideString::FromUTF8(szPattern, szPattern.GetLength());
    CFX_WideString wsValue =
        CFX_WideString::FromUTF8(szValue, szValue.GetLength());
    if (!bCompelte) {
      switch (patternType) {
        case XFA_VT_DATETIME: {
          FX_STRSIZE iTChar = wsPattern.Find(L'T');
          CFX_WideString wsDatePattern = FX_WSTRC(L"date{");
          wsDatePattern += wsPattern.Left(iTChar);
          wsDatePattern += FX_WSTRC(L"} ");
          CFX_WideString wsTimePattern = FX_WSTRC(L"time{");
          wsTimePattern += wsPattern.Mid(iTChar + 1);
          wsTimePattern += FX_WSTRC(L"}");
          wsPattern = wsDatePattern + wsTimePattern;
        } break;
        case XFA_VT_DATE: {
          wsPattern = FX_WSTRC(L"date{") + wsPattern;
          wsPattern += FX_WSTRC(L"}");
        } break;
        case XFA_VT_TIME: {
          wsPattern = FX_WSTRC(L"time{") + wsPattern;
          wsPattern += FX_WSTRC(L"}");
        } break;
        case XFA_VT_TEXT: {
          wsPattern = FX_WSTRC(L"text{") + wsPattern;
          wsPattern += FX_WSTRC(L"}");
        } break;
        case XFA_VT_FLOAT: {
          wsPattern = FX_WSTRC(L"num{") + wsPattern;
          wsPattern += FX_WSTRC(L"}");
        } break;
        default: {
          CFX_WideString wsTestPattern;
          wsTestPattern = FX_WSTRC(L"num{") + wsPattern;
          wsTestPattern += FX_WSTRC(L"}");
          CXFA_LocaleValue tempLocaleValue(XFA_VT_FLOAT, wsValue, wsTestPattern,
                                           pLocale, (CXFA_LocaleMgr*)pMgr);
          if (tempLocaleValue.IsValid()) {
            wsPattern = wsTestPattern;
            patternType = XFA_VT_FLOAT;
          } else {
            wsTestPattern = FX_WSTRC(L"text{") + wsPattern;
            wsTestPattern += FX_WSTRC(L"}");
            wsPattern = wsTestPattern;
            patternType = XFA_VT_TEXT;
          }
        } break;
      }
    }
    CXFA_LocaleValue localeValue(patternType, wsValue, wsPattern, pLocale,
                                 (CXFA_LocaleMgr*)pMgr);
    CFX_WideString wsRet;
    if (localeValue.FormatPatterns(wsRet, wsPattern, pLocale,
                                   XFA_VALUEPICTURE_Display)) {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                FX_UTF8Encode(wsRet, wsRet.GetLength()));
    } else {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Format");
  }
}
void CXFA_FM2JSContext::Left(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FX_BOOL argIsNull = FALSE;
    if ((HValueIsNull(hThis, argOne)) || (HValueIsNull(hThis, argTwo))) {
      argIsNull = TRUE;
    }
    if (argIsNull) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString sourceString;
      HValueToUTF8String(argOne, sourceString);
      int32_t count = HValueToInteger(hThis, argTwo);
      if (count < 0) {
        count = 0;
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                sourceString.Left(count));
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Left");
  }
}
void CXFA_FM2JSContext::Len(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString sourceString;
      HValueToUTF8String(argOne, sourceString);
      FXJSE_Value_SetInteger(args.GetReturnValue(), sourceString.GetLength());
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Len");
  }
}
void CXFA_FM2JSContext::Lower(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 3)) {
    CFX_ByteString argString;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE localeValue = 0;
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      if (argc == 2) {
        localeValue = GetSimpleHValue(hThis, args, 1);
      }
      HValueToUTF8String(argOne, argString);
      CFX_WideTextBuf lowStringBuf;
      CFX_WideString wsArgString =
          CFX_WideString::FromUTF8(argString, argString.GetLength());
      const FX_WCHAR* pData = wsArgString;
      int32_t iLen = argString.GetLength();
      int32_t i = 0;
      int32_t ch = 0;
      while (i < iLen) {
        ch = *(pData + i);
        if (ch >= 0x41 && ch <= 0x5A) {
          ch += 32;
        } else if (ch >= 0xC0 && ch <= 0xDE) {
          ch += 32;
        } else if (ch == 0x100 || ch == 0x102 || ch == 0x104) {
          ch += 1;
        }
        lowStringBuf.AppendChar(ch);
        ++i;
      }
      lowStringBuf.AppendChar(0);
      FXJSE_Value_SetUTF8String(
          args.GetReturnValue(),
          FX_UTF8Encode(lowStringBuf.GetBuffer(), lowStringBuf.GetLength()));
      if (argc == 2) {
        FXJSE_Value_Release(localeValue);
      }
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Lower");
  }
}
void CXFA_FM2JSContext::Ltrim(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString sourceString;
      HValueToUTF8String(argOne, sourceString);
      sourceString.TrimLeft();
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), sourceString);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Ltrim");
  }
}
void CXFA_FM2JSContext::Parse(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    if (HValueIsNull(hThis, argTwo)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString szPattern;
      HValueToUTF8String(argOne, szPattern);
      CFX_ByteString szValue;
      HValueToUTF8String(argTwo, szValue);
      CXFA_Document* pDoc = pContext->GetDocument();
      IFX_LocaleMgr* pMgr = (IFX_LocaleMgr*)pDoc->GetLocalMgr();
      CXFA_Object* pThisNode = pDoc->GetScriptContext()->GetThisObject();
      FXSYS_assert(pThisNode->IsNode());
      CXFA_WidgetData widgetData((CXFA_Node*)pThisNode);
      IFX_Locale* pLocale = widgetData.GetLocal();
      FX_DWORD patternType;
      FX_BOOL bCompletePattern =
          XFA_PATTERN_STRING_Type(szPattern, patternType);
      CFX_WideString wsPattern =
          CFX_WideString::FromUTF8(szPattern, szPattern.GetLength());
      CFX_WideString wsValue =
          CFX_WideString::FromUTF8(szValue, szValue.GetLength());
      CFX_ByteString szParsedValue;
      if (bCompletePattern) {
        CXFA_LocaleValue localeValue(patternType, wsValue, wsPattern, pLocale,
                                     (CXFA_LocaleMgr*)pMgr);
        if (localeValue.IsValid()) {
          szParsedValue = FX_UTF8Encode(localeValue.GetValue());
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
        } else {
          FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
        }
      } else {
        switch (patternType) {
          case XFA_VT_DATETIME: {
            FX_STRSIZE iTChar = wsPattern.Find(L'T');
            CFX_WideString wsDatePattern = FX_WSTRC(L"date{");
            wsDatePattern += wsPattern.Left(iTChar);
            wsDatePattern += FX_WSTRC(L"} ");
            CFX_WideString wsTimePattern = FX_WSTRC(L"time{");
            wsTimePattern += wsPattern.Mid(iTChar + 1);
            wsTimePattern += FX_WSTRC(L"}");
            wsPattern = wsDatePattern + wsTimePattern;
            CXFA_LocaleValue localeValue(patternType, wsValue, wsPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              szParsedValue = FX_UTF8Encode(localeValue.GetValue());
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
            } else {
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
            }
          } break;
          case XFA_VT_DATE: {
            wsPattern = FX_WSTRC(L"date{") + wsPattern;
            wsPattern += FX_WSTRC(L"}");
            CXFA_LocaleValue localeValue(patternType, wsValue, wsPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              szParsedValue = FX_UTF8Encode(localeValue.GetValue());
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
            } else {
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
            }
          } break;
          case XFA_VT_TIME: {
            wsPattern = FX_WSTRC(L"time{") + wsPattern;
            wsPattern += FX_WSTRC(L"}");
            CXFA_LocaleValue localeValue(patternType, wsValue, wsPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              szParsedValue = FX_UTF8Encode(localeValue.GetValue());
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
            } else {
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
            }
          } break;
          case XFA_VT_TEXT: {
            wsPattern = FX_WSTRC(L"text{") + wsPattern;
            wsPattern += FX_WSTRC(L"}");
            CXFA_LocaleValue localeValue(XFA_VT_TEXT, wsValue, wsPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              szParsedValue = FX_UTF8Encode(localeValue.GetValue());
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
            } else {
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
            }
          } break;
          case XFA_VT_FLOAT: {
            wsPattern = FX_WSTRC(L"num{") + wsPattern;
            wsPattern += FX_WSTRC(L"}");
            CXFA_LocaleValue localeValue(XFA_VT_FLOAT, wsValue, wsPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              FXJSE_Value_SetDouble(args.GetReturnValue(),
                                    localeValue.GetDoubleNum());
            } else {
              FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
            }
          } break;
          default: {
            CFX_WideString wsTestPattern;
            wsTestPattern = FX_WSTRC(L"num{") + wsPattern;
            wsTestPattern += FX_WSTRC(L"}");
            CXFA_LocaleValue localeValue(XFA_VT_FLOAT, wsValue, wsTestPattern,
                                         pLocale, (CXFA_LocaleMgr*)pMgr);
            if (localeValue.IsValid()) {
              FXJSE_Value_SetDouble(args.GetReturnValue(),
                                    localeValue.GetDoubleNum());
            } else {
              wsTestPattern = FX_WSTRC(L"text{") + wsPattern;
              wsTestPattern += FX_WSTRC(L"}");
              CXFA_LocaleValue localeValue(XFA_VT_TEXT, wsValue, wsTestPattern,
                                           pLocale, (CXFA_LocaleMgr*)pMgr);
              if (localeValue.IsValid()) {
                szParsedValue = FX_UTF8Encode(localeValue.GetValue());
                FXJSE_Value_SetUTF8String(args.GetReturnValue(), szParsedValue);
              } else {
                FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
              }
            }
          } break;
        }
      }
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Parse");
  }
}
void CXFA_FM2JSContext::Replace(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc == 2) || (argc == 3)) {
    FX_BOOL bFlags = FALSE;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE argThree = 0;
    CFX_ByteString oneString;
    CFX_ByteString twoString;
    CFX_ByteString threeString;
    if ((HValueIsNull(hThis, argOne)) || (HValueIsNull(hThis, argTwo))) {
      bFlags = TRUE;
    } else {
      HValueToUTF8String(argOne, oneString);
      HValueToUTF8String(argTwo, twoString);
    }
    if (argc == 3) {
      argThree = GetSimpleHValue(hThis, args, 2);
      HValueToUTF8String(argThree, threeString);
    }
    int32_t iSrcLen = oneString.GetLength();
    int32_t iFindLen = twoString.GetLength();
    CFX_ByteTextBuf resultString;
    int32_t iFindIndex = 0;
    uint8_t ch = 0;
    for (int32_t u = 0; u < iSrcLen; ++u) {
      ch = oneString.GetAt(u);
      if (ch == twoString.GetAt(iFindIndex)) {
        int32_t iTemp = u + 1;
        ++iFindIndex;
        uint8_t chTemp = 0;
        while (iFindIndex < iFindLen) {
          chTemp = oneString.GetAt(iTemp);
          if (chTemp == twoString.GetAt(iFindIndex)) {
            ++iTemp;
            ++iFindIndex;
          } else {
            iFindIndex = 0;
            break;
          }
        }
        if (iFindIndex == iFindLen) {
          resultString << threeString;
          u += iFindLen - 1;
          iFindIndex = 0;
        } else {
          resultString.AppendChar(ch);
        }
      } else {
        resultString.AppendChar(ch);
      }
    }
    resultString.AppendChar(0);
    FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                              resultString.GetByteString());
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    if (argc == 3) {
      FXJSE_Value_Release(argThree);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Replace");
  }
}
void CXFA_FM2JSContext::Right(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argTwo = GetSimpleHValue(hThis, args, 1);
    FX_BOOL argIsNull = FALSE;
    if ((HValueIsNull(hThis, argOne)) || (HValueIsNull(hThis, argTwo))) {
      argIsNull = TRUE;
    }
    if (argIsNull) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString sourceString;
      HValueToUTF8String(argOne, sourceString);
      int32_t count = HValueToInteger(hThis, argTwo);
      if (count < 0) {
        count = 0;
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                sourceString.Right(count));
    }
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Right");
  }
}
void CXFA_FM2JSContext::Rtrim(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString sourceString;
      HValueToUTF8String(argOne, sourceString);
      sourceString.TrimRight();
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), sourceString);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Rtrim");
  }
}
void CXFA_FM2JSContext::Space(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      int32_t count = 0;
      count = HValueToInteger(hThis, argOne);
      count = (count < 0) ? 0 : count;
      CFX_ByteTextBuf spaceString;
      int32_t index = 0;
      while (index < count) {
        spaceString.AppendByte(' ');
        index++;
      }
      spaceString.AppendByte(0);
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                spaceString.GetByteString());
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Space");
  }
}
void CXFA_FM2JSContext::Str(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT fNumber;
    int32_t iWidth = 10;
    int32_t iPrecision = 0;
    FXJSE_HVALUE numberValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE widthValue = 0;
    FXJSE_HVALUE precisionValue = 0;
    if (FXJSE_Value_IsNull(numberValue)) {
      bFlags = TRUE;
    } else {
      fNumber = HValueToFloat(hThis, numberValue);
    }
    if (argc > 1) {
      widthValue = GetSimpleHValue(hThis, args, 1);
      iWidth = (int32_t)HValueToFloat(hThis, widthValue);
    }
    if (argc == 3) {
      precisionValue = GetSimpleHValue(hThis, args, 2);
      iPrecision = (int32_t)HValueToFloat(hThis, precisionValue);
      if (iPrecision < 0) {
        iPrecision = 0;
      }
    }
    if (!bFlags) {
      CFX_ByteString numberString;
      CFX_ByteString formatStr = "%";
      if (iPrecision) {
        formatStr += ".";
        formatStr += CFX_ByteString::FormatInteger(iPrecision);
      }
      formatStr += "f";
      numberString.Format(formatStr, fNumber);
      const FX_CHAR* pData = numberString;
      int32_t iLength = numberString.GetLength();
      int32_t u = 0;
      while (u < iLength) {
        if (*(pData + u) == '.') {
          break;
        }
        ++u;
      }
      CFX_ByteTextBuf resultBuf;
      if (u > iWidth || (iPrecision + u) >= iWidth) {
        int32_t i = 0;
        while (i < iWidth) {
          resultBuf.AppendChar('*');
          ++i;
        }
        resultBuf.AppendChar(0);
      } else {
        if (u == iLength) {
          if (iLength > iWidth) {
            int32_t i = 0;
            while (i < iWidth) {
              resultBuf.AppendChar('*');
              ++i;
            }
          } else {
            int32_t i = 0;
            int32_t iSpace = iWidth - iLength;
            while (i < iSpace) {
              resultBuf.AppendChar(' ');
              ++i;
            }
            resultBuf << pData;
          }
        } else {
          int32_t iLeavingSpace = 0;
          if (iPrecision == 0) {
            iLeavingSpace = iWidth - (u + iPrecision);
          } else {
            iLeavingSpace = iWidth - (u + iPrecision + 1);
          }
          int32_t i = 0;
          while (i < iLeavingSpace) {
            resultBuf.AppendChar(' ');
            ++i;
          }
          i = 0;
          while (i < u) {
            resultBuf.AppendChar(*(pData + i));
            ++i;
          }
          if (iPrecision != 0) {
            resultBuf.AppendChar('.');
          }
          u++;
          i = 0;
          while (u < iLength) {
            if (i >= iPrecision) {
              break;
            }
            resultBuf.AppendChar(*(pData + u));
            ++i;
            ++u;
          }
          while (i < iPrecision) {
            resultBuf.AppendChar('0');
            ++i;
          }
          resultBuf.AppendChar(0);
        }
      }
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                resultBuf.GetByteString());
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(numberValue);
    if (argc > 1) {
      FXJSE_Value_Release(widthValue);
      if (argc == 3) {
        FXJSE_Value_Release(precisionValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Str");
  }
}
void CXFA_FM2JSContext::Stuff(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc == 3) || (argc == 4)) {
    FX_BOOL bFlags = FALSE;
    CFX_ByteString sourceString;
    CFX_ByteString insertString;
    int32_t iLength = 0;
    int32_t iStart = 0;
    int32_t iDelete = 0;
    FXJSE_HVALUE sourceValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE startValue = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE deleteValue = GetSimpleHValue(hThis, args, 2);
    FXJSE_HVALUE insertValue = 0;
    if ((FXJSE_Value_IsNull(sourceValue)) || (FXJSE_Value_IsNull(startValue)) ||
        (FXJSE_Value_IsNull(deleteValue))) {
      bFlags = TRUE;
    } else {
      HValueToUTF8String(sourceValue, sourceString);
      iLength = sourceString.GetLength();
      iStart = (int32_t)HValueToFloat(hThis, startValue);
      if (iStart < 1) {
        iStart = 1;
      }
      if (iStart > iLength) {
        iStart = iLength;
      }
      iDelete = (int32_t)HValueToFloat(hThis, deleteValue);
      if (iDelete <= 0) {
        iDelete = 0;
      }
    }
    if (argc == 4) {
      insertValue = GetSimpleHValue(hThis, args, 3);
      HValueToUTF8String(insertValue, insertString);
    }
    iStart -= 1;
    CFX_ByteTextBuf resultString;
    int32_t i = 0;
    while (i < iStart) {
      resultString.AppendChar(sourceString.GetAt(i));
      ++i;
    }
    resultString << insertString;
    i = iStart + iDelete;
    while (i < iLength) {
      resultString.AppendChar(sourceString.GetAt(i));
      ++i;
    }
    resultString.AppendChar(0);
    FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                              resultString.GetByteString());
    FXJSE_Value_Release(sourceValue);
    FXJSE_Value_Release(startValue);
    FXJSE_Value_Release(deleteValue);
    if (argc == 4) {
      FXJSE_Value_Release(insertValue);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Stuff");
  }
}
void CXFA_FM2JSContext::Substr(FXJSE_HOBJECT hThis,
                               const CFX_ByteStringC& szFuncName,
                               CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if (argc == 3) {
    FXJSE_HVALUE stringValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE startValue = GetSimpleHValue(hThis, args, 1);
    FXJSE_HVALUE endValue = GetSimpleHValue(hThis, args, 2);
    if (HValueIsNull(hThis, stringValue) || (HValueIsNull(hThis, startValue)) ||
        (HValueIsNull(hThis, endValue))) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      CFX_ByteString szSourceStr;
      int32_t iStart = 0;
      int32_t iCount = 0;
      HValueToUTF8String(stringValue, szSourceStr);
      int32_t iLength = szSourceStr.GetLength();
      if (iLength == 0) {
        FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
      } else {
        iStart = (int32_t)HValueToFloat(hThis, startValue);
        iCount = (int32_t)HValueToFloat(hThis, endValue);
        if (iStart < 1) {
          iStart = 1;
        }
        if (iStart > iLength) {
          iStart = iLength;
        }
        if (iCount <= 0) {
          iCount = 0;
        }
        iStart -= 1;
        FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                  szSourceStr.Mid(iStart, iCount));
      }
    }
    FXJSE_Value_Release(stringValue);
    FXJSE_Value_Release(startValue);
    FXJSE_Value_Release(endValue);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Substr");
  }
}
void CXFA_FM2JSContext::Uuid(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc == 0) || (argc == 1)) {
    int32_t iNum = 0;
    FXJSE_HVALUE argOne = 0;
    if (argc == 1) {
      argOne = GetSimpleHValue(hThis, args, 0);
      iNum = (int32_t)HValueToFloat(hThis, argOne);
    }
    FX_GUID guid;
    FX_GUID_CreateV4(&guid);
    CFX_ByteString bsUId;
    FX_GUID_ToString(&guid, bsUId, iNum);
    FXJSE_Value_SetUTF8String(args.GetReturnValue(), bsUId);
    if (argc == 1) {
      FXJSE_Value_Release(argOne);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Uuid");
  }
}
void CXFA_FM2JSContext::Upper(FXJSE_HOBJECT hThis,
                              const CFX_ByteStringC& szFuncName,
                              CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 3)) {
    CFX_ByteString argString;
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE localeValue = 0;
    if (HValueIsNull(hThis, argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      if (argc == 2) {
        localeValue = GetSimpleHValue(hThis, args, 1);
      }
      HValueToUTF8String(argOne, argString);
      CFX_WideTextBuf upperStringBuf;
      CFX_WideString wsArgString =
          CFX_WideString::FromUTF8(argString, argString.GetLength());
      const FX_WCHAR* pData = wsArgString;
      int32_t iLen = wsArgString.GetLength();
      int32_t i = 0;
      int32_t ch = 0;
      while (i < iLen) {
        ch = *(pData + i);
        if (ch >= 0x61 && ch <= 0x7A) {
          ch -= 32;
        } else if (ch >= 0xE0 && ch <= 0xFE) {
          ch -= 32;
        } else if (ch == 0x101 || ch == 0x103 || ch == 0x105) {
          ch -= 1;
        }
        upperStringBuf.AppendChar(ch);
        ++i;
      }
      upperStringBuf.AppendChar(0);
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                FX_UTF8Encode(upperStringBuf.GetBuffer(),
                                              upperStringBuf.GetLength()));
      if (argc == 2) {
        FXJSE_Value_Release(localeValue);
      }
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Upper");
  }
}
void CXFA_FM2JSContext::WordNum(FXJSE_HOBJECT hThis,
                                const CFX_ByteStringC& szFuncName,
                                CFXJSE_Arguments& args) {
  int32_t argc = args.GetLength();
  if ((argc > 0) && (argc < 4)) {
    FX_BOOL bFlags = FALSE;
    FX_FLOAT fNumber;
    int32_t iIdentifier = 0;
    CFX_ByteString localeString;
    FXJSE_HVALUE numberValue = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE identifierValue = 0;
    FXJSE_HVALUE localeValue = 0;
    if (FXJSE_Value_IsNull(numberValue)) {
      bFlags = TRUE;
    } else {
      fNumber = HValueToFloat(hThis, numberValue);
    }
    if (argc > 1) {
      identifierValue = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(identifierValue)) {
        bFlags = TRUE;
      } else {
        iIdentifier = (int32_t)HValueToFloat(hThis, identifierValue);
      }
    }
    if (argc == 3) {
      localeValue = GetSimpleHValue(hThis, args, 2);
      if (FXJSE_Value_IsNull(localeValue)) {
        bFlags = TRUE;
      } else {
        HValueToUTF8String(localeValue, localeString);
      }
    }
    if (!bFlags) {
      if ((fNumber < 0) || (fNumber > 922337203685477550)) {
        FXJSE_Value_SetUTF8String(args.GetReturnValue(), "*");
      } else {
        CFX_ByteTextBuf resultBuf;
        CFX_ByteString numberString;
        numberString.Format("%.2f", fNumber);
        WordUS(numberString, iIdentifier, resultBuf);
        FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                  resultBuf.GetByteString());
      }
    } else {
      FXJSE_Value_SetNull(args.GetReturnValue());
    }
    FXJSE_Value_Release(numberValue);
    if (argc > 1) {
      FXJSE_Value_Release(identifierValue);
      if (argc == 3) {
        FXJSE_Value_Release(localeValue);
      }
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"WordNum");
  }
}
void CXFA_FM2JSContext::TrillionUS(const CFX_ByteStringC& szData,
                                   CFX_ByteTextBuf& strBuf) {
  CFX_ByteStringC pUnits[] = {"zero", "one", "two",   "three", "four",
                              "five", "six", "seven", "eight", "nine"};
  CFX_ByteStringC pCapUnits[] = {"Zero", "One", "Two",   "Three", "Four",
                                 "Five", "Six", "Seven", "Eight", "Nine"};
  CFX_ByteStringC pTens[] = {"Ten",      "Eleven",  "Twelve",  "Thirteen",
                             "Fourteen", "Fifteen", "Sixteen", "Seventeen",
                             "Eighteen", "Nineteen"};
  CFX_ByteStringC pLastTens[] = {"Twenty", "Thirty",  "Forty",  "Fifty",
                                 "Sixty",  "Seventy", "Eighty", "Ninety"};
  CFX_ByteStringC pComm[] = {" Hundred ", " Thousand ", " Million ",
                             " Billion ", "Trillion"};
  int32_t iComm = 0;
  const FX_CHAR* pData = szData.GetCStr();
  int32_t iLength = szData.GetLength();
  if (iLength > 12) {
    iComm = 4;
  } else if (iLength > 9) {
    iComm = 3;
  } else if (iLength > 6) {
    iComm = 2;
  } else if (iLength > 3) {
    iComm = 1;
  }
  int32_t iIndex = 0;
  int32_t iFirstCount = iLength % 3;
  if (iFirstCount == 0) {
    iFirstCount = 3;
  }
  if (iFirstCount == 3) {
    if (*(pData + iIndex) != '0') {
      strBuf << pCapUnits[*(pData + iIndex) - '0'];
      strBuf << pComm[0];
    }
    if (*(pData + iIndex + 1) == '0') {
      strBuf << pCapUnits[*(pData + iIndex + 2) - '0'];
    } else {
      if (*(pData + iIndex + 1) > '1') {
        strBuf << pLastTens[*(pData + iIndex + 1) - '2'];
        strBuf << "-";
        strBuf << pUnits[*(pData + iIndex + 2) - '0'];
      } else if (*(pData + iIndex + 1) == '1') {
        strBuf << pTens[*(pData + iIndex + 2) - '0'];
      } else if (*(pData + iIndex + 1) == '0') {
        strBuf << pCapUnits[*(pData + iIndex + 2) - '0'];
      }
    }
    iIndex += 3;
  } else if (iFirstCount == 2) {
    if (*(pData + iIndex) == '0') {
      strBuf << pCapUnits[*(pData + iIndex + 1) - '0'];
    } else {
      if (*(pData + iIndex) > '1') {
        strBuf << pLastTens[*(pData + iIndex) - '2'];
        strBuf << "-";
        strBuf << pUnits[*(pData + iIndex + 1) - '0'];
      } else if (*(pData + iIndex) == '1') {
        strBuf << pTens[*(pData + iIndex + 1) - '0'];
      } else if (*(pData + iIndex) == '0') {
        strBuf << pCapUnits[*(pData + iIndex + 1) - '0'];
      }
    }
    iIndex += 2;
  } else if (iFirstCount == 1) {
    strBuf << pCapUnits[*(pData + iIndex) - '0'];
    iIndex += 1;
  }
  if (iLength > 3 && iFirstCount > 0) {
    strBuf << pComm[iComm];
    --iComm;
  }
  while (iIndex < iLength) {
    if (*(pData + iIndex) != '0') {
      strBuf << pCapUnits[*(pData + iIndex) - '0'];
      strBuf << pComm[0];
    }
    if (*(pData + iIndex + 1) == '0') {
      strBuf << pCapUnits[*(pData + iIndex + 2) - '0'];
    } else {
      if (*(pData + iIndex + 1) > '1') {
        strBuf << pLastTens[*(pData + iIndex + 1) - '2'];
        strBuf << "-";
        strBuf << pUnits[*(pData + iIndex + 2) - '0'];
      } else if (*(pData + iIndex + 1) == '1') {
        strBuf << pTens[*(pData + iIndex + 2) - '0'];
      } else if (*(pData + iIndex + 1) == '0') {
        strBuf << pCapUnits[*(pData + iIndex + 2) - '0'];
      }
    }
    if (iIndex < iLength - 3) {
      strBuf << pComm[iComm];
      --iComm;
    }
    iIndex += 3;
  }
}
void CXFA_FM2JSContext::WordUS(const CFX_ByteStringC& szData,
                               int32_t iStyle,
                               CFX_ByteTextBuf& strBuf) {
  const FX_CHAR* pData = szData.GetCStr();
  int32_t iLength = szData.GetLength();
  switch (iStyle) {
    case 0: {
      int32_t iIndex = 0;
      while (iIndex < iLength) {
        if (*(pData + iIndex) == '.') {
          break;
        }
        ++iIndex;
      }
      iLength = iIndex;
      iIndex = 0;
      int32_t iCount = 0;
      while (iIndex < iLength) {
        iCount = (iLength - iIndex) % 12;
        if (!iCount && iLength - iIndex > 0) {
          iCount = 12;
        }
        TrillionUS(CFX_ByteStringC(pData + iIndex, iCount), strBuf);
        iIndex += iCount;
        if (iIndex < iLength) {
          strBuf << " Trillion ";
        }
      }
    } break;
    case 1: {
      int32_t iIndex = 0;
      while (iIndex < iLength) {
        if (*(pData + iIndex) == '.') {
          break;
        }
        ++iIndex;
      }
      iLength = iIndex;
      iIndex = 0;
      int32_t iCount = 0;
      while (iIndex < iLength) {
        iCount = (iLength - iIndex) % 12;
        if (!iCount && iLength - iIndex > 0) {
          iCount = 12;
        }
        TrillionUS(CFX_ByteStringC(pData + iIndex, iCount), strBuf);
        iIndex += iCount;
        if (iIndex < iLength) {
          strBuf << " Trillion ";
        }
      }
      strBuf << " Dollars";
    } break;
    case 2: {
      int32_t iIndex = 0;
      while (iIndex < iLength) {
        if (*(pData + iIndex) == '.') {
          break;
        }
        ++iIndex;
      }
      int32_t iInteger = iIndex;
      iIndex = 0;
      int32_t iCount = 0;
      while (iIndex < iInteger) {
        iCount = (iInteger - iIndex) % 12;
        if (!iCount && iLength - iIndex > 0) {
          iCount = 12;
        }
        TrillionUS(CFX_ByteStringC(pData + iIndex, iCount), strBuf);
        iIndex += iCount;
        if (iIndex < iInteger) {
          strBuf << " Trillion ";
        }
      }
      strBuf << " Dollars";
      if (iInteger < iLength) {
        strBuf << " And ";
        iIndex = iInteger + 1;
        int32_t iCount = 0;
        while (iIndex < iLength) {
          iCount = (iLength - iIndex) % 12;
          if (!iCount && iLength - iIndex > 0) {
            iCount = 12;
          }
          TrillionUS(CFX_ByteStringC(pData + iIndex, iCount), strBuf);
          iIndex += iCount;
          if (iIndex < iLength) {
            strBuf << " Trillion ";
          }
        }
        strBuf << " Cents";
      }
    } break;
    default:
      break;
  }
}
void CXFA_FM2JSContext::Get(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc == 1) {
    CXFA_Document* pDoc = pContext->GetDocument();
    if (!pDoc) {
      return;
    }
    IXFA_AppProvider* pAppProvider =
        pDoc->GetParser()->GetNotify()->GetAppProvider();
    if (!pAppProvider) {
      return;
    }
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    CFX_ByteString urlString;
    HValueToUTF8String(argOne, urlString);
    IFX_FileRead* pFile = pAppProvider->DownloadURL(
        CFX_WideString::FromUTF8(urlString, urlString.GetLength()));
    if (pFile) {
      int32_t size = pFile->GetSize();
      uint8_t* pData = FX_Alloc(uint8_t, size);
      pFile->ReadBlock(pData, size);
      FXJSE_Value_SetUTF8String(args.GetReturnValue(),
                                CFX_ByteStringC(pData, size));
      FX_Free(pData);
      pFile->Release();
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Get");
  }
}
void CXFA_FM2JSContext::Post(FXJSE_HOBJECT hThis,
                             const CFX_ByteStringC& szFuncName,
                             CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if ((argc >= 2) && (argc <= 5)) {
    CXFA_Document* pDoc = pContext->GetDocument();
    if (!pDoc) {
      return;
    }
    IXFA_AppProvider* pAppProvider =
        pDoc->GetParser()->GetNotify()->GetAppProvider();
    if (!pAppProvider) {
      return;
    }
    CFX_ByteString bsURL;
    CFX_ByteString bsData;
    CFX_ByteString bsContentType;
    CFX_ByteString bsEncode;
    CFX_ByteString bsHeader;
    FXJSE_HVALUE argOne;
    FXJSE_HVALUE argTwo;
    FXJSE_HVALUE argThree;
    FXJSE_HVALUE argFour;
    FXJSE_HVALUE argFive;
    argOne = GetSimpleHValue(hThis, args, 0);
    HValueToUTF8String(argOne, bsURL);
    argTwo = GetSimpleHValue(hThis, args, 1);
    HValueToUTF8String(argTwo, bsData);
    if (argc > 2) {
      argThree = GetSimpleHValue(hThis, args, 2);
      HValueToUTF8String(argThree, bsContentType);
    }
    if (argc > 3) {
      argFour = GetSimpleHValue(hThis, args, 3);
      HValueToUTF8String(argFour, bsEncode);
    }
    if (argc > 4) {
      argFive = GetSimpleHValue(hThis, args, 4);
      HValueToUTF8String(argFive, bsHeader);
    }
    CFX_WideString decodedResponse;
    FX_BOOL bFlags = pAppProvider->PostRequestURL(
        CFX_WideString::FromUTF8(bsURL, bsURL.GetLength()),
        CFX_WideString::FromUTF8(bsData, bsData.GetLength()),
        CFX_WideString::FromUTF8(bsContentType, bsContentType.GetLength()),
        CFX_WideString::FromUTF8(bsEncode, bsEncode.GetLength()),
        CFX_WideString::FromUTF8(bsHeader, bsHeader.GetLength()),
        decodedResponse);
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    if (argc > 2) {
      FXJSE_Value_Release(argThree);
    }
    if (argc > 3) {
      FXJSE_Value_Release(argFour);
    }
    if (argc > 4) {
      FXJSE_Value_Release(argFive);
    }
    if (bFlags) {
      FXJSE_Value_SetUTF8String(
          args.GetReturnValue(),
          FX_UTF8Encode(decodedResponse, decodedResponse.GetLength()));
    } else {
      pContext->ThrowScriptErrorMessage(XFA_IDS_SERVER_DENY);
    }
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Post");
  }
}
void CXFA_FM2JSContext::Put(FXJSE_HOBJECT hThis,
                            const CFX_ByteStringC& szFuncName,
                            CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if ((argc == 2) || (argc == 3)) {
    CXFA_Document* pDoc = pContext->GetDocument();
    if (!pDoc) {
      return;
    }
    IXFA_AppProvider* pAppProvider =
        pDoc->GetParser()->GetNotify()->GetAppProvider();
    if (!pAppProvider) {
      return;
    }
    CFX_ByteString bsURL;
    CFX_ByteString bsData;
    CFX_ByteString bsEncode;
    FXJSE_HVALUE argOne;
    FXJSE_HVALUE argTwo;
    FXJSE_HVALUE argThree;
    argOne = GetSimpleHValue(hThis, args, 0);
    HValueToUTF8String(argOne, bsURL);
    argTwo = GetSimpleHValue(hThis, args, 1);
    HValueToUTF8String(argTwo, bsData);
    if (argc > 2) {
      argThree = GetSimpleHValue(hThis, args, 2);
      HValueToUTF8String(argThree, bsEncode);
    }
    FX_BOOL bFlags = pAppProvider->PutRequestURL(
        CFX_WideString::FromUTF8(bsURL, bsURL.GetLength()),
        CFX_WideString::FromUTF8(bsData, bsData.GetLength()),
        CFX_WideString::FromUTF8(bsEncode, bsEncode.GetLength()));
    FXJSE_Value_Release(argOne);
    FXJSE_Value_Release(argTwo);
    if (argc > 2) {
      FXJSE_Value_Release(argThree);
    }
    if (bFlags) {
      FXJSE_Value_SetUTF8String(args.GetReturnValue(), "");
    } else {
      pContext->ThrowScriptErrorMessage(XFA_IDS_SERVER_DENY);
    }
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Put");
  }
}
void CXFA_FM2JSContext::assign_value_operator(FXJSE_HOBJECT hThis,
                                              const CFX_ByteStringC& szFuncName,
                                              CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  if (args.GetLength() == 2) {
    FXJSE_HVALUE lValue = args.GetValue(0);
    FXJSE_HVALUE rValue = GetSimpleHValue(hThis, args, 1);
    FX_BOOL bSetStatus = TRUE;
    if (FXJSE_Value_IsArray(lValue)) {
      FXJSE_HVALUE leftLengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(lValue, "length", leftLengthValue);
      int32_t iLeftLength = FXJSE_Value_ToInteger(leftLengthValue);
      FXJSE_Value_Release(leftLengthValue);
      FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(lValue, 1, propertyValue);
      if (FXJSE_Value_IsNull(propertyValue)) {
        for (int32_t i = 2; i < iLeftLength; i++) {
          FXJSE_Value_GetObjectPropByIdx(lValue, i, jsObjectValue);
          bSetStatus = SetObjectDefaultValue(jsObjectValue, rValue);
          if (!bSetStatus) {
            pContext->ThrowScriptErrorMessage(XFA_IDS_NOT_DEFAUL_VALUE);
            break;
          }
        }
      } else {
        CFX_ByteString propertyStr;
        FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
        for (int32_t i = 2; i < iLeftLength; i++) {
          FXJSE_Value_GetObjectPropByIdx(lValue, i, jsObjectValue);
          FXJSE_Value_SetObjectProp(jsObjectValue, propertyStr, rValue);
        }
      }
      FXJSE_Value_Release(jsObjectValue);
      FXJSE_Value_Release(propertyValue);
    } else if (FXJSE_Value_IsObject(lValue)) {
      bSetStatus = SetObjectDefaultValue(lValue, rValue);
      if (!bSetStatus) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_NOT_DEFAUL_VALUE);
      }
    }
    FXJSE_Value_Set(args.GetReturnValue(), rValue);
    FXJSE_Value_Release(lValue);
    FXJSE_Value_Release(rValue);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::logical_or_operator(FXJSE_HOBJECT hThis,
                                            const CFX_ByteStringC& szFuncName,
                                            CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_FLOAT first = HValueToFloat(hThis, argFirst);
      FX_FLOAT second = HValueToFloat(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first || second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::logical_and_operator(FXJSE_HOBJECT hThis,
                                             const CFX_ByteStringC& szFuncName,
                                             CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_FLOAT first = HValueToFloat(hThis, argFirst);
      FX_FLOAT second = HValueToFloat(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first && second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::equality_operator(FXJSE_HOBJECT hThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    if (fm_ref_equal(hThis, args)) {
      FXJSE_Value_SetInteger(args.GetReturnValue(), 1);
    } else {
      FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
      FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
        FXJSE_Value_SetInteger(
            args.GetReturnValue(),
            (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond))
                ? 1
                : 0);
      } else if (FXJSE_Value_IsUTF8String(argFirst) &&
                 FXJSE_Value_IsUTF8String(argSecond)) {
        CFX_ByteString firstOutput;
        CFX_ByteString secondOutput;
        FXJSE_Value_ToUTF8String(argFirst, firstOutput);
        FXJSE_Value_ToUTF8String(argSecond, secondOutput);
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               firstOutput.Equal(secondOutput) ? 1 : 0);
      } else {
        FX_DOUBLE first = HValueToDouble(hThis, argFirst);
        FX_DOUBLE second = HValueToDouble(hThis, argSecond);
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               (first == second) ? 1 : 0);
      }
      FXJSE_Value_Release(argFirst);
      FXJSE_Value_Release(argSecond);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::notequality_operator(FXJSE_HOBJECT hThis,
                                             const CFX_ByteStringC& szFuncName,
                                             CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    if (fm_ref_equal(hThis, args)) {
      FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
    } else {
      FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
      FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
      if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
        FXJSE_Value_SetInteger(
            args.GetReturnValue(),
            (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond))
                ? 0
                : 1);
      } else if (FXJSE_Value_IsUTF8String(argFirst) &&
                 FXJSE_Value_IsUTF8String(argSecond)) {
        CFX_ByteString firstOutput;
        CFX_ByteString secondOutput;
        FXJSE_Value_ToUTF8String(argFirst, firstOutput);
        FXJSE_Value_ToUTF8String(argSecond, secondOutput);
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               firstOutput.Equal(secondOutput) ? 0 : 1);
      } else {
        FX_DOUBLE first = HValueToDouble(hThis, argFirst);
        FX_DOUBLE second = HValueToDouble(hThis, argSecond);
        FXJSE_Value_SetInteger(args.GetReturnValue(),
                               (first == second) ? 0 : 1);
      }
      FXJSE_Value_Release(argFirst);
      FXJSE_Value_Release(argSecond);
    }
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
FX_BOOL CXFA_FM2JSContext::fm_ref_equal(FXJSE_HOBJECT hThis,
                                        CFXJSE_Arguments& args) {
  FX_BOOL bRet = FALSE;
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  FXJSE_HVALUE argFirst = args.GetValue(0);
  FXJSE_HVALUE argSecond = args.GetValue(0);
  if (FXJSE_Value_IsArray(argFirst) && FXJSE_Value_IsArray(argSecond)) {
    FXJSE_HVALUE firstFlagValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE secondFlagValue = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectPropByIdx(argFirst, 0, firstFlagValue);
    FXJSE_Value_GetObjectPropByIdx(argSecond, 0, secondFlagValue);
    if ((FXJSE_Value_ToInteger(firstFlagValue) == 3) &&
        (FXJSE_Value_ToInteger(secondFlagValue) == 3)) {
      FXJSE_HVALUE firstJSObject = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE secondJSObject = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(argFirst, 2, firstJSObject);
      FXJSE_Value_GetObjectPropByIdx(argSecond, 2, secondJSObject);
      if (!FXJSE_Value_IsNull(firstJSObject) &&
          !FXJSE_Value_IsNull(secondJSObject)) {
        bRet = (FXJSE_Value_ToObject(firstJSObject, NULL) ==
                FXJSE_Value_ToObject(secondJSObject, NULL));
      }
      FXJSE_Value_Release(firstJSObject);
      FXJSE_Value_Release(secondJSObject);
    }
    FXJSE_Value_Release(firstFlagValue);
    FXJSE_Value_Release(secondFlagValue);
  }
  FXJSE_Value_Release(argFirst);
  FXJSE_Value_Release(argSecond);
  return bRet;
}
void CXFA_FM2JSContext::less_operator(FXJSE_HOBJECT hThis,
                                      const CFX_ByteStringC& szFuncName,
                                      CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
    } else if (FXJSE_Value_IsUTF8String(argFirst) &&
               FXJSE_Value_IsUTF8String(argSecond)) {
      CFX_ByteString firstOutput;
      CFX_ByteString secondOutput;
      FXJSE_Value_ToUTF8String(argFirst, firstOutput);
      FXJSE_Value_ToUTF8String(argSecond, secondOutput);
      FXJSE_Value_SetInteger(args.GetReturnValue(),
                             (firstOutput.Compare(secondOutput) == -1) ? 1 : 0);
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first < second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::lessequal_operator(FXJSE_HOBJECT hThis,
                                           const CFX_ByteStringC& szFuncName,
                                           CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetInteger(
          args.GetReturnValue(),
          (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) ? 1
                                                                          : 0);
    } else if (FXJSE_Value_IsUTF8String(argFirst) &&
               FXJSE_Value_IsUTF8String(argSecond)) {
      CFX_ByteString firstOutput;
      CFX_ByteString secondOutput;
      FXJSE_Value_ToUTF8String(argFirst, firstOutput);
      FXJSE_Value_ToUTF8String(argSecond, secondOutput);
      FXJSE_Value_SetInteger(args.GetReturnValue(),
                             (firstOutput.Compare(secondOutput) != 1) ? 1 : 0);
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first <= second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::greater_operator(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szFuncName,
                                         CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetInteger(args.GetReturnValue(), 0);
    } else if (FXJSE_Value_IsUTF8String(argFirst) &&
               FXJSE_Value_IsUTF8String(argSecond)) {
      CFX_ByteString firstOutput;
      CFX_ByteString secondOutput;
      FXJSE_Value_ToUTF8String(argFirst, firstOutput);
      FXJSE_Value_ToUTF8String(argSecond, secondOutput);
      FXJSE_Value_SetInteger(args.GetReturnValue(),
                             (firstOutput.Compare(secondOutput) == 1) ? 1 : 0);
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first > second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::greaterequal_operator(FXJSE_HOBJECT hThis,
                                              const CFX_ByteStringC& szFuncName,
                                              CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) || FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetInteger(
          args.GetReturnValue(),
          (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) ? 1
                                                                          : 0);
    } else if (FXJSE_Value_IsUTF8String(argFirst) &&
               FXJSE_Value_IsUTF8String(argSecond)) {
      CFX_ByteString firstOutput;
      CFX_ByteString secondOutput;
      FXJSE_Value_ToUTF8String(argFirst, firstOutput);
      FXJSE_Value_ToUTF8String(argSecond, secondOutput);
      FXJSE_Value_SetInteger(args.GetReturnValue(),
                             (firstOutput.Compare(secondOutput) != -1) ? 1 : 0);
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first >= second) ? 1 : 0);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::plus_operator(FXJSE_HOBJECT hThis,
                                      const CFX_ByteStringC& szFuncName,
                                      CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = args.GetValue(0);
    FXJSE_HVALUE argSecond = args.GetValue(1);
    if (HValueIsNull(hThis, argFirst) && HValueIsNull(hThis, argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetDouble(args.GetReturnValue(), first + second);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::minus_operator(FXJSE_HOBJECT hThis,
                                       const CFX_ByteStringC& szFuncName,
                                       CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetDouble(args.GetReturnValue(), first - second);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::multiple_operator(FXJSE_HOBJECT hThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      FXJSE_Value_SetDouble(args.GetReturnValue(), first * second);
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::divide_operator(FXJSE_HOBJECT hThis,
                                        const CFX_ByteStringC& szFuncName,
                                        CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  if (args.GetLength() == 2) {
    FXJSE_HVALUE argFirst = GetSimpleHValue(hThis, args, 0);
    FXJSE_HVALUE argSecond = GetSimpleHValue(hThis, args, 1);
    if (FXJSE_Value_IsNull(argFirst) && FXJSE_Value_IsNull(argSecond)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argFirst);
      FX_DOUBLE second = HValueToDouble(hThis, argSecond);
      if (second == 0.0) {
        pContext->ThrowScriptErrorMessage(XFA_IDS_DIVIDE_ZERO);
      } else {
        FXJSE_Value_SetDouble(args.GetReturnValue(), first / second);
      }
    }
    FXJSE_Value_Release(argFirst);
    FXJSE_Value_Release(argSecond);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::positive_operator(FXJSE_HOBJECT hThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FXJSE_Value_SetDouble(args.GetReturnValue(),
                            0.0 + HValueToDouble(hThis, argOne));
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::negative_operator(FXJSE_HOBJECT hThis,
                                          const CFX_ByteStringC& szFuncName,
                                          CFXJSE_Arguments& args) {
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FXJSE_Value_SetDouble(args.GetReturnValue(),
                            0.0 - HValueToDouble(hThis, argOne));
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::logical_not_operator(FXJSE_HOBJECT hThis,
                                             const CFX_ByteStringC& szFuncName,
                                             CFXJSE_Arguments& args) {
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    if (FXJSE_Value_IsNull(argOne)) {
      FXJSE_Value_SetNull(args.GetReturnValue());
    } else {
      FX_DOUBLE first = HValueToDouble(hThis, argOne);
      FXJSE_Value_SetInteger(args.GetReturnValue(), (first == 0.0) ? 1 : 0);
    }
    FXJSE_Value_Release(argOne);
  } else {
    CXFA_FM2JSContext* pContext =
        (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::dot_accessor(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  if ((argc == 4) || (argc == 5)) {
    FX_BOOL bIsStar = TRUE;
    FXJSE_HVALUE argAccessor = args.GetValue(0);
    CFX_ByteString bsAccessorName = args.GetUTF8String(1);
    CFX_ByteString szName = args.GetUTF8String(2);
    int32_t iIndexFlags = args.GetInt32(3);
    int32_t iIndexValue = 0;
    FXJSE_HVALUE argIndex = NULL;
    if (argc == 5) {
      bIsStar = FALSE;
      argIndex = args.GetValue(4);
      iIndexValue = HValueToInteger(hThis, argIndex);
    }
    CFX_ByteString szSomExp;
    GenerateSomExpression(szName, iIndexFlags, iIndexValue, bIsStar, szSomExp);
    if (FXJSE_Value_IsArray(argAccessor)) {
      FXJSE_HVALUE hLengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argAccessor, "length", hLengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(hLengthValue);
      FXJSE_Value_Release(hLengthValue);
      int32_t iCounter = 0;
      FXJSE_HVALUE** hResolveValues = FX_Alloc(FXJSE_HVALUE*, iLength - 2);
      int32_t* iSizes = FX_Alloc(int32_t, iLength - 2);
      for (int32_t i = 0; i < (iLength - 2); i++) {
        iSizes[i] = 0;
      }
      FXJSE_HVALUE hJSObjValue = FXJSE_Value_Create(hruntime);
      FX_BOOL bAttribute = FALSE;
      for (int32_t i = 2; i < iLength; i++) {
        FXJSE_Value_GetObjectPropByIdx(argAccessor, i, hJSObjValue);
        XFA_RESOLVENODE_RS resoveNodeRS;
        int32_t iRet = ResolveObjects(hThis, hJSObjValue, szSomExp,
                                      resoveNodeRS, TRUE, szName.IsEmpty());
        if (iRet > 0) {
          ParseResolveResult(hThis, resoveNodeRS, hJSObjValue,
                             hResolveValues[i - 2], iSizes[i - 2], bAttribute);
          iCounter += iSizes[i - 2];
        }
      }
      FXJSE_Value_Release(hJSObjValue);
      if (iCounter > 0) {
        FXJSE_HVALUE* rgValues = FX_Alloc(FXJSE_HVALUE, iCounter + 2);
        for (int32_t i = 0; i < (iCounter + 2); i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 1);
        if (bAttribute) {
          FXJSE_Value_SetUTF8String(rgValues[1], szName);
        } else {
          FXJSE_Value_SetNull(rgValues[1]);
        }
        int32_t iIndex = 2;
        for (int32_t i = 0; i < iLength - 2; i++) {
          for (int32_t j = 0; j < iSizes[i]; j++) {
            FXJSE_Value_Set(rgValues[iIndex], hResolveValues[i][j]);
            iIndex++;
          }
        }
        FXJSE_Value_SetArray(args.GetReturnValue(), (iCounter + 2), rgValues);
        for (int32_t i = 0; i < (iCounter + 2); i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
        FX_Free(rgValues);
      } else {
        CFX_WideString wsPropertyName =
            CFX_WideString::FromUTF8(szName, szName.GetLength());
        CFX_WideString wsSomExpression =
            CFX_WideString::FromUTF8(szSomExp, szSomExp.GetLength());
        pContext->ThrowScriptErrorMessage(XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT,
                                          (const FX_WCHAR*)wsPropertyName,
                                          (const FX_WCHAR*)wsSomExpression);
      }
      for (int32_t i = 0; i < iLength - 2; i++) {
        for (int32_t j = 0; j < iSizes[i]; j++) {
          FXJSE_Value_Release(hResolveValues[i][j]);
        }
        if (iSizes[i] > 0) {
          FX_Free(hResolveValues[i]);
        }
      }
      FX_Free(hResolveValues);
      FX_Free(iSizes);
    } else {
      XFA_RESOLVENODE_RS resoveNodeRS;
      int32_t iRet = 0;
      if (FXJSE_Value_IsObject(argAccessor) ||
          (FXJSE_Value_IsNull(argAccessor) && bsAccessorName.IsEmpty())) {
        iRet = ResolveObjects(hThis, argAccessor, szSomExp, resoveNodeRS, TRUE,
                              szName.IsEmpty());
      } else if (!FXJSE_Value_IsObject(argAccessor) &&
                 !bsAccessorName.IsEmpty()) {
        FX_BOOL bGetObject =
            GetObjectByName(hThis, argAccessor, bsAccessorName);
        if (bGetObject) {
          iRet = ResolveObjects(hThis, argAccessor, szSomExp, resoveNodeRS,
                                TRUE, szName.IsEmpty());
        }
      }
      if (iRet > 0) {
        FXJSE_HVALUE* hResolveValues;
        int32_t iSize = 0;
        FX_BOOL bAttribute = FALSE;
        ParseResolveResult(hThis, resoveNodeRS, argAccessor, hResolveValues,
                           iSize, bAttribute);
        FXJSE_HVALUE* rgValues = FX_Alloc(FXJSE_HVALUE, iSize + 2);
        for (int32_t i = 0; i < (iSize + 2); i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 1);
        if (bAttribute) {
          FXJSE_Value_SetUTF8String(rgValues[1], szName);
        } else {
          FXJSE_Value_SetNull(rgValues[1]);
        }
        for (int32_t i = 0; i < iSize; i++) {
          FXJSE_Value_Set(rgValues[i + 2], hResolveValues[i]);
        }
        FXJSE_Value_SetArray(args.GetReturnValue(), (iSize + 2), rgValues);
        for (int32_t i = 0; i < (iSize + 2); i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
        FX_Free(rgValues);
        for (int32_t i = 0; i < iSize; i++) {
          FXJSE_Value_Release(hResolveValues[i]);
        }
        FX_Free(hResolveValues);
      } else {
        CFX_WideString wsPropertyName =
            CFX_WideString::FromUTF8(szName, szName.GetLength());
        CFX_WideString wsSomExpression =
            CFX_WideString::FromUTF8(szSomExp, szSomExp.GetLength());
        pContext->ThrowScriptErrorMessage(XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT,
                                          (const FX_WCHAR*)wsPropertyName,
                                          (const FX_WCHAR*)wsSomExpression);
      }
    }
    if (argc == 5) {
      FXJSE_Value_Release(argIndex);
    }
    FXJSE_Value_Release(argAccessor);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::dotdot_accessor(FXJSE_HOBJECT hThis,
                                        const CFX_ByteStringC& szFuncName,
                                        CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  if ((argc == 4) || (argc == 5)) {
    FX_BOOL bIsStar = TRUE;
    FXJSE_HVALUE argAccessor = args.GetValue(0);
    CFX_ByteString bsAccessorName = args.GetUTF8String(1);
    CFX_ByteString szName = args.GetUTF8String(2);
    int32_t iIndexFlags = args.GetInt32(3);
    int32_t iIndexValue = 0;
    FXJSE_HVALUE argIndex = NULL;
    if (argc == 5) {
      bIsStar = FALSE;
      argIndex = args.GetValue(4);
      iIndexValue = HValueToInteger(hThis, argIndex);
    }
    CFX_ByteString szSomExp;
    GenerateSomExpression(szName, iIndexFlags, iIndexValue, bIsStar, szSomExp);
    if (FXJSE_Value_IsArray(argAccessor)) {
      FXJSE_HVALUE hLengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argAccessor, "length", hLengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(hLengthValue);
      int32_t iCounter = 0;
      FXJSE_HVALUE** hResolveValues = FX_Alloc(FXJSE_HVALUE*, iLength - 2);
      int32_t* iSizes = FX_Alloc(int32_t, iLength - 2);
      FXJSE_HVALUE hJSObjValue = FXJSE_Value_Create(hruntime);
      FX_BOOL bAttribute = FALSE;
      for (int32_t i = 2; i < iLength; i++) {
        FXJSE_Value_GetObjectPropByIdx(argAccessor, i, hJSObjValue);
        XFA_RESOLVENODE_RS resoveNodeRS;
        int32_t iRet =
            ResolveObjects(hThis, hJSObjValue, szSomExp, resoveNodeRS, FALSE);
        if (iRet > 0) {
          ParseResolveResult(hThis, resoveNodeRS, hJSObjValue,
                             hResolveValues[i - 2], iSizes[i - 2], bAttribute);
          iCounter += iSizes[i - 2];
        }
      }
      FXJSE_Value_Release(hJSObjValue);
      if (iCounter > 0) {
        FXJSE_HVALUE* rgValues = FX_Alloc(FXJSE_HVALUE, iCounter + 2);
        for (int32_t i = 0; i < (iCounter + 2); i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 1);
        if (bAttribute) {
          FXJSE_Value_SetUTF8String(rgValues[1], szName);
        } else {
          FXJSE_Value_SetNull(rgValues[1]);
        }
        int32_t iIndex = 2;
        for (int32_t i = 0; i < iLength - 2; i++) {
          for (int32_t j = 0; j < iSizes[i]; j++) {
            FXJSE_Value_Set(rgValues[iIndex], hResolveValues[i][j]);
            iIndex++;
          }
        }
        FXJSE_Value_SetArray(args.GetReturnValue(), (iCounter + 2), rgValues);
        for (int32_t i = 0; i < (iCounter + 2); i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
        FX_Free(rgValues);
      } else {
        CFX_WideString wsPropertyName =
            CFX_WideString::FromUTF8(szName, szName.GetLength());
        CFX_WideString wsSomExpression =
            CFX_WideString::FromUTF8(szSomExp, szSomExp.GetLength());
        pContext->ThrowScriptErrorMessage(XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT,
                                          (const FX_WCHAR*)wsPropertyName,
                                          (const FX_WCHAR*)wsSomExpression);
      }
      for (int32_t i = 0; i < iLength - 2; i++) {
        for (int32_t j = 0; j < iSizes[i]; j++) {
          FXJSE_Value_Release(hResolveValues[i][j]);
        }
        FX_Free(hResolveValues[i]);
      }
      FX_Free(hResolveValues);
      FX_Free(iSizes);
      FXJSE_Value_Release(hLengthValue);
    } else {
      XFA_RESOLVENODE_RS resoveNodeRS;
      int32_t iRet = 0;
      if (FXJSE_Value_IsObject(argAccessor) ||
          (FXJSE_Value_IsNull(argAccessor) && bsAccessorName.IsEmpty())) {
        iRet =
            ResolveObjects(hThis, argAccessor, szSomExp, resoveNodeRS, FALSE);
      } else if (!FXJSE_Value_IsObject(argAccessor) &&
                 !bsAccessorName.IsEmpty()) {
        FX_BOOL bGetObject =
            GetObjectByName(hThis, argAccessor, bsAccessorName);
        if (bGetObject) {
          iRet =
              ResolveObjects(hThis, argAccessor, szSomExp, resoveNodeRS, FALSE);
        }
      }
      if (iRet > 0) {
        FXJSE_HVALUE* hResolveValues;
        int32_t iSize = 0;
        FX_BOOL bAttribute = FALSE;
        ParseResolveResult(hThis, resoveNodeRS, argAccessor, hResolveValues,
                           iSize, bAttribute);
        FXJSE_HVALUE* rgValues = FX_Alloc(FXJSE_HVALUE, iSize + 2);
        for (int32_t i = 0; i < (iSize + 2); i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 1);
        if (bAttribute) {
          FXJSE_Value_SetUTF8String(rgValues[1], szName);
        } else {
          FXJSE_Value_SetNull(rgValues[1]);
        }
        for (int32_t i = 0; i < iSize; i++) {
          FXJSE_Value_Set(rgValues[i + 2], hResolveValues[i]);
        }
        FXJSE_Value_SetArray(args.GetReturnValue(), (iSize + 2), rgValues);
        for (int32_t i = 0; i < (iSize + 2); i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
        FX_Free(rgValues);
        for (int32_t i = 0; i < iSize; i++) {
          FXJSE_Value_Release(hResolveValues[i]);
        }
        FX_Free(hResolveValues);
      } else {
        CFX_WideString wsPropertyName =
            CFX_WideString::FromUTF8(szName, szName.GetLength());
        CFX_WideString wsSomExpression =
            CFX_WideString::FromUTF8(szSomExp, szSomExp.GetLength());
        pContext->ThrowScriptErrorMessage(XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT,
                                          (const FX_WCHAR*)wsPropertyName,
                                          (const FX_WCHAR*)wsSomExpression);
      }
    }
    if (argc == 5) {
      FXJSE_Value_Release(argIndex);
    }
    FXJSE_Value_Release(argAccessor);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::eval_translation(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szFuncName,
                                         CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  int32_t argc = args.GetLength();
  if (argc == 1) {
    FXJSE_HVALUE argOne = GetSimpleHValue(hThis, args, 0);
    CFX_ByteString argString;
    HValueToUTF8String(argOne, argString);
    if (argString.IsEmpty()) {
      pContext->ThrowScriptErrorMessage(XFA_IDS_ARGUMENT_MISMATCH);
    } else {
      CFX_WideString scriptString =
          CFX_WideString::FromUTF8(argString, argString.GetLength());
      CFX_WideTextBuf wsJavaScriptBuf;
      CFX_WideString wsError;
      XFA_FM2JS_Translate(scriptString, wsJavaScriptBuf, wsError);
      if (wsError.IsEmpty()) {
        CFX_WideString javaScript = wsJavaScriptBuf.GetWideString();
        FXJSE_Value_SetUTF8String(
            args.GetReturnValue(),
            FX_UTF8Encode(javaScript, javaScript.GetLength()));
      } else {
        pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
      }
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_INCORRECT_NUMBER_OF_METHOD,
                                      L"Eval");
  }
}
void CXFA_FM2JSContext::is_fm_object(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FXJSE_Value_SetBoolean(args.GetReturnValue(), FXJSE_Value_IsObject(argOne));
    FXJSE_Value_Release(argOne);
  } else {
    FXJSE_Value_SetBoolean(args.GetReturnValue(), FALSE);
  }
}
void CXFA_FM2JSContext::is_fm_array(FXJSE_HOBJECT hThis,
                                    const CFX_ByteStringC& szFuncName,
                                    CFXJSE_Arguments& args) {
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    FX_BOOL bIsArray = FXJSE_Value_IsArray(argOne);
    FXJSE_Value_SetBoolean(args.GetReturnValue(), bIsArray);
    FXJSE_Value_Release(argOne);
  } else {
    FXJSE_Value_SetBoolean(args.GetReturnValue(), FALSE);
  }
}
void CXFA_FM2JSContext::get_fm_value(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t iLength = args.GetLength();
  if (iLength == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (FXJSE_Value_IsArray(argOne)) {
      FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(argOne, 1, propertyValue);
      FXJSE_Value_GetObjectPropByIdx(argOne, 2, jsobjectValue);
      if (FXJSE_Value_IsNull(propertyValue)) {
        GetObjectDefaultValue(jsobjectValue, args.GetReturnValue());
      } else {
        CFX_ByteString propertyStr;
        FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
        FXJSE_Value_GetObjectProp(jsobjectValue, propertyStr,
                                  args.GetReturnValue());
      }
      FXJSE_Value_Release(propertyValue);
      FXJSE_Value_Release(jsobjectValue);
    } else if (FXJSE_Value_IsObject(argOne)) {
      GetObjectDefaultValue(argOne, args.GetReturnValue());
    } else {
      FXJSE_Value_Set(args.GetReturnValue(), argOne);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::get_fm_jsobj(FXJSE_HOBJECT hThis,
                                     const CFX_ByteStringC& szFuncName,
                                     CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  if (argc == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (FXJSE_Value_IsArray(argOne)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argOne, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXSYS_assert(iLength >= 3);
      FXJSE_Value_Release(lengthValue);
      FXJSE_Value_GetObjectPropByIdx(argOne, 2, args.GetReturnValue());
    } else {
      FXJSE_Value_Set(args.GetReturnValue(), argOne);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::fm_var_filter(FXJSE_HOBJECT hThis,
                                      const CFX_ByteStringC& szFuncName,
                                      CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t argc = args.GetLength();
  if (argc == 1) {
    FXJSE_HVALUE argOne = args.GetValue(0);
    if (FXJSE_Value_IsArray(argOne)) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argOne, "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXSYS_assert(iLength >= 3);
      FXJSE_Value_Release(lengthValue);
      FXJSE_HVALUE flagsValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(argOne, 0, flagsValue);
      int32_t iFlags = FXJSE_Value_ToInteger(flagsValue);
      FXJSE_Value_Release(flagsValue);
      if (iFlags == 4) {
        FXJSE_HVALUE rgValues[3];
        for (int32_t i = 0; i < 3; i++) {
          rgValues[i] = FXJSE_Value_Create(hruntime);
        }
        FXJSE_Value_SetInteger(rgValues[0], 3);
        FXJSE_Value_SetNull(rgValues[1]);
        FXJSE_Value_SetNull(rgValues[2]);
        FXJSE_Value_SetArray(args.GetReturnValue(), 3, rgValues);
        for (int32_t i = 0; i < 3; i++) {
          FXJSE_Value_Release(rgValues[i]);
        }
      } else if (iFlags == 3) {
        FXJSE_HVALUE objectValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argOne, 2, objectValue);
        if (!FXJSE_Value_IsNull(objectValue)) {
          FXJSE_Value_Set(args.GetReturnValue(), argOne);
        } else {
          pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
        }
        FXJSE_Value_Release(objectValue);
      } else {
        FXJSE_HVALUE simpleValue = GetSimpleHValue(hThis, args, 0);
        FXJSE_Value_Set(args.GetReturnValue(), simpleValue);
        FXJSE_Value_Release(simpleValue);
      }
    } else {
      FXJSE_HVALUE simpleValue = GetSimpleHValue(hThis, args, 0);
      FXJSE_Value_Set(args.GetReturnValue(), simpleValue);
      FXJSE_Value_Release(simpleValue);
    }
    FXJSE_Value_Release(argOne);
  } else {
    pContext->ThrowScriptErrorMessage(XFA_IDS_COMPILER_ERROR);
  }
}
void CXFA_FM2JSContext::concat_fm_object(FXJSE_HOBJECT hThis,
                                         const CFX_ByteStringC& szFuncName,
                                         CFXJSE_Arguments& args) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  uint32_t iLength = 0;
  int32_t argCount = args.GetLength();
  FXJSE_HVALUE* argValues = FX_Alloc(FXJSE_HVALUE, argCount);
  for (int32_t i = 0; i < argCount; i++) {
    argValues[i] = args.GetValue(i);
    if (FXJSE_Value_IsArray(argValues[i])) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argValues[i], "length", lengthValue);
      int32_t length = FXJSE_Value_ToInteger(lengthValue);
      iLength = iLength + ((length > 2) ? (length - 2) : 0);
      FXJSE_Value_Release(lengthValue);
    }
    iLength += 1;
  }
  FXJSE_HVALUE* returnValues = FX_Alloc(FXJSE_HVALUE, iLength);
  for (int32_t i = 0; i < (int32_t)iLength; i++) {
    returnValues[i] = FXJSE_Value_Create(hruntime);
  }
  int32_t index = 0;
  for (int32_t i = 0; i < argCount; i++) {
    if (FXJSE_Value_IsArray(argValues[i])) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argValues[i], "length", lengthValue);
      int32_t length = FXJSE_Value_ToInteger(lengthValue);
      for (int32_t j = 2; j < length; j++) {
        FXJSE_Value_GetObjectPropByIdx(argValues[i], j, returnValues[index]);
        index++;
      }
      FXJSE_Value_Release(lengthValue);
    }
    FXJSE_Value_Set(returnValues[index], argValues[i]);
    index++;
  }
  FXJSE_Value_SetArray(args.GetReturnValue(), iLength, returnValues);
  for (int32_t i = 0; i < argCount; i++) {
    FXJSE_Value_Release(argValues[i]);
  }
  FX_Free(argValues);
  for (int32_t i = 0; i < (int32_t)iLength; i++) {
    FXJSE_Value_Release(returnValues[i]);
  }
  FX_Free(returnValues);
}
FXJSE_HVALUE CXFA_FM2JSContext::GetSimpleHValue(FXJSE_HOBJECT hThis,
                                                CFXJSE_Arguments& args,
                                                uint32_t index) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  FXSYS_assert(index < (uint32_t)args.GetLength());
  FXJSE_HVALUE argIndex = args.GetValue(index);
  if (FXJSE_Value_IsArray(argIndex)) {
    FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectProp(argIndex, "length", lengthValue);
    int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
    FXJSE_Value_Release(lengthValue);
    FXJSE_HVALUE simpleValue = FXJSE_Value_Create(hruntime);
    if (iLength > 2) {
      FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(argIndex, 1, propertyValue);
      FXJSE_Value_GetObjectPropByIdx(argIndex, 2, jsobjectValue);
      if (FXJSE_Value_IsNull(propertyValue)) {
        GetObjectDefaultValue(jsobjectValue, simpleValue);
      } else {
        CFX_ByteString propertyStr;
        FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
        FXJSE_Value_GetObjectProp(jsobjectValue, propertyStr, simpleValue);
      }
      FXJSE_Value_Release(propertyValue);
      FXJSE_Value_Release(jsobjectValue);
    } else {
      FXJSE_Value_SetUndefined(simpleValue);
    }
    FXJSE_Value_Release(argIndex);
    return simpleValue;
  } else if (FXJSE_Value_IsObject(argIndex)) {
    FXJSE_HVALUE defaultValue = FXJSE_Value_Create(hruntime);
    GetObjectDefaultValue(argIndex, defaultValue);
    FXJSE_Value_Release(argIndex);
    return defaultValue;
  } else {
    return argIndex;
  }
}
FX_BOOL CXFA_FM2JSContext::HValueIsNull(FXJSE_HOBJECT hThis, FXJSE_HVALUE arg) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  FX_BOOL isNull = FALSE;
  if (FXJSE_Value_IsNull(arg)) {
    isNull = TRUE;
  } else if (FXJSE_Value_IsArray(arg)) {
    int32_t iLength = hvalue_get_array_length(hThis, arg);
    if (iLength > 2) {
      FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
      FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectPropByIdx(arg, 1, propertyValue);
      FXJSE_Value_GetObjectPropByIdx(arg, 2, jsObjectValue);
      if (FXJSE_Value_IsNull(propertyValue)) {
        FXJSE_HVALUE defaultValue = FXJSE_Value_Create(hruntime);
        GetObjectDefaultValue(jsObjectValue, defaultValue);
        if (FXJSE_Value_IsNull(defaultValue)) {
          isNull = TRUE;
        }
        FXJSE_Value_Release(defaultValue);
      } else {
        CFX_ByteString propertyStr;
        FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
        FXJSE_HVALUE newPropertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectProp(jsObjectValue, propertyStr, newPropertyValue);
        if (FXJSE_Value_IsNull(newPropertyValue)) {
          isNull = TRUE;
        }
        FXJSE_Value_Release(newPropertyValue);
      }
      FXJSE_Value_Release(propertyValue);
      FXJSE_Value_Release(jsObjectValue);
    } else {
      isNull = TRUE;
    }
  } else if (FXJSE_Value_IsObject(arg)) {
    FXJSE_HVALUE defaultValue = FXJSE_Value_Create(hruntime);
    GetObjectDefaultValue(arg, defaultValue);
    if (FXJSE_Value_IsNull(defaultValue)) {
      isNull = TRUE;
    }
    FXJSE_Value_Release(defaultValue);
  }
  return isNull;
}
int32_t CXFA_FM2JSContext::hvalue_get_array_length(FXJSE_HOBJECT hThis,
                                                   FXJSE_HVALUE arg) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t iLength = 0;
  if (FXJSE_Value_IsArray(arg)) {
    FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectProp(arg, "length", lengthValue);
    iLength = FXJSE_Value_ToInteger(lengthValue);
    FXJSE_Value_Release(lengthValue);
  }
  return iLength;
}
FX_BOOL CXFA_FM2JSContext::simpleValueCompare(FXJSE_HOBJECT hThis,
                                              FXJSE_HVALUE firstValue,
                                              FXJSE_HVALUE secondValue) {
  FX_BOOL bReturn = FALSE;
  if (FXJSE_Value_IsUTF8String(firstValue)) {
    CFX_ByteString firstString, secondString;
    HValueToUTF8String(firstValue, firstString);
    HValueToUTF8String(secondValue, secondString);
    bReturn = firstString.Equal(secondString);
  } else if (FXJSE_Value_IsNumber(firstValue)) {
    FX_FLOAT first = HValueToFloat(hThis, firstValue);
    FX_FLOAT second = HValueToFloat(hThis, secondValue);
    bReturn = (first == second);
  } else if (FXJSE_Value_IsBoolean(firstValue)) {
    bReturn = (FXJSE_Value_ToBoolean(firstValue) ==
               FXJSE_Value_ToBoolean(secondValue));
  } else if (FXJSE_Value_IsNull(firstValue) &&
             FXJSE_Value_IsNull(secondValue)) {
    bReturn = TRUE;
  }
  return bReturn;
}
void CXFA_FM2JSContext::unfoldArgs(FXJSE_HOBJECT hThis,
                                   CFXJSE_Arguments& args,
                                   FXJSE_HVALUE*& resultValues,
                                   int32_t& iCount,
                                   int32_t iStart) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  iCount = 0;
  int32_t argc = args.GetLength();
  FXJSE_HVALUE* argsValue = FX_Alloc(FXJSE_HVALUE, argc);
  for (int32_t i = iStart; i < argc; i++) {
    argsValue[i] = args.GetValue(i);
    if (FXJSE_Value_IsArray(argsValue[i])) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argsValue[i], "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      iCount += ((iLength > 2) ? (iLength - 2) : 0);
    } else {
      iCount += 1;
    }
  }
  resultValues = FX_Alloc(FXJSE_HVALUE, iCount);
  for (int32_t i = 0; i < iCount; i++) {
    resultValues[i] = FXJSE_Value_Create(hruntime);
  }
  int32_t index = 0;
  for (int32_t i = iStart; i < argc; i++) {
    if (FXJSE_Value_IsArray(argsValue[i])) {
      FXJSE_HVALUE lengthValue = FXJSE_Value_Create(hruntime);
      FXJSE_Value_GetObjectProp(argsValue[i], "length", lengthValue);
      int32_t iLength = FXJSE_Value_ToInteger(lengthValue);
      FXJSE_Value_Release(lengthValue);
      if (iLength > 2) {
        FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
        FXJSE_HVALUE jsObjectValue = FXJSE_Value_Create(hruntime);
        FXJSE_Value_GetObjectPropByIdx(argsValue[i], 1, propertyValue);
        if (FXJSE_Value_IsNull(propertyValue)) {
          for (int32_t j = 2; j < iLength; j++) {
            FXJSE_Value_GetObjectPropByIdx(argsValue[i], j, jsObjectValue);
            GetObjectDefaultValue(jsObjectValue, resultValues[index]);
            index++;
          }
        } else {
          CFX_ByteString propertyString;
          FXJSE_Value_ToUTF8String(propertyValue, propertyString);
          for (int32_t j = 2; j < iLength; j++) {
            FXJSE_Value_GetObjectPropByIdx(argsValue[i], j, jsObjectValue);
            FXJSE_Value_GetObjectProp(jsObjectValue, propertyString,
                                      resultValues[index]);
            index++;
          }
        }
        FXJSE_Value_Release(propertyValue);
        FXJSE_Value_Release(jsObjectValue);
      }
    } else if (FXJSE_Value_IsObject(argsValue[i])) {
      GetObjectDefaultValue(argsValue[i], resultValues[index]);
      index++;
    } else {
      FXJSE_Value_Set(resultValues[index], argsValue[i]);
      index++;
    }
  }
  for (int32_t i = iStart; i < argc; i++) {
    FXJSE_Value_Release(argsValue[i]);
  }
  FX_Free(argsValue);
}
void CXFA_FM2JSContext::GetObjectDefaultValue(FXJSE_HVALUE hObjectValue,
                                              FXJSE_HVALUE hDefaultValue) {
  CXFA_Object* pNode = (CXFA_Object*)FXJSE_Value_ToObject(hObjectValue, NULL);
  if (pNode && pNode->IsNode()) {
    ((CXFA_Node*)pNode)
        ->Script_Som_DefaultValue(hDefaultValue, FALSE, (XFA_ATTRIBUTE)-1);
  } else {
    FXJSE_Value_SetNull(hDefaultValue);
  }
}
FX_BOOL CXFA_FM2JSContext::SetObjectDefaultValue(FXJSE_HVALUE hObjectValue,
                                                 FXJSE_HVALUE hNewValue) {
  FX_BOOL bSuccess = FALSE;
  CXFA_Object* pNode = (CXFA_Object*)FXJSE_Value_ToObject(hObjectValue, NULL);
  if (pNode && pNode->IsNode()) {
    ((CXFA_Node*)pNode)
        ->Script_Som_DefaultValue(hNewValue, TRUE, (XFA_ATTRIBUTE)-1);
    bSuccess = TRUE;
  }
  return bSuccess;
}
void CXFA_FM2JSContext::GenerateSomExpression(const CFX_ByteStringC& szName,
                                              int32_t iIndexFlags,
                                              int32_t iIndexValue,
                                              FX_BOOL bIsStar,
                                              CFX_ByteString& szSomExp) {
  if (bIsStar) {
    szSomExp = szName + "[*]";
    return;
  }
  if (iIndexFlags == 0) {
    szSomExp = szName;
    return;
  }
  if (iIndexFlags == 1 || iIndexValue == 0) {
    szSomExp = szName + "[" +
               CFX_ByteString::FormatInteger(iIndexValue, FXFORMAT_SIGNED) +
               "]";
  } else if (iIndexFlags == 2) {
    szSomExp = (iIndexValue < 0) ? (szName + "[-") : (szName + "[+");
    iIndexValue = (iIndexValue < 0) ? (0 - iIndexValue) : iIndexValue;
    szSomExp += CFX_ByteString::FormatInteger(iIndexValue);
    szSomExp += "]";
  } else {
    szSomExp = (iIndexValue < 0) ? (szName + "[") : (szName + "[-");
    iIndexValue = (iIndexValue < 0) ? (0 - iIndexValue) : iIndexValue;
    szSomExp += CFX_ByteString::FormatInteger(iIndexValue);
    szSomExp += "]";
  }
}
FX_BOOL CXFA_FM2JSContext::GetObjectByName(
    FXJSE_HOBJECT hThis,
    FXJSE_HVALUE accessorValue,
    const CFX_ByteStringC& szAccessorName) {
  FX_BOOL bFlags = FALSE;
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return bFlags;
  }
  IXFA_ScriptContext* pScriptContext = pDoc->GetScriptContext();
  XFA_RESOLVENODE_RS resoveNodeRS;
  FX_DWORD dwFlags = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Properties |
                     XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent;
  int32_t iRet = pScriptContext->ResolveObjects(
      pScriptContext->GetThisObject(),
      CFX_WideString::FromUTF8(szAccessorName.GetCStr(),
                               szAccessorName.GetLength()),
      resoveNodeRS, dwFlags);
  if (iRet >= 1 && resoveNodeRS.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    FXJSE_Value_Set(accessorValue, pScriptContext->GetJSValueFromMap(
                                       resoveNodeRS.nodes.GetAt(0)));
    bFlags = TRUE;
  }
  return bFlags;
}
int32_t CXFA_FM2JSContext::ResolveObjects(FXJSE_HOBJECT hThis,
                                          FXJSE_HVALUE hRefValue,
                                          const CFX_ByteStringC& bsSomExp,
                                          XFA_RESOLVENODE_RS& resoveNodeRS,
                                          FX_BOOL bdotAccessor,
                                          FX_BOOL bHasNoResolveName) {
  CFX_WideString wsSomExpression =
      CFX_WideString::FromUTF8(bsSomExp.GetCStr(), bsSomExp.GetLength());
  int32_t iRet = -1;
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  CXFA_Document* pDoc = pContext->GetDocument();
  if (!pDoc) {
    return iRet;
  }
  IXFA_ScriptContext* pScriptContext = pDoc->GetScriptContext();
  CXFA_Object* pNode = NULL;
  FX_DWORD dFlags = 0UL;
  if (bdotAccessor) {
    if (FXJSE_Value_IsNull(hRefValue)) {
      pNode = pScriptContext->GetThisObject();
      dFlags = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_Parent;
    } else {
      pNode = (CXFA_Object*)FXJSE_Value_ToObject(hRefValue, NULL);
      FXSYS_assert(pNode);
      if (bHasNoResolveName) {
        CFX_WideString wsName;
        if (pNode->IsNode()) {
          CXFA_Node* pXFANode = (CXFA_Node*)pNode;
          pXFANode->GetAttribute(XFA_ATTRIBUTE_Name, wsName, FALSE);
        }
        if (wsName.IsEmpty()) {
          CFX_WideStringC className;
          pNode->GetClassName(className);
          wsName = FX_WSTRC(L"#") + className;
        }
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
    pNode = (CXFA_Object*)FXJSE_Value_ToObject(hRefValue, NULL);
    dFlags = XFA_RESOLVENODE_AnyChild;
  }
  iRet = pScriptContext->ResolveObjects(pNode, wsSomExpression, resoveNodeRS,
                                        dFlags);
  return iRet;
}
void CXFA_FM2JSContext::ParseResolveResult(
    FXJSE_HOBJECT hThis,
    const XFA_RESOLVENODE_RS& resoveNodeRS,
    FXJSE_HVALUE hParentValue,
    FXJSE_HVALUE*& resultValues,
    int32_t& iSize,
    FX_BOOL& bAttribute) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hRuntime = pContext->GetScriptRuntime();
  iSize = 0;
  resultValues = NULL;
  if (resoveNodeRS.dwFlags == XFA_RESOVENODE_RSTYPE_Nodes) {
    bAttribute = FALSE;
    iSize = resoveNodeRS.nodes.GetSize();
    resultValues = FX_Alloc(FXJSE_HVALUE, iSize);
    for (int32_t i = 0; i < iSize; i++) {
      resultValues[i] = FXJSE_Value_Create(hRuntime);
      FXJSE_Value_Set(
          resultValues[i],
          pContext->GetDocument()->GetScriptContext()->GetJSValueFromMap(
              resoveNodeRS.nodes.GetAt(i)));
    }
  } else {
    CXFA_HVALUEArray objectProperties(hRuntime);
    int32_t iRet = resoveNodeRS.GetAttributeResult(objectProperties);
    bAttribute = (iRet == 0);
    if (bAttribute) {
      if (FXJSE_Value_IsObject(hParentValue)) {
        iSize = 1;
        resultValues = FX_Alloc(FXJSE_HVALUE, 1);
        resultValues[0] = FXJSE_Value_Create(hRuntime);
        FXJSE_Value_Set(resultValues[0], hParentValue);
      }
    } else {
      iSize = iRet;
      resultValues = FX_Alloc(FXJSE_HVALUE, iSize);
      for (int32_t i = 0; i < iSize; i++) {
        resultValues[i] = FXJSE_Value_Create(hRuntime);
        FXJSE_Value_Set(resultValues[i], objectProperties[i]);
      }
    }
  }
}
int32_t CXFA_FM2JSContext::HValueToInteger(FXJSE_HOBJECT hThis,
                                           FXJSE_HVALUE hValue) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  int32_t iValue = 0;
  if (FXJSE_Value_IsArray(hValue)) {
    FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectPropByIdx(hValue, 1, propertyValue);
    FXJSE_Value_GetObjectPropByIdx(hValue, 2, jsobjectValue);
    if (FXJSE_Value_IsNull(propertyValue)) {
      GetObjectDefaultValue(jsobjectValue, newProperty);
    } else {
      CFX_ByteString propertyStr;
      FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
      FXJSE_Value_GetObjectProp(jsobjectValue, propertyStr, newProperty);
    }
    iValue = HValueToInteger(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
    FXJSE_Value_Release(jsobjectValue);
    FXJSE_Value_Release(propertyValue);
    return iValue;
  } else if (FXJSE_Value_IsObject(hValue)) {
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    GetObjectDefaultValue(hValue, newProperty);
    iValue = HValueToInteger(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
    return iValue;
  } else if (FXJSE_Value_IsUTF8String(hValue)) {
    CFX_ByteString szValue;
    FXJSE_Value_ToUTF8String(hValue, szValue);
    iValue = FXSYS_atoi(szValue);
  } else {
    iValue = FXJSE_Value_ToInteger(hValue);
  }
  return iValue;
}
FX_DOUBLE CXFA_FM2JSContext::StringToDouble(
    const CFX_ByteStringC& szStringVal) {
  return XFA_ByteStringToDouble(szStringVal);
}
FX_FLOAT CXFA_FM2JSContext::HValueToFloat(FXJSE_HOBJECT hThis,
                                          FXJSE_HVALUE arg) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  FX_FLOAT fRet = 0.0f;
  if (FXJSE_Value_IsArray(arg)) {
    FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectPropByIdx(arg, 1, propertyValue);
    FXJSE_Value_GetObjectPropByIdx(arg, 2, jsobjectValue);
    if (FXJSE_Value_IsNull(propertyValue)) {
      GetObjectDefaultValue(jsobjectValue, newProperty);
    } else {
      CFX_ByteString propertyStr;
      FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
      FXJSE_Value_GetObjectProp(jsobjectValue, propertyStr, newProperty);
    }
    fRet = HValueToFloat(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
    FXJSE_Value_Release(jsobjectValue);
    FXJSE_Value_Release(propertyValue);
  } else if (FXJSE_Value_IsObject(arg)) {
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    GetObjectDefaultValue(arg, newProperty);
    fRet = HValueToFloat(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
  } else if (FXJSE_Value_IsUTF8String(arg)) {
    CFX_ByteString bsOutput;
    FXJSE_Value_ToUTF8String(arg, bsOutput);
    fRet = (FX_FLOAT)StringToDouble(bsOutput);
  } else if (FXJSE_Value_IsUndefined(arg)) {
    fRet = 0;
  } else {
    fRet = FXJSE_Value_ToFloat(arg);
  }
  return fRet;
}
FX_DOUBLE CXFA_FM2JSContext::HValueToDouble(FXJSE_HOBJECT hThis,
                                            FXJSE_HVALUE arg) {
  CXFA_FM2JSContext* pContext =
      (CXFA_FM2JSContext*)FXJSE_Value_ToObject(hThis, NULL);
  FXJSE_HRUNTIME hruntime = pContext->GetScriptRuntime();
  FX_DOUBLE dRet = 0;
  if (FXJSE_Value_IsArray(arg)) {
    FXJSE_HVALUE propertyValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE jsobjectValue = FXJSE_Value_Create(hruntime);
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    FXJSE_Value_GetObjectPropByIdx(arg, 1, propertyValue);
    FXJSE_Value_GetObjectPropByIdx(arg, 2, jsobjectValue);
    if (FXJSE_Value_IsNull(propertyValue)) {
      GetObjectDefaultValue(jsobjectValue, newProperty);
    } else {
      CFX_ByteString propertyStr;
      FXJSE_Value_ToUTF8String(propertyValue, propertyStr);
      FXJSE_Value_GetObjectProp(jsobjectValue, propertyStr, newProperty);
    }
    dRet = HValueToDouble(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
    FXJSE_Value_Release(jsobjectValue);
    FXJSE_Value_Release(propertyValue);
  } else if (FXJSE_Value_IsObject(arg)) {
    FXJSE_HVALUE newProperty = FXJSE_Value_Create(hruntime);
    GetObjectDefaultValue(arg, newProperty);
    dRet = HValueToDouble(hThis, newProperty);
    FXJSE_Value_Release(newProperty);
  } else if (FXJSE_Value_IsUTF8String(arg)) {
    CFX_ByteString bsOutput;
    FXJSE_Value_ToUTF8String(arg, bsOutput);
    dRet = StringToDouble(bsOutput);
  } else if (FXJSE_Value_IsUndefined(arg)) {
    dRet = 0;
  } else {
    dRet = FXJSE_Value_ToDouble(arg);
  }
  return dRet;
}
void CXFA_FM2JSContext::HValueToUTF8String(FXJSE_HVALUE arg,
                                           CFX_ByteString& szOutputString) {
  if (FXJSE_Value_IsNull(arg) || FXJSE_Value_IsUndefined(arg)) {
    szOutputString = "";
  } else if (FXJSE_Value_IsBoolean(arg)) {
    szOutputString = FXJSE_Value_ToBoolean(arg) ? "1" : "0";
  } else {
    szOutputString = "";
    FXJSE_Value_ToUTF8String(arg, szOutputString);
  }
}
static FXJSE_FUNCTION formcalc_fm2js_functions[] = {
    {"Abs", CXFA_FM2JSContext::Abs},
    {"Avg", CXFA_FM2JSContext::Avg},
    {"Ceil", CXFA_FM2JSContext::Ceil},
    {"Count", CXFA_FM2JSContext::Count},
    {"Floor", CXFA_FM2JSContext::Floor},
    {"Max", CXFA_FM2JSContext::Max},
    {"Min", CXFA_FM2JSContext::Min},
    {"Mod", CXFA_FM2JSContext::Mod},
    {"Round", CXFA_FM2JSContext::Round},
    {"Sum", CXFA_FM2JSContext::Sum},
    {"Date", CXFA_FM2JSContext::Date},
    {"Date2Num", CXFA_FM2JSContext::Date2Num},
    {"DateFmt", CXFA_FM2JSContext::DateFmt},
    {"IsoDate2Num", CXFA_FM2JSContext::IsoDate2Num},
    {"IsoTime2Num", CXFA_FM2JSContext::IsoTime2Num},
    {"LocalDateFmt", CXFA_FM2JSContext::LocalDateFmt},
    {"LocalTimeFmt", CXFA_FM2JSContext::LocalTimeFmt},
    {"Num2Date", CXFA_FM2JSContext::Num2Date},
    {"Num2GMTime", CXFA_FM2JSContext::Num2GMTime},
    {"Num2Time", CXFA_FM2JSContext::Num2Time},
    {"Time", CXFA_FM2JSContext::Time},
    {"Time2Num", CXFA_FM2JSContext::Time2Num},
    {"TimeFmt", CXFA_FM2JSContext::TimeFmt},
    {"Apr", CXFA_FM2JSContext::Apr},
    {"Cterm", CXFA_FM2JSContext::CTerm},
    {"FV", CXFA_FM2JSContext::FV},
    {"Ipmt", CXFA_FM2JSContext::IPmt},
    {"NPV", CXFA_FM2JSContext::NPV},
    {"Pmt", CXFA_FM2JSContext::Pmt},
    {"PPmt", CXFA_FM2JSContext::PPmt},
    {"PV", CXFA_FM2JSContext::PV},
    {"Rate", CXFA_FM2JSContext::Rate},
    {"Term", CXFA_FM2JSContext::Term},
    {"Choose", CXFA_FM2JSContext::Choose},
    {"Exists", CXFA_FM2JSContext::Exists},
    {"HasValue", CXFA_FM2JSContext::HasValue},
    {"Oneof", CXFA_FM2JSContext::Oneof},
    {"Within", CXFA_FM2JSContext::Within},
    {"If", CXFA_FM2JSContext::If},
    {"Eval", CXFA_FM2JSContext::Eval},
    {"Translate", CXFA_FM2JSContext::eval_translation},
    {"Ref", CXFA_FM2JSContext::Ref},
    {"UnitType", CXFA_FM2JSContext::UnitType},
    {"UnitValue", CXFA_FM2JSContext::UnitValue},
    {"At", CXFA_FM2JSContext::At},
    {"Concat", CXFA_FM2JSContext::Concat},
    {"Decode", CXFA_FM2JSContext::Decode},
    {"Encode", CXFA_FM2JSContext::Encode},
    {"Format", CXFA_FM2JSContext::Format},
    {"Left", CXFA_FM2JSContext::Left},
    {"Len", CXFA_FM2JSContext::Len},
    {"Lower", CXFA_FM2JSContext::Lower},
    {"Ltrim", CXFA_FM2JSContext::Ltrim},
    {"Parse", CXFA_FM2JSContext::Parse},
    {"Replace", CXFA_FM2JSContext::Replace},
    {"Right", CXFA_FM2JSContext::Right},
    {"Rtrim", CXFA_FM2JSContext::Rtrim},
    {"Space", CXFA_FM2JSContext::Space},
    {"Str", CXFA_FM2JSContext::Str},
    {"Stuff", CXFA_FM2JSContext::Stuff},
    {"Substr", CXFA_FM2JSContext::Substr},
    {"Uuid", CXFA_FM2JSContext::Uuid},
    {"Upper", CXFA_FM2JSContext::Upper},
    {"WordNum", CXFA_FM2JSContext::WordNum},
    {"Get", CXFA_FM2JSContext::Get},
    {"Post", CXFA_FM2JSContext::Post},
    {"Put", CXFA_FM2JSContext::Put},
    {"positive_operator", CXFA_FM2JSContext::positive_operator},
    {"negative_operator", CXFA_FM2JSContext::negative_operator},
    {"logical_or_operator", CXFA_FM2JSContext::logical_or_operator},
    {"logical_and_operator", CXFA_FM2JSContext::logical_and_operator},
    {"logical_not_operator", CXFA_FM2JSContext::logical_not_operator},
    {"equality_operator", CXFA_FM2JSContext::equality_operator},
    {"notequality_operator", CXFA_FM2JSContext::notequality_operator},
    {"less_operator", CXFA_FM2JSContext::less_operator},
    {"lessequal_operator", CXFA_FM2JSContext::lessequal_operator},
    {"greater_operator", CXFA_FM2JSContext::greater_operator},
    {"greaterequal_operator", CXFA_FM2JSContext::greaterequal_operator},
    {"plus_operator", CXFA_FM2JSContext::plus_operator},
    {"minus_operator", CXFA_FM2JSContext::minus_operator},
    {"multiple_operator", CXFA_FM2JSContext::multiple_operator},
    {"divide_operator", CXFA_FM2JSContext::divide_operator},
    {"assign_value_operator", CXFA_FM2JSContext::assign_value_operator},
    {"dot_accessor", CXFA_FM2JSContext::dot_accessor},
    {"dotdot_accessor", CXFA_FM2JSContext::dotdot_accessor},
    {"concat_fm_object", CXFA_FM2JSContext::concat_fm_object},
    {"is_fm_object", CXFA_FM2JSContext::is_fm_object},
    {"is_fm_array", CXFA_FM2JSContext::is_fm_array},
    {"get_fm_value", CXFA_FM2JSContext::get_fm_value},
    {"get_fm_jsobj", CXFA_FM2JSContext::get_fm_jsobj},
    {"fm_var_filter", CXFA_FM2JSContext::fm_var_filter},
};
CXFA_FM2JSContext::CXFA_FM2JSContext()
    : m_hFMClass(nullptr), m_pDocument(nullptr) {
  FX_memset(&m_fmClass, 0, sizeof(FXJSE_CLASS));
}
CXFA_FM2JSContext::~CXFA_FM2JSContext() {
  m_pDocument = NULL;
  if (m_hValue) {
    FXJSE_Value_Release(m_hValue);
    m_hValue = NULL;
  }
  m_hScriptRuntime = NULL;
}
CXFA_FM2JSContext* CXFA_FM2JSContext::Create() {
  return new CXFA_FM2JSContext;
}
void CXFA_FM2JSContext::Initialize(FXJSE_HRUNTIME hScriptRuntime,
                                   FXJSE_HCONTEXT hScriptContext,
                                   CXFA_Document* pDoc) {
  m_pDocument = pDoc;
  m_hScriptRuntime = hScriptRuntime;
  m_fmClass.name = "XFA_FM2JS_FormCalcClass";
  m_fmClass.constructor = NULL;
  m_fmClass.properties = NULL;
  m_fmClass.methods = formcalc_fm2js_functions;
  m_fmClass.propNum = 0;
  m_fmClass.methNum =
      sizeof(formcalc_fm2js_functions) / sizeof(formcalc_fm2js_functions[0]);
  m_hFMClass = FXJSE_DefineClass(hScriptContext, &m_fmClass);
  m_hValue = FXJSE_Value_Create(hScriptRuntime);
  FXJSE_Value_SetNull(m_hValue);
  FXJSE_Value_SetObject(m_hValue, this, m_hFMClass);
}
void CXFA_FM2JSContext::GlobalPropertyGetter(FXJSE_HVALUE hValue) {
  FXJSE_Value_Set(hValue, m_hValue);
}
void CXFA_FM2JSContext::Release() {
  delete this;
}
void CXFA_FM2JSContext::ThrowScriptErrorMessage(int32_t iStringID, ...) {
  IXFA_AppProvider* pAppProvider = m_pDocument->GetNotify()->GetAppProvider();
  FXSYS_assert(pAppProvider);
  CFX_WideString wsFormat;
  pAppProvider->LoadString(iStringID, wsFormat);
  CFX_WideString wsMessage;
  va_list arg_ptr;
  va_start(arg_ptr, iStringID);
  wsMessage.FormatV((const FX_WCHAR*)wsFormat, arg_ptr);
  va_end(arg_ptr);
  FXJSE_ThrowMessage("", FX_UTF8Encode(wsMessage, wsMessage.GetLength()));
}
