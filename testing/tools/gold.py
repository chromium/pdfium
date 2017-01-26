# Copyright 2015 The PDFium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import json
import os
import shlex
import shutil

# This module collects and writes output in a format expected by the
# Gold baseline tool. Based on meta data provided explicitly and by
# adding a series of test results it can be used to produce
# a JSON file that is uploaded to Google Storage and ingested by Gold.
#
# The output will look similar this:
#
# {
#    "build_number" : "2",
#    "gitHash" : "a4a338179013b029d6dd55e737b5bd648a9fb68c",
#    "key" : {
#       "arch" : "arm64",
#       "compiler" : "Clang",
#    },
#    "results" : [
#       {
#          "key" : {
#             "config" : "vk",
#             "name" : "yuv_nv12_to_rgb_effect",
#             "source_type" : "gm"
#          },
#          "md5" : "7db34da246868d50ab9ddd776ce6d779",
#          "options" : {
#             "ext" : "png",
#             "gamma_correct" : "no"
#          }
#       },
#       {
#          "key" : {
#             "config" : "vk",
#             "name" : "yuv_to_rgb_effect",
#             "source_type" : "gm"
#          },
#          "md5" : "0b955f387740c66eb23bf0e253c80d64",
#          "options" : {
#             "ext" : "png",
#             "gamma_correct" : "no"
#          }
#       }
#    ],
# }
#
class GoldResults(object):
  def __init__(self, source_type, outputDir, propertiesStr, keyStr,
               ignore_hashes_file):
    """
    source_type is the source_type (=corpus) field used for all results.
    output_dir is the directory where the resulting images are copied and
               the dm.json file is written.
    propertiesStr is a string with space separated key/value pairs that
               is used to set the top level fields in the output JSON file.
    keyStr is a string with space separated key/value pairs that
               is used to set the 'key' field in the output JSON file.
    ignore_hashes_file is a file that contains a list of image hashes
               that should be ignored.
    """
    self._source_type = source_type
    self._properties = self._parseKeyValuePairs(propertiesStr)
    self._properties["key"] = self._parseKeyValuePairs(keyStr)
    self._results =  []
    self._outputDir = outputDir

    # make sure the output directory exists.
    if not os.path.exists(outputDir):
      os.makedirs(outputDir)

    self._ignore_hashes = set()
    if ignore_hashes_file:
      with open(ignore_hashes_file, 'r') as ig_file:
        hashes=[x.strip() for x in ig_file.readlines() if x.strip()]
        self._ignore_hashes = set(hashes)

  def AddTestResult(self, testName, md5Hash, outputImagePath):
    # If the hash is in the list of hashes to ignore then we don'try
    # make a copy, but add it to the result.
    imgExt = os.path.splitext(outputImagePath)[1].lstrip(".")
    if md5Hash not in self._ignore_hashes:
      # Copy the image to <output_dir>/<md5Hash>.<image_extension>
      if not imgExt:
        raise ValueError("File %s does not have an extension" % outputImagePath)
      newFilePath = os.path.join(self._outputDir, md5Hash + '.' + imgExt)
      shutil.copy2(outputImagePath, newFilePath)

    # Add an entry to the list of test results
    self._results.append({
      "key": {
        "name": testName,
        "source_type": self._source_type,
      },
      "md5": md5Hash,
      "options": {
        "ext": imgExt,
        "gamma_correct": "no"
      }
    })

  def _parseKeyValuePairs(self, kvStr):
    kvPairs = shlex.split(kvStr)
    if len(kvPairs) % 2:
      raise ValueError("Uneven number of key/value pairs. Got %s" % kvStr)
    return { kvPairs[i]:kvPairs[i+1] for i in range(0, len(kvPairs), 2) }

  def WriteResults(self):
    self._properties.update({
      "results": self._results
    })

    outputFileName = os.path.join(self._outputDir, "dm.json")
    with open(outputFileName, 'wb') as outfile:
      json.dump(self._properties, outfile, indent=1)
      outfile.write("\n")

# Produce example output for manual testing.
if __name__ == "__main__":
  # Create a test directory with three empty 'image' files.
  testDir = "./testdirectory"
  if not os.path.exists(testDir):
    os.makedirs(testDir)
  open(os.path.join(testDir, "image1.png"), 'wb').close()
  open(os.path.join(testDir, "image2.png"), 'wb').close()
  open(os.path.join(testDir, "image3.png"), 'wb').close()

  # Create an instance and add results.
  propStr = """build_number 2 "builder name" Builder-Name gitHash a4a338179013b029d6dd55e737b5bd648a9fb68c"""

  keyStr = "arch arm64 compiler Clang configuration Debug"

  hash_file = os.path.join(testDir, "ignore_hashes.txt")
  with open(hash_file, 'wb') as f:
    f.write("\n".join(["hash-1","hash-4"]) + "\n")

  gr = GoldResults("pdfium", testDir, propStr, keyStr, hash_file)
  gr.AddTestResult("test-1", "hash-1", os.path.join(testDir, "image1.png"))
  gr.AddTestResult("test-2", "hash-2", os.path.join(testDir, "image2.png"))
  gr.AddTestResult("test-3", "hash-3", os.path.join(testDir, "image3.png"))
  gr.WriteResults()
