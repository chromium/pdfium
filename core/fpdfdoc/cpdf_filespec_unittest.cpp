// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_filespec.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

TEST(cpdf_filespec, EncodeDecodeFileName) {
  static const std::vector<pdfium::NullTermWstrFuncTestData> test_data = {
    // Empty src string.
    {L"", L""},
    // only file name.
    {L"test.pdf", L"test.pdf"},
#if BUILDFLAG(IS_WIN)
    // With drive identifier.
    {L"r:\\pdfdocs\\spec.pdf", L"/r/pdfdocs/spec.pdf"},
    // Relative path.
    {L"My Document\\test.pdf", L"My Document/test.pdf"},
    // Absolute path without drive identifier.
    {L"\\pdfdocs\\spec.pdf", L"//pdfdocs/spec.pdf"},
    // Absolute path with double backslashes.
    {L"\\\\pdfdocs\\spec.pdf", L"/pdfdocs/spec.pdf"},
// Network resource name. It is not supported yet.
// {L"pclib/eng:\\pdfdocs\\spec.pdf", L"/pclib/eng/pdfdocs/spec.pdf"},
#elif BUILDFLAG(IS_APPLE)
    // Absolute path with colon separator.
    {L"Mac HD:PDFDocs:spec.pdf", L"/Mac HD/PDFDocs/spec.pdf"},
    // Relative path with colon separator.
    {L"PDFDocs:spec.pdf", L"PDFDocs/spec.pdf"},
#else
    // Relative path.
    {L"./docs/test.pdf", L"./docs/test.pdf"},
    // Relative path with parent dir.
    {L"../test_docs/test.pdf", L"../test_docs/test.pdf"},
    // Absolute path.
    {L"/usr/local/home/test.pdf", L"/usr/local/home/test.pdf"},
#endif
  };
  for (const auto& data : test_data) {
    EXPECT_EQ(data.expected, CPDF_FileSpec::EncodeFileName(data.input));
    // DecodeFileName is the reverse procedure of EncodeFileName.
    EXPECT_EQ(data.input, CPDF_FileSpec::DecodeFileName(data.expected));
  }
}

TEST(cpdf_filespec, GetFileName) {
  {
    // String object.
    static const pdfium::NullTermWstrFuncTestData test_data = {
#if BUILDFLAG(IS_WIN)
      L"/C/docs/test.pdf",
      L"C:\\docs\\test.pdf"
#elif BUILDFLAG(IS_APPLE)
      L"/Mac HD/docs/test.pdf",
      L"Mac HD:docs:test.pdf"
#else
      L"/docs/test.pdf",
      L"/docs/test.pdf"
#endif
    };
    auto str_obj = pdfium::MakeRetain<CPDF_String>(nullptr, test_data.input);
    CPDF_FileSpec file_spec(str_obj);
    EXPECT_EQ(test_data.expected, file_spec.GetFileName());
  }
  {
    // Dictionary object.
    static constexpr std::array<pdfium::NullTermWstrFuncTestData, 5> test_data =
        {{
#if BUILDFLAG(IS_WIN)
            {L"/C/docs/test.pdf", L"C:\\docs\\test.pdf"},
            {L"/D/docs/test.pdf", L"D:\\docs\\test.pdf"},
            {L"/E/docs/test.pdf", L"E:\\docs\\test.pdf"},
            {L"/F/docs/test.pdf", L"F:\\docs\\test.pdf"},
            {L"/G/docs/test.pdf", L"G:\\docs\\test.pdf"},
#elif BUILDFLAG(IS_APPLE)
            {L"/Mac HD/docs1/test.pdf", L"Mac HD:docs1:test.pdf"},
            {L"/Mac HD/docs2/test.pdf", L"Mac HD:docs2:test.pdf"},
            {L"/Mac HD/docs3/test.pdf", L"Mac HD:docs3:test.pdf"},
            {L"/Mac HD/docs4/test.pdf", L"Mac HD:docs4:test.pdf"},
            {L"/Mac HD/docs5/test.pdf", L"Mac HD:docs5:test.pdf"},
#else
            {L"/docs/a/test.pdf", L"/docs/a/test.pdf"},
            {L"/docs/b/test.pdf", L"/docs/b/test.pdf"},
            {L"/docs/c/test.pdf", L"/docs/c/test.pdf"},
            {L"/docs/d/test.pdf", L"/docs/d/test.pdf"},
            {L"/docs/e/test.pdf", L"/docs/e/test.pdf"},
#endif
        }};
    // Keyword fields in reverse order of precedence to retrieve the file name.
    constexpr std::array<const char*, 5> keywords = {
        {"Unix", "Mac", "DOS", "F", "UF"}};
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    CPDF_FileSpec file_spec(dict_obj);
    EXPECT_TRUE(file_spec.GetFileName().IsEmpty());
    for (size_t i = 0; i < std::size(keywords); ++i) {
      dict_obj->SetNewFor<CPDF_String>(keywords[i], test_data[i].input);
      EXPECT_EQ(test_data[i].expected, file_spec.GetFileName());
    }

    // With all the former fields and 'FS' field suggests 'URL' type.
    dict_obj->SetNewFor<CPDF_String>("FS", "URL");
    // Url string is not decoded.
    EXPECT_EQ(test_data[4].input, file_spec.GetFileName());
  }
  {
    // Invalid object.
    auto name_obj = pdfium::MakeRetain<CPDF_Name>(nullptr, "test.pdf");
    CPDF_FileSpec file_spec(name_obj);
    EXPECT_TRUE(file_spec.GetFileName().IsEmpty());
  }
  {
    // Invalid CPDF_Name objects in dictionary. See https://crbug.com/959183
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    CPDF_FileSpec file_spec(dict_obj);
    for (const char* key : {"Unix", "Mac", "DOS", "F", "UF"}) {
      dict_obj->SetNewFor<CPDF_Name>(key, "http://evil.org");
      EXPECT_TRUE(file_spec.GetFileName().IsEmpty());
    }
    dict_obj->SetNewFor<CPDF_String>("FS", "URL");
    EXPECT_TRUE(file_spec.GetFileName().IsEmpty());
  }
}

TEST(cpdf_filespec, GetFileStream) {
  {
    // Invalid object.
    auto name_obj = pdfium::MakeRetain<CPDF_Name>(nullptr, "test.pdf");
    CPDF_FileSpec file_spec(name_obj);
    EXPECT_FALSE(file_spec.GetFileStream());
  }
  {
    // Dictionary object missing its embedded files dictionary.
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    CPDF_FileSpec file_spec(dict_obj);
    EXPECT_FALSE(file_spec.GetFileStream());
  }
  {
    // Dictionary object with an empty embedded files dictionary.
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    dict_obj->SetNewFor<CPDF_Dictionary>("EF");
    CPDF_FileSpec file_spec(dict_obj);
    EXPECT_FALSE(file_spec.GetFileStream());
  }
  {
    CPDF_IndirectObjectHolder object_holder;
    // Dictionary object with a non-empty embedded files dictionary.
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    dict_obj->SetNewFor<CPDF_Dictionary>("EF");
    CPDF_FileSpec file_spec(dict_obj);

    const wchar_t file_name[] = L"test.pdf";
    constexpr std::array<const char*, 5> keys = {
        {"Unix", "Mac", "DOS", "F", "UF"}};
    constexpr std::array<const char*, 5> streams = {
        {"test1", "test2", "test3", "test4", "test5"}};
    static_assert(std::size(keys) == std::size(streams), "size mismatch");
    RetainPtr<CPDF_Dictionary> file_dict = dict_obj->GetMutableDictFor("EF");

    // Keys in reverse order of precedence to retrieve the file content stream.
    for (size_t i = 0; i < std::size(keys); ++i) {
      // Set the file name.
      dict_obj->SetNewFor<CPDF_String>(keys[i], file_name);

      // Set the file stream.
      auto stream_object = object_holder.NewIndirect<CPDF_Stream>(
          ByteStringView(streams[i]).unsigned_span());
      ASSERT_TRUE(stream_object);
      const uint32_t stream_object_number = stream_object->GetObjNum();
      ASSERT_GT(stream_object_number, 0u);
      file_dict->SetNewFor<CPDF_Reference>(keys[i], &object_holder,
                                           stream_object_number);

      // Check that the file content stream is as expected.
      EXPECT_EQ(streams[i],
                file_spec.GetFileStream()->GetUnicodeText().ToUTF8());

      if (i == 2) {
        dict_obj->SetNewFor<CPDF_String>("FS", "URL");
        EXPECT_FALSE(file_spec.GetFileStream());
      }
    }
  }
}

TEST(cpdf_filespec, GetParamsDict) {
  {
    // Invalid object.
    auto name_obj = pdfium::MakeRetain<CPDF_Name>(nullptr, "test.pdf");
    CPDF_FileSpec file_spec(name_obj);
    EXPECT_FALSE(file_spec.GetParamsDict());
  }
  {
    CPDF_IndirectObjectHolder object_holder;

    // Dictionary object.
    auto dict_obj = pdfium::MakeRetain<CPDF_Dictionary>();
    dict_obj->SetNewFor<CPDF_Dictionary>("EF");
    dict_obj->SetNewFor<CPDF_String>("UF", L"test.pdf");
    CPDF_FileSpec file_spec(dict_obj);
    EXPECT_FALSE(file_spec.GetParamsDict());

    // Add a file stream to the embedded files dictionary.
    RetainPtr<CPDF_Dictionary> file_dict = dict_obj->GetMutableDictFor("EF");
    static constexpr char kHello[] = "hello";
    auto stream_object =
        object_holder.NewIndirect<CPDF_Stream>(pdfium::as_byte_span(kHello));
    ASSERT_TRUE(stream_object);
    const uint32_t stream_object_number = stream_object->GetObjNum();
    ASSERT_GT(stream_object_number, 0u);
    file_dict->SetNewFor<CPDF_Reference>("UF", &object_holder,
                                         stream_object_number);

    // Add a params dictionary to the file stream.
    RetainPtr<CPDF_Stream> stream = file_dict->GetMutableStreamFor("UF");
    RetainPtr<CPDF_Dictionary> stream_dict = stream->GetMutableDict();
    stream_dict->SetNewFor<CPDF_Dictionary>("Params");
    EXPECT_TRUE(file_spec.GetParamsDict());

    // Add a parameter to the params dictionary.
    RetainPtr<CPDF_Dictionary> params_dict =
        stream_dict->GetMutableDictFor("Params");
    params_dict->SetNewFor<CPDF_Number>("Size", 6);
    EXPECT_EQ(6, file_spec.GetParamsDict()->GetIntegerFor("Size"));
  }
}
