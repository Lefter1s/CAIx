#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/CAIx.ico

convert ../../src/qt/res/icons/CAIx-16.png ../../src/qt/res/icons/CAIx-32.png ../../src/qt/res/icons/CAIx-48.png ${ICON_DST}
