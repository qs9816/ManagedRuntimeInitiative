/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */

/*
 *
 * (C) Copyright IBM Corp. 1998-2004 - All Rights Reserved
 *
 */

#include "LETypes.h"
#include "MorphTables.h"
#include "SubtableProcessor.h"
#include "NonContextualGlyphSubst.h"
#include "NonContextualGlyphSubstProc.h"
#include "SegmentArrayProcessor.h"
#include "LEGlyphStorage.h"
#include "LESwaps.h"

SegmentArrayProcessor::SegmentArrayProcessor()
{
}

SegmentArrayProcessor::SegmentArrayProcessor(const MorphSubtableHeader *morphSubtableHeader)
  : NonContextualGlyphSubstitutionProcessor(morphSubtableHeader)
{
    const NonContextualGlyphSubstitutionHeader *header = (const NonContextualGlyphSubstitutionHeader *) morphSubtableHeader;

    segmentArrayLookupTable = (const SegmentArrayLookupTable *) &header->table;
}

SegmentArrayProcessor::~SegmentArrayProcessor()
{
}

void SegmentArrayProcessor::process(LEGlyphStorage &glyphStorage)
{
    const LookupSegment *segments = segmentArrayLookupTable->segments;
    le_int32 glyphCount = glyphStorage.getGlyphCount();
    le_int32 glyph;

    for (glyph = 0; glyph < glyphCount; glyph += 1) {
        LEGlyphID thisGlyph = glyphStorage[glyph];
        const LookupSegment *lookupSegment = segmentArrayLookupTable->lookupSegment(segments, thisGlyph);

        if (lookupSegment != NULL)  {
            TTGlyphID firstGlyph = SWAPW(lookupSegment->firstGlyph);
            le_int16  offset = SWAPW(lookupSegment->value);

            if (offset != 0) {
                TTGlyphID  *glyphArray = (TTGlyphID *) ((char *) subtableHeader + offset);
                TTGlyphID   newGlyph   = (TTGlyphID)SWAPW(glyphArray[LE_GET_GLYPH(thisGlyph) - firstGlyph]);

                glyphStorage[glyph] = LE_SET_GLYPH(thisGlyph, newGlyph);
            }
        }
    }
}
