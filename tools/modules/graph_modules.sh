#!/bin/bash
#
# Copyright 2019 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Generate a chart of dependencies and includes in "dot" format.
# Invoke in pdfium/ top-level directory

BUILD_DIR=out/Default

function crunch {
  echo '  edge [color=black,constraint=true]'
  gn desc $BUILD_DIR $1 deps | grep -v '//:' | grep -v test | \
      grep -v constants | grep -v samples | grep -v matches | \
      sed "s|\\(.*\\)|  \"$1\" -> \"\\1\"|"
  echo '  edge [color=red,constraint=false]'
  gn desc $BUILD_DIR $1 allow_circular_includes_from | grep -v '//:' | \
      grep -v test | grep -v samples | grep -v matches | \
      grep -v 'how to display' | sed "s|\\(.*\\)|  \"\\1\" -> \"$1\"|"
}

TARGETS=`gn ls $BUILD_DIR | grep -v test | grep -v v8 | grep -v third_party | \
             grep -v build | grep -v '//:'`

echo 'digraph FRED {'
echo '  node [shape=rectangle]'
for TARGET in $TARGETS; do
  crunch $TARGET
done
echo '}'
