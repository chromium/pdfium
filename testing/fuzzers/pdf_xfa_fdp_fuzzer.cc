// Copyright 2021 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuzzer/FuzzedDataProvider.h>

#include <string>
#include <vector>

#include "public/fpdf_formfill.h"
#include "testing/fuzzers/pdf_fuzzer_templates.h"
#include "testing/fuzzers/pdfium_fuzzer_helper.h"
#include "third_party/base/containers/adapters.h"
#include "third_party/base/cxx17_backports.h"

class PDFiumXFAFuzzer : public PDFiumFuzzerHelper {
 public:
  PDFiumXFAFuzzer() = default;
  ~PDFiumXFAFuzzer() override = default;

  int GetFormCallbackVersion() const override { return 2; }

  // Return false if XFA doesn't load as otherwise we're duplicating the work
  // done by the non-xfa fuzzer.
  bool OnFormFillEnvLoaded(FPDF_DOCUMENT doc) override {
    int form_type = FPDF_GetFormType(doc);
    if (form_type != FORMTYPE_XFA_FULL && form_type != FORMTYPE_XFA_FOREGROUND)
      return false;
    return FPDF_LoadXFA(doc);
  }
};

// Possible names of an XFA script function
std::string GenXfaScriptFuncName(FuzzedDataProvider* data_provider) {
  static const char* const kXfaScriptFuncs[] = {
      "Abs",       "Apr",        "At",           "Avg",          "Ceil",
      "Choose",    "Concat",     "Count",        "Cterm",        "Date",
      "Date2Num",  "DateFmt",    "Decode",       "Encode",       "Eval",
      "Exists",    "Floor",      "Format",       "FV",           "Get",
      "HasValue",  "If",         "Ipmt",         "IsoDate2Num",  "IsoTime2Num",
      "Left",      "Len",        "LocalDateFmt", "LocalTimeFmt", "Lower",
      "Ltrim",     "Max",        "Min",          "Mod",          "NPV",
      "Num2Date",  "Num2GMTime", "Num2Time",     "Oneof",        "Parse",
      "Pmt",       "Post",       "PPmt",         "Put",          "PV",
      "Rate",      "Ref",        "Replace",      "Right",        "Round",
      "Rtrim",     "Space",      "Str",          "Stuff",        "Substr",
      "Sum",       "Term",       "Time",         "Time2Num",     "TimeFmt",
      "Translate", "UnitType",   "UnitValue",    "Upper",        "Uuid",
      "Within",    "WordNum",
  };

  size_t elem_selector = data_provider->ConsumeIntegralInRange<size_t>(
      0, pdfium::size(kXfaScriptFuncs) - 1);
  return kXfaScriptFuncs[elem_selector];
}

std::string MaybeQuote(FuzzedDataProvider* data_provider, std::string body) {
  if (data_provider->ConsumeIntegralInRange<uint32_t>(0, 100) < 20) {
    return "\"" + body + "\"";
  }
  return body;
}

// Possible arguments to a XFA script function
std::string GenXfaScriptParam(FuzzedDataProvider* data_provider) {
  static const char* const kXfaFuncParams[] = {
      "$",
      "-0",
      "04/13/2019",
      ".05",
      "-1",
      "1",
      " 1 | 0",
      "10 * 10 * 10 * 9 * 123",
      "1024",
      "10 * a + 9",
      "1.2131",
      "[1,2,3]",
      "%123",
      "[1,2,3][0]",
      "123124",
      "123342123",
      "13:13:13",
      "13:13:13 GMT",
      "19960315T20:20:20",
      "1 and 1",
      "1 and 2",
      "2",
      "20000201",
      "2009-06-01T13:45:30",
      "2009-06-15T01:45:30",
      "2009-06-15T13:45:30-07:00",
      "2009-06-15T13:45:30.5275000",
      " 2 < 3 + 1",
      "2 + 3 + 9",
      "3",
      "3 * 1",
      "3 -9",
      "5 < 5",
      "-99",
      "99",
      "9999999",
      "99999999999",
      "A",
      "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
      "ÁÂÃÄÅÆ",
      "<a><b></b></a>",
      "&Acirc;",
      "&AElig;&Aacute;&Acirc;&Aacute;",
      "Amount[*]",
      "~!@#$%^&amp;*()_+",
      "&amp;|",
      "&apos",
      "apr",
      "april",
      "B",
      "<br>",
      "C",
      "de_DE",
      "es_ES",
      "feb",
      "febuary",
      "HH:MM:SS",
      "<html>",
      "html",
      "HTML",
      "jan",
      "january",
      "json",
      "lkdjfglsdkfgj",
      "mar",
      "march",
      "name[0]",
      "name1",
      "name2",
      "name3",
      "name4",
      "name[*].numAmount",
      "&quot;",
      "Space",
      "Str",
      "url",
      "xhtml",
      "xml",
      "XML&quot;",
  };

  size_t elem_selector = data_provider->ConsumeIntegralInRange<size_t>(
      0, pdfium::size(kXfaFuncParams) - 1);
  return MaybeQuote(data_provider, kXfaFuncParams[elem_selector]);
}

// Possible XFA tags
std::string GenXfaTag(FuzzedDataProvider* data_provider) {
  static const char* const kXfaElemTags[] = {
      "accessibleContent",
      "acrobat",
      "acrobat",
      "acrobat7",
      "ADBE_JSConsole",
      "ADBE_JSDebugger",
      "addSilentPrint",
      "addViewerPreferences",
      "adjustData",
      "adobeExtensionLevel",
      "agent",
      "alwaysEmbed",
      "amd",
      "appearanceFilter",
      "arc",
      "area",
      "assist",
      "attributes",
      "autoSave",
      "barcode",
      "base",
      "batchOutput",
      "behaviorOverride",
      "bind",
      "bindItems",
      "bookend",
      "boolean",
      "border",
      "break",
      "breakAfter",
      "breakBefore",
      "button",
      "cache",
      "calculate",
      "calendarSymbols",
      "caption",
      "certificate",
      "certificates",
      "change",
      "checkButton",
      "choiceList",
      "color",
      "comb",
      "command",
      "common",
      "compress",
      "compression",
      "compressLogicalStructure",
      "compressObjectStream",
      "config",
      "config",
      "conformance",
      "connect",
      "connectionSet",
      "connectString",
      "contentArea",
      "contentCopy",
      "copies",
      "corner",
      "creator",
      "currencySymbol",
      "currencySymbols",
      "currentPage",
      "data",
      "dataGroup",
      "dataModel",
      "dataValue",
      "dataWindow",
      "date",
      "datePattern",
      "datePatterns",
      "dateTime",
      "dateTimeEdit",
      "dateTimeSymbols",
      "day",
      "dayNames",
      "debug",
      "decimal",
      "defaultTypeface",
      "defaultUi",
      "delete",
      "delta",
      "deltas",
      "desc",
      "destination",
      "digestMethod",
      "digestMethods",
      "documentAssembly",
      "draw",
      "driver",
      "dSigData",
      "duplexOption",
      "dynamicRender",
      "edge",
      "effectiveInputPolicy",
      "effectiveOutputPolicy",
      "embed",
      "encoding",
      "encodings",
      "encrypt",
      "encryption",
      "encryptionLevel",
      "encryptionMethod",
      "encryptionMethods",
      "enforce",
      "equate",
      "equateRange",
      "era",
      "eraNames",
      "event",
      "eventPseudoModel",
      "exclGroup",
      "exclude",
      "excludeNS",
      "exData",
      "execute",
      "exObject",
      "extras",
      "field",
      "fill",
      "filter",
      "flipLabel",
      "float",
      "font",
      "fontInfo",
      "form",
      "format",
      "formFieldFilling",
      "groupParent",
      "handler",
      "hostPseudoModel",
      "hyphenation",
      "ifEmpty",
      "image",
      "imageEdit",
      "includeXDPContent",
      "incrementalLoad",
      "incrementalMerge",
      "insert",
      "instanceManager",
      "integer",
      "interactive",
      "issuers",
      "items",
      "jog",
      "keep",
      "keyUsage",
      "labelPrinter",
      "layout",
      "layoutPseudoModel",
      "level",
      "line",
      "linear",
      "linearized",
      "list",
      "locale",
      "localeSet",
      "lockDocument",
      "log",
      "logPseudoModel",
      "manifest",
      "map",
      "margin",
      "mdp",
      "medium",
      "mediumInfo",
      "meridiem",
      "meridiemNames",
      "message",
      "messaging",
      "mode",
      "modifyAnnots",
      "month",
      "monthNames",
      "msgId",
      "nameAttr",
      "neverEmbed",
      "numberOfCopies",
      "numberPattern",
      "numberPatterns",
      "numberSymbol",
      "numberSymbols",
      "numericEdit",
      "object",
      "occur",
      "oid",
      "oids",
      "openAction",
      "operation",
      "output",
      "outputBin",
      "outputXSL",
      "overflow",
      "overprint",
      "packet",
      "packets",
      "pageArea",
      "pageOffset",
      "pageRange",
      "pageSet",
      "pagination",
      "paginationOverride",
      "para",
      "part",
      "password",
      "passwordEdit",
      "pattern",
      "pcl",
      "pdf",
      "pdfa",
      "permissions",
      "pickTrayByPDFSize",
      "picture",
      "plaintextMetadata",
      "presence",
      "present",
      "present",
      "print",
      "printerName",
      "printHighQuality",
      "printScaling",
      "producer",
      "proto",
      "ps",
      "psMap",
      "query",
      "radial",
      "range",
      "reason",
      "reasons",
      "record",
      "recordSet",
      "rectangle",
      "ref",
      "relevant",
      "rename",
      "renderPolicy",
      "rootElement",
      "runScripts",
      "script",
      "scriptModel",
      "select",
      "setProperty",
      "severity",
      "signature",
      "signatureProperties",
      "signaturePseudoModel",
      "signData",
      "signing",
      "silentPrint",
      "soapAction",
      "soapAddress",
      "solid",
      "source",
      "sourceSet",
      "speak",
      "staple",
      "startNode",
      "startPage",
      "stipple",
      "subform",
      "subform",
      "subformSet",
      "subjectDN",
      "subjectDNs",
      "submit",
      "submitFormat",
      "submitUrl",
      "subsetBelow",
      "suppressBanner",
      "tagged",
      "template",
      "template",
      "templateCache",
      "#text",
      "text",
      "textedit",
      "textEdit",
      "threshold",
      "time",
      "timePattern",
      "timePatterns",
      "timeStamp",
      "to",
      "toolTip",
      "trace",
      "transform",
      "traversal",
      "traverse",
      "treeList",
      "type",
      "typeface",
      "typefaces",
      "ui",
      "update",
      "uri",
      "user",
      "validate",
      "validate",
      "validateApprovalSignatures",
      "validationMessaging",
      "value",
      "variables",
      "version",
      "versionControl",
      "viewerPreferences",
      "webClient",
      "whitespace",
      "window",
      "wsdlAddress",
      "wsdlConnection",
      "xdc",
      "xdp",
      "xfa",
      "#xHTML",
      "#xml",
      "xmlConnection",
      "xsdConnection",
      "xsl",
      "zpl",
  };

  size_t elem_selector = data_provider->ConsumeIntegralInRange<size_t>(
      0, pdfium::size(kXfaElemTags) - 1);
  return kXfaElemTags[elem_selector];
}

// Possible XFA attributes values
std::string GenXfaTagValue(FuzzedDataProvider* data_provider) {
  static const char* const kXfaTagVals[] = {
      "0",         "0pt",          "-1",
      "123",       "1pt",          "203.2mm",
      "22.1404mm", "255",          "256",
      "321",       "5431.21mm",    "6.35mm",
      "8in",       "8pt",          "application/x-javascript",
      "bold",      "bold",         "consumeData",
      "en_US",     "form1",        "initialize",
      "italic",    "middle",       "name2",
      "name3",     "name4",        "name5",
      "Page1",     "RadioList[0]", "subform_1",
      "tb",        "Verdana",      "Verdana",
  };

  size_t elem_selector = data_provider->ConsumeIntegralInRange<size_t>(
      0, pdfium::size(kXfaTagVals) - 1);
  return MaybeQuote(data_provider, kXfaTagVals[elem_selector]);
}

// possible XFA attributes
std::string GenXfaTagName(FuzzedDataProvider* data_provider) {
  static const char* const kXfaTagNames[] = {
      "activity",    "activity",    "baselineShift",
      "contentType", "h",           "id",
      "layout",      "layout",      "leftInset",
      "locale",      "long",        "marginLeft",
      "marginRight", "marginRight", "mergeMode",
      "name",        "ref",         "scriptTest",
      "short",       "size",        "spaceAbove",
      "spaceBelow",  "startNew",    "stock",
      "tetIndent",   "timeStamp",   "typeface",
      "uuid",        "vAlign",      "value",
      "w",           "weight",      "x",
      "y",
  };
  size_t elem_selector = data_provider->ConsumeIntegralInRange<size_t>(
      0, pdfium::size(kXfaTagNames) - 1);
  return kXfaTagNames[elem_selector];
}

// Will create a simple XFA script that calls a single function.
std::string GenXfacript(FuzzedDataProvider* data_provider) {
  std::string xfa_string = GenXfaScriptFuncName(data_provider);
  xfa_string += "(";

  int num_params = data_provider->ConsumeIntegralInRange(0, 3);
  // 0 case we do nothing.
  if (num_params == 1) {
    xfa_string += GenXfaScriptParam(data_provider);
  } else if (num_params == 2) {
    xfa_string += GenXfaScriptParam(data_provider);
    xfa_string += ",";
    xfa_string += GenXfaScriptParam(data_provider);
  } else if (num_params == 3) {
    xfa_string += GenXfaScriptParam(data_provider);
    xfa_string += ",";
    xfa_string += GenXfaScriptParam(data_provider);
    xfa_string += ",";
    xfa_string += GenXfaScriptParam(data_provider);
  }
  xfa_string += ")";
  return xfa_string;
}

// Will create a single XFA attributes, with both lhs and rhs.
std::string getXfaElemAttributes(FuzzedDataProvider* data_provider) {
  // Generate a set of tags, and a set of values for the tags.
  return GenXfaTagName(data_provider) + " = " + GenXfaTagValue(data_provider);
}

// Creates an XFA structure wrapped in <xdp tags.
std::string GenXfaTree(FuzzedDataProvider* data_provider) {
  std::string xfa_string = "<xdp xmlns=\"http://ns.adobe.com/xdp/\">";

  // One stack iteration
  int stack_iterations = data_provider->ConsumeIntegralInRange(1, 3);
  for (int si = 0; si < stack_iterations; si++) {
    int elem_count = data_provider->ConsumeIntegralInRange(1, 6);
    std::vector<std::string> xml_stack;
    xml_stack.reserve(elem_count);
    for (int i = 0; i < elem_count; i++) {
      xfa_string += "<";
      std::string tag = GenXfaTag(data_provider);

      // in 30% of cases, add attributes
      std::string attribute_string;
      if (data_provider->ConsumeIntegralInRange(1, 100) > 70) {
        size_t attribute_count = data_provider->ConsumeIntegralInRange(1, 5);
        for (; 0 < attribute_count; attribute_count--) {
          attribute_string += getXfaElemAttributes(data_provider);
        }
      }
      xfa_string += attribute_string;
      xfa_string += tag + ">";

      // If needed, add a body to the tag
      if (tag == "script") {
        xfa_string += GenXfacript(data_provider);
      }

      // Push the tag to the stack so we can close it when done
      xml_stack.push_back(tag);
    }
    for (const std::string& tag : pdfium::base::Reversed(xml_stack)) {
      xfa_string += "</" + tag + ">";
    }
  }
  xfa_string += "</xdp>";
  return xfa_string;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzedDataProvider data_provider(data, size);
  std::string xfa_string = GenXfaTree(&data_provider);

  // Add 1 for newline before endstream.
  std::string xfa_stream_len = std::to_string(xfa_string.size() + 1);

  // Compose the fuzzer
  std::string xfa_final_str = std::string(kSimplePdfTemplate);
  xfa_final_str.replace(xfa_final_str.find("$1"), 2, xfa_stream_len);
  xfa_final_str.replace(xfa_final_str.find("$2"), 2, xfa_string);

#ifdef PDFIUM_FUZZER_DUMP
  for (size_t i = 0; i < xfa_final_str.size(); i++) {
    putc(xfa_final_str[i], stdout);
  }
#endif

  PDFiumXFAFuzzer fuzzer;
  fuzzer.RenderPdf(xfa_final_str.c_str(), xfa_final_str.size());
  return 0;
}
