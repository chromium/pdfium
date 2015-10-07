# Building PDFium

## Prerequisites

Get the chromium depot tools via the instructions at
http://www.chromium.org/developers/how-tos/install-depot-tools (this provides
the gclient utilty needed below).

Also install Python, Subversion, and Git and make sure they're in your path.

Optionally, you may want to install the [Ninja](http://martine.github.io/ninja/)
build system (recommended) rather than using your platform-specific build
system.

## Get the code

```
mkdir pdfium
cd pdfium
gclient config --name . --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
```

## Generate the build files

Now we use the GYP library to generate the build files.

At this point, you have two options. The first option is to use the [Ninja]
(http://martine.github.io/ninja/) build system. This is the default as of
mid-September, 2015. Previously, the second option was the default. Most PDFium
developers use Ninja, as does our [continuous build system]
(http://build.chromium.org/p/client.pdfium/).

On Windows: `build\gyp_pdfium
` For all other platforms: `build/gyp_pdfium
`

The second option is to generate platform-specific build files, i.e. Makefiles
on Linux, sln files on Windows, and xcodeproj files on Mac. To do so, set the
GYP\_GENERATORS environment variable appropriately (e.g. "make", "msvs", or
"xcode") before running the above command.

## Building the code

If you used Ninja, you can build the sample program by: `ninja -C out/Debug
pdfium_test
` You can build the entire product (which includes a few unit tests) by: `ninja
-C out/Debug
`

If you're not using Ninja, then building is platform-specific.

On Linux: `make pdfium_test
`

On Mac, open build/all.xcodeproj

On Windows, open build\all.sln

## Running the sample program

The pdfium\_test program supports reading, parsing, and rasterizing the pages of
a .pdf file to .ppm output image files (windows supports two other formats, and
.png support is available for all platforms in an alternate branch (see branches
section below)). For example: `out/Debug/pdfium_test --ppm path/to/myfile.pdf
`

## Waterfall

The current health of the source tree can be found at
http://build.chromium.org/p/client.pdfium/console

## Branches

There is a branch for a forthcoming feature called XFA that you can get by
following the steps above, then: `git checkout origin/xfa build/gyp_pdfium ninja
-C out/Debug
`

The XFA version of the sample pdfium\_test program supports rasterizing to .png
format files. For example: `out/Debug/pdfium_test --png path/to/myfile.pdf
`
