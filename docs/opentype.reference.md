
OpenType Field Reference
----------------------------------------------------------------


TrueType Collection Header
----------------------------------------------------------------

        TTC Header 1.0
        0   4   ttcTag
        4   2   majorVersion
        6   2   minorVersion
        8   4   numFonts
        12  4*  offsetTable[numFonts]


sfnt File Header
----------------------------------------------------------------
        Offset Table
        0   4   sfntVersion
        4   2   numTables
        6   2   searchRange
        8   2   entrySelector
        10  2   rangeShift


Table Record Entry
----------------------------------------------------------------
        0   4   tableTag
        4   4   checksum
        8   4   offset
        12  4   length


(HEAD) Font Header Table
----------------------------------------------------------------
        0   2   majorVersion
        2   2   minorVersion
        4   4   fontRevision
        8   4   checkSumAdjustment
        12  4   magicNumber (0x5F0F3CF5)
        16  2   flags
        18  2   unitsPerEm
        20  8   created
        28  8   modified
        36  2   xMin
        38  2   yMin
        40  2   xMax
        42  2   yMax
        44  2   macStyle
        46  2   lowestRecPPEM
        48  2   fontDirectionHint
        50  2   indexToLocFormat
        52  2   glyphDataFormat


(HHEA) Horizontal Header Table
----------------------------------------------------------------
        0   2   majorVersion
        2   2   minorVersion
        4   2   ascender
        6   2   descender
        8   2   lineGap
        10  2   advanceWidthMax
        12  2   minLeftSideBearing
        14  2   minRightSideBearing
        16  2   xMaxExtent
        18  2   caretSlopeRise
        20  2   caretSlopeRun
        22  2   caretOffset
        24  8   (reserved)
        32  2   metricDataFormat
        34  2   numberofHMetrics


(MAXP) Maximum Profile
----------------------------------------------------------------
        0   4   version
        4   2   numGlyphs
        6   2   maxPoints
        8   2   maxContours
        10  2   maxCompositePoints
        12  2   maxCompositeContours
        14  2   maxZones
        16  2   maxTwilightPoints
        18  2   maxStorage
        20  2   maxFunctionDefs
        22  2   maxInstructionDefs
        24  2   maxStackElements
        26  2   maxSizeOfInstructions
        28  2   maxComponentElements
        30  2   maxComponentDepth


(POST) Postscript Table
----------------------------------------------------------------
        0   4   version
        4   4   italicAngle
        8   2   underlinePosition
        10  2   underlineThickness
        12  4   isFixedPitch
        16  4   minMemType42
        20  4   maxMemType42
        24  4   minMemType1
        28  4   maxMemType1
        32  2   numGlyphs                   Version 2
        34  2*  glyphNameIndex[numGlyphs]   Version 2
        -   -   pascal strings              Version 2


(CMAP) Character to Glyph Index Mapping Table
----------------------------------------------------------------
        0   2   version
        2   2   numTables
        4   -   encodingRecrods[numTables]

        Encoding Records
        0   2   platformID  3=Windows
        2   2   encodingID  1=Unicode BMP
        4   4   offset

        Format 4:
        0   2   format
        2   2   length
        4   2   language
        6   2   segCountX2
        8   2   searchRange
        10  2   entrySelector
        12  2   rangeShift
        14  2*  endCode[segCount]
        -   2   reservedPad
        -   2*  startCode[segCount]
        -   2*  idDelta[segCount]
        -   2*  idRangeOffset[segCount]
        -   2   glyphIdArray[]


(NAME) Naming Table Format 0 (Format 1 appears to be rare.)
----------------------------------------------------------------
        0   2   format
        2   2   count
        4   2   stringOffset
        6   -   nameRecord[count]
        -   -   strings

        Name Record
        0   2   platformID  3=Windows
        2   2   encodingID  1=Unicode BMP
        4   2   languageID  0=Any 1033=en-US
        6   2   nameID
        8   2   length
        10  2   offset


(OS/2) OS/2 and Windows Metrics Table
----------------------------------------------------------------
        0   2   version (0-5)
        2   2   xAvgCharWidth
        4   2   usWeightClass
        6   2   usWidthClass
        8   2   fsType
        10  2   ySubscriptXSize
        12  2   ySubscriptYSize
        14  2   ySubscriptXOffset
        16  2   ySubscriptYOffset
        18  2   ySuperscriptXSize
        20  2   ySuperscriptYSize
        22  2   ySuperscriptXOffset
        24  2   ySuperscriptYOffset
        26  2   yStrikeoutSize
        28  2   yStrikeoutPosition
        30  2   sFamilyClass
        32  10  panose[10]
        42  4   ulUnicodeRange1
        46  4   ulUnicodeRange2
        50  4   ulUnicodeRange3
        54  4   ulUnicodeRange4
        58  4   achVendID
        62  2   fsSelection
        64  2   usFirstCharIndex
        66  2   usLastCharIndex
        68  2   sTypoAscender
        70  2   sTypoDescender
        72  2   sTypoLineGap
        74  2   usWinAascent
        76  2   usWinDescent
        78  4   ulCodePageRange1        Version 1
        82  4   ulCodePageRange2        Version 1
        86  2   sxHeight                Version 2
        88  2   sCapHeight              Version 2
        90  2   usDefaultChar           Version 2
        92  2   usBreakChar             Version 2
        94  2   usMaxContext            Version 2
        96  2   usLowerOpticalPointSize Version 5
        98  2   usUpperOpticalPointSize Version 5


(GLYF) Glyph Headers
----------------------------------------------------------------
        0   2   numberOfControus
        2   2   xMin
        4   2   yMin
        6   2   xMax
        8   2   yMax

        Simple Glyph Description
        0   2*  endPtsOfContours[numberOfContours]
        -   2   instructionLength
        -   1*  instructions[instructionLength]
        -   1   flags[...]
        -   1/2 xCoordinates[...]
        -   1/2 yCoordinates[...]

                Flags
                1   On-Curve
                2   x-Short Vector
                4   y-Short Vector
                8   Repeat
                16  This x is same (Positive x-Short vector)
                32  This y is same (Positive y-Short vector)

        Composite Glyph Table
        0   2   flags
        2   2   glyphIndex
        -   -   arg1
        -   -   arg2

                Flags
                0x0001  ARG_1_AND_2_ARE_WORDS
                0x0002  ARGS_ARE_XY_VALUES
                0x0004  ROUND_XY_TO_GRID
                0x0008  WE_HAVE_A_SCALE
                0x0020  MORE_COMPONENTS
                0x0040  WE_HAVE_AN_X_AND_Y_SCALE
                0x0080  WE_HAVE_A_TWO_BY_TWO
                0x0100  WE_HAVE_INSTRUCTIONS
                0x0200  USE_MY_METRICS
                0x0400  OVERLAP_COMPOUND
                0x0800  SCALED_COMPONENT_OFFSET
                0x1000  UNSCALED_COMPONENT_OFFSET
