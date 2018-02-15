Name: FreeType
URL: http://www.freetype.org/
Version: VER-2-9-20
Revision: 4a03f17449ae45f0dacf4de4694ccd6e5e1b24d1
Security Critical: yes
License: FreeType License (FTL)
License File: FTL.TXT

Description:
FreeType library.

Local Modifications:

include/pstables.h: A copy of freetype/src/psnames/pstables.h.  This file is not
part of the public Freetype API, but pdfium needs it.  Since it won't be
avilable when building with the system Freetype, we provide it as a convenience.

0000-include.patch: Modifications to configuration header files.

Most of the updating can be automated will roll-freetype.sh.
