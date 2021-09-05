# SPDX-License-Identifier: BSD-3-Clause

src-y += base/ftinit.c
src-y += base/ftbase.c
src-y += base/ftbbox.c
src-y += base/ftbitmap.c
src-y += base/ftdebug.c
src-y += base/ftsystem.c
src-y += base/ftglyph.c
src-y += sfnt/sfnt.c
src-y += truetype/truetype.c
src-y += raster/raster.c
src-y += smooth/smooth.c
src-y += psnames/psnames.c
src-y += pshinter/pshinter

MODULE_CFLAGS += -DFT2_BUILD_LIBRARY -Wno-self-assign -Wno-sign-compare
MODULE_CFLAGS += -DFT_CONFIG_STANDARD_LIBRARY_H="<freetype/sc/config/ftstdlib.h>"
MODULE_CFLAGS += -DFT_CONFIG_OPTIONS_H="<freetype/sc/config/ftoption.h>"
MODULE_CFLAGS += -DFT_CONFIG_MODULES_H="<freetype/sc/config/ftmodule.h>"
