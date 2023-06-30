// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "samples/helpers/dump.h"

#include <limits.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <utility>

#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_doc.h"
#include "public/fpdf_transformpage.h"
#include "testing/fx_string_testhelpers.h"

using GetBoxInfoFunc =
    std::function<bool(FPDF_PAGE, float*, float*, float*, float*)>;

namespace {

std::wstring ConvertToWString(const unsigned short* buf,
                              unsigned long buf_size) {
  std::wstring result;
  result.reserve(buf_size);
  std::copy(buf, buf + buf_size, std::back_inserter(result));
  return result;
}

void DumpBoxInfo(GetBoxInfoFunc func,
                 const char* box_type,
                 FPDF_PAGE page,
                 int page_idx) {
  FS_RECTF rect;
  bool ret = func(page, &rect.left, &rect.bottom, &rect.right, &rect.top);
  if (!ret) {
    printf("Page %d: No %s.\n", page_idx, box_type);
    return;
  }
  printf("Page %d: %s: %0.2f %0.2f %0.2f %0.2f\n", page_idx, box_type,
         rect.left, rect.bottom, rect.right, rect.top);
}

void DumpStructureElementAttributes(FPDF_STRUCTELEMENT_ATTR attr, int indent) {
  static const size_t kBufSize = 1024;
  int count = FPDF_StructElement_Attr_GetCount(attr);
  for (int i = 0; i < count; i++) {
    char name[kBufSize] = {};
    unsigned long len = ULONG_MAX;
    if (!FPDF_StructElement_Attr_GetName(attr, i, name, sizeof(name), &len)) {
      printf("%*s FPDF_StructElement_Attr_GetName failed for %d\n", indent, "",
             i);
      continue;
    }

    FPDF_OBJECT_TYPE type = FPDF_StructElement_Attr_GetType(attr, name);
    if (type == FPDF_OBJECT_BOOLEAN) {
      int value;
      if (!FPDF_StructElement_Attr_GetBooleanValue(attr, name, &value)) {
        printf("%*s %s: Failed FPDF_StructElement_Attr_GetBooleanValue\n",
               indent, "", name);
        continue;
      }
      printf("%*s %s: %d\n", indent, "", name, value);
    } else if (type == FPDF_OBJECT_NUMBER) {
      float value;
      if (!FPDF_StructElement_Attr_GetNumberValue(attr, name, &value)) {
        printf("%*s %s: Failed FPDF_StructElement_Attr_GetNumberValue\n",
               indent, "", name);
        continue;
      }
      printf("%*s %s: %f\n", indent, "", name, value);
    } else if (type == FPDF_OBJECT_STRING || type == FPDF_OBJECT_NAME) {
      unsigned short buffer[kBufSize] = {};
      if (!FPDF_StructElement_Attr_GetStringValue(attr, name, buffer,
                                                  sizeof(buffer), &len)) {
        printf("%*s %s: Failed FPDF_StructElement_Attr_GetStringValue\n",
               indent, "", name);
        continue;
      }
      printf("%*s %s: %ls\n", indent, "", name,
             ConvertToWString(buffer, len).c_str());
    } else if (type == FPDF_OBJECT_UNKNOWN) {
      printf("%*s %s: FPDF_OBJECT_UNKNOWN\n", indent, "", name);
    } else {
      printf("%*s %s: NOT_YET_IMPLEMENTED: %d\n", indent, "", name, type);
    }
  }
}

}  // namespace

void DumpChildStructure(FPDF_STRUCTELEMENT child, int indent) {
  static const size_t kBufSize = 1024;
  unsigned short buf[kBufSize];
  unsigned long len = FPDF_StructElement_GetType(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s S: %ls\n", indent * 2, "", ConvertToWString(buf, len).c_str());
  }

  int attr_count = FPDF_StructElement_GetAttributeCount(child);
  for (int i = 0; i < attr_count; i++) {
    FPDF_STRUCTELEMENT_ATTR child_attr =
        FPDF_StructElement_GetAttributeAtIndex(child, i);
    if (!child_attr) {
      continue;
    }
    printf("%*s A[%d]:\n", indent * 2, "", i);
    DumpStructureElementAttributes(child_attr, indent * 2 + 2);
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetActualText(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s ActualText: %ls\n", indent * 2, "",
           ConvertToWString(buf, len).c_str());
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetAltText(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s AltText: %ls\n", indent * 2, "",
           ConvertToWString(buf, len).c_str());
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetID(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s ID: %ls\n", indent * 2, "", ConvertToWString(buf, len).c_str());
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetLang(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s Lang: %ls\n", indent * 2, "",
           ConvertToWString(buf, len).c_str());
  }

  int mcid = FPDF_StructElement_GetMarkedContentID(child);
  if (mcid != -1) {
    printf("%*s MCID: %d\n", indent * 2, "", mcid);
  }

  FPDF_STRUCTELEMENT parent = FPDF_StructElement_GetParent(child);
  if (parent) {
    memset(buf, 0, sizeof(buf));
    len = FPDF_StructElement_GetID(parent, buf, kBufSize);
    if (len > 0) {
      printf("%*s Parent ID: %ls\n", indent * 2, "",
             ConvertToWString(buf, len).c_str());
    }
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetTitle(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s Title: %ls\n", indent * 2, "",
           ConvertToWString(buf, len).c_str());
  }

  memset(buf, 0, sizeof(buf));
  len = FPDF_StructElement_GetObjType(child, buf, kBufSize);
  if (len > 0) {
    printf("%*s Type: %ls\n", indent * 2, "",
           ConvertToWString(buf, len).c_str());
  }

  for (int i = 0; i < FPDF_StructElement_CountChildren(child); ++i) {
    FPDF_STRUCTELEMENT sub_child = FPDF_StructElement_GetChildAtIndex(child, i);
    // If the child is not an Element then this will return null. This can
    // happen if the element is things like an object reference or a stream.
    if (!sub_child) {
      continue;
    }

    DumpChildStructure(sub_child, indent + 1);
  }
}

void DumpPageInfo(FPDF_PAGE page, int page_idx) {
  DumpBoxInfo(&FPDFPage_GetMediaBox, "MediaBox", page, page_idx);
  DumpBoxInfo(&FPDFPage_GetCropBox, "CropBox", page, page_idx);
  DumpBoxInfo(&FPDFPage_GetBleedBox, "BleedBox", page, page_idx);
  DumpBoxInfo(&FPDFPage_GetTrimBox, "TrimBox", page, page_idx);
  DumpBoxInfo(&FPDFPage_GetArtBox, "ArtBox", page, page_idx);
}

void DumpPageStructure(FPDF_PAGE page, int page_idx) {
  ScopedFPDFStructTree tree(FPDF_StructTree_GetForPage(page));
  if (!tree) {
    fprintf(stderr, "Failed to load struct tree for page %d\n", page_idx);
    return;
  }

  printf("Structure Tree for Page %d\n", page_idx);
  for (int i = 0; i < FPDF_StructTree_CountChildren(tree.get()); ++i) {
    FPDF_STRUCTELEMENT child = FPDF_StructTree_GetChildAtIndex(tree.get(), i);
    if (!child) {
      fprintf(stderr, "Failed to load child %d for page %d\n", i, page_idx);
      continue;
    }
    DumpChildStructure(child, 0);
  }
  printf("\n\n");
}

void DumpMetaData(FPDF_DOCUMENT doc) {
  static constexpr const char* kMetaTags[] = {
      "Title",   "Author",   "Subject",      "Keywords",
      "Creator", "Producer", "CreationDate", "ModDate"};
  for (const char* meta_tag : kMetaTags) {
    char meta_buffer[4096];
    unsigned long len =
        FPDF_GetMetaText(doc, meta_tag, meta_buffer, sizeof(meta_buffer));
    if (!len) {
      continue;
    }

    auto* meta_string = reinterpret_cast<unsigned short*>(meta_buffer);
    printf("%-12s = %ls (%lu bytes)\n", meta_tag,
           GetPlatformWString(meta_string).c_str(), len);
  }
}
