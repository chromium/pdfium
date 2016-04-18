# PDFium

## News

As of 2016-02-04, the XFA branch is deprecated.  Instead, see
[the section on configuration](#BuildConfig) below.

As of 2016-04-15, Visual Studio toolchain from depot_tools will be used as the
default. Set DEPOT_TOOLS_WIN_TOOLCHAIN=0 if you need to use system toolchain.
See [Windows development subsection](#WinDev) for details.


## Prerequisites

Get the chromium depot tools via the instructions at
http://www.chromium.org/developers/how-tos/install-depot-tools (this provides
the gclient utilty needed below).

Also install Python, Subversion, and Git and make sure they're in your path.

###<a name="WinDev"></a> Windows development

PDFium uses similar Windows toolchain as Chromium:

#### Open source contributors:
Visual Studio 2015 Update 2 or later is highly recommended.

Run `set DEPOT_TOOLS_WIN_TOOLCHAIN=0`, or set that variable in your global
environment.

Compilation is done through ninja, **not** Visual Studio.

#### Google employees

Run: `download_from_google_storage --config` and follow the
authentication instructions. **Note that you must authenticate with your
@google.com credentials**. Enter "0" if asked for a project-id.

Once you've done this, the toolchain will be installed automatically for
you in Step 3, below (near the end of the step).

The toolchain will be in `depot_tools\win_toolchain`, and windbg can be
found in `depot_tools\win_toolchain\vs2013_files\win8sdk\Debuggers`.

If you want the IDE for debugging and editing, you will need to install
it separately, but this is optional and not needed to build PDFium.


## Get the code

The name of the top-level directory does not matter. In our examples, we use
"repo". This directory must not have been used before by `gclient config` as
each directory can only house a single gclient configuration.

```
mkdir repo
cd repo
gclient config --unmanaged https://pdfium.googlesource.com/pdfium.git
gclient sync
cd pdfium
```

## Generate the build files

We use the GYP library to generate the build files.

At this point, you have two options. The first option is to use the [Ninja]
(http://martine.github.io/ninja/) build system (also included with the
depot\_tools checkout). This is the default as of mid-September, 2015.
Previously, the second option (platform-specific build files) was the default.
Most PDFium developers use Ninja, as does our [continuous build system]
(http://build.chromium.org/p/client.pdfium/).

 * On Windows: `build\gyp_pdfium`
 * For all other platforms: `build/gyp_pdfium`

The second option is to generate platform-specific build files, i.e. Makefiles
on Linux, sln files on Windows, and xcodeproj files on Mac. To do so, set the
GYP\_GENERATORS environment variable appropriately (e.g. "make", "msvs", or
"xcode") before running the above command.

###<a name="BuildConfig"></a> Selecting build configuration

PDFium may be built either with or without JavaScript support, and with
or without XFA forms support.  Both of these features are enabled by
default. Also note that the XFA feature requires JavaScript.

To build without XFA, set `pdf_enable_xfa=0` before running `gyp_pdfium`.
To build without JavaScript, set `pdf_enable_v8=0 pdf_enable_xfa=0` before
running `gyp_pdfium`. For example
```
GYP_DEFINES='pdf_enable_v8=0 pdf_enable_xfa=0' build/gyp_pdfium
```
gives the smallest possible build configuration.

### Using goma (Googlers only)

If you would like to build using goma, pass `use_goma=1` to `gyp_pdfium`. If
you installed goma in a non-standard location, you will also need to set
`gomadir`. e.g.

```
build/gyp_pdfium -D use_goma=1 -D gomadir=path/to/goma
```

## Building the code

If you used Ninja, you can build the sample program by: `ninja -C out/Debug
pdfium_test` You can build the entire product (which includes a few unit
tests) by: `ninja -C out/Debug`.

If you're not using Ninja, then building is platform-specific.

 * On Linux: `make pdfium_test`
 * On Mac: `open build/all.xcodeproj`
 * On Windows: open build\all.sln

## Running the sample program

The pdfium\_test program supports reading, parsing, and rasterizing the pages of
a .pdf file to .ppm or .png output image files (windows supports two other
formats). For example: `out/Debug/pdfium_test --ppm path/to/myfile.pdf`. Note
that this will write output images to `path/to/myfile.pdf.<n>.ppm`.

## Testing

There are currently several test suites that can be run:

 * pdfium\_unittests
 * pdfium\_embeddertests
 * testing/tools/run\_corpus\_tests.py
 * testing/tools/run\_javascript\_tests.py
 * testing/tools/run\_pixel\_tests.py

It is possible the tests in the `testing` directory can fail due to font
differences on the various platforms. These tests are reliable on the bots. If
you see failures, it can be a good idea to run the tests on the tip-of-tree
checkout to see if the same failures appear.

## Waterfall

The current health of the source tree can be found at
http://build.chromium.org/p/client.pdfium/console

## Community

There are several mailing lists that are setup:

 * [PDFium](https://groups.google.com/forum/#!forum/pdfium)
 * [PDFium Reviews](https://groups.google.com/forum/#!forum/pdfium-reviews)
 * [PDFium Bugs](https://groups.google.com/forum/#!forum/pdfium-bugs)

Note, the Reviews and Bugs lists are typically read-only.

## Bugs

 We will be using this
[bug tracker](https://code.google.com/p/pdfium/issues/list), but for security
bugs, please use [Chromium's security bug template]
(https://code.google.com/p/chromium/issues/entry?template=Security%20Bug)
and add the "Cr-Internals-Plugins-PDF" label.

## Contributing code

For contributing code, we will follow
[Chromium's process](http://dev.chromium.org/developers/contributing-code)
as much as possible. The main exceptions is:

1. Code has to conform to the existing style and not Chromium/Google style.

## Branches

Prior to 2016-02-04, there existed an actively developed origin/xfa branch.
The origin/xfa branch is now an evolutionary dead-end. Everything you need
to build either with or without the XFA feature is on origin/master.

