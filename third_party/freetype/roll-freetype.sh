#!/bin/bash
#
# Copyright 2017 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e
set -o pipefail
set -u

REVIEWERS=`paste -s -d, third_party/freetype/OWNERS`
roll-dep -r "${REVIEWERS}" "$@" third_party/freetype/src/
FT_VERSION=`git -C third_party/freetype/src/ describe --long`
FT_COMMIT=`git -C third_party/freetype/src/ rev-parse HEAD`
FT_CPE_VERSION=$(echo ${FT_VERSION} | sed -r -e's/^VER-([0-9]+)-([0-9]+)-([0-9]+)-[0-9]+-g[0-9a-f]+$/\1.\2.\3/')

# Make sure our copy of pstables.h matches the one in freetype/src.
# May need to --bypass-hooks to prevent formatting of this file.
cp third_party/freetype/src/src/psnames/pstables.h \
  third_party/freetype/include/pstables.h

sed \
  -e "s/^Version: .*\$/Version: ${FT_VERSION%-*}/" \
  -e "s/^Revision: .*\$/Revision: ${FT_COMMIT}/" \
  -e "s@^CPEPrefix: cpe:/a:freetype:freetype:.*\$@CPEPrefix: cpe:/a:freetype:freetype:${FT_CPE_VERSION}@" \
  third_party/freetype/README.pdfium > third_party/freetype/README.pdfium.new
mv third_party/freetype/README.pdfium.new third_party/freetype/README.pdfium

git add third_party/freetype/README.pdfium
git add third_party/freetype/include/pstables.h
git commit --quiet --amend --no-edit
