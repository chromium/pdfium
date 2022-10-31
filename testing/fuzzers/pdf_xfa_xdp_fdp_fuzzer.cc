// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuzzer/FuzzedDataProvider.h>

#include <string>

#include "public/fpdf_formfill.h"
#include "testing/fuzzers/pdf_fuzzer_templates.h"
#include "testing/fuzzers/pdfium_fuzzer_helper.h"

class PDFiumXDPFuzzer : public PDFiumFuzzerHelper {
 public:
  PDFiumXDPFuzzer() = default;
  ~PDFiumXDPFuzzer() override = default;

  int GetFormCallbackVersion() const override { return 2; }

  bool OnFormFillEnvLoaded(FPDF_DOCUMENT doc) override {
    int form_type = FPDF_GetFormType(doc);
    if (form_type != FORMTYPE_XFA_FULL && form_type != FORMTYPE_XFA_FOREGROUND)
      return false;
    return FPDF_LoadXFA(doc);
  }
};

struct Tag {
  const char* tag_name;
  const char* tag_start;
  const char* tag_end;
};

const Tag kTagData[]{
    {.tag_name = "config",
     .tag_start =
         R""(<xfa:config xmlns:xfa="http://www.xfa.org/schema/xci/3.1/">)"",
     .tag_end = "</xfa:config>"},
    {.tag_name = "template",
     .tag_start =
         R""(<template xmlns="http://www.xfa.org/schema/xfa-template/2.6/">)"",
     .tag_end = "</template>"},
    {.tag_name = "sourceSet",
     .tag_start =
         R""(<sourceSet xmlns="http://www.xfa.org/schema/xfa-source-set/2.7/">)"",
     .tag_end = "</sourceSet>"},
    {.tag_name = "localeSet",
     .tag_start =
         R""(<localeSet xmlns="http://www.xfa.org/schema/xfa-locale-set/2.7/">)"",
     .tag_end = "</localeSet>"},
    {.tag_name = "dataSet",
     .tag_start =
         R""(<xfa:datasets xmlns:xfa="http://www.xfa.org/schema/xfa-data/1.0/">)"",
     .tag_end = "</xfa:datasets>"},
    {.tag_name = "connectionSet",
     .tag_start =
         R""(<connectionSet xmlns="http://www.xfa.org/schema/xfa-connection-set/2.8/">)"",
     .tag_end = "</connectionSet>"},
    {.tag_name = "xdc",
     .tag_start =
         R""(<xsl:xdc xmlns:xdc="http://www.xfa.org/schema/xdc/1.0/">)"",
     .tag_end = "</xsl:xdc>"},
    {.tag_name = "signature",
     .tag_start = R""(<signature xmlns="http://www.w3.org/2000/09/xmldsig#">)"",
     .tag_end = "</signature>"},
    {.tag_name = "stylesheet",
     .tag_start =
         R""(<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" id="identifier">)"",
     .tag_end = "</stylesheet>"},
    {.tag_name = "xfdf",
     .tag_start =
         R""(<xfdf xmlns="http://ns.adobe.com/xfdf/" xml:space="preserve">)"",
     .tag_end = "</xfdf>"},
    {.tag_name = "xmpmeta",
     .tag_start =
         R""(<xmpmeta xmlns="http://ns.adobe.com/xmpmeta/" xml:space="preserve">)"",
     .tag_end = "</xmpmeta>"}};

std::string CreateObject(int obj_num, const std::string& body) {
  std::string obj_template = R""($1 0 obj
$2
endobj
)"";

  obj_template.replace(obj_template.find("$1"), 2, std::to_string(obj_num));
  obj_template.replace(obj_template.find("$2"), 2, body);
  return obj_template;
}

std::string CreateStreamObject(int obj_num, const std::string& body) {
  std::string obj_template = R""($1 0 obj
<</Length $2>>
stream
$3
endstream
endobj
)"";

  obj_template.replace(obj_template.find("$1"), 2, std::to_string(obj_num));
  obj_template.replace(obj_template.find("$2"), 2,
                       std::to_string(body.size() + 1));
  obj_template.replace(obj_template.find("$3"), 2, body);

  return obj_template;
}

std::string GenXrefEntry(size_t offset) {
  return std::string(10 - std::to_string(offset).size(), '0') +
         std::to_string(offset) + " 00000 n\n";
}

std::string GenTagBody(const Tag& tag, FuzzedDataProvider* data_provider) {
  std::string tag_content = data_provider->ConsumeRandomLengthString();
  return tag.tag_start + tag_content + tag.tag_end;
}

std::string GenXDPPdfFile(FuzzedDataProvider* data_provider) {
  std::vector<std::string> pdf_objects;
  std::string pdf_header =
      std::string(reinterpret_cast<const char*>(kSimplePdfHeader),
                  sizeof(kSimplePdfHeader));

  pdf_objects.push_back(CreateObject(1, kCatalog));

  std::string xfa_obj = kSimpleXfaObjWrapper;
  Tag tag1 = data_provider->PickValueInArray(kTagData);
  Tag tag2 = data_provider->PickValueInArray(kTagData);
  Tag tag3 = data_provider->PickValueInArray(kTagData);
  xfa_obj.replace(xfa_obj.find("$1"), 2, tag1.tag_name);
  xfa_obj.replace(xfa_obj.find("$2"), 2, tag2.tag_name);
  xfa_obj.replace(xfa_obj.find("$3"), 2, tag3.tag_name);
  pdf_objects.push_back(CreateObject(2, xfa_obj));
  pdf_objects.push_back(CreateObject(3, kSimplePagesObj));
  pdf_objects.push_back(CreateObject(4, kSimplePageObj));

  // preamble
  pdf_objects.push_back(CreateStreamObject(5, kSimplePreamble));

  // The three XFA tags
  pdf_objects.push_back(CreateStreamObject(6, GenTagBody(tag1, data_provider)));
  pdf_objects.push_back(CreateStreamObject(7, GenTagBody(tag2, data_provider)));
  pdf_objects.push_back(CreateStreamObject(8, GenTagBody(tag3, data_provider)));

  // postamble
  pdf_objects.push_back(CreateStreamObject(9, kSimplePostamble));

  // Create the xref table
  std::string xref = R""(xref
0 10
0000000000 65535 f
)"";

  // Add xref entries
  size_t curr_offset = pdf_header.size();
  for (const auto& ostr : pdf_objects) {
    xref += GenXrefEntry(curr_offset);
    curr_offset += ostr.size();
  }

  std::string footer = R""(trailer
<</Root 1 0 R /Size 10>>
startxref
$1
%%EOF)"";
  footer.replace(footer.find("$1"), 2, std::to_string(curr_offset));

  std::string pdf_core;
  for (const auto& ostr : pdf_objects) {
    pdf_core += ostr;
  }

  // Return the full PDF
  return pdf_header + pdf_core + xref + footer;
}

bool IsValidForFuzzing(const uint8_t* data, size_t size) {
  if (size > 2048) {
    return false;
  }
  const char* ptr = reinterpret_cast<const char*>(data);
  for (size_t i = 0; i < size; i++) {
    if (!std::isspace(ptr[i]) && !std::isprint(ptr[i])) {
      return false;
    }
  }
  return true;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (!IsValidForFuzzing(data, size)) {
    return 0;
  }

  FuzzedDataProvider data_provider(data, size);
  std::string xfa_final_str = GenXDPPdfFile(&data_provider);

#ifdef PDFIUM_FUZZER_DUMP
  for (size_t i = 0; i < xfa_final_str.size(); i++) {
    putc(xfa_final_str[i], stdout);
  }
#endif

  PDFiumXDPFuzzer fuzzer;
  fuzzer.RenderPdf(xfa_final_str.c_str(), xfa_final_str.size());
  return 0;
}
