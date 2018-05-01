// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/string_write_stream.h"
#include "testing/test_support.h"

TEST(CFX_XMLInstructionTest, GetType) {
  CFX_XMLInstruction node(L"acrobat");
  EXPECT_EQ(FX_XMLNODE_Instruction, node.GetType());
}

TEST(CFX_XMLInstructionTest, AcrobatInstruction) {
  CFX_XMLInstruction node(L"acrobat");
  EXPECT_TRUE(node.IsAcrobat());
  EXPECT_FALSE(node.IsOriginalXFAVersion());
}

TEST(CFX_XMLInstructionTest, OriginalXFAInstruction) {
  CFX_XMLInstruction node(L"originalXFAVersion");
  EXPECT_TRUE(node.IsOriginalXFAVersion());
  EXPECT_FALSE(node.IsAcrobat());
}

TEST(CFX_XMLInstructionTest, TargetData) {
  CFX_XMLInstruction node(L"acrobat");
  EXPECT_EQ(0U, node.GetTargetData().size());

  node.AppendData(L"firstString");
  node.AppendData(L"secondString");

  auto& data = node.GetTargetData();
  ASSERT_EQ(2U, data.size());
  EXPECT_EQ(L"firstString", data[0]);
  EXPECT_EQ(L"secondString", data[1]);
}

TEST(CFX_XMLInstructionTest, Clone) {
  CFX_XMLInstruction node(L"acrobat");
  node.AppendData(L"firstString");
  node.AppendData(L"secondString");

  auto clone = node.Clone();
  EXPECT_TRUE(clone != nullptr);

  ASSERT_EQ(FX_XMLNODE_Instruction, clone->GetType());
  CFX_XMLInstruction* inst = static_cast<CFX_XMLInstruction*>(clone.get());

  EXPECT_TRUE(inst->IsAcrobat());

  auto& data = inst->GetTargetData();
  ASSERT_EQ(2U, data.size());
  EXPECT_EQ(L"firstString", data[0]);
  EXPECT_EQ(L"secondString", data[1]);
}

TEST(CFX_XMLInstructionTest, SaveXML) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLInstruction node(L"xml");
  node.Save(stream);
  EXPECT_EQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", stream->ToString());
}

TEST(CFX_XMLInstructionTest, SaveAcrobat) {
  auto stream = pdfium::MakeRetain<StringWriteStream>();
  CFX_XMLInstruction node(L"acrobat");
  node.AppendData(L"http://www.xfa.org/schema/xfa-template/3.3/");
  node.AppendData(L"Display:1");

  node.Save(stream);
  EXPECT_EQ(
      "<?acrobat http://www.xfa.org/schema/xfa-template/3.3/ Display:1 ?>\n",
      stream->ToString());
}

TEST(CFX_XMLInstructionTest, ParseAndReSave) {
  const char* input =
      "<?acrobat http://www.xfa.org/schema/xfa-template/3.3/ Display:1 ?>\n"
      "<node></node>";

  auto in_stream = pdfium::MakeRetain<CFX_MemoryStream>(
      reinterpret_cast<uint8_t*>(const_cast<char*>(input)), strlen(input),
      false);

  CFX_XMLElement root(L"root");
  CFX_XMLParser parser(&root, in_stream);
  ASSERT_TRUE(parser.Parse());
  ASSERT_TRUE(root.GetFirstChild() != nullptr);
  ASSERT_EQ(FX_XMLNODE_Instruction, root.GetFirstChild()->GetType());

  CFX_XMLInstruction* node =
      static_cast<CFX_XMLInstruction*>(root.GetFirstChild());
  ASSERT_TRUE(node != nullptr);
  EXPECT_TRUE(node->IsAcrobat());

  auto& data = node->GetTargetData();
  ASSERT_EQ(2U, data.size());
  EXPECT_EQ(L"http://www.xfa.org/schema/xfa-template/3.3/", data[0]);
  EXPECT_EQ(L"Display:1", data[1]);

  auto out_stream = pdfium::MakeRetain<StringWriteStream>();
  node->Save(out_stream);
  EXPECT_EQ(
      "<?acrobat http://www.xfa.org/schema/xfa-template/3.3/ Display:1 ?>\n",
      out_stream->ToString());
}
