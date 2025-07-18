#!/bin/bash
#
# Copyright 2024 The PDFium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This ia a script to help update reference images for pixel tests and
# corpus tests for non-liunx (or non-agg) platforms. It uses results stored
# in pdfium-gold so one need not generate these locally for the various
# platforms.
#
# Given a "red bot" result, the first step is to identify the failing
# "stdout" test log for the failing test step on that bot (be it pixel
# or corpus).
#
# The next step is to identify the suffix the platform appends to the
# expected test results, for example for the win_xfa_skia bot, this
# is skia_win.
#
# The script will then fetch the build log into a file in the current
# directory named, e.g. skia_win.log. It leaves this file present after
# execution, so that the results can be manually double-checked if desired.
#
# Lastly, the script fetches each of the image files into the current
# directory (A possible improvement is to try and parse out where they
# belong). Move them by hand into the target directories.
#
set -e
if [ $# -ne 2 ] ; then
    echo "Usage: $0 <image_suffix> <log_url>, e.g."
    echo -n "$0 skia_win "
    echo -n "https://logs.chromium.org/logs/pdfium/buildbucket/cr-buildbucket/"
    echo -n "8753735982839766497/+/u/corpus_tests__oneshot_rendering_enabled_/"
    echo "stdout?format=raw"
    exit 1
fi

# Fetch test results
curl -o ./$1.log $2

# Parse log to identify failing tests.
TESTS=`grep 'cache[/\\]builder[/\\]' ./$1.log | grep -v Failed | \
            grep -ho '[A-Za-z0-9_-]*[.]pdf' | sed 's/\(.*\)[.]pdf/\1/'`
for TEST in $TESTS ; do
    IDX=0
    while true ; do
        REV=`grep "^Untriaged.*$TEST.pdf.$IDX" ./$1.log | \
                   sed 's/^.*digest=\([0-9a-f]*\).*$/\1/'`
        if [ -z "$REV" ] ; then
            break
        fi
        curl -o ./${TEST}_expected_$1.pdf.$IDX.png \
             https://pdfium-gold.skia.org/img/images/$REV.png
        optipng ./${TEST}_expected_$1.pdf.$IDX.png
        IDX=$(($IDX + 1))
    done
done
