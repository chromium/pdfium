#!/bin/bash
#
# Copyright 2015 PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Script to generate expected result files.

# Arbitrary timestamp, expressed in seconds since the epoch, used to make sure
# that tests that depend on the current time are stable. Happens to be the
# timestamp of the first commit to repo, 2014/5/9 17:48:50.
TEST_SEED_TIME=1399672130

# Do this before "set -e" so "which" failing is not fatal.
PNGOPTIMIZER="$(which optipng)"

set -e
while (( "$#" )); do
  INFILE="$1"
  echo $INFILE | grep -qs ' ' && echo space in filename detected && exit 1
  EVTFILE="${INFILE%.*}.evt"
  SEND_EVENTS=
  if [ -f "$EVTFILE" ]; then
    SEND_EVENTS="--send-events"
  fi
  out/Debug/pdfium_test $SEND_EVENTS --time=$TEST_SEED_TIME --png $INFILE
  RESULTS="$INFILE.*.png"
  for RESULT in $RESULTS ; do
    EXPECTED=`echo -n $RESULT | sed 's/[.]pdf[.]/_expected.pdf./'`
    mv $RESULT $EXPECTED
    if [ -n "$PNGOPTIMIZER" ]; then
      "$PNGOPTIMIZER" $EXPECTED
    fi
  done
  shift
done
