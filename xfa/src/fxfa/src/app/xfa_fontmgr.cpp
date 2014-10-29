// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_fontmgr.h"
#include "xfa_ffdoc.h"
#include "xfa_ffConfigAcc.h"
#include "xfa_ffapp.h"
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
static const XFA_FONTINFO g_XFAFontsMap[] = {
    {0x01d5d33e,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x01e4f102,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x030549dc,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x032edd44,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x03eac6fc,	(FX_LPCWSTR)L"PoorRichard-Regular",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x03ed90e6,	(FX_LPCWSTR)L"Nina",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x077b56b3,	(FX_LPCWSTR)L"KingsoftPhoneticPlain",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x078ed524,	(FX_LPCWSTR)L"MicrosoftSansSerif",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x089b18a9,	(FX_LPCWSTR)L"Arial",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x0b2cad72,	(FX_LPCWSTR)L"MonotypeCorsiva",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x0bb003e7,	(FX_LPCWSTR)L"Kartika",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x0bb469df,	(FX_LPCWSTR)L"VinerHandITC",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x0bc1a851,	(FX_LPCWSTR)L"SegoeUI",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x0c112ebd,	(FX_LPCWSTR)L"KozukaGothicPro-VIM",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x0cfcb9c1,	(FX_LPCWSTR)L"AdobeThai",	(FX_LPCWSTR)L"Kokila,Arial Narrow",	0,	847},
    {0x0e7de0f9,	(FX_LPCWSTR)L"Playbill",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x0eff47c3,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x107ad374,	(FX_LPCWSTR)L"Constantia",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x12194c2d,	(FX_LPCWSTR)L"KunstlerScript",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x135ef6a1,	(FX_LPCWSTR)L"MinionProSmBd",	(FX_LPCWSTR)L"Bell MT,Corbel,Times New Roman,Cambria,Berlin Sans FB",	0,	1252},
    {0x158c4049,	(FX_LPCWSTR)L"Garamond",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x160ecb24,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x161ed07e,	(FX_LPCWSTR)L"MSGothic",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x171d1ed1,	(FX_LPCWSTR)L"SnapITC-Regular",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x18d1188f,	(FX_LPCWSTR)L"Cambria",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x18eaf350,	(FX_LPCWSTR)L"ArialUnicodeMS",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x1a92d115,	(FX_LPCWSTR)L"MingLiU",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x1cc217c6,	(FX_LPCWSTR)L"TrebuchetMS",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x1d649596,	(FX_LPCWSTR)L"BasemicTimes",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x1e34ee60,	(FX_LPCWSTR)L"BellMT",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x1eb36945,	(FX_LPCWSTR)L"CooperBlack",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x1ef7787d,	(FX_LPCWSTR)L"BatangChe",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x20b3bd3a,	(FX_LPCWSTR)L"BrushScriptMT",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x220877aa,	(FX_LPCWSTR)L"Candara",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x22135007,	(FX_LPCWSTR)L"FreestyleScript-Regular",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x251059c3,	(FX_LPCWSTR)L"Chiller",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x25bed6dd,	(FX_LPCWSTR)L"MSReferenceSansSerif",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x28154c81,	(FX_LPCWSTR)L"Parchment-Regular",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x29711eb9,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x2b1993b4,	(FX_LPCWSTR)L"Basemic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2b316339,	(FX_LPCWSTR)L"NiagaraSolid-Reg",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2c147529,	(FX_LPCWSTR)L"FootlightMTLight",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2c198928,	(FX_LPCWSTR)L"HarlowSolid",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2c6ac6b2,	(FX_LPCWSTR)L"LucidaBright",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x2c9f38e2,	(FX_LPCWSTR)L"KozukaMinchoPro-VIR",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2d5a47b0,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x2def26bf,	(FX_LPCWSTR)L"BernardMT-Condensed",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x2fd8930b,	(FX_LPCWSTR)L"KozukaMinchoPr6NR",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3115525a,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x31327817,	(FX_LPCWSTR)L"MyriadPro",	(FX_LPCWSTR)L"Calibri,Corbel,Candara,Cambria Math,Franklin Gothic Medium,Arial Narrow,Times New Roman",	0,	1252},
    {0x32244975,    (FX_LPCWSTR)L"Helvetica", (FX_LPCWSTR)L"Arial",	 0,   1252},
    {0x32ac995c,	(FX_LPCWSTR)L"Terminal",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x338d648a,	(FX_LPCWSTR)L"NiagaraEngraved-Reg",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x33bb65f2,	(FX_LPCWSTR)L"Sylfaen",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x3402c30e,	(FX_LPCWSTR)L"MSPMincho",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x3412bf31,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x36eb39b9,	(FX_LPCWSTR)L"BerlinSansFB",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x36f42055,	(FX_LPCWSTR)L"UniversATT",	(FX_LPCWSTR)L"Microsoft Sans Serif",	0,	1252},
    {0x3864c4f6,	(FX_LPCWSTR)L"HighTowerText",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x3a257d03,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3cdae668,	(FX_LPCWSTR)L"FreestyleScript",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x3d55aed7,	(FX_LPCWSTR)L"Jokerman",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3d5b4385,	(FX_LPCWSTR)L"PMingLiU",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x3d9b7669,	(FX_LPCWSTR)L"EstrangeloEdessa",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3e532d74,	(FX_LPCWSTR)L"FranklinGothicMedium",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3e6aa32d,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x3f6c36a8,	(FX_LPCWSTR)L"Gautami",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x3ff32662,	(FX_LPCWSTR)L"Chiller-Regular",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x409de312,	(FX_LPCWSTR)L"ModernNo.20",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x41443c5e,	(FX_LPCWSTR)L"Georgia",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x4160ade5,	(FX_LPCWSTR)L"BellGothicStdBlack",	(FX_LPCWSTR)L"Arial,Arial Unicode MS,Book Antiqua,Dotum,Georgia",	0,	1252},
    {0x421976c4,	(FX_LPCWSTR)L"Modern-Regular",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x422a7252,	(FX_LPCWSTR)L"Stencil",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x42c8554f,	(FX_LPCWSTR)L"Fixedsys",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x435cb41d,	(FX_LPCWSTR)L"Roman",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x47882383,	(FX_LPCWSTR)L"CourierNew",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x480a2338,	(FX_LPCWSTR)L"BerlinSansFBDemi",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x480bf7a4,	(FX_LPCWSTR)L"CourierStd",	(FX_LPCWSTR)L"Courier New,Verdana",	0,	1252},
    {0x481ad6ed,	(FX_LPCWSTR)L"VladimirScript",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0x4911577a,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x4a788d72,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x4bf88566,	(FX_LPCWSTR)L"SegoeCondensed",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x4ccf51a4,	(FX_LPCWSTR)L"BerlinSansFB-Reg",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x4ea967ce,	(FX_LPCWSTR)L"GulimChe",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x4f68bd79,	(FX_LPCWSTR)L"LetterGothicStd",	(FX_LPCWSTR)L"Courier New,Verdana",	0,	1252},
    {0x51a0d0e6,	(FX_LPCWSTR)L"KozukaGothicPr6NM",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x531b3dea,	(FX_LPCWSTR)L"BasemicSymbol",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x5333fd39,	(FX_LPCWSTR)L"CalifornianFB-Reg",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x53561a54,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x55e0dde6,	(FX_LPCWSTR)L"LucidaSansTypewriter",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x574d4d3d,	(FX_LPCWSTR)L"AdobeArabic",	(FX_LPCWSTR)L"Arial Narrow",	0,	1252},
    {0x5792e759,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x5921978e,	(FX_LPCWSTR)L"LucidaSansUnicode",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x594e2da4,	(FX_LPCWSTR)L"Vrinda",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x59baa9a2,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x5cfedf4f,	(FX_LPCWSTR)L"BaskOldFace",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x5f97921c,	(FX_LPCWSTR)L"AdobeMyungjoStdM",	(FX_LPCWSTR)L"Batang,Bookman Old Style,Consolas,STZhongsong",	0,	936},
    {0x5fefbfad,	(FX_LPCWSTR)L"Batang",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x605342b9,	(FX_LPCWSTR)L"DotumChe",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x608c5f9a,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x61efd0d1,	(FX_LPCWSTR)L"MaturaMTScriptCapitals",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x626608a9,	(FX_LPCWSTR)L"MVBoli",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x630501a3,	(FX_LPCWSTR)L"SmallFonts",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x65d0e2a9,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x669f29e1,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x673a9e5f,	(FX_LPCWSTR)L"Tunga",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x691aa4ce,	(FX_LPCWSTR)L"NiagaraSolid",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x696259b7,	(FX_LPCWSTR)L"Corbel",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x696ee9be,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0x6c59cf69,	(FX_LPCWSTR)L"Dotum",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x707fa561,	(FX_LPCWSTR)L"Gungsuh",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x71416bb2,	(FX_LPCWSTR)L"ZWAdobeF",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x71b41801,	(FX_LPCWSTR)L"Verdana",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x73f25e4c,	(FX_LPCWSTR)L"PalatinoLinotype",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x73f4d19f,	(FX_LPCWSTR)L"NiagaraEngraved",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x74001694,	(FX_LPCWSTR)L"MyriadProBlack",	(FX_LPCWSTR)L"Book Antiqua,Constantia,Dotum,Georgia",	0,	1252},
    {0x74b14d8f,	(FX_LPCWSTR)L"Haettenschweiler",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x74cb44ee,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0x76b4d7ff,	(FX_LPCWSTR)L"Shruti",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x788b3533,	(FX_LPCWSTR)L"Webdings",	(FX_LPCWSTR)L"Arial",	6,	42},
    {0x797dde99,	(FX_LPCWSTR)L"MSSerif",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x7a0f9e9e,	(FX_LPCWSTR)L"MSMincho",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0x7b439caf,	(FX_LPCWSTR)L"OldEnglishTextMT",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8213a433,	(FX_LPCWSTR)L"LucidaSans-Typewriter",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x82fec929,	(FX_LPCWSTR)L"AdobeSongStdL",	(FX_LPCWSTR)L"Centaur,Calibri,STSong,Bell MT,Garamond,Times New Roman",	0,	936},
    {0x83581825,	(FX_LPCWSTR)L"Modern",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x835a2823,	(FX_LPCWSTR)L"Algerian",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x83dab9f5,	(FX_LPCWSTR)L"Script",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x847b56da,	(FX_LPCWSTR)L"Tahoma",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8a783cb2,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8b5cac0e,	(FX_LPCWSTR)L"Onyx",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8c6a499e,	(FX_LPCWSTR)L"Gulim",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8e0af790,	(FX_LPCWSTR)L"JuiceITC",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x8e8d43b2,	(FX_LPCWSTR)L"Centaur",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x8ee4dcca,	(FX_LPCWSTR)L"BookshelfSymbol7",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x90794800,	(FX_LPCWSTR)L"BellGothicStdLight",	(FX_LPCWSTR)L"Bell MT,Calibri,Times New Roman",	0,	1252},
    {0x909b516a,	(FX_LPCWSTR)L"Century",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x92ae370d,	(FX_LPCWSTR)L"MSOutlook",	(FX_LPCWSTR)L"Arial",	4,	42},
    {0x93c9fbf1,	(FX_LPCWSTR)L"LucidaFax",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x9565085e,	(FX_LPCWSTR)L"BookAntiqua",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0x9856d95d,	(FX_LPCWSTR)L"AdobeMingStdL",	(FX_LPCWSTR)L"Arial,Arial Unicode MS,Cambria,BatangChe",	0,	949},
    {0x9bbadd6b,	(FX_LPCWSTR)L"ColonnaMT",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x9cbd16a4,	(FX_LPCWSTR)L"ShowcardGothic-Reg",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0x9d73008e,	(FX_LPCWSTR)L"MSSansSerif",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xa0607db1,	(FX_LPCWSTR)L"GungsuhChe",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0xa0bcf6a1,	(FX_LPCWSTR)L"LatinWide",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xa1429b36,	(FX_LPCWSTR)L"Symbol",	(FX_LPCWSTR)L"Arial",	6,	42},
    {0xa1fa5abc,	(FX_LPCWSTR)L"Wingdings2",	(FX_LPCWSTR)L"Arial",	6,	42},
    {0xa1fa5abd,	(FX_LPCWSTR)L"Wingdings3",	(FX_LPCWSTR)L"Arial",	6,	42},
    {0xa427bad4,	(FX_LPCWSTR)L"InformalRoman-Regular",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xa8b92ece,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xa8d83ece,	(FX_LPCWSTR)L"CalifornianFB",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xaa3e082c,	(FX_LPCWSTR)L"Kingsoft-Phonetic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xaa6bcabe,	(FX_LPCWSTR)L"HarlowSolidItalic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xade5337c,	(FX_LPCWSTR)L"MSUIGothic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xb08dd941,	(FX_LPCWSTR)L"WideLatin",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xb207f05d,	(FX_LPCWSTR)L"PoorRichard",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xb3bc492f,	(FX_LPCWSTR)L"JuiceITC-Regular",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xb5545399,	(FX_LPCWSTR)L"Marlett",	(FX_LPCWSTR)L"Arial",	4,	42},
    {0xb5dd1ebb,	(FX_LPCWSTR)L"BritannicBold",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xb699c1c5,	(FX_LPCWSTR)L"LucidaCalligraphy-Italic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xb725d629,	(FX_LPCWSTR)L"TimesNewRoman",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xb7eaebeb,	(FX_LPCWSTR)L"AdobeHeitiStdR",	(FX_LPCWSTR)L"Batang,Century,Dotum",	0,	936},
    {0xbd29c486,	(FX_LPCWSTR)L"BerlinSansFBDemi-Bold",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xbe8a8db4,	(FX_LPCWSTR)L"BookshelfSymbolSeven",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xc16c0118,	(FX_LPCWSTR)L"AdobeHebrew",	(FX_LPCWSTR)L"Bell MT,Berlin Sans FB,Calibri",	0,	1252},
    {0xc318b0af,	(FX_LPCWSTR)L"MyriadProLight",	(FX_LPCWSTR)L"Calibri,STFangsong,Times New Roman",	0,	1252},
    {0xc65e5659,	(FX_LPCWSTR)L"CambriaMath",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xc75c8f05,	(FX_LPCWSTR)L"LucidaConsole",	(FX_LPCWSTR)L"Arial",	1,	1252},
    {0xca7c35d6,	(FX_LPCWSTR)L"Calibri",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xcb053f53,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xcb7190f9,	(FX_LPCWSTR)L"Magneto-Bold",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xcca00cc5,	(FX_LPCWSTR)L"System",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xccad6f76,	(FX_LPCWSTR)L"Jokerman-Regular",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xccc5818c,	(FX_LPCWSTR)L"EuroSign",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xcf3d7234,	(FX_LPCWSTR)L"LucidaHandwriting-Italic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xcf7b8fdb,	(FX_LPCWSTR)L"MinionPro",	(FX_LPCWSTR)L"Bell MT,Corbel,Times New Roman,Cambria,Berlin Sans FB",	0,	1252},
    {0xcfe5755f,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0xd011f4ee,	(FX_LPCWSTR)L"MSPGothic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd060e7ef,	(FX_LPCWSTR)L"Vivaldi",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xd07edec1,	(FX_LPCWSTR)L"FranklinGothic-Medium",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd107243f,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xd1881562,	(FX_LPCWSTR)L"ArialNarrow",	(FX_LPCWSTR)L"Arial Narrow",	0,	1252},
    {0xd22b7dce,	(FX_LPCWSTR)L"BodoniMTPosterCompressed",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd22bfa60,	(FX_LPCWSTR)L"ComicSansMS",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xd3bd0e35,	(FX_LPCWSTR)L"Bauhaus93",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd429ee7a,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xd6679c12,	(FX_LPCWSTR)L"BernardMTCondensed",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd8e8a027,	(FX_LPCWSTR)L"LucidaSans",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xd9fe7761,	(FX_LPCWSTR)L"HighTowerText-Reg",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xda7e551e,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdaa6842d,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdaaab93f,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdaeb0713,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdafedbef,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdb00a3d9,	(FX_LPCWSTR)L"Broadway",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xdb1f5ad4,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdb326e7f,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdb69595a,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdba0082c,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdbd0ab18,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdc1a7db1,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xdc33075f,	(FX_LPCWSTR)L"KristenITC-Regular",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xdcc7009c,	(FX_LPCWSTR)L"Harrington",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xdd712466,	(FX_LPCWSTR)L"ArialBlack",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xdde87b3e,	(FX_LPCWSTR)L"Impact",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xdf69fb32,	(FX_LPCWSTR)L"SnapITC",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xdf8b25e8,	(FX_LPCWSTR)L"CenturyGothic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xe0f705c0,	(FX_LPCWSTR)L"KristenITC",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xe1427573,	(FX_LPCWSTR)L"Raavi",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xe2cea0cb,	(FX_LPCWSTR)L"Magneto",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xe36a9e17,	(FX_LPCWSTR)L"Ravie",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xe433f8e2,	(FX_LPCWSTR)L"Parchment",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xe43dff4a,	(FX_LPCWSTR)L"Wingdings",	(FX_LPCWSTR)L"Arial",	4,	42},
    {0xe4e2c405,	(FX_LPCWSTR)L"MTExtra",	(FX_LPCWSTR)L"Arial",	6,	42},
    {0xe618cc35,	(FX_LPCWSTR)L"InformalRoman",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xe6c27ffc,	(FX_LPCWSTR)L"Mistral",	(FX_LPCWSTR)L"Arial",	8,	1252},
    {0xe7ebf4b9,	(FX_LPCWSTR)L"Courier",	(FX_LPCWSTR)L"Courier New",	0,	1252},
    {0xe8bc4a9d,	(FX_LPCWSTR)L"MSReferenceSpecialty",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xe90fb013,	(FX_LPCWSTR)L"TempusSansITC",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xec637b42,	(FX_LPCWSTR)L"Consolas",	(FX_LPCWSTR)L"Verdana",	1,	1252},
    {0xed3a683b,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xef264cd1,	(FX_LPCWSTR)L"LucidaHandwriting",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xf086bca2,	(FX_LPCWSTR)L"BaskervilleOldFace",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xf1028030,	(FX_LPCWSTR)L"Mangal",	(FX_LPCWSTR)L"Arial",	2,	1252},
    {0xf1da7eb9,	(FX_LPCWSTR)L"ShowcardGothic",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xf210f06a,	(FX_LPCWSTR)L"ArialMT",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xf477f16a,	(FX_LPCWSTR)L"Latha",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xf616f3dd,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"Arial",	1,	936},
    {0xfa479aa6,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Arial",	0,	936},
    {0xfcd19697,	(FX_LPCWSTR)L"BookmanOldStyle",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xfe209a82,	(FX_LPCWSTR)L"LucidaCalligraphy",	(FX_LPCWSTR)L"Arial",	0,	1252},
    {0xfef135f8,	(FX_LPCWSTR)L"AdobeHeitiStd-Regular",	(FX_LPCWSTR)L"Batang,Century,Dotum",	0,	936},
};
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_
static const XFA_FONTINFO g_XFAFontsMap[] = {
    {0x01d5d33e,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x01e4f102,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,WenQuanYi Zen Hei Sharp,WenQuanYi Zen Hei,WenQuanYi Micro Hei",	1,	936},
    {0x030549dc,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Zen Hei Mono,WenQuanYi Micro Hei",	1,	936},
    {0x032edd44,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Zen Hei Mono,WenQuanYi Micro Hei",	1,	936},
    {0x03eac6fc,	(FX_LPCWSTR)L"PoorRichard-Regular",	(FX_LPCWSTR)L"FreeSerif",	2,	1252},
    {0x03ed90e6,	(FX_LPCWSTR)L"Nina",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x077b56b3,	(FX_LPCWSTR)L"KingsoftPhoneticPlain",	(FX_LPCWSTR)L"utkal,Kedage,Mallige,Kedage,AR PL UKai CN,AR PL UKai HK",	0,	1252},
    {0x078ed524,	(FX_LPCWSTR)L"MicrosoftSansSerif",	(FX_LPCWSTR)L"FreeSerif,WenQuanYi Micro Hei",	0,	1252},
    {0x089b18a9,	(FX_LPCWSTR)L"Arial",	(FX_LPCWSTR)L"DejaVu Sans Condensed,FreeSerif,WenQuanYi Micro Hei",	0,	1252},
    {0x0b2cad72,	(FX_LPCWSTR)L"MonotypeCorsiva",	(FX_LPCWSTR)L"FreeSerif",	8,	1252},
    {0x0bb003e7,	(FX_LPCWSTR)L"Kartika",	(FX_LPCWSTR)L"FreeSans,Liberation Sans,Nimbus Sans L,Garuda,FreeSerif,WenQuanYi Micro Hei",	2,	1252},
    {0x0bb469df,	(FX_LPCWSTR)L"VinerHandITC",	(FX_LPCWSTR)L"Ubuntu,Liberation Sans,Liberation Serif",	8,	1252},
    {0x0bc1a851,	(FX_LPCWSTR)L"SegoeUI",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x0c112ebd,	(FX_LPCWSTR)L"KozukaGothicPro-VIM",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x0cfcb9c1,	(FX_LPCWSTR)L"AdobeThai",	(FX_LPCWSTR)L" Waree",	0,	847},
    {0x0e7de0f9,	(FX_LPCWSTR)L"Playbill",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x0eff47c3,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0x107ad374,	(FX_LPCWSTR)L"Constantia",	(FX_LPCWSTR)L"FreeSerif,WenQuanYi Micro Hei,Ubuntu",	2,	1252},
    {0x12194c2d,	(FX_LPCWSTR)L"KunstlerScript",	(FX_LPCWSTR)L"Liberation Serif",	8,	1252},
    {0x135ef6a1,	(FX_LPCWSTR)L"MinionProSmBd",	(FX_LPCWSTR)L"Liberation Serif",	0,	1252},
    {0x158c4049,	(FX_LPCWSTR)L"Garamond",	(FX_LPCWSTR)L"Liberation Serif,Ubuntu,FreeSerif",	2,	1252},
    {0x160ecb24,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x161ed07e,	(FX_LPCWSTR)L"MSGothic",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	1,	1252},
    {0x171d1ed1,	(FX_LPCWSTR)L"SnapITC-Regular",	(FX_LPCWSTR)L"Nimbus Sans L,DejaVu Sans",	0,	1252},
    {0x18d1188f,	(FX_LPCWSTR)L"Cambria",	(FX_LPCWSTR)L"FreeSerif,FreeMono",	2,	1252},
    {0x18eaf350,	(FX_LPCWSTR)L"ArialUnicodeMS",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x1a92d115,	(FX_LPCWSTR)L"MingLiU",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	1252},
    {0x1cc217c6,	(FX_LPCWSTR)L"TrebuchetMS",	(FX_LPCWSTR)L"Liberation Serif,FreeSerif,Ubuntu",	0,	1252},
    {0x1d649596,	(FX_LPCWSTR)L"BasemicTimes",	(FX_LPCWSTR)L"Liberation Serif,FreeSerif,Ubuntu",	0,	1252},
    {0x1e34ee60,	(FX_LPCWSTR)L"BellMT",	(FX_LPCWSTR)L"Ubuntu,Liberation Serif",	2,	1252},
    {0x1eb36945,	(FX_LPCWSTR)L"CooperBlack",	(FX_LPCWSTR)L"FreeMono,Liberation Mono, WenQuanYi Micro Hei Mono",	2,	1252},
    {0x1ef7787d,	(FX_LPCWSTR)L"BatangChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	1252},
    {0x20b3bd3a,	(FX_LPCWSTR)L"BrushScriptMT",	(FX_LPCWSTR)L"URW Chancery L,Liberation Sans",	8,	1252},
    {0x220877aa,	(FX_LPCWSTR)L"Candara",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x22135007,	(FX_LPCWSTR)L"FreestyleScript-Regular",	(FX_LPCWSTR)L"Liberation Sans",	8,	1252},
    {0x251059c3,	(FX_LPCWSTR)L"Chiller",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x25bed6dd,	(FX_LPCWSTR)L"MSReferenceSansSerif",	(FX_LPCWSTR)L"DejaVu Sans Condensed,AR PL UKai HK",	0,	1252},
    {0x28154c81,	(FX_LPCWSTR)L"Parchment-Regular",	(FX_LPCWSTR)L"Liberation Sans",	8,	1252},
    {0x29711eb9,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"AR PL UKai HK",	0,	936},
    {0x2b1993b4,	(FX_LPCWSTR)L"Basemic",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x2b316339,	(FX_LPCWSTR)L"NiagaraSolid-Reg",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x2c147529,	(FX_LPCWSTR)L"FootlightMTLight",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x2c198928,	(FX_LPCWSTR)L"HarlowSolid",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x2c6ac6b2,	(FX_LPCWSTR)L"LucidaBright",	(FX_LPCWSTR)L"Liberation Sans",	2,	1252},
    {0x2c9f38e2,	(FX_LPCWSTR)L"KozukaMinchoPro-VIR",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x2d5a47b0,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"AR PL UKai HK",	0,	936},
    {0x2def26bf,	(FX_LPCWSTR)L"BernardMT-Condensed",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x2fd8930b,	(FX_LPCWSTR)L"KozukaMinchoPr6NR",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x3115525a,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x31327817,	(FX_LPCWSTR)L"MyriadPro",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x32244975,    (FX_LPCWSTR)L"Helvetica", (FX_LPCWSTR)L"Ubuntu,DejaVu Sans Condensed,Liberation Sans",	 0,   1252},
    {0x32ac995c,	(FX_LPCWSTR)L"Terminal",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x338d648a,	(FX_LPCWSTR)L"NiagaraEngraved-Reg",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x33bb65f2,	(FX_LPCWSTR)L"Sylfaen",	(FX_LPCWSTR)L"DejaVu Sans",	2,	1252},
    {0x3402c30e,	(FX_LPCWSTR)L"MSPMincho",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	2,	1252},
    {0x3412bf31,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x36eb39b9,	(FX_LPCWSTR)L"BerlinSansFB",	(FX_LPCWSTR)L"Liberation Serif,Ubuntu,FreeSerif",	0,	1252},
    {0x36f42055,	(FX_LPCWSTR)L"UniversATT",	(FX_LPCWSTR)L"Microsoft Sans Serif",	0,	1252},
    {0x3864c4f6,	(FX_LPCWSTR)L"HighTowerText",	(FX_LPCWSTR)L"DejaVu Serif",	2,	1252},
    {0x3a257d03,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x3c7d1d07,	(FX_LPCWSTR)L"Garamond3LTStd",  (FX_LPCWSTR)L"Ubuntu Condensed,DejaVu Sans Condensed,Liberation Serif,Ubuntu,FreeSerif",	2,	1252},
    {0x3cdae668,	(FX_LPCWSTR)L"FreestyleScript",	(FX_LPCWSTR)L"DejaVu Sans",	8,	1252},
    {0x3d55aed7,	(FX_LPCWSTR)L"Jokerman",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x3d5b4385,	(FX_LPCWSTR)L"PMingLiU",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	2,	1252},
    {0x3d9b7669,	(FX_LPCWSTR)L"EstrangeloEdessa",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x3e532d74,	(FX_LPCWSTR)L"FranklinGothicMedium",	(FX_LPCWSTR)L"Ubuntu",	0,	1252},
    {0x3e6aa32d,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	936},
    {0x3f6c36a8,	(FX_LPCWSTR)L"Gautami",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x3ff32662,	(FX_LPCWSTR)L"Chiller-Regular",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x409de312,	(FX_LPCWSTR)L"ModernNo.20",	(FX_LPCWSTR)L"Nimbus Sans L,Nimbus Sans L,FreeSans",	2,	1252},
    {0x41443c5e,	(FX_LPCWSTR)L"Georgia",	(FX_LPCWSTR)L"FreeSans",	2,	1252},
    {0x4160ade5,	(FX_LPCWSTR)L"BellGothicStdBlack",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x421976c4,	(FX_LPCWSTR)L"Modern-Regular",	(FX_LPCWSTR)L"FreeSans",	2,	1252},
    {0x422a7252,	(FX_LPCWSTR)L"Stencil",	(FX_LPCWSTR)L"FreeSans,Liberation Sans",	0,	1252},
    {0x42c8554f,	(FX_LPCWSTR)L"Fixedsys",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x435cb41d,	(FX_LPCWSTR)L"Roman",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x47882383,	(FX_LPCWSTR)L"CourierNew",	(FX_LPCWSTR)L"FreeMono,WenQuanYi Micro Hei Mono,AR PL UKai CN,AR PL UKai HK,AR PL UKai TW,AR PL UKai TW MBE,DejaVu Sans",	1,	1252},
    {0x480a2338,	(FX_LPCWSTR)L"BerlinSansFBDemi",	(FX_LPCWSTR)L" Liberation Serif",	0,	1252},
    {0x480bf7a4,	(FX_LPCWSTR)L"CourierStd",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x481ad6ed,	(FX_LPCWSTR)L"VladimirScript",	(FX_LPCWSTR)L"DejaVu Serif",	8,	1252},
    {0x4911577a,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	936},
    {0x4a788d72,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0x4bf88566,	(FX_LPCWSTR)L"SegoeCondensed",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x4ccf51a4,	(FX_LPCWSTR)L"BerlinSansFB-Reg",	(FX_LPCWSTR)L"Liberation Serif",	0,	1252},
    {0x4ea967ce,	(FX_LPCWSTR)L"GulimChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UKai CN,AR PL UKai HK,AR PL UKai TW,AR PL UKai TW MBE,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	1252},
    {0x4f68bd79,	(FX_LPCWSTR)L"LetterGothicStd",	(FX_LPCWSTR)L"FreeMono,Liberation Mono,WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,AR PL UKai CN,AR PL UKai HK,AR PL UKai TW,AR PL UKai TW MBE",	0,	1252},
    {0x51a0d0e6,	(FX_LPCWSTR)L"KozukaGothicPr6NM",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x531b3dea,	(FX_LPCWSTR)L"BasemicSymbol",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x5333fd39,	(FX_LPCWSTR)L"CalifornianFB-Reg",	(FX_LPCWSTR)L"URW Chancery L,FreeSerif",	2,	1252},
    {0x53561a54,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x55e0dde6,	(FX_LPCWSTR)L"LucidaSansTypewriter",	(FX_LPCWSTR)L"DejaVu Sans Mono,Nimbus Mono L,Liberation Mono,Courier 10 Pitch,FreeMono",	0,	1252},
    {0x574d4d3d,	(FX_LPCWSTR)L"AdobeArabic",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x5792e759,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono",	0,	936},
    {0x5921978e,	(FX_LPCWSTR)L"LucidaSansUnicode",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x594e2da4,	(FX_LPCWSTR)L"Vrinda",	(FX_LPCWSTR)L"FreeSans,FreeSerif",	0,	1252},
    {0x59baa9a2,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x5cfedf4f,	(FX_LPCWSTR)L"BaskOldFace",	(FX_LPCWSTR)L"Ubuntu,Liberation Serif",	0,	1252},
    {0x5e16ac91,    (FX_LPCWSTR)L"TrajanPro",   (FX_LPCWSTR)L"Nimbus Sans L,AR PL UMing HK,AR PL UKai HK,AR PL UMing TW,AR PL UMing TW MBE,DejaVu Sans,DejaVu Serif",  0,  1252},
    {0x5f388196,    (FX_LPCWSTR)L"ITCLegacySansStdMedium",  (FX_LPCWSTR)L"Liberation Serif,FreeSerif,FreeSans,Ubuntu", 0, 1252},
    {0x5f97921c,	(FX_LPCWSTR)L"AdobeMyungjoStdM",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x5fefbfad,	(FX_LPCWSTR)L"Batang",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	2,	1252},
    {0x605342b9,	(FX_LPCWSTR)L"DotumChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UKai CN,AR PL UKai HK,AR PL UKai TW,AR PL UKai TW MBE,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	1252},
    {0x608c5f9a,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x61efd0d1,	(FX_LPCWSTR)L"MaturaMTScriptCapitals",	(FX_LPCWSTR)L"DejaVu Serif,DejaVu Sans",	0,	1252},
    {0x626608a9,	(FX_LPCWSTR)L"MVBoli",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x630501a3,	(FX_LPCWSTR)L"SmallFonts",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x65d0e2a9,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x669f29e1,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"AR PL UMing CN,AR PL UKai CN, AR PL UMing HK",	0,	936},
    {0x673a9e5f,	(FX_LPCWSTR)L"Tunga",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x691aa4ce,	(FX_LPCWSTR)L"NiagaraSolid",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x696259b7,	(FX_LPCWSTR)L"Corbel",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0x696ee9be,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono",	0,	936},
    {0x6c59cf69,	(FX_LPCWSTR)L"Dotum",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x707fa561,	(FX_LPCWSTR)L"Gungsuh",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	2,	1252},
    {0x71416bb2,	(FX_LPCWSTR)L"ZWAdobeF",	(FX_LPCWSTR)L"Dingbats,FreeSerif",	0,	1252},
    {0x71b41801,	(FX_LPCWSTR)L"Verdana",	(FX_LPCWSTR)L"DejaVu Sans Condensed,DejaVu Sans",	0,	1252},
    {0x73f25e4c,	(FX_LPCWSTR)L"PalatinoLinotype",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x73f4d19f,	(FX_LPCWSTR)L"NiagaraEngraved",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0x74001694,	(FX_LPCWSTR)L"MyriadProBlack",	(FX_LPCWSTR)L"AR PL UKai HK",	0,	1252},
    {0x74b14d8f,	(FX_LPCWSTR)L"Haettenschweiler",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0x74cb44ee,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	936},
    {0x76b4d7ff,	(FX_LPCWSTR)L"Shruti",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x788b3533,	(FX_LPCWSTR)L"Webdings",	(FX_LPCWSTR)L"FreeSans",	6,	42},
    {0x797dde99,	(FX_LPCWSTR)L"MSSerif",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x7a0f9e9e,	(FX_LPCWSTR)L"MSMincho",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono",	1,	1252},
    {0x7b439caf,	(FX_LPCWSTR)L"OldEnglishTextMT",	(FX_LPCWSTR)L"Liberation Sans,Ubuntu",	0,	1252},
    {0x8213a433,	(FX_LPCWSTR)L"LucidaSans-Typewriter",	(FX_LPCWSTR)L"Liberation Mono",	0,	1252},
    {0x82fec929,	(FX_LPCWSTR)L"AdobeSongStdL",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0x83581825,	(FX_LPCWSTR)L"Modern",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x835a2823,	(FX_LPCWSTR)L"Algerian",	(FX_LPCWSTR)L"FreeSans,Liberation Sans,Ubuntu",	0,	1252},
    {0x83dab9f5,	(FX_LPCWSTR)L"Script",	(FX_LPCWSTR)L"FreeSans",	0,	1252},
    {0x847b56da,	(FX_LPCWSTR)L"Tahoma",	(FX_LPCWSTR)L"DejaVu Sans Condensed,FreeSerif",	0,	1252},
    {0x8a783cb2,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x8b5cac0e,	(FX_LPCWSTR)L"Onyx",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x8c6a499e,	(FX_LPCWSTR)L"Gulim",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0x8e0af790,	(FX_LPCWSTR)L"JuiceITC",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x8e8d43b2,	(FX_LPCWSTR)L"Centaur",	(FX_LPCWSTR)L"Khmer OS,Khmer OS System",	2,	1252},
    {0x8ee4dcca,	(FX_LPCWSTR)L"BookshelfSymbol7",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x90794800,	(FX_LPCWSTR)L"BellGothicStdLight",	(FX_LPCWSTR)L"Liberation Sans",	0,	1252},
    {0x909b516a,	(FX_LPCWSTR)L"Century",	(FX_LPCWSTR)L"Liberation Sans,Liberation Mono,Liberation Serif",	2,	1252},
    {0x92ae370d,	(FX_LPCWSTR)L"MSOutlook",	(FX_LPCWSTR)L"Liberation Sans",	4,	42},
    {0x93c9fbf1,	(FX_LPCWSTR)L"LucidaFax",	(FX_LPCWSTR)L"Liberation Sans",	2,	1252},
    {0x9565085e,	(FX_LPCWSTR)L"BookAntiqua",	(FX_LPCWSTR)L"Liberation Sans,Liberation Serif",	2,	1252},
    {0x9856d95d,	(FX_LPCWSTR)L"AdobeMingStdL",	(FX_LPCWSTR)L"AR PL UMing HK",	0,	949},
    {0x9bbadd6b,	(FX_LPCWSTR)L"ColonnaMT",	(FX_LPCWSTR)L"Khmer OS,Khmer OS System",	0,	1252},
    {0x9cbd16a4,	(FX_LPCWSTR)L"ShowcardGothic-Reg",	(FX_LPCWSTR)L"Liberation Sans,Ubuntu",	0,	1252},
    {0x9d73008e,	(FX_LPCWSTR)L"MSSansSerif",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0xa0607db1,	(FX_LPCWSTR)L"GungsuhChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	1252},
    {0xa0bcf6a1,	(FX_LPCWSTR)L"LatinWide",	(FX_LPCWSTR)L"FreeSerif",	2,	1252},
    {0xa1429b36,	(FX_LPCWSTR)L"Symbol",	(FX_LPCWSTR)L"FreeSerif",	6,	42},
    {0xa1fa5abc,	(FX_LPCWSTR)L"Wingdings2",	(FX_LPCWSTR)L"FreeSerif",	6,	42},
    {0xa1fa5abd,	(FX_LPCWSTR)L"Wingdings3",	(FX_LPCWSTR)L"FreeSerif",	6,	42},
    {0xa427bad4,	(FX_LPCWSTR)L"InformalRoman-Regular",	(FX_LPCWSTR)L"FreeSerif",	8,	1252},
    {0xa8b92ece,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"AR PL UMing CN",	0,	936},
    {0xa8d83ece,	(FX_LPCWSTR)L"CalifornianFB",	(FX_LPCWSTR)L"FreeSerif",	2,	1252},
    {0xaa3e082c,	(FX_LPCWSTR)L"Kingsoft-Phonetic",	(FX_LPCWSTR)L"utkal,Kedage,Mallige,AR PL UKai CN",	0,	1252},
    {0xaa6bcabe,	(FX_LPCWSTR)L"HarlowSolidItalic",	(FX_LPCWSTR)L"Liberation Serif",	0,	1252},
    {0xade5337c,	(FX_LPCWSTR)L"MSUIGothic",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0xb08dd941,	(FX_LPCWSTR)L"WideLatin",	(FX_LPCWSTR)L"Liberation Serif",	2,	1252},
    {0xb12765e0,    (FX_LPCWSTR)L"ITCLegacySansStdBook",  (FX_LPCWSTR)L"AR PL UMing HK,AR PL UKai HK,FreeSerif,Ubuntu,FreeSans",   0,  1252},
    {0xb207f05d,	(FX_LPCWSTR)L"PoorRichard",	(FX_LPCWSTR)L"Liberation Serif",	2,	1252},
    {0xb3bc492f,	(FX_LPCWSTR)L"JuiceITC-Regular",	(FX_LPCWSTR)L"Liberation Serif",	0,	1252},
    {0xb5545399,	(FX_LPCWSTR)L"Marlett",	(FX_LPCWSTR)L"Liberation Serif",	4,	42},
    {0xb5dd1ebb,	(FX_LPCWSTR)L"BritannicBold",	(FX_LPCWSTR)L"Liberation Serif",	0,	1252},
    {0xb699c1c5,	(FX_LPCWSTR)L"LucidaCalligraphy-Italic",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xb725d629,	(FX_LPCWSTR)L"TimesNewRoman",	(FX_LPCWSTR)L"Liberation Sans",	2,	1252},
    {0xb7eaebeb,	(FX_LPCWSTR)L"AdobeHeitiStdR",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0xbd29c486,	(FX_LPCWSTR)L"BerlinSansFBDemi-Bold",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xbe8a8db4,	(FX_LPCWSTR)L"BookshelfSymbolSeven",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0xc16c0118,	(FX_LPCWSTR)L"AdobeHebrew",	(FX_LPCWSTR)L"Ubuntu,Liberation Serif",	0,	1252},
    {0xc318b0af,	(FX_LPCWSTR)L"MyriadProLight",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	1252},
    {0xc65e5659,	(FX_LPCWSTR)L"CambriaMath",	(FX_LPCWSTR)L"FreeSerif,FreeMono",	2,	1252},
    {0xc75c8f05,	(FX_LPCWSTR)L"LucidaConsole",	(FX_LPCWSTR)L"DejaVu Sans Mono,FreeMono,Liberation Mono,WenQuanYi Micro Hei Mono",	1,	1252},
    {0xca7c35d6,	(FX_LPCWSTR)L"Calibri",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0xcb053f53,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0xcb7190f9,	(FX_LPCWSTR)L"Magneto-Bold",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xcca00cc5,	(FX_LPCWSTR)L"System",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0xccad6f76,	(FX_LPCWSTR)L"Jokerman-Regular",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0xccc5818c,	(FX_LPCWSTR)L"EuroSign",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xcf3d7234,	(FX_LPCWSTR)L"LucidaHandwriting-Italic",	(FX_LPCWSTR)L"Nimbus Sans L,DejaVu Serif",	0,	1252},
    {0xcf7b8fdb,	(FX_LPCWSTR)L"MinionPro",	(FX_LPCWSTR)L"DejaVu Sans",	0,	1252},
    {0xcfe5755f,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	936},
    {0xd011f4ee,	(FX_LPCWSTR)L"MSPGothic",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	1252},
    {0xd060e7ef,	(FX_LPCWSTR)L"Vivaldi",	(FX_LPCWSTR)L"Liberation Sans,Ubuntu",	8,	1252},
    {0xd07edec1,	(FX_LPCWSTR)L"FranklinGothic-Medium",	(FX_LPCWSTR)L"Ubuntu",	0,	1252},
    {0xd107243f,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0xd1881562,	(FX_LPCWSTR)L"ArialNarrow",	(FX_LPCWSTR)L"FreeSerif",	0,	1252},
    {0xd22b7dce,	(FX_LPCWSTR)L"BodoniMTPosterCompressed",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xd22bfa60,	(FX_LPCWSTR)L"ComicSansMS",	(FX_LPCWSTR)L"FreeMono,Liberation Mono",	8,	1252},
    {0xd3bd0e35,	(FX_LPCWSTR)L"Bauhaus93",	(FX_LPCWSTR)L"Liberation Sans,Ubuntu",	0,	1252},
    {0xd429ee7a,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono",	0,	936},
    {0xd6679c12,	(FX_LPCWSTR)L"BernardMTCondensed",	(FX_LPCWSTR)L"Nimbus Sans L,URW Chancery L,KacstOne,Liberation Sans",	0,	1252},
    {0xd8e8a027,	(FX_LPCWSTR)L"LucidaSans",	(FX_LPCWSTR)L"Nimbus Sans L,DejaVu Serif Condensed,Liberation Mono,Ubuntu",	0,	1252},
    {0xd9fe7761,	(FX_LPCWSTR)L"HighTowerText-Reg",	(FX_LPCWSTR)L"Ubuntu,Liberation Serif",	2,	1252},
    {0xda7e551e,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono",	0,	936},
    {0xdaa6842d,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0xdaaab93f,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	0,	936},
    {0xdaeb0713,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	0,	936},
    {0xdafedbef,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0xdb00a3d9,	(FX_LPCWSTR)L"Broadway",	(FX_LPCWSTR)L"DejaVu Sans,FreeMono,Liberation Mono",	0,	1252},
    {0xdb1f5ad4,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0xdb326e7f,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	0,	936},
    {0xdb69595a,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	0,	936},
    {0xdba0082c,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L" WenQuanYi Micro Hei Mono,WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp",	0,	936},
    {0xdbd0ab18,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0xdc1a7db1,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0xdc33075f,	(FX_LPCWSTR)L"KristenITC-Regular",	(FX_LPCWSTR)L"DejaVu Sans Condensed,Ubuntu,Liberation Sans",	8,	1252},
    {0xdcc7009c,	(FX_LPCWSTR)L"Harrington",	(FX_LPCWSTR)L"Liberation Serif,FreeSerif,Ubuntu",	0,	1252},
    {0xdd712466,	(FX_LPCWSTR)L"ArialBlack",	(FX_LPCWSTR)L"DejaVu Sans,DejaVu Serif,FreeMono",	0,	1252},
    {0xdde87b3e,	(FX_LPCWSTR)L"Impact",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xdf69fb32,	(FX_LPCWSTR)L"SnapITC",	(FX_LPCWSTR)L"DejaVu Sans,DejaVu Serif,FreeMono",	0,	1252},
    {0xdf8b25e8,	(FX_LPCWSTR)L"CenturyGothic",	(FX_LPCWSTR)L"Liberation Mono,Liberation Sans,Liberation Serif",	0,	1252},
    {0xe0f705c0,	(FX_LPCWSTR)L"KristenITC",	(FX_LPCWSTR)L"DejaVu Sans Condensed,Ubuntu,Liberation Sans",	8,	1252},
    {0xe1427573,	(FX_LPCWSTR)L"Raavi",	(FX_LPCWSTR)L"FreeSerif,Liberation Serif,Khmer OS",	0,	1252},
    {0xe2cea0cb,	(FX_LPCWSTR)L"Magneto",	(FX_LPCWSTR)L"DejaVu Serif,DejaVu Serif Condensed,DejaVu Sans",	0,	1252},
    {0xe36a9e17,	(FX_LPCWSTR)L"Ravie",	(FX_LPCWSTR)L"DejaVu Serif,DejaVu Sans,FreeMono",	0,	1252},
    {0xe433f8e2,	(FX_LPCWSTR)L"Parchment",	(FX_LPCWSTR)L"DejaVu Serif",	8,	1252},
    {0xe43dff4a,	(FX_LPCWSTR)L"Wingdings",	(FX_LPCWSTR)L"DejaVu Serif",	4,	42},
    {0xe4e2c405,	(FX_LPCWSTR)L"MTExtra",	(FX_LPCWSTR)L"DejaVu Serif",	6,	42},
    {0xe618cc35,	(FX_LPCWSTR)L"InformalRoman",	(FX_LPCWSTR)L"Nimbus Sans L,DejaVu Sans Condensed,Ubuntu,Liberation Sans",	8,	1252},
    {0xe6c27ffc,	(FX_LPCWSTR)L"Mistral",	(FX_LPCWSTR)L"DejaVu Serif",	8,	1252},
    {0xe7ebf4b9,	(FX_LPCWSTR)L"Courier",	(FX_LPCWSTR)L"DejaVu Sans,DejaVu Sans Condensed,FreeSerif",	0,	1252},
    {0xe8bc4a9d,	(FX_LPCWSTR)L"MSReferenceSpecialty",	(FX_LPCWSTR)L"DejaVu Serif",	0,	1252},
    {0xe90fb013,	(FX_LPCWSTR)L"TempusSansITC",	(FX_LPCWSTR)L"Ubuntu,Liberation Serif,FreeSerif",	0,	1252},
    {0xec637b42,	(FX_LPCWSTR)L"Consolas",	(FX_LPCWSTR)L"DejaVu Sans Condensed,FreeSerif,FreeSans",	1,	1252},
    {0xed3a683b,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"AR PL UKai HK,AR PL UMing HK,AR PL UKai CN",	0,	936},
    {0xef264cd1,	(FX_LPCWSTR)L"LucidaHandwriting",	(FX_LPCWSTR)L"DejaVu Serif,DejaVu Sans,FreeMono,Liberation Mono",	0,	1252},
    {0xf086bca2,	(FX_LPCWSTR)L"BaskervilleOldFace",	(FX_LPCWSTR)L"Liberation Serif,Ubuntu,FreeSerif",	0,	1252},
    {0xf1028030,	(FX_LPCWSTR)L"Mangal",	(FX_LPCWSTR)L"FreeSans,Garuda,Liberation Sans,Nimbus Sans L,FreeSerif,WenQuanYi Micro Hei",	2,	1252},
    {0xf1da7eb9,	(FX_LPCWSTR)L"ShowcardGothic",	(FX_LPCWSTR)L"DejaVu Serif Condensed,DejaVu Sans Condensed,Liberation Sans,Ubuntu",	0,	1252},
    {0xf210f06a,	(FX_LPCWSTR)L"ArialMT",	(FX_LPCWSTR)L"Liberation Sans,FreeSans,Nimbus Sans L,Khmer OS System,Khmer OS",	0,	1252},
    {0xf477f16a,	(FX_LPCWSTR)L"Latha",	(FX_LPCWSTR)L"Nimbus Sans L,FreeSerif,Nimbus Sans L",	0,	1252},
    {0xf616f3dd,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	1,	936},
    {0xfa479aa6,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
    {0xfcd19697,	(FX_LPCWSTR)L"BookmanOldStyle",	(FX_LPCWSTR)L"Liberation Mono,Liberation Sans,Liberation Serif",	0,	1252},
    {0xfe209a82,	(FX_LPCWSTR)L"LucidaCalligraphy",	(FX_LPCWSTR)L"DejaVu Serif,DejaVu Sans,FreeMono",	0,	1252},
    {0xfef135f8,	(FX_LPCWSTR)L"AdobeHeitiStd-Regular",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,WenQuanYi Zen Hei,WenQuanYi Zen Hei Sharp,WenQuanYi Micro Hei",	0,	936},
};
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
static const XFA_FONTINFO g_XFAFontsMap[] = {
    {0x01d5d33e,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	0,	936},
    {0x01e4f102,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0x030549dc,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0x032edd44,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0x03eac6fc,	(FX_LPCWSTR)L"PoorRichard-Regular",	(FX_LPCWSTR)L"Noteworthy,Avenir Next Condensed,Impact",	2,	1252},
    {0x03ed90e6,	(FX_LPCWSTR)L"Nina",	(FX_LPCWSTR)L"Microsoft Sans Serif",	0,	1252},
    {0x077b56b3,	(FX_LPCWSTR)L"KingsoftPhoneticPlain",	(FX_LPCWSTR)L"LastResort,Apple Chancery,STIXVariants,STIXSizeOneSym,STIXSizeOneSym,Apple Braille",	0,	1252},
    {0x078ed524,	(FX_LPCWSTR)L"MicrosoftSansSerif",	(FX_LPCWSTR)L"Songti SC,Apple Symbols",	0,	1252},
    {0x089b18a9,	(FX_LPCWSTR)L"Arial",	(FX_LPCWSTR)L"Arial Unicode MS,Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x0b2cad72,	(FX_LPCWSTR)L"MonotypeCorsiva",	(FX_LPCWSTR)L"Arial Narrow,Impact",	8,	1252},
    {0x0bb003e7,	(FX_LPCWSTR)L"Kartika",	(FX_LPCWSTR)L"Arial Unicode MS,Microsoft Sans Serif,Arial Narrow,Damascus",	2,	1252},
    {0x0bb469df,	(FX_LPCWSTR)L"VinerHandITC",	(FX_LPCWSTR)L"Comic Sans MS,Songti SC,STSong",	8,	1252},
    {0x0bc1a851,	(FX_LPCWSTR)L"SegoeUI",	(FX_LPCWSTR)L"Apple Symbols",	0,	1252},
    {0x0c112ebd,	(FX_LPCWSTR)L"KozukaGothicPro-VIM",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x0cfcb9c1,	(FX_LPCWSTR)L"AdobeThai",	(FX_LPCWSTR)L"Avenir Next Condensed Ultra Light",	0,	847},
    {0x0e7de0f9,	(FX_LPCWSTR)L"Playbill",	(FX_LPCWSTR)L"STIXNonUnicode",	0,	1252},
    {0x0eff47c3,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Kaiti SC,Songti SC,STHeiti",	0,	936},
    {0x107ad374,	(FX_LPCWSTR)L"Constantia",	(FX_LPCWSTR)L"Arial Unicode MS,Palatino,Baskerville",	2,	1252},
    {0x12194c2d,	(FX_LPCWSTR)L"KunstlerScript",	(FX_LPCWSTR)L"Avenir Next Condensed Demi Bold,Arial Narrow",	8,	1252},
    {0x135ef6a1,	(FX_LPCWSTR)L"MinionProSmBd",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x158c4049,	(FX_LPCWSTR)L"Garamond",	(FX_LPCWSTR)L"Impact,Arial Narrow",	2,	1252},
    {0x160ecb24,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"STFangsong,Songti SC",	0,	936},
    {0x161ed07e,	(FX_LPCWSTR)L"MSGothic",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,Microsoft Sans Serif,Apple Symbols",	1,	1252},
    {0x171d1ed1,	(FX_LPCWSTR)L"SnapITC-Regular",	(FX_LPCWSTR)L"STHeiti,Arial Black",	0,	1252},
    {0x18d1188f,	(FX_LPCWSTR)L"Cambria",	(FX_LPCWSTR)L"Arial Unicode MS",	2,	1252},
    {0x18eaf350,	(FX_LPCWSTR)L"ArialUnicodeMS",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	936},
    {0x1a92d115,	(FX_LPCWSTR)L"MingLiU",	(FX_LPCWSTR)L"Heiti SC,STHeiti",	1,	1252},
    {0x1cc217c6,	(FX_LPCWSTR)L"TrebuchetMS",	(FX_LPCWSTR)L"Damascus,Impact,Arial Narrow",	0,	1252},
    {0x1d649596,	(FX_LPCWSTR)L"BasemicTimes",	(FX_LPCWSTR)L"Liberation Serif,Impact,Arial Narrow",	0,	1252},
    {0x1e34ee60,	(FX_LPCWSTR)L"BellMT",	(FX_LPCWSTR)L"Papyrus,STIXNonUnicode,Microsoft Sans Serif,Avenir Light",	2,	1252},
    {0x1eb36945,	(FX_LPCWSTR)L"CooperBlack",	(FX_LPCWSTR)L"Marion,STIXNonUnicode,Arial Rounded MT Bold,Lucida Grande",	2,	1252},
    {0x1ef7787d,	(FX_LPCWSTR)L"BatangChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,Arial Unicode MS,Heiti TC",	1,	1252},
    {0x20b3bd3a,	(FX_LPCWSTR)L"BrushScriptMT",	(FX_LPCWSTR)L"STIXNonUnicode,Damascus,Arial Narrow,Avenir Next Condensed,Cochin",	8,	1252},
    {0x220877aa,	(FX_LPCWSTR)L"Candara",	(FX_LPCWSTR)L"Cochin,Baskerville,Marion",	0,	1252},
    {0x22135007,	(FX_LPCWSTR)L"FreestyleScript-Regular",	(FX_LPCWSTR)L"STIXNonUnicode,Nadeem,Zapf Dingbats",	8,	1252},
    {0x251059c3,	(FX_LPCWSTR)L"Chiller",	(FX_LPCWSTR)L"Zapf Dingbats,Damascus,STIXNonUnicode,Papyrus,KufiStandardGK,Baghdad",	0,	1252},
    {0x25bed6dd,	(FX_LPCWSTR)L"MSReferenceSansSerif",	(FX_LPCWSTR)L"Tahoma,Apple Symbols,Apple LiGothic,Arial Unicode MS,Lucida Grande,Microsoft Sans Serif",	0,	1252},
    {0x28154c81,	(FX_LPCWSTR)L"Parchment-Regular",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	8,	1252},
    {0x29711eb9,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Kaiti SC,Songti SC",	0,	936},
    {0x2b1993b4,	(FX_LPCWSTR)L"Basemic",	(FX_LPCWSTR)L"Impact,Arial Narrow",	0,	1252},
    {0x2b316339,	(FX_LPCWSTR)L"NiagaraSolid-Reg",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x2c147529,	(FX_LPCWSTR)L"FootlightMTLight",	(FX_LPCWSTR)L"STIXNonUnicode,Avenir Next Condensed Heavy,PT Sans,Noteworthy",	0,	1252},
    {0x2c198928,	(FX_LPCWSTR)L"HarlowSolid",	(FX_LPCWSTR)L"Avenir Medium,Avenir Next Medium,Arial Unicode MS",	0,	1252},
    {0x2c6ac6b2,	(FX_LPCWSTR)L"LucidaBright",	(FX_LPCWSTR)L"PT Sans Narrow,Papyrus,Damascus,STIXNonUnicode,Arial Rounded MT Bold,Comic Sans MS,Avenir Next",	2,	1252},
    {0x2c9f38e2,	(FX_LPCWSTR)L"KozukaMinchoPro-VIR",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x2d5a47b0,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Kaiti SC,Songti SC",	0,	936},
    {0x2def26bf,	(FX_LPCWSTR)L"BernardMT-Condensed",	(FX_LPCWSTR)L"Impact,Avenir Next Condensed Demi Bold,American Typewriter",	0,	1252},
    {0x2fd8930b,	(FX_LPCWSTR)L"KozukaMinchoPr6NR",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x3115525a,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Hiragino Sans GB,STHeiti",	0,	1252},
    {0x31327817,	(FX_LPCWSTR)L"MyriadPro",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x32244975,    (FX_LPCWSTR)L"Helvetica", (FX_LPCWSTR)L"Arial Narrow,Arial Unicode MS,Damascus,STIXNonUnicode,Liberation Sans,Nimbus Sans L,Avenir Next Medium",	 0,   1252},
    {0x32ac995c,	(FX_LPCWSTR)L"Terminal",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x338d648a,	(FX_LPCWSTR)L"NiagaraEngraved-Reg",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x33bb65f2,	(FX_LPCWSTR)L"Sylfaen",	(FX_LPCWSTR)L"Arial Unicode MS,Marion",	2,	1252},
    {0x3402c30e,	(FX_LPCWSTR)L"MSPMincho",	(FX_LPCWSTR)L"Arial Unicode MS,Apple SD Gothic Neo",	2,	1252},
    {0x3412bf31,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	0,	936},
    {0x36eb39b9,	(FX_LPCWSTR)L"BerlinSansFB",	(FX_LPCWSTR)L"American Typewriter,Impact",	0,	1252},
    {0x36f42055,	(FX_LPCWSTR)L"UniversATT",	(FX_LPCWSTR)L"Microsoft Sans Serif",	0,	1252},
    {0x3864c4f6,	(FX_LPCWSTR)L"HighTowerText",	(FX_LPCWSTR)L"STIXGeneral,.Helvetica Neue Desk UI",	2,	1252},
    {0x3a257d03,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Hiragino Sans GB,STHeiti",	0,	1252},
    {0x3cdae668,	(FX_LPCWSTR)L"FreestyleScript",	(FX_LPCWSTR)L"Nadeem,Zapf Dingbats,STIXNonUnicode",	8,	1252},
    {0x3d55aed7,	(FX_LPCWSTR)L"Jokerman",	(FX_LPCWSTR)L"Papyrus,Lucida Grande,Heiti TC,American Typewriter",	0,	1252},
    {0x3d5b4385,	(FX_LPCWSTR)L"PMingLiU",	(FX_LPCWSTR)L"Heiti SC,STHeiti",	2,	1252},
    {0x3d9b7669,	(FX_LPCWSTR)L"EstrangeloEdessa",	(FX_LPCWSTR)L"American Typewriter,Marion",	0,	1252},
    {0x3e532d74,	(FX_LPCWSTR)L"FranklinGothicMedium",	(FX_LPCWSTR)L"Impact,Arial Narrow",	0,	1252},
    {0x3e6aa32d,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"STHeiti,STFangsong",	1,	936},
    {0x3f6c36a8,	(FX_LPCWSTR)L"Gautami",	(FX_LPCWSTR)L"Damascus,STIXNonUnicode,STIXGeneral,American Typewriter",	0,	1252},
    {0x3ff32662,	(FX_LPCWSTR)L"Chiller-Regular",	(FX_LPCWSTR)L"Papyrus,KufiStandardGK,Baghdad",	0,	1252},
    {0x409de312,	(FX_LPCWSTR)L"ModernNo.20",	(FX_LPCWSTR)L"Avenir Next Condensed,Impact",	2,	1252},
    {0x41443c5e,	(FX_LPCWSTR)L"Georgia",	(FX_LPCWSTR)L".Helvetica Neue Desk UI,Arial Unicode MS",	2,	1252},
    {0x4160ade5,	(FX_LPCWSTR)L"BellGothicStdBlack",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x421976c4,	(FX_LPCWSTR)L"Modern-Regular",	(FX_LPCWSTR)L"Impact",	2,	1252},
    {0x422a7252,	(FX_LPCWSTR)L"Stencil",	(FX_LPCWSTR)L"STIXNonUnicode,Songti SC,Georgia,Baskerville",	0,	1252},
    {0x42c8554f,	(FX_LPCWSTR)L"Fixedsys",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x435cb41d,	(FX_LPCWSTR)L"Roman",	(FX_LPCWSTR)L"Arial Narrow",	0,	1252},
    {0x47882383,	(FX_LPCWSTR)L"CourierNew",	(FX_LPCWSTR)L"PCMyungjo,Osaka,Arial Unicode MS,Songti SC",	1,	1252},
    {0x480a2338,	(FX_LPCWSTR)L"BerlinSansFBDemi",	(FX_LPCWSTR)L"STIXNonUnicode,American Typewriter,Avenir Next Condensed Heavy",	0,	1252},
    {0x480bf7a4,	(FX_LPCWSTR)L"CourierStd",	(FX_LPCWSTR)L"Courier New",	0,	1252},
    {0x481ad6ed,	(FX_LPCWSTR)L"VladimirScript",	(FX_LPCWSTR)L"STIXNonUnicode,Avenir Next Condensed,Impact",	8,	1252},
    {0x4911577a,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"STHeiti,Heiti TC",	1,	936},
    {0x4a788d72,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Kaiti SC,Songti SC",	0,	936},
    {0x4bf88566,	(FX_LPCWSTR)L"SegoeCondensed",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x4ccf51a4,	(FX_LPCWSTR)L"BerlinSansFB-Reg",	(FX_LPCWSTR)L"STIXNonUnicode,American Typewriter,Impact",	0,	1252},
    {0x4ea967ce,	(FX_LPCWSTR)L"GulimChe",	(FX_LPCWSTR)L"Arial Unicode MS,Heiti TC,STFangsong",	1,	1252},
    {0x4f68bd79,	(FX_LPCWSTR)L"LetterGothicStd",	(FX_LPCWSTR)L"Courier New,Andale Mono,Ayuthaya,PCMyungjo,Osaka",	0,	1252},
    {0x51a0d0e6,	(FX_LPCWSTR)L"KozukaGothicPr6NM",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x531b3dea,	(FX_LPCWSTR)L"BasemicSymbol",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x5333fd39,	(FX_LPCWSTR)L"CalifornianFB-Reg",	(FX_LPCWSTR)L"American Typewriter,Avenir Next Condensed,Impact",	2,	1252},
    {0x53561a54,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"STFangsong,Songti SC,STSong",	0,	936},
    {0x55e0dde6,	(FX_LPCWSTR)L"LucidaSansTypewriter",	(FX_LPCWSTR)L"Menlo,Courier New,Andale Mono",	0,	1252},
    {0x574d4d3d,	(FX_LPCWSTR)L"AdobeArabic",	(FX_LPCWSTR)L"Arial Narrow",	0,	1252},
    {0x5792e759,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0x5921978e,	(FX_LPCWSTR)L"LucidaSansUnicode",	(FX_LPCWSTR)L"Lucida Grande,Arial Unicode MS,Menlo",	0,	1252},
    {0x594e2da4,	(FX_LPCWSTR)L"Vrinda",	(FX_LPCWSTR)L"Geeza Pro,Damascus,STIXGeneral,Gill Sans",	0,	1252},
    {0x59baa9a2,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Hiragino Sans GB,STHeiti",	0,	1252},
    {0x5cfedf4f,	(FX_LPCWSTR)L"BaskOldFace",	(FX_LPCWSTR)L"Avenir Next Condensed Heavy,PT Sans,Avenir Next Condensed",	0,	1252},
    {0x5e16ac91,    (FX_LPCWSTR)L"TrajanPro",   (FX_LPCWSTR)L"Arial Narrow,PT Sans Narrow,Damascus",  0,  1252},
    {0x5f97921c,	(FX_LPCWSTR)L"AdobeMyungjoStdM",	(FX_LPCWSTR)L"AppleMyungjo,AppleGothic,Arial Unicode MS",	0,	936},
    {0x5fefbfad,	(FX_LPCWSTR)L"Batang",	(FX_LPCWSTR)L"Arial Unicode MS,Songti SC",	2,	1252},
    {0x605342b9,	(FX_LPCWSTR)L"DotumChe",	(FX_LPCWSTR)L"Arial Unicode MS,Heiti TC",	1,	1252},
    {0x608c5f9a,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Hiragino Sans GB,STHeiti,Heiti TC",	0,	936},
    {0x61efd0d1,	(FX_LPCWSTR)L"MaturaMTScriptCapitals",	(FX_LPCWSTR)L"Kokonor,Damascus,STIXNonUnicode,STHeiti,Arial Black,Avenir Next Heavy",	0,	1252},
    {0x626608a9,	(FX_LPCWSTR)L"MVBoli",	(FX_LPCWSTR)L"Apple Braille,Geeza Pro,Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x630501a3,	(FX_LPCWSTR)L"SmallFonts",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x65d0e2a9,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"STFangsong,Songti SC,STSong",	0,	936},
    {0x669f29e1,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"STHeiti,Heiti TC",	0,	936},
    {0x673a9e5f,	(FX_LPCWSTR)L"Tunga",	(FX_LPCWSTR)L"Damascus,STIXNonUnicode,Avenir Next Condensed,Avenir Next Condensed Ultra Light,Futura",	0,	1252},
    {0x691aa4ce,	(FX_LPCWSTR)L"NiagaraSolid",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x696259b7,	(FX_LPCWSTR)L"Corbel",	(FX_LPCWSTR)L"Cochin,Baskerville,Marion",	0,	1252},
    {0x696ee9be,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"STHeiti,Heiti TC,Songti SC,Arial Unicode MS",	0,	936},
    {0x6c59cf69,	(FX_LPCWSTR)L"Dotum",	(FX_LPCWSTR)L"Arial Unicode MS,Songti SC",	0,	1252},
    {0x707fa561,	(FX_LPCWSTR)L"Gungsuh",	(FX_LPCWSTR)L"Arial Unicode MS,Heiti TC",	2,	1252},
    {0x71416bb2,	(FX_LPCWSTR)L"ZWAdobeF",	(FX_LPCWSTR)L"STIXSizeFourSym,STIXSizeThreeSym,STIXSizeTwoSym,STIXSizeOneSym",	0,	1252},
    {0x71b41801,	(FX_LPCWSTR)L"Verdana",	(FX_LPCWSTR)L"Tahoma,Marion,Apple Symbols,.Helvetica Neue Desk UI,Lucida Grande,Courier New",	0,	1252},
    {0x73f25e4c,	(FX_LPCWSTR)L"PalatinoLinotype",	(FX_LPCWSTR)L"Palatino,Arial Unicode MS",	0,	1252},
    {0x73f4d19f,	(FX_LPCWSTR)L"NiagaraEngraved",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x74001694,	(FX_LPCWSTR)L"MyriadProBlack",	(FX_LPCWSTR)L"Palatino,Baskerville,Marion,Cochin",	0,	1252},
    {0x74b14d8f,	(FX_LPCWSTR)L"Haettenschweiler",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x74cb44ee,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0x76b4d7ff,	(FX_LPCWSTR)L"Shruti",	(FX_LPCWSTR)L"Damascus,STIXNonUnicode,Arial Unicode MS,American Typewriter",	0,	1252},
    {0x788b3533,	(FX_LPCWSTR)L"Webdings",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	6,	42},
    {0x797dde99,	(FX_LPCWSTR)L"MSSerif",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x7a0f9e9e,	(FX_LPCWSTR)L"MSMincho",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,Arial Unicode MS,Apple SD Gothic Neo",	1,	1252},
    {0x7b439caf,	(FX_LPCWSTR)L"OldEnglishTextMT",	(FX_LPCWSTR)L"STIXNonUnicode,Arial Unicode MS,Baskerville,Avenir Next Medium",	0,	1252},
    {0x8213a433,	(FX_LPCWSTR)L"LucidaSans-Typewriter",	(FX_LPCWSTR)L"Comic Sans MS,Avenir Next,Arial Rounded MT Bold",	0,	1252},
    {0x82fec929,	(FX_LPCWSTR)L"AdobeSongStdL",	(FX_LPCWSTR)L"Heiti TC,STHeiti",	0,	936},
    {0x83581825,	(FX_LPCWSTR)L"Modern",	(FX_LPCWSTR)L"Avenir Next Condensed,Impact",	0,	1252},
    {0x835a2823,	(FX_LPCWSTR)L"Algerian",	(FX_LPCWSTR)L"STIXNonUnicode,Baskerville,Avenir Next Medium,American Typewriter",	0,	1252},
    {0x83dab9f5,	(FX_LPCWSTR)L"Script",	(FX_LPCWSTR)L"Arial Narrow",	0,	1252},
    {0x847b56da,	(FX_LPCWSTR)L"Tahoma",	(FX_LPCWSTR)L"Songti SC,Apple Symbols",	0,	1252},
    {0x8a783cb2,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	0,	1252},
    {0x8b5cac0e,	(FX_LPCWSTR)L"Onyx",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x8c6a499e,	(FX_LPCWSTR)L"Gulim",	(FX_LPCWSTR)L"Arial Unicode MS,Songti SC",	0,	1252},
    {0x8e0af790,	(FX_LPCWSTR)L"JuiceITC",	(FX_LPCWSTR)L"Nadeem,Al Bayan",	0,	1252},
    {0x8e8d43b2,	(FX_LPCWSTR)L"Centaur",	(FX_LPCWSTR)L"Avenir Next Condensed,Noteworthy,Impact",	2,	1252},
    {0x8ee4dcca,	(FX_LPCWSTR)L"BookshelfSymbol7",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x90794800,	(FX_LPCWSTR)L"BellGothicStdLight",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0x909b516a,	(FX_LPCWSTR)L"Century",	(FX_LPCWSTR)L"Damascus,Andale Mono,Songti SC,Arial Unicode MS",	2,	1252},
    {0x92ae370d,	(FX_LPCWSTR)L"MSOutlook",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	4,	42},
    {0x93c9fbf1,	(FX_LPCWSTR)L"LucidaFax",	(FX_LPCWSTR)L"PT Sans Narrow,Papyrus,Kokonor,Geeza Pro,Arial Rounded MT Bold,Lucida Grande,Futura",	2,	1252},
    {0x9565085e,	(FX_LPCWSTR)L"BookAntiqua",	(FX_LPCWSTR)L"Palatino,Microsoft Sans Serif,Apple Symbols",	2,	1252},
    {0x9856d95d,	(FX_LPCWSTR)L"AdobeMingStdL",	(FX_LPCWSTR)L"AHiragino Sans GB,Heiti TC,STHeiti",	0,	949},
    {0x9bbadd6b,	(FX_LPCWSTR)L"ColonnaMT",	(FX_LPCWSTR)L"Noteworthy,Avenir Next Condensed,Impact",	0,	1252},
    {0x9cbd16a4,	(FX_LPCWSTR)L"ShowcardGothic-Reg",	(FX_LPCWSTR)L"Arial Unicode MS,Georgia,American Typewriter",	0,	1252},
    {0x9d73008e,	(FX_LPCWSTR)L"MSSansSerif",	(FX_LPCWSTR)L"Songti SC,Apple Symbols",	0,	1252},
    {0xa0607db1,	(FX_LPCWSTR)L"GungsuhChe",	(FX_LPCWSTR)L"WenQuanYi Zen Hei Mono,AR PL UMing CN,AR PL UMing HK,AR PL UMing TW,AR PL UMing TW MBE,Arial Unicode MS,Heiti TC,STFangsong",	1,	1252},
    {0xa0bcf6a1,	(FX_LPCWSTR)L"LatinWide",	(FX_LPCWSTR)L"Zapfino,Arial Black,STHeiti",	2,	1252},
    {0xa1429b36,	(FX_LPCWSTR)L"Symbol",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	6,	42},
    {0xa1fa5abc,	(FX_LPCWSTR)L"Wingdings2",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	6,	42},
    {0xa1fa5abd,	(FX_LPCWSTR)L"Wingdings3",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	6,	42},
    {0xa427bad4,	(FX_LPCWSTR)L"InformalRoman-Regular",	(FX_LPCWSTR)L"STIXNonUnicode,Arial Narrow,Avenir Next Condensed Demi Bold",	8,	1252},
    {0xa8b92ece,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	0,	936},
    {0xa8d83ece,	(FX_LPCWSTR)L"CalifornianFB",	(FX_LPCWSTR)L"American Typewriter,Avenir Next Condensed,Impact",	2,	1252},
    {0xaa3e082c,	(FX_LPCWSTR)L"Kingsoft-Phonetic",	(FX_LPCWSTR)L"STIXVariants,STIXSizeOneSym,Apple Braille",	0,	1252},
    {0xaa6bcabe,	(FX_LPCWSTR)L"HarlowSolidItalic",	(FX_LPCWSTR)L"STIXNonUnicode,Avenir Medium,Avenir Next Medium,Arial Unicode MS",	0,	1252},
    {0xade5337c,	(FX_LPCWSTR)L"MSUIGothic",	(FX_LPCWSTR)L"Arial Unicode MS,Apple SD Gothic Neo",	0,	1252},
    {0xb08dd941,	(FX_LPCWSTR)L"WideLatin",	(FX_LPCWSTR)L"Marion,Papyrus,Nanum Pen Script,Zapf Dingbats,Damascus,Zapfino,Arial Black,STHeiti",	2,	1252},
    {0xb12765e0,    (FX_LPCWSTR)L"ITCLegacySansStdBook",  (FX_LPCWSTR)L"LastResort,.Helvetica Neue Desk UI,Arial Unicode MS,Palatino",   0,  1252},
    {0xb207f05d,	(FX_LPCWSTR)L"PoorRichard",	(FX_LPCWSTR)L"Noteworthy,Avenir Next Condensed,Impact",	2,	1252},
    {0xb3bc492f,	(FX_LPCWSTR)L"JuiceITC-Regular",	(FX_LPCWSTR)L"Nadeem,Al Bayan,STIXNonUnicode",	0,	1252},
    {0xb5545399,	(FX_LPCWSTR)L"Marlett",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	4,	42},
    {0xb5dd1ebb,	(FX_LPCWSTR)L"BritannicBold",	(FX_LPCWSTR)L"Damascus,STIXNonUnicode,Avenir Next Condensed Heavy,PT Sans",	0,	1252},
    {0xb699c1c5,	(FX_LPCWSTR)L"LucidaCalligraphy-Italic",	(FX_LPCWSTR)L"STHeiti,Arial Black",	0,	1252},
    {0xb725d629,	(FX_LPCWSTR)L"TimesNewRoman",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	2,	1252},
    {0xb7eaebeb,	(FX_LPCWSTR)L"AdobeHeitiStdR",	(FX_LPCWSTR)L"Heiti TC,STHeiti",	0,	936},
    {0xbd29c486,	(FX_LPCWSTR)L"BerlinSansFBDemi-Bold",	(FX_LPCWSTR)L"American Typewriter,Avenir Next Condensed Heavy",	0,	1252},
    {0xbe8a8db4,	(FX_LPCWSTR)L"BookshelfSymbolSeven",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xc16c0118,	(FX_LPCWSTR)L"AdobeHebrew",	(FX_LPCWSTR)L".Helvetica Neue Desk UI,Palatino,American Typewriter",	0,	1252},
    {0xc318b0af,	(FX_LPCWSTR)L"MyriadProLight",	(FX_LPCWSTR)L"Palatino,Baskerville,Marion",	0,	1252},
    {0xc65e5659,	(FX_LPCWSTR)L"CambriaMath",	(FX_LPCWSTR)L"Arial Unicode MS",	2,	1252},
    {0xc75c8f05,	(FX_LPCWSTR)L"LucidaConsole",	(FX_LPCWSTR)L"Courier New,Menlo,Andale Mono",	1,	1252},
    {0xca7c35d6,	(FX_LPCWSTR)L"Calibri",	(FX_LPCWSTR)L"Apple Symbols,HeadLineA",	0,	1252},
    {0xcb053f53,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Arial Unicode MS",	0,	936},
    {0xcb7190f9,	(FX_LPCWSTR)L"Magneto-Bold",	(FX_LPCWSTR)L"Lucida Grande",	0,	1252},
    {0xcca00cc5,	(FX_LPCWSTR)L"System",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xccad6f76,	(FX_LPCWSTR)L"Jokerman-Regular",	(FX_LPCWSTR)L"Lucida Grande",	0,	1252},
    {0xccc5818c,	(FX_LPCWSTR)L"EuroSign",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xcf3d7234,	(FX_LPCWSTR)L"LucidaHandwriting-Italic",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xcf7b8fdb,	(FX_LPCWSTR)L"MinionPro",	(FX_LPCWSTR)L"Bell MT,Corbel,Times New Roman,Cambria,Berlin Sans FB",	0,	1252},
    {0xcfe5755f,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0xd011f4ee,	(FX_LPCWSTR)L"MSPGothic",	(FX_LPCWSTR)L"Arial Unicode MS,Apple SD Gothic Neo",	0,	1252},
    {0xd060e7ef,	(FX_LPCWSTR)L"Vivaldi",	(FX_LPCWSTR)L"STIXNonUnicode,Arial Unicode MS,Avenir Medium,Avenir Next Medium",	8,	1252},
    {0xd07edec1,	(FX_LPCWSTR)L"FranklinGothic-Medium",	(FX_LPCWSTR)L"Impact,Arial Narrow",	0,	1252},
    {0xd107243f,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	0,	936},
    {0xd1881562,	(FX_LPCWSTR)L"ArialNarrow",	(FX_LPCWSTR)L"PT Sans Narrow,Apple Symbols",	0,	1252},
    {0xd22b7dce,	(FX_LPCWSTR)L"BodoniMTPosterCompressed",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xd22bfa60,	(FX_LPCWSTR)L"ComicSansMS",	(FX_LPCWSTR)L"Damascus,Georgia,.Helvetica Neue Desk UI,Lucida Grande,Arial Unicode MS",	8,	1252},
    {0xd3bd0e35,	(FX_LPCWSTR)L"Bauhaus93",	(FX_LPCWSTR)L"STIXNonUnicode,Arial Unicode MS,Avenir Next,Avenir",	0,	1252},
    {0xd429ee7a,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0xd6679c12,	(FX_LPCWSTR)L"BernardMTCondensed",	(FX_LPCWSTR)L"Impact,Avenir Next Condensed Demi Bold",	0,	1252},
    {0xd8e8a027,	(FX_LPCWSTR)L"LucidaSans",	(FX_LPCWSTR)L"Arial Narrow,Khmer MN,Kokonor,Damascus,Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xd9fe7761,	(FX_LPCWSTR)L"HighTowerText-Reg",	(FX_LPCWSTR)L"STIXGeneral,.Helvetica Neue Desk UI,Trebuchet MS",	2,	1252},
    {0xda7e551e,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Arial Unicode MS",	0,	936},
    {0xdaa6842d,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"STFangsong,Songti SC,STSong",	0,	936},
    {0xdaaab93f,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0xdaeb0713,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0xdafedbef,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Kaiti SC,Songti SC,STHeiti",	0,	936},
    {0xdb00a3d9,	(FX_LPCWSTR)L"Broadway",	(FX_LPCWSTR)L"Papyrus,STIXNonUnicode,Arial Black,Avenir Next Heavy,Heiti TC",	0,	1252},
    {0xdb1f5ad4,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Kaiti SC,Songti SC,STHeiti",	0,	936},
    {0xdb326e7f,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0xdb69595a,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Kaiti SC,Songti SC,STHeiti",	0,	936},
    {0xdba0082c,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"Songti SC,Arial Unicode MS",	0,	936},
    {0xdbd0ab18,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Kaiti SC,Songti SC",	0,	936},
    {0xdc1a7db1,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Kaiti SC,Songti SC",	0,	936},
    {0xdc33075f,	(FX_LPCWSTR)L"KristenITC-Regular",	(FX_LPCWSTR)L"STIXNonUnicode,Damascus,Songti SC,STSong",	8,	1252},
    {0xdcc7009c,	(FX_LPCWSTR)L"Harrington",	(FX_LPCWSTR)L"STIXNonUnicode,Avenir Next Condensed Heavy,Noteworthy",	0,	1252},
    {0xdd712466,	(FX_LPCWSTR)L"ArialBlack",	(FX_LPCWSTR)L"Geeza Pro,Damascus,Songti SC,STSong",	0,	1252},
    {0xdde87b3e,	(FX_LPCWSTR)L"Impact",	(FX_LPCWSTR)L"Arial Narrow,Marion",	0,	1252},
    {0xdf69fb32,	(FX_LPCWSTR)L"SnapITC",	(FX_LPCWSTR)L"Arial Narrow,PT Sans Narrow,Marion,STHeiti,Arial Black",	0,	1252},
    {0xdf8b25e8,	(FX_LPCWSTR)L"CenturyGothic",	(FX_LPCWSTR)L"Damascus,Andale Mono,Songti SC,Arial Unicode MS",	0,	1252},
    {0xe0f705c0,	(FX_LPCWSTR)L"KristenITC",	(FX_LPCWSTR)L"Songti SC,STSong",	8,	1252},
    {0xe1427573,	(FX_LPCWSTR)L"Raavi",	(FX_LPCWSTR)L"Damascus,STIXNonUnicode,Marion,Papyrus,Avenir Next Condensed Heavy,American Typewriter",	0,	1252},
    {0xe2cea0cb,	(FX_LPCWSTR)L"Magneto",	(FX_LPCWSTR)L"STIXNonUnicode,Damascus,Geeza Pro,Lucida Grande,Georgia,Heiti TC",	0,	1252},
    {0xe36a9e17,	(FX_LPCWSTR)L"Ravie",	(FX_LPCWSTR)L"STHeiti,Arial Black",	0,	1252},
    {0xe433f8e2,	(FX_LPCWSTR)L"Parchment",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	8,	1252},
    {0xe43dff4a,	(FX_LPCWSTR)L"Wingdings",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	4,	42},
    {0xe4e2c405,	(FX_LPCWSTR)L"MTExtra",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	6,	42},
    {0xe618cc35,	(FX_LPCWSTR)L"InformalRoman",	(FX_LPCWSTR)L"Arial Narrow",	8,	1252},
    {0xe6c27ffc,	(FX_LPCWSTR)L"Mistral",	(FX_LPCWSTR)L"Apple Symbols",	8,	1252},
    {0xe7ebf4b9,	(FX_LPCWSTR)L"Courier",	(FX_LPCWSTR)L"Courier New",	0,	1252},
    {0xe8bc4a9d,	(FX_LPCWSTR)L"MSReferenceSpecialty",	(FX_LPCWSTR)L"Microsoft Sans Serif,Apple Symbols",	0,	1252},
    {0xe90fb013,	(FX_LPCWSTR)L"TempusSansITC",	(FX_LPCWSTR)L"STIXNonUnicode,Microsoft Sans Serif,Avenir Light",	0,	1252},
    {0xec637b42,	(FX_LPCWSTR)L"Consolas",	(FX_LPCWSTR)L"AR PL UKai CN,AR PL UKai HK,AR PL UKai TW,AR PL UKai TW MBE,AR PL UMing CN,AR PL UMing HK,Microsoft Sans Serif,Tahoma",	1,	1252},
    {0xed3a683b,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Kaiti SC,Songti SC,",	0,	936},
    {0xef264cd1,	(FX_LPCWSTR)L"LucidaHandwriting",	(FX_LPCWSTR)L"Arial Narrow,Avenir Next Condensed Demi Bold,Avenir Next Condensed,Avenir Next Condensed Medium,STHeiti,Arial Black",	0,	1252},
    {0xf086bca2,	(FX_LPCWSTR)L"BaskervilleOldFace",	(FX_LPCWSTR)L"STIXNonUnicode,Avenir Next Condensed Heavy,PT Sans",	0,	1252},
    {0xf1028030,	(FX_LPCWSTR)L"Mangal",	(FX_LPCWSTR)L"Arial Unicode MS,Microsoft Sans Serif,Arial Narrow,Tahoma",	2,	1252},
    {0xf1da7eb9,	(FX_LPCWSTR)L"ShowcardGothic",	(FX_LPCWSTR)L"Papyrus,Arial Unicode MS,Georgia,American Typewriter",	0,	1252},
    {0xf210f06a,	(FX_LPCWSTR)L"ArialMT",	(FX_LPCWSTR)L"Arial Unicode MS,Arial Narrow,STIXNonUnicode,Damascus,Avenir Next Condensed Demi Bold,Avenir Next Condensed Medium,Avenir Next Condensed",	0,	1252},
    {0xf477f16a,	(FX_LPCWSTR)L"Latha",	(FX_LPCWSTR)L"Arial Narrow,Damascus,STIXNonUnicode,American Typewriter",	0,	1252},
    {0xf616f3dd,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"STHeiti,Heiti TC,STFangsong",	1,	936},
    {0xfa479aa6,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Arial Unicode MS",	0,	936},
    {0xfcd19697,	(FX_LPCWSTR)L"BookmanOldStyle",	(FX_LPCWSTR)L"Geeza Pro,Damascus,Andale Mono,Songti SC,Arial Unicode MS",	0,	1252},
    {0xfe209a82,	(FX_LPCWSTR)L"LucidaCalligraphy",	(FX_LPCWSTR)L"Kokonor,Damascus,STIXNonUnicode,STHeiti,Arial Black",	0,	1252},
    {0xfef135f8,	(FX_LPCWSTR)L"AdobeHeitiStd-Regular",	(FX_LPCWSTR)L"Heiti TC,STHeiti",	0,	936},
};
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_
static const XFA_FONTINFO g_XFAFontsMap[] = {
    {0x01d5d33e,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x01e4f102,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x030549dc,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x032edd44,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x03eac6fc,	(FX_LPCWSTR)L"PoorRichard-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback,Droid Arabic Naskh,Droid Sans Ethiopic",	2,	1252},
    {0x03ed90e6,	(FX_LPCWSTR)L"Nina",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x077b56b3,	(FX_LPCWSTR)L"KingsoftPhoneticPlain",	(FX_LPCWSTR)L"Droid Sans Thai,Droid Sans Armenian,Droid Arabic Naskh,Droid Sans Ethiopic,Droid Sans Fallback",	0,	1252},
    {0x078ed524,	(FX_LPCWSTR)L"MicrosoftSansSerif",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x089b18a9,	(FX_LPCWSTR)L"Arial",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x0b2cad72,	(FX_LPCWSTR)L"MonotypeCorsiva",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x0bb003e7,	(FX_LPCWSTR)L"Kartika",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	2,	1252},
    {0x0bb469df,	(FX_LPCWSTR)L"VinerHandITC",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x0bc1a851,	(FX_LPCWSTR)L"SegoeUI",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x0c112ebd,	(FX_LPCWSTR)L"KozukaGothicPro-VIM",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x0cfcb9c1,	(FX_LPCWSTR)L"AdobeThai",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	847},
    {0x0e7de0f9,	(FX_LPCWSTR)L"Playbill",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0x0eff47c3,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x107ad374,	(FX_LPCWSTR)L"Constantia",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x12194c2d,	(FX_LPCWSTR)L"KunstlerScript",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x135ef6a1,	(FX_LPCWSTR)L"MinionProSmBd",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x158c4049,	(FX_LPCWSTR)L"Garamond",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x160ecb24,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x161ed07e,	(FX_LPCWSTR)L"MSGothic",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x171d1ed1,	(FX_LPCWSTR)L"SnapITC-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x18d1188f,	(FX_LPCWSTR)L"Cambria",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0x18eaf350,	(FX_LPCWSTR)L"ArialUnicodeMS",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x1a92d115,	(FX_LPCWSTR)L"MingLiU",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x1cc217c6,	(FX_LPCWSTR)L"TrebuchetMS",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x1d649596,	(FX_LPCWSTR)L"BasemicTimes",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x1e34ee60,	(FX_LPCWSTR)L"BellMT",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x1eb36945,	(FX_LPCWSTR)L"CooperBlack",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x1ef7787d,	(FX_LPCWSTR)L"BatangChe",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x20b3bd3a,	(FX_LPCWSTR)L"BrushScriptMT",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic",	8,	1252},
    {0x220877aa,	(FX_LPCWSTR)L"Candara",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x22135007,	(FX_LPCWSTR)L"FreestyleScript-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x251059c3,	(FX_LPCWSTR)L"Chiller",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif",	0,	1252},
    {0x25bed6dd,	(FX_LPCWSTR)L"MSReferenceSansSerif",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x28154c81,	(FX_LPCWSTR)L"Parchment-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x29711eb9,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x2b1993b4,	(FX_LPCWSTR)L"Basemic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2b316339,	(FX_LPCWSTR)L"NiagaraSolid-Reg",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2c147529,	(FX_LPCWSTR)L"FootlightMTLight",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2c198928,	(FX_LPCWSTR)L"HarlowSolid",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2c6ac6b2,	(FX_LPCWSTR)L"LucidaBright",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto",	2,	1252},
    {0x2c9f38e2,	(FX_LPCWSTR)L"KozukaMinchoPro-VIR",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2d5a47b0,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x2def26bf,	(FX_LPCWSTR)L"BernardMT-Condensed",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x2fd8930b,	(FX_LPCWSTR)L"KozukaMinchoPr6NR",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x3115525a,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x31327817,	(FX_LPCWSTR)L"MyriadPro",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x32244975,    (FX_LPCWSTR)L"Helvetica", (FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto",	 0,   1252},
    {0x32ac995c,	(FX_LPCWSTR)L"Terminal",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x338d648a,	(FX_LPCWSTR)L"NiagaraEngraved-Reg",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x33bb65f2,	(FX_LPCWSTR)L"Sylfaen",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x3402c30e,	(FX_LPCWSTR)L"MSPMincho",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0x3412bf31,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x36eb39b9,	(FX_LPCWSTR)L"BerlinSansFB",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x36f42055,	(FX_LPCWSTR)L"UniversATT",	(FX_LPCWSTR)L"Microsoft Sans Serif",	0,	1252},
    {0x3864c4f6,	(FX_LPCWSTR)L"HighTowerText",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x3a257d03,	(FX_LPCWSTR)L"FangSong_GB2312",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x3cdae668,	(FX_LPCWSTR)L"FreestyleScript",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x3d55aed7,	(FX_LPCWSTR)L"Jokerman",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x3d5b4385,	(FX_LPCWSTR)L"PMingLiU",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0x3d9b7669,	(FX_LPCWSTR)L"EstrangeloEdessa",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x3e532d74,	(FX_LPCWSTR)L"FranklinGothicMedium",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x3e6aa32d,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x3f6c36a8,	(FX_LPCWSTR)L"Gautami",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x3ff32662,	(FX_LPCWSTR)L"Chiller-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x409de312,	(FX_LPCWSTR)L"ModernNo.20",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x41443c5e,	(FX_LPCWSTR)L"Georgia",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x4160ade5,	(FX_LPCWSTR)L"BellGothicStdBlack",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x421976c4,	(FX_LPCWSTR)L"Modern-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x422a7252,	(FX_LPCWSTR)L"Stencil",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x42c8554f,	(FX_LPCWSTR)L"Fixedsys",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x435cb41d,	(FX_LPCWSTR)L"Roman",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x47882383,	(FX_LPCWSTR)L"CourierNew",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x480a2338,	(FX_LPCWSTR)L"BerlinSansFBDemi",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x480bf7a4,	(FX_LPCWSTR)L"CourierStd",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x481ad6ed,	(FX_LPCWSTR)L"VladimirScript",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0x4911577a,	(FX_LPCWSTR)L"YouYuan",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x4a788d72,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x4bf88566,	(FX_LPCWSTR)L"SegoeCondensed",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x4ccf51a4,	(FX_LPCWSTR)L"BerlinSansFB-Reg",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x4ea967ce,	(FX_LPCWSTR)L"GulimChe",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x4f68bd79,	(FX_LPCWSTR)L"LetterGothicStd",	(FX_LPCWSTR)L"Droid Sans Mono,Droid Arabic Naskh,Droid Sans Ethiopic,Droid Sans Mono,Droid Serif,Droid Sans Fallback",	0,	1252},
    {0x51a0d0e6,	(FX_LPCWSTR)L"KozukaGothicPr6NM",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x531b3dea,	(FX_LPCWSTR)L"BasemicSymbol",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x5333fd39,	(FX_LPCWSTR)L"CalifornianFB-Reg",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x53561a54,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x55e0dde6,	(FX_LPCWSTR)L"LucidaSansTypewriter",	(FX_LPCWSTR)L"Droid Sans Mono,Droid Arabic Naskh,Droid Sans Ethiopic",	0,	1252},
    {0x574d4d3d,	(FX_LPCWSTR)L"AdobeArabic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x5792e759,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x5921978e,	(FX_LPCWSTR)L"LucidaSansUnicode",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x594e2da4,	(FX_LPCWSTR)L"Vrinda",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0x59baa9a2,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x5cfedf4f,	(FX_LPCWSTR)L"BaskOldFace",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x5f97921c,	(FX_LPCWSTR)L"AdobeMyungjoStdM",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x5fefbfad,	(FX_LPCWSTR)L"Batang",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0x605342b9,	(FX_LPCWSTR)L"DotumChe",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x608c5f9a,	(FX_LPCWSTR)L"KaiTi_GB2312",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x61efd0d1,	(FX_LPCWSTR)L"MaturaMTScriptCapitals",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0x626608a9,	(FX_LPCWSTR)L"MVBoli",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0x630501a3,	(FX_LPCWSTR)L"SmallFonts",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x65d0e2a9,	(FX_LPCWSTR)L"FZYTK--GBK1-0",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x669f29e1,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x673a9e5f,	(FX_LPCWSTR)L"Tunga",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x691aa4ce,	(FX_LPCWSTR)L"NiagaraSolid",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x696259b7,	(FX_LPCWSTR)L"Corbel",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x696ee9be,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x6c59cf69,	(FX_LPCWSTR)L"Dotum",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x707fa561,	(FX_LPCWSTR)L"Gungsuh",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0x71416bb2,	(FX_LPCWSTR)L"ZWAdobeF",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Armenian,Droid Sans Ethiopic,Droid Sans Georgian,Droid Sans Hebrew,Droid Sans Thai",	0,	1252},
    {0x71b41801,	(FX_LPCWSTR)L"Verdana",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x73f25e4c,	(FX_LPCWSTR)L"PalatinoLinotype",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x73f4d19f,	(FX_LPCWSTR)L"NiagaraEngraved",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x74001694,	(FX_LPCWSTR)L"MyriadProBlack",	(FX_LPCWSTR)L"Book Antiqua,Constantia,Dotum,Georgia",	0,	1252},
    {0x74b14d8f,	(FX_LPCWSTR)L"Haettenschweiler",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x74cb44ee,	(FX_LPCWSTR)L"NSimSun",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0x76b4d7ff,	(FX_LPCWSTR)L"Shruti",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0x788b3533,	(FX_LPCWSTR)L"Webdings",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	6,	42},
    {0x797dde99,	(FX_LPCWSTR)L"MSSerif",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x7a0f9e9e,	(FX_LPCWSTR)L"MSMincho",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0x7b439caf,	(FX_LPCWSTR)L"OldEnglishTextMT",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x8213a433,	(FX_LPCWSTR)L"LucidaSans-Typewriter",	(FX_LPCWSTR)L"Droid Sans Mono,Droid Serif,Roboto,Droid Sans Fallback",	0,	1252},
    {0x82fec929,	(FX_LPCWSTR)L"AdobeSongStdL",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0x83581825,	(FX_LPCWSTR)L"Modern",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x835a2823,	(FX_LPCWSTR)L"Algerian",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x83dab9f5,	(FX_LPCWSTR)L"Script",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x847b56da,	(FX_LPCWSTR)L"Tahoma",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x8a783cb2,	(FX_LPCWSTR)L"SimSun-PUA",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x8b5cac0e,	(FX_LPCWSTR)L"Onyx",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x8c6a499e,	(FX_LPCWSTR)L"Gulim",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0x8e0af790,	(FX_LPCWSTR)L"JuiceITC",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x8e8d43b2,	(FX_LPCWSTR)L"Centaur",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x8ee4dcca,	(FX_LPCWSTR)L"BookshelfSymbol7",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x90794800,	(FX_LPCWSTR)L"BellGothicStdLight",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x909b516a,	(FX_LPCWSTR)L"Century",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x92ae370d,	(FX_LPCWSTR)L"MSOutlook",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	4,	42},
    {0x93c9fbf1,	(FX_LPCWSTR)L"LucidaFax",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	2,	1252},
    {0x9565085e,	(FX_LPCWSTR)L"BookAntiqua",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0x9856d95d,	(FX_LPCWSTR)L"AdobeMingStdL",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	949},
    {0x9bbadd6b,	(FX_LPCWSTR)L"ColonnaMT",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0x9cbd16a4,	(FX_LPCWSTR)L"ShowcardGothic-Reg",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallbac",	0,	1252},
    {0x9d73008e,	(FX_LPCWSTR)L"MSSansSerif",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xa0607db1,	(FX_LPCWSTR)L"GungsuhChe",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0xa0bcf6a1,	(FX_LPCWSTR)L"LatinWide",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0xa1429b36,	(FX_LPCWSTR)L"Symbol",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	6,	42},
    {0xa1fa5abc,	(FX_LPCWSTR)L"Wingdings2",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	6,	42},
    {0xa1fa5abd,	(FX_LPCWSTR)L"Wingdings3",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	6,	42},
    {0xa427bad4,	(FX_LPCWSTR)L"InformalRoman-Regular",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic",	8,	1252},
    {0xa8b92ece,	(FX_LPCWSTR)L"FZSTK--GBK1-0",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xa8d83ece,	(FX_LPCWSTR)L"CalifornianFB",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0xaa3e082c,	(FX_LPCWSTR)L"Kingsoft-Phonetic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xaa6bcabe,	(FX_LPCWSTR)L"HarlowSolidItalic",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xade5337c,	(FX_LPCWSTR)L"MSUIGothic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xb08dd941,	(FX_LPCWSTR)L"WideLatin",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	2,	1252},
    {0xb207f05d,	(FX_LPCWSTR)L"PoorRichard",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0xb3bc492f,	(FX_LPCWSTR)L"JuiceITC-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xb5545399,	(FX_LPCWSTR)L"Marlett",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	4,	42},
    {0xb5dd1ebb,	(FX_LPCWSTR)L"BritannicBold",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic",	0,	1252},
    {0xb699c1c5,	(FX_LPCWSTR)L"LucidaCalligraphy-Italic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xb725d629,	(FX_LPCWSTR)L"TimesNewRoman",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0xb7eaebeb,	(FX_LPCWSTR)L"AdobeHeitiStdR",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xbd29c486,	(FX_LPCWSTR)L"BerlinSansFBDemi-Bold",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xbe8a8db4,	(FX_LPCWSTR)L"BookshelfSymbolSeven",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xc16c0118,	(FX_LPCWSTR)L"AdobeHebrew",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback,Droid Arabic Naskh,Droid Sans Ethiopic",	0,	1252},
    {0xc318b0af,	(FX_LPCWSTR)L"MyriadProLight",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xc65e5659,	(FX_LPCWSTR)L"CambriaMath",	(FX_LPCWSTR)L"Droid Sans Fallback",	2,	1252},
    {0xc75c8f05,	(FX_LPCWSTR)L"LucidaConsole",	(FX_LPCWSTR)L"Droid Sans Mono,Droid Serif,Roboto,Droid Sans Fallback",	1,	1252},
    {0xca7c35d6,	(FX_LPCWSTR)L"Calibri",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0xcb053f53,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xcb7190f9,	(FX_LPCWSTR)L"Magneto-Bold",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xcca00cc5,	(FX_LPCWSTR)L"System",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xccad6f76,	(FX_LPCWSTR)L"Jokerman-Regular",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xccc5818c,	(FX_LPCWSTR)L"EuroSign",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xcf3d7234,	(FX_LPCWSTR)L"LucidaHandwriting-Italic",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xcf7b8fdb,	(FX_LPCWSTR)L"MinionPro",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xcfe5755f,	(FX_LPCWSTR)L"Simhei",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0xd011f4ee,	(FX_LPCWSTR)L"MSPGothic",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0xd060e7ef,	(FX_LPCWSTR)L"Vivaldi",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0xd07edec1,	(FX_LPCWSTR)L"FranklinGothic-Medium",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xd107243f,	(FX_LPCWSTR)L"SimSun",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xd1881562,	(FX_LPCWSTR)L"ArialNarrow",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xd22b7dce,	(FX_LPCWSTR)L"BodoniMTPosterCompressed",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xd22bfa60,	(FX_LPCWSTR)L"ComicSansMS",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Fallback",	8,	1252},
    {0xd3bd0e35,	(FX_LPCWSTR)L"Bauhaus93",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xd429ee7a,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xd6679c12,	(FX_LPCWSTR)L"BernardMTCondensed",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xd8e8a027,	(FX_LPCWSTR)L"LucidaSans",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto",	0,	1252},
    {0xd9fe7761,	(FX_LPCWSTR)L"HighTowerText-Reg",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	2,	1252},
    {0xda7e551e,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdaa6842d,	(FX_LPCWSTR)L"STZhongsong",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdaaab93f,	(FX_LPCWSTR)L"STFangsong",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdaeb0713,	(FX_LPCWSTR)L"STSong",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	936},
    {0xdafedbef,	(FX_LPCWSTR)L"STCaiyun",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdb00a3d9,	(FX_LPCWSTR)L"Broadway",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xdb1f5ad4,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdb326e7f,	(FX_LPCWSTR)L"STKaiti",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdb69595a,	(FX_LPCWSTR)L"STHupo",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdba0082c,	(FX_LPCWSTR)L"STXihei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdbd0ab18,	(FX_LPCWSTR)L"STXingkai",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdc1a7db1,	(FX_LPCWSTR)L"STLiti",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xdc33075f,	(FX_LPCWSTR)L"KristenITC-Regular",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto",	8,	1252},
    {0xdcc7009c,	(FX_LPCWSTR)L"Harrington",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xdd712466,	(FX_LPCWSTR)L"ArialBlack",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xdde87b3e,	(FX_LPCWSTR)L"Impact",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xdf69fb32,	(FX_LPCWSTR)L"SnapITC",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0xdf8b25e8,	(FX_LPCWSTR)L"CenturyGothic",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0xe0f705c0,	(FX_LPCWSTR)L"KristenITC",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto",	8,	1252},
    {0xe1427573,	(FX_LPCWSTR)L"Raavi",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0xe2cea0cb,	(FX_LPCWSTR)L"Magneto",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0xe36a9e17,	(FX_LPCWSTR)L"Ravie",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0xe433f8e2,	(FX_LPCWSTR)L"Parchment",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0xe43dff4a,	(FX_LPCWSTR)L"Wingdings",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	4,	42},
    {0xe4e2c405,	(FX_LPCWSTR)L"MTExtra",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	6,	42},
    {0xe618cc35,	(FX_LPCWSTR)L"InformalRoman",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif",	8,	1252},
    {0xe6c27ffc,	(FX_LPCWSTR)L"Mistral",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	8,	1252},
    {0xe7ebf4b9,	(FX_LPCWSTR)L"Courier",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	1252},
    {0xe8bc4a9d,	(FX_LPCWSTR)L"MSReferenceSpecialty",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xe90fb013,	(FX_LPCWSTR)L"TempusSansITC",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xec637b42,	(FX_LPCWSTR)L"Consolas",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	1252},
    {0xed3a683b,	(FX_LPCWSTR)L"STXinwei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xef264cd1,	(FX_LPCWSTR)L"LucidaHandwriting",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0xf086bca2,	(FX_LPCWSTR)L"BaskervilleOldFace",	(FX_LPCWSTR)L"Roboto,Droid Serif,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xf1028030,	(FX_LPCWSTR)L"Mangal",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	2,	1252},
    {0xf1da7eb9,	(FX_LPCWSTR)L"ShowcardGothic",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallbac",	0,	1252},
    {0xf210f06a,	(FX_LPCWSTR)L"ArialMT",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif",	0,	1252},
    {0xf477f16a,	(FX_LPCWSTR)L"Latha",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Roboto,Droid Serif,Droid Sans Mono",	0,	1252},
    {0xf616f3dd,	(FX_LPCWSTR)L"LiSu",	(FX_LPCWSTR)L"Droid Sans Fallback",	1,	936},
    {0xfa479aa6,	(FX_LPCWSTR)L"MicrosoftYaHei",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
    {0xfcd19697,	(FX_LPCWSTR)L"BookmanOldStyle",	(FX_LPCWSTR)L"Droid Serif,Roboto,Droid Sans Mono,Droid Sans Fallback",	0,	1252},
    {0xfe209a82,	(FX_LPCWSTR)L"LucidaCalligraphy",	(FX_LPCWSTR)L"Droid Arabic Naskh,Droid Sans Ethiopic,Droid Serif,Roboto,Droid Sans Mono",	0,	1252},
    {0xfef135f8,	(FX_LPCWSTR)L"AdobeHeitiStd-Regular",	(FX_LPCWSTR)L"Droid Sans Fallback",	0,	936},
};
#endif
void XFA_LocalFontNameToEnglishName(FX_WSTR wsLocalName, CFX_WideString &wsEnglishName)
{
    wsEnglishName = wsLocalName;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ || _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_ || _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ || _FXM_PLATFORM_ ==  _FXM_PLATFORM_ANDROID_
    FX_DWORD dwLocalNameHash = FX_HashCode_String_GetW(wsLocalName.GetPtr(), wsLocalName.GetLength(), TRUE);
    FX_INT32 iStart = 0;
    FX_INT32 iEnd = sizeof(g_XFAFontsMap) / sizeof(XFA_FONTINFO) - 1;
    FX_INT32 iMid = 0;
    do {
        iMid = (iStart + iEnd) / 2;
        FX_DWORD dwFontNameHash = g_XFAFontsMap[iMid].dwFontNameHash;
        if (dwFontNameHash == dwLocalNameHash) {
            wsEnglishName = g_XFAFontsMap[iMid].pPsName;
            break;
        } else if (dwFontNameHash < dwLocalNameHash) {
            iStart = iMid + 1;
        } else {
            iEnd = iMid - 1;
        }
    } while (iEnd >= iStart);
#endif
}
const XFA_FONTINFO* XFA_GetFontINFOByFontName(FX_WSTR wsFontName)
{
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_ || _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_ || _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_ || _FXM_PLATFORM_ ==  _FXM_PLATFORM_ANDROID_
    CFX_WideString wsFontNameTemp = wsFontName;
    wsFontNameTemp.Remove(L' ');
    FX_DWORD dwCurFontNameHash = FX_HashCode_String_GetW(wsFontNameTemp, wsFontNameTemp.GetLength(), TRUE);
    FX_INT32 iStart = 0;
    FX_INT32 iEnd = sizeof(g_XFAFontsMap) / sizeof(XFA_FONTINFO) - 1;
    FX_INT32 iMid = 0;
    const XFA_FONTINFO *pFontInfo = NULL;
    do {
        iMid = (iStart + iEnd) / 2;
        FX_DWORD dwFontNameHash = g_XFAFontsMap[iMid].dwFontNameHash;
        if (dwFontNameHash == dwCurFontNameHash) {
            pFontInfo = &g_XFAFontsMap[iMid];
            break;
        } else if (dwFontNameHash < dwCurFontNameHash) {
            iStart = iMid + 1;
        } else {
            iEnd = iMid - 1;
        }
    } while (iEnd >= iStart);
    return pFontInfo;
#else
    return NULL;
#endif
}
IXFA_FontMgr* XFA_GetDefaultFontMgr()
{
    return FX_NEW CXFA_DefFontMgr;
}
CXFA_DefFontMgr::~CXFA_DefFontMgr()
{
    FX_INT32 iCounts = m_CacheFonts.GetSize();
    for (FX_INT32 i = 0; i < iCounts; i++) {
        ((IFX_Font*)m_CacheFonts[i])->Release();
    }
    m_CacheFonts.RemoveAll();
}
#define _FXFA_USEGASFONTMGR_
IFX_Font* CXFA_DefFontMgr::GetFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage )
{
    CFX_WideString wsFontName = wsFontFamily;
    IFX_FontMgr* pFDEFontMgr = ((CXFA_FFDoc*)hDoc)->GetApp()->GetFDEFontMgr();
#ifdef _FXFA_USEGASFONTMGR_
    const XFA_FONTINFO *pCurFont = NULL;
    FX_BOOL bGetFontInfo = TRUE;
    IFX_Font* pFont = pFDEFontMgr->LoadFont((FX_LPCWSTR)wsFontName, dwFontStyles, wCodePage);
#else
    const XFA_FONTINFO *pCurFont = XFA_GetFontINFOByFontName(wsFontName);
    FX_BOOL bGetFontInfo = FALSE;
    IFX_Font* pFont = IFX_Font::LoadFont((FX_LPCWSTR)wsFontName, dwFontStyles | FX_FONTSTYLE_ExactMatch, pCurFont ? pCurFont->wCodePage : wCodePage, pFDEFontMgr);
#endif
    if (!pFont && hDoc) {
        if (bGetFontInfo) {
            pCurFont = XFA_GetFontINFOByFontName(wsFontName);
        }
        if (pCurFont != NULL && pCurFont->pReplaceFont != NULL) {
            FX_DWORD dwStyle = 0;
            if (dwFontStyles & FX_FONTSTYLE_Bold) {
                dwStyle |= FX_FONTSTYLE_Bold;
            }
            if (dwFontStyles & FX_FONTSTYLE_Italic) {
                dwStyle |= FX_FONTSTYLE_Italic;
            }
            FX_LPCWSTR pReplace = pCurFont->pReplaceFont;
            FX_INT32 iLength = FXSYS_wcslen(pReplace);
            while (iLength > 0) {
                FX_LPCWSTR pNameText = pReplace;
                while (*pNameText != L',' && iLength > 0) {
                    pNameText++;
                    iLength--;
                }
                CFX_WideString wsReplace = CFX_WideString(pReplace, pNameText - pReplace);
#ifdef _FXFA_USEGASFONTMGR_
                pFont = pFDEFontMgr->LoadFont(wsReplace, dwStyle, wCodePage);
#else
                pFont = IFX_Font::LoadFont((FX_LPCWSTR)wsReplace, dwStyle | FX_FONTSTYLE_ExactMatch, pCurFont->wCodePage, pFDEFontMgr);
#endif
                if (pFont != NULL) {
                    break;
                }
                iLength--;
                pNameText++;
                pReplace = pNameText;
            }
        }
    }
    if (pFont) {
        m_CacheFonts.Add(pFont);
    }
    return pFont;
}
IFX_Font* CXFA_DefFontMgr::GetDefaultFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage)
{
    IFX_FontMgr* pFDEFontMgr = ((CXFA_FFDoc*)hDoc)->GetApp()->GetFDEFontMgr();
#ifdef _FXFA_USEGASFONTMGR_
    IFX_Font* pFont = pFDEFontMgr->LoadFont((FX_LPCWSTR)L"Arial Narrow", dwFontStyles, wCodePage);
#else
    const XFA_FONTINFO *pCurFont = XFA_GetFontINFOByFontName(wsFontFamily);
    IFX_Font* pFont = IFX_Font::LoadFont((FX_LPCWSTR)L"Arial Narrow", dwFontStyles, pCurFont ? pCurFont->wCodePage : 1252, pFDEFontMgr);
#endif
    if (!pFont)
#ifdef _FXFA_USEGASFONTMGR_
        pFont = pFDEFontMgr->LoadFont((FX_LPCWSTR)NULL, dwFontStyles, wCodePage);
#else
        pFont = IFX_Font::LoadFont((FX_LPCWSTR)NULL, dwFontStyles, pCurFont ? pCurFont->wCodePage : 1252, pFDEFontMgr);
#endif
    FXSYS_assert(pFont != NULL);
    if (pFont) {
        m_CacheFonts.Add(pFont);
    }
    return pFont;
}
struct XFA_PDFFONTNAME {
    FX_LPCSTR lpPsName;
    FX_LPCSTR lpNormal;
    FX_LPCSTR lpBold;
    FX_LPCSTR lpItalic;
    FX_LPCSTR lpBoldItalic;
};
const XFA_PDFFONTNAME g_XFAPDFFontName[] = {
    {"Adobe PI Std", "AdobePIStd", "AdobePIStd", "AdobePIStd", "AdobePIStd"},
    {"Myriad Pro Light", "MyriadPro-Light", "MyriadPro-Semibold", "MyriadPro-LightIt", "MyriadPro-SemiboldIt"},
};
CXFA_PDFFontMgr::CXFA_PDFFontMgr(CXFA_FFDoc* pDoc)
{
    m_pDoc = pDoc;
}
CXFA_PDFFontMgr::~CXFA_PDFFontMgr()
{
    FX_POSITION ps = m_FDE2PDFFont.GetStartPosition();
    while (ps) {
        IFX_Font* pFDEFont;
        void* pPDFFont;
        m_FDE2PDFFont.GetNextAssoc(ps, (void*&)pFDEFont, pPDFFont);
        pFDEFont->SetFontProvider(NULL);
    }
    m_FDE2PDFFont.RemoveAll();
    ps = m_FontArray.GetStartPosition();
    while (ps) {
        CFX_ByteString strKey;
        IFX_Font* pFont = NULL;
        m_FontArray.GetNextAssoc(ps, strKey, (void*&)pFont);
        if (pFont != NULL) {
            pFont->Release();
        }
    }
    m_FontArray.RemoveAll();
}
IFX_Font* CXFA_PDFFontMgr::FindFont(CFX_ByteString strPsName, FX_BOOL bBold, FX_BOOL bItalic, CPDF_Font** pDstPDFFont, FX_BOOL bStrictMatch)
{
    CPDF_Document* pDoc = m_pDoc->GetPDFDoc();
    if (pDoc == NULL) {
        return NULL;
    }
    CPDF_Dictionary* pFontSetDict = pDoc->GetRoot()->GetDict(FX_BSTRC("AcroForm"))->GetDict(FX_BSTRC("DR"));
    if (!pFontSetDict) {
        return NULL;
    }
    pFontSetDict = (CPDF_Dictionary*)pFontSetDict->GetDict(FX_BSTRC("Font"));
    if (!pFontSetDict) {
        return NULL;
    }
    strPsName.Remove(' ');
    IFX_FontMgr* pFDEFontMgr = m_pDoc->GetApp()->GetFDEFontMgr();
    FX_POSITION pos = pFontSetDict->GetStartPos();
    while (pos) {
        CFX_ByteString key;
        CPDF_Object* pObj = pFontSetDict->GetNextElement(pos, key);
        if (!PsNameMatchDRFontName(strPsName, bBold, bItalic, key, bStrictMatch)) {
            continue;
        }
        CPDF_Object* pDirect = pObj->GetDirect();
        if (pDirect == NULL || pDirect->GetType() != PDFOBJ_DICTIONARY) {
            return NULL;
        }
        CPDF_Dictionary* pFontDict = (CPDF_Dictionary*)pDirect;
        if (pFontDict->GetString("Type") != FX_BSTRC("Font")) {
            return NULL;
        }
        CPDF_Font* pPDFFont = pDoc->LoadFont(pFontDict);
        if (!pPDFFont) {
            return NULL;
        }
        if (!pPDFFont->IsEmbedded()) {
            *pDstPDFFont = pPDFFont;
            return NULL;
        }
        return IFX_Font::LoadFont(&pPDFFont->m_Font, pFDEFontMgr);
    }
    return NULL;
}
IFX_Font* CXFA_PDFFontMgr::GetFont(FX_WSTR wsFontFamily, FX_DWORD dwFontStyles, CPDF_Font** pPDFFont, FX_BOOL bStrictMatch )
{
    FX_DWORD dwHashCode = FX_HashCode_String_GetW(wsFontFamily.GetPtr(), wsFontFamily.GetLength());
    CFX_ByteString strKey;
    IFX_Font* pFont = NULL;
    strKey.Format("%u%u", dwHashCode, dwFontStyles);
    if (m_FontArray.Lookup(strKey, (void*&)pFont)) {
        return pFont;
    }
    CFX_ByteString bsPsName;
    bsPsName = CFX_ByteString::FromUnicode(wsFontFamily);
    FX_BOOL bBold = (dwFontStyles & FX_FONTSTYLE_Bold) == FX_FONTSTYLE_Bold;
    FX_BOOL bItalic = (dwFontStyles & FX_FONTSTYLE_Italic) == FX_FONTSTYLE_Italic;
    CFX_ByteString strFontName = PsNameToFontName(bsPsName, bBold, bItalic);
    pFont = FindFont(strFontName, bBold, bItalic, pPDFFont, bStrictMatch);
    if (pFont) {
        m_FontArray.SetAt(strKey, pFont);
    }
    return pFont;
}
CFX_ByteString CXFA_PDFFontMgr::PsNameToFontName(const CFX_ByteString& strPsName,
        FX_BOOL bBold, FX_BOOL bItalic)
{
    FX_INT32 nCount = sizeof(g_XFAPDFFontName) / sizeof(XFA_PDFFONTNAME);
    for (FX_INT32 i = 0; i < nCount; i++) {
        if (strPsName == g_XFAPDFFontName[i].lpPsName) {
            FX_INT32 index = 1 + ((bItalic << 1) | bBold);
            return *(&g_XFAPDFFontName[i].lpPsName + index);
        }
    }
    return strPsName;
}
FX_BOOL CXFA_PDFFontMgr::PsNameMatchDRFontName(FX_BSTR bsPsName, FX_BOOL bBold, FX_BOOL bItalic, const CFX_ByteString& bsDRFontName, FX_BOOL bStrictMatch)
{
    CFX_ByteString bsDRName = bsDRFontName;
    bsDRName.Remove('-');
    FX_INT32 iPsLen = bsPsName.GetLength();
    FX_INT32 nIndex = bsDRName.Find(bsPsName);
    if (nIndex != -1 && !bStrictMatch) {
        return TRUE;
    }
    if (nIndex != 0) {
        return FALSE;
    }
    FX_INT32 iDifferLength = bsDRName.GetLength() - iPsLen;
    if (iDifferLength > 1 || (bBold || bItalic)) {
        FX_INT32 iBoldIndex = bsDRName.Find(FX_BSTRC("Bold"));
        FX_BOOL bBoldFont = iBoldIndex > 0;
        if (bBold ^ bBoldFont) {
            return FALSE;
        }
        if (bBoldFont) {
            iDifferLength = FX_MIN(iDifferLength - 4, bsDRName.GetLength() - iBoldIndex - 4);
        }
        FX_BOOL bItalicFont = TRUE;
        if (bsDRName.Find(FX_BSTRC("Italic")) > 0) {
            iDifferLength -= 6;
        } else if (bsDRName.Find(FX_BSTRC("It")) > 0) {
            iDifferLength -= 2;
        } else if (bsDRName.Find(FX_BSTRC("Oblique")) > 0) {
            iDifferLength -= 7;
        } else {
            bItalicFont = FALSE;
        }
        if (bItalic ^ bItalicFont) {
            return FALSE;
        }
        if (iDifferLength > 1) {
            CFX_ByteString bsDRTailer = bsDRName.Right(iDifferLength);
            if (bsDRTailer.Equal(FX_BSTRC("MT")) || bsDRTailer.Equal(FX_BSTRC("PSMT")) || bsDRTailer.Equal(FX_BSTRC("Regular")) || bsDRTailer.Equal(FX_BSTRC("Reg"))) {
                return TRUE;
            }
            if (bBoldFont || bItalicFont) {
                return FALSE;
            }
            FX_BOOL bMatch = FALSE;
            switch (bsPsName.GetAt(iPsLen - 1)) {
                case 'L': {
                        if (bsDRName.Right(5).Equal(FX_BSTRC("Light"))) {
                            bMatch = TRUE;
                        }
                    }
                    break;
                case 'R': {
                        if ( bsDRName.Right(7).Equal(FX_BSTRC("Regular")) || bsDRName.Right(3).Equal(FX_BSTRC("Reg"))) {
                            bMatch = TRUE;
                        }
                    }
                    break;
                case 'M': {
                        if (bsDRName.Right(5).Equal(FX_BSTRC("Medium"))) {
                            bMatch = TRUE;
                        }
                    }
                    break;
                default:
                    break;
            }
            return bMatch;
        }
    }
    return TRUE;
}
FX_BOOL CXFA_PDFFontMgr::GetCharWidth(IFX_Font* pFont, FX_WCHAR wUnicode, FX_INT32 &iWidth, FX_BOOL bCharCode)
{
    if (wUnicode != 0x20 || bCharCode) {
        return FALSE;
    }
    CPDF_Font* pPDFFont = (CPDF_Font*)m_FDE2PDFFont.GetValueAt(pFont);
    if (!pPDFFont) {
        return FALSE;
    }
    wUnicode = (FX_WCHAR)pPDFFont->CharCodeFromUnicode(wUnicode);
    iWidth = pPDFFont->GetCharWidthF(wUnicode);
    return TRUE;
}
CXFA_FontMgr::CXFA_FontMgr()
    : m_pDefFontMgr(NULL)
{
}
CXFA_FontMgr::~CXFA_FontMgr()
{
    DelAllMgrMap();
}
IFX_Font* CXFA_FontMgr::GetFont(XFA_HDOC hDoc, FX_WSTR wsFontFamily, FX_DWORD dwFontStyles,
                                FX_WORD wCodePage )
{
    FX_DWORD dwHash = FX_HashCode_String_GetW(wsFontFamily.GetPtr(), wsFontFamily.GetLength(), FALSE);
    CFX_ByteString bsKey;
    bsKey.Format("%u%u%u", dwHash, dwFontStyles, wCodePage);
    IFX_Font* pFont = NULL;
    if (m_FontArray.Lookup(bsKey, (void*&)pFont)) {
        return pFont;
    }
    CFX_WideString wsEnglishName;
    XFA_LocalFontNameToEnglishName(wsFontFamily, wsEnglishName);
    CXFA_PDFFontMgr* pMgr = (CXFA_PDFFontMgr*)m_PDFFontMgrArray.GetValueAt(hDoc);
    CPDF_Font* pPDFFont = NULL;
    if (pMgr != NULL) {
        pFont = pMgr->GetFont(wsEnglishName, dwFontStyles, &pPDFFont);
        if (pFont) {
            return pFont;
        }
    }
    if (pFont == NULL && m_pDefFontMgr != NULL) {
        pFont = m_pDefFontMgr->GetFont(hDoc, wsFontFamily, dwFontStyles, wCodePage);
    }
    if (pFont == NULL && pMgr != NULL) {
        pPDFFont = NULL;
        pFont = pMgr->GetFont(wsEnglishName, dwFontStyles, &pPDFFont, FALSE);
        if (pFont) {
            return pFont;
        }
    }
    if (pFont == NULL && m_pDefFontMgr != NULL) {
        pFont = m_pDefFontMgr->GetDefaultFont(hDoc, wsFontFamily, dwFontStyles, wCodePage);
    }
    FXSYS_assert(pFont != NULL);
    if (pFont) {
        if (pPDFFont) {
            pMgr->m_FDE2PDFFont.SetAt(pFont, pPDFFont);
            pFont->SetFontProvider(pMgr);
        }
        m_FontArray.SetAt(bsKey, pFont);
    }
    return pFont;
}
void CXFA_FontMgr::LoadDocFonts(XFA_HDOC hDoc)
{
    if (!m_PDFFontMgrArray.GetValueAt(hDoc)) {
        m_PDFFontMgrArray.SetAt(hDoc, FX_NEW CXFA_PDFFontMgr((CXFA_FFDoc*)hDoc));
    }
}
void CXFA_FontMgr::ReleaseDocFonts(XFA_HDOC hDoc)
{
    CXFA_PDFFontMgr* pMgr = NULL;
    if (m_PDFFontMgrArray.Lookup(hDoc, (void*&)pMgr)) {
        if (pMgr != NULL) {
            delete pMgr;
        }
        m_PDFFontMgrArray.RemoveKey(hDoc);
    }
}
void CXFA_FontMgr::DelAllMgrMap()
{
    FX_POSITION ps = m_PDFFontMgrArray.GetStartPosition();
    while (ps) {
        XFA_HDOC hDoc = NULL;
        CXFA_PDFFontMgr* pMgr = NULL;
        m_PDFFontMgrArray.GetNextAssoc(ps, (void*&)hDoc, (void*&)pMgr);
        if (pMgr != NULL) {
            delete pMgr;
        }
    }
    m_PDFFontMgrArray.RemoveAll();
    m_FontArray.RemoveAll();
}
void CXFA_FontMgr::SetDefFontMgr(IXFA_FontMgr* pFontMgr)
{
    m_pDefFontMgr = pFontMgr;
}
