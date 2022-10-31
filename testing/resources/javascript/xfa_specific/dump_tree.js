// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Note: Be sure that this file is only included inside a CDATA block,
// otherwise the less-than comparision below will break the XML parse.

function dumpTree(node, level) {
  level = level || 0;
  var indentation = "| ".repeat(level);
  try {
    app.alert(indentation + node.className);
    var children = node.nodes;
    if (children) {
      for (var i = 0; i < children.length; ++i) {
        dumpTree(children.item(i), level + 1);
      }
    }
  } catch (e) {
    app.alert(indentation + "Error: " + e);
  }
}
