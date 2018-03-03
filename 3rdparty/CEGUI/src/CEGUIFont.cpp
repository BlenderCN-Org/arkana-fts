/***********************************************************************
    filename: 	CEGUIFont.cpp
    created:	21/2/2004
    author:		Paul D Turner

    purpose:	Implements Font class
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2006 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/

#include "CEGUIFont.h"
#include "CEGUIExceptions.h"
#include "CEGUISystem.h"
#include "CEGUITextUtils.h"
#include "CEGUIXMLAttributes.h"
#include "CEGUIPropertyHelper.h"
#include <algorithm>
#include <stdio.h>

namespace CEGUI
{

// amount of bits in a uint
#define BITS_PER_UINT   (sizeof (uint) * 8)
// must be a power of two
#define GLYPHS_PER_PAGE 256

static const String FontNameAttribute ("Name");
static const String FontFilenameAttribute ("Filename");
static const String FontResourceGroupAttribute ("ResourceGroup");
static const String FontAutoScaledAttribute ("AutoScaled");
static const String FontNativeHorzResAttribute ("NativeHorzRes");
static const String FontNativeVertResAttribute ("NativeVertRes");

const argb_t Font::DefaultColour = 0xFFFFFFFF;
String Font::d_defaultResourceGroup;

BaseFontTrigger::BaseFontTrigger(const String &in_sText)
    : d_sTriggerText(in_sText)
    , d_eatenTextLen(0)
{
    d_minTrigLen = d_sTriggerText.length();
    // Replace the length of the special signs with the length of what they have
    // to be at least.
    for(size_t i = 0 ; i < d_sTriggerText.length() ; ++i) {
        if(d_sTriggerText[i] == '%') {
            i++;
            switch(d_sTriggerText[i]) {
            case 'c':
                // %c means at least one hex digit, so 2 for %c is replaced by 1.
                d_minTrigLen--;
                break;
            default:
                break;
            }
        }
    }
}

BaseFontTrigger::~BaseFontTrigger()
{
}

size_t BaseFontTrigger::countCharsOccupiedByMe(const String &in_sText, size_t in_startPos, size_t in_nLen)
{
    size_t charsOccupiedByMe = 0;
    size_t sCandidatePos = in_startPos; // Just a pointer to move around.

    if(in_nLen == String::npos)
        in_nLen = in_sText.length() - in_startPos;

    while(sCandidatePos < in_startPos + in_nLen) {
        sCandidatePos = in_sText.find(d_sTriggerText[0], sCandidatePos);
        if(sCandidatePos == String::npos)
            break;

        // Check if that is a trigger.
        size_t sTrigLen = this->readTrigger(in_sText, sCandidatePos);
        if(sTrigLen == 0) {
            // No, it wasn't, search for the next one.
            sCandidatePos++;
        } else {
            // Yes, it was, count it.
            charsOccupiedByMe += sTrigLen;
        }
    }

    return charsOccupiedByMe;
}

String BaseFontTrigger::removeAllOccurences(const String &in_sText, size_t in_startPos)
{
    String sNewString = in_sText;

    size_t sCandidatePos = in_startPos; // Just a pointer to move around.

    while(sCandidatePos < sNewString.length()) {
        sCandidatePos = sNewString.find(d_sTriggerText[0], sCandidatePos);
        if(sCandidatePos == String::npos)
            break;

        // Check if that is a trigger.
        size_t sTrigLen = this->readTrigger(sNewString, sCandidatePos);
        if(sTrigLen == 0) {
            // No, it wasn't, search for the next one.
            sCandidatePos++;
        } else {
            // Yes, it was, remove it.
            sNewString.erase(sCandidatePos, sTrigLen);
        }
    }

    return sNewString;
}

size_t BaseFontTrigger::readTrigger(const String &in_sText, size_t in_startPos)
{
    size_t s = in_startPos, i = 0; // Just a pointer to move around.
    d_eatenTextLen = 0;

    d_lCols.clear(); // Empty the list from previous read attempts.

    // First, we check if there is still enough place in the text:
    if(in_startPos + d_minTrigLen > in_sText.length())
        return 0;

    // Check that all characters up to the first formatting character fit.
    // This is to fast-eliminate non-triggers.
    for(s = in_startPos, i = 0 ; s < in_sText.length() &&
                                 i < d_sTriggerText.length() ; ++s, ++i) {
        // Check for special inputs.
        if(d_sTriggerText[i] == '%') {
            i++;
            switch(d_sTriggerText[i]) {
            case 'c':
            {
                // Read a 24-bit hex color-code out of the text and store it in the list.
                utf8 buf[7] = {0};
                argb_t argb = 0;

                // How many hex colors do we have?
                size_t nHexs = 0;
                for(nHexs = 0 ; (nHexs < 6) && (in_sText[s + nHexs] != '\0') && (isdigit(in_sText[s + nHexs]) || (in_sText[s + nHexs] >= 'A' && in_sText[s + nHexs] <= 'F')) ; nHexs++)
                    ;

                // None? this is no trigger then!
                if(nHexs == 0) {
                    return 0;
                }

                switch(nHexs) {
                case 3:
                    buf[1] = buf[3] = buf[5] = '0';
                    in_sText.copy(&buf[0], 1, s);
                    in_sText.copy(&buf[2], 1, s+1);
                    in_sText.copy(&buf[4], 1, s+2);
                    sscanf(reinterpret_cast<const char *>(buf), "%x", &argb);
                    break;
                case 1:
                {
                    unsigned int n = 0;
                    in_sText.copy(buf, 1, s);
                    sscanf(reinterpret_cast<const char *>(buf), "%x", &n);
                    switch(n) {
                    case 0: argb = 0x00000000; break;
                    case 2: argb = 0x00000066; break;
                    case 3: argb = 0x00009900; break;
                    case 4: argb = 0x00FF0000; break;
                    case 5: argb = 0x00660000; break;
                    case 6: argb = 0x00990099; break;
                    case 7: argb = 0x00FF6600; break;
                    case 8: argb = 0x00FFFF00; break;
                    case 9: argb = 0x0000FF00; break;
                    case 10: argb = 0x00006699; break;
                    case 11: argb = 0x0000FFFF; break;
                    case 12: argb = 0x000000FF; break;
                    case 13: argb = 0x00FF00FF; break;
                    case 14: argb = 0x00666666; break;
                    case 15: argb = 0x00CCCCCC; break;
                    case 1:
                    default: argb = 0x00FFFFFF; break;
                    }
                    break;
                }
                default:
                    in_sText.copy(buf, nHexs, s);
                    sscanf(reinterpret_cast<const char *>(buf), "%x", &argb);
                    break;
                }

                // Set alpha to full, no transparency.
                /// \todo: set alpha to the window's transparency??
                argb += 0xFF000000;
                d_lCols.push_back(argb); // Add to the list of colors.

                // Go behind the hex number.
                s += nHexs - 1;
                i++;
                continue;
            }
            default:
                break;
            }
        }

        if(in_sText[s] != d_sTriggerText[i])
            return 0;
    }

    d_eatenTextLen = s - in_startPos;
    return d_eatenTextLen;
}

ColourStartFontTrigger::ColourStartFontTrigger(const String &in_sTriggerString)
    : BaseFontTrigger(in_sTriggerString)
{
}

ColourStartFontTrigger::~ColourStartFontTrigger()
{
}

void ColourStartFontTrigger::doTriggerAction(ColourRect &out_curColour, const ColourRect &in_defColour)
{
    // Use only the first color read.
    argb_t col = d_lCols.front();

    // Get the original text's alpha value.
    uint8 a = (uint8)(((double)in_defColour.d_top_left.getAlpha() +
                       (double)in_defColour.d_top_right.getAlpha() +
                       (double)in_defColour.d_bottom_left.getAlpha() +
                       (double)in_defColour.d_bottom_right.getAlpha()
                      )/4.0*255.0);

    col -= 0xFF000000;
    col += (uint32)a*0x01000000; // And use it for our colour.

    out_curColour.setColours(col);
}

ColourStopFontTrigger::ColourStopFontTrigger(const String &in_sTriggerString)
    : BaseFontTrigger(in_sTriggerString)
{
}

ColourStopFontTrigger::~ColourStopFontTrigger()
{
}

void ColourStopFontTrigger::doTriggerAction(ColourRect &out_curColour, const ColourRect &in_defColour)
{
    out_curColour = in_defColour;
}

Font::Font (const String& name, const String& fontname, const String& resourceGroup) :
    d_name (name),
    d_fileName (fontname),
    d_resourceGroup (resourceGroup),
    d_ascender (0),
    d_descender (0),
    d_height (0),
    d_autoScale (false),
    d_horzScaling (1.0f),
    d_vertScaling (1.0f),
    d_nativeHorzRes (DefaultNativeHorzRes),
    d_nativeVertRes (DefaultNativeVertRes),
    d_maxCodepoint (0),
    d_glyphPageLoaded (0),
    d_triggersEnabled(false),
    d_tabWidth(80),
    d_tabEnabled(false)
{
    addFontProperties ();
}


Font::Font (const XMLAttributes& attributes) :
    d_name (attributes.getValueAsString (FontNameAttribute)),
    d_fileName (attributes.getValueAsString (FontFilenameAttribute)),
    d_resourceGroup (attributes.getValueAsString (FontResourceGroupAttribute)),
    d_ascender (0),
    d_descender (0),
    d_height (0),
    d_autoScale (attributes.getValueAsBool(FontAutoScaledAttribute, false)),
    d_nativeHorzRes (attributes.getValueAsInteger (FontNativeHorzResAttribute, int (DefaultNativeHorzRes))),
    d_nativeVertRes (attributes.getValueAsInteger (FontNativeVertResAttribute, int (DefaultNativeVertRes))),
    d_maxCodepoint (0),
    d_glyphPageLoaded (0),
    d_triggersEnabled(false),
    d_tabWidth(0),
    d_tabEnabled(false)
{
    addFontProperties ();

    Size size = System::getSingleton ().getRenderer()->getSize ();
    d_horzScaling = size.d_width / d_nativeHorzRes;
    d_vertScaling = size.d_height / d_nativeVertRes;
}


/*************************************************************************
    Destroys a Font object
*************************************************************************/
Font::~Font(void)
{
    delete [] d_glyphPageLoaded;

    // Remove all triggers from the registered triggers list.
    while(!d_lpTriggers.empty()) {
        delete d_lpTriggers.front();
        d_lpTriggers.pop_front();
    }
}


/*************************************************************************
    Define a glyph mapping (handle a <Mapping /> XML element)
*************************************************************************/
void Font::defineMapping (const XMLAttributes& attributes)
{
    throw FileIOException("Font::defineMapping - The <Mapping> XML element is not supported for this font type");
}


/*************************************************************************
    Set the maximal glyph index. This reserves the respective
    number of bits in the d_glyphPageLoaded array.
*************************************************************************/
void Font::setMaxCodepoint (utf32 codepoint)
{
    d_maxCodepoint = codepoint;

    delete [] d_glyphPageLoaded;

    uint npages = (codepoint + GLYPHS_PER_PAGE) / GLYPHS_PER_PAGE;
    uint size = (npages + BITS_PER_UINT - 1) / BITS_PER_UINT;
    d_glyphPageLoaded = new uint [size];
    memset (d_glyphPageLoaded, 0, size * sizeof (uint));
}

/*************************************************************************
    Checks if the string text contains a special trigger at position pos.
    If it does, returns the length (in characters) of the trigger, if it
    doesn't, returns 0.
*************************************************************************/
size_t Font::hasTriggerHere(const String &text, size_t pos)
{
    if(!d_triggersEnabled)
        return 0;

    size_t totalTrigLenFound = 0;

    // Check all triggers:
    for(std::list<BaseFontTrigger *>::iterator i = d_lpTriggers.begin() ; i != d_lpTriggers.end() ; ++i) {
        totalTrigLenFound += (*i)->readTrigger(text, pos + totalTrigLenFound);
    }
    return totalTrigLenFound;
}

/*************************************************************************
    Checks if the string text contains a special trigger at position pos.
    If it does, returns an object that can do an action with this trigger.
    The returned object doesn't have to be deleted!
*************************************************************************/
BaseFontTrigger *Font::getTriggerHere(const String &text, size_t pos)
{
    if(!d_triggersEnabled)
        return NULL;

    // Check all triggers:
    for(std::list<BaseFontTrigger *>::iterator i = d_lpTriggers.begin() ; i != d_lpTriggers.end() ; ++i) {
        if((*i)->readTrigger(text, pos) > 0)
            return *i;
    }
    return NULL;
}

/*************************************************************************
    Creates a new string that is exactly the same as \a in_sText, except
    that, beginning from position \a in_startPos, all triggers have been
    removed. This is the text as it will be displayed.
*************************************************************************/
String Font::removeAllTriggers(const String &in_sText, size_t in_startPos)
{
    if(!d_triggersEnabled)
        return in_sText;

    // Remove all triggers:
    String sFinal = in_sText;
    for(std::list<BaseFontTrigger *>::iterator i = d_lpTriggers.begin() ; i != d_lpTriggers.end() ; ++i) {
        sFinal = (*i)->removeAllOccurences(sFinal, in_startPos);
    }
    return sFinal;
}

/*************************************************************************
    Returns the amount of characters in the string, beginning from in_startPos,
    that are occupied by a trigger and thus would be removed by the method
    \a removeAllTriggers.
*************************************************************************/
size_t Font::charsOccupiedByTriggers(const String &in_sText, size_t in_startPos, size_t in_nLen)
{
    if(!d_triggersEnabled)
        return 0;

    // Count all triggers' chars:
    size_t nChars = 0;
    for(std::list<BaseFontTrigger *>::iterator i = d_lpTriggers.begin() ; i != d_lpTriggers.end() ; ++i) {
        nChars += (*i)->countCharsOccupiedByMe(in_sText, in_startPos, in_nLen);
    }
    return nChars;
}

/*************************************************************************
    Return a pointer to the glyphDat struct for the given codepoint,
    or 0 if the codepoint does not have a glyph defined.
*************************************************************************/
const FontGlyph *Font::getGlyphData (utf32 codepoint)
{
    if (codepoint > d_maxCodepoint)
        return 0;

    if (d_glyphPageLoaded)
    {
        // Check if glyph page has been rasterized
        uint page = codepoint / GLYPHS_PER_PAGE;
        uint mask = 1 << (page & (BITS_PER_UINT - 1));
        if (!(d_glyphPageLoaded [page / BITS_PER_UINT] & mask))
        {
            d_glyphPageLoaded [page / BITS_PER_UINT] |= mask;
            rasterize (codepoint & ~(GLYPHS_PER_PAGE - 1),
                       codepoint | (GLYPHS_PER_PAGE - 1));
        }
    }

    CodepointMap::const_iterator pos = d_cp_map.find (codepoint);
    return (pos != d_cp_map.end()) ? &pos->second : 0;
}


/*************************************************************************
    Return the pixel width of the specified text if rendered with this Font.
*************************************************************************/
float Font::getTextExtent(const String& text, float x_scale)
{
    // If needed, we just operate on the text w/o the triggers.
    String sNoTrigText = this->removeAllTriggers(text, 0);

    const FontGlyph* glyph;
    float cur_extent = 0, adv_extent = 0, width;
    size_t textlen = sNoTrigText.length();

    for (size_t c = 0; c < textlen;)
    {
        glyph = getGlyphData(sNoTrigText[c]);

        if (glyph)
        {

            if (d_tabEnabled && sNoTrigText[c] == '\t')
            {
                // Calculate the width of the tab.
                // Not sure about the difference of cur_extent and adv_extent.
                // I need the current position here.
                width = d_tabWidth - (((size_t)PixelAligned(adv_extent)) % d_tabWidth);
            } else {
                width = glyph->getRenderedAdvance(x_scale);
            }

            if (adv_extent + width > cur_extent)
                cur_extent = adv_extent + width;

            adv_extent += glyph->getAdvance(x_scale);
        }

        ++c;
    }

    return ceguimax(adv_extent, cur_extent);
}


/*************************************************************************
    Return the index of the closest text character in String 'text' that
    corresponds to pixel location 'pixel' if the text were rendered.
*************************************************************************/
size_t Font::getCharAtPixel(const String& text, size_t start_char, float pixel, float x_scale)
{
    // If needed, we just operate on the text w/o the triggers.
    String sNoTrigText = this->removeAllTriggers(text, 0);

    const FontGlyph* glyph;
    float cur_extent = 0;
    size_t char_count = sNoTrigText.length();

    // handle simple cases
    if ((pixel <= 0) || (char_count <= start_char))
        return start_char;

    for (size_t c = start_char; c < char_count;)
    {
        glyph = getGlyphData(sNoTrigText[c]);

        if (glyph)
        {
            cur_extent += glyph->getAdvance(x_scale);

            if (pixel < cur_extent)
                return c;
        }

         ++c;
    }

    return char_count;
}


/*************************************************************************
    Return the number of lines the given text would be formatted to.
*************************************************************************/
size_t Font::getFormattedLineCount(const String& text, const Rect& format_area, TextFormatting fmt, float x_scale)
{
    // handle simple non-wrapped cases.
    if ((fmt == LeftAligned) || (fmt == Centred) || (fmt == RightAligned) || (fmt == Justified))
    {
        return std::count(text.begin(), text.end(), static_cast<utf8>('\n')) + 1;
    }

    // handle wraping cases
    size_t lineStart = 0, lineEnd = 0;
    String	sourceLine;

    float	wrap_width = format_area.getWidth();
    String  whitespace = TextUtils::DefaultWhitespace;
    String	thisWord;
    size_t	line_count = 0, currpos = 0;
    float lineWidth, wordWidth;

    // If needed, we just operate on the text w/o the triggers.
    String sNoTrigText = this->removeAllTriggers(text, 0);

    while (lineEnd < sNoTrigText.length())
    {
        if ((lineEnd = sNoTrigText.find_first_of('\n', lineStart)) == String::npos)
        {
            lineEnd = sNoTrigText.length();
        }

        sourceLine = sNoTrigText.substr(lineStart, lineEnd - lineStart);
        lineStart = lineEnd + 1;

        // get first word.
        currpos = getNextWord(sourceLine, 0, thisWord);
        lineWidth = getTextExtent(thisWord);

        // while there are words left in the string...
        while (String::npos != sourceLine.find_first_not_of(whitespace, currpos))
        {
            // get next word of the string...
            currpos += getNextWord(sourceLine, currpos, thisWord);
            wordWidth = getTextExtent(thisWord, x_scale);

            // if the new word would make the string too long
            if ((lineWidth + wordWidth) > wrap_width)
            {
                // too long, so that's another line of text
                line_count++;

                // remove whitespace from next word - it will form start of next line
                thisWord = thisWord.substr(thisWord.find_first_not_of(whitespace));
                wordWidth = getTextExtent(thisWord, x_scale);

                // reset for a new line.
                lineWidth = 0;
            }

            // add the next word to the line
            lineWidth += wordWidth;
        }

        // plus one for final line
        line_count++;
    }

    return line_count;
}


/*************************************************************************
    Renders text on the display.  Return number of lines output.
*************************************************************************/
size_t Font::drawText(const String& text, const Rect& draw_area, float z, const Rect& clip_rect, TextFormatting fmt, const ColourRect& colours, float x_scale, float y_scale, bool in_bInternal)
{
    size_t thisCount;
    size_t lineCount = 0;

    float	y_base = draw_area.d_top + getBaseline(y_scale);

    Rect tmpDrawArea(
        PixelAligned(draw_area.d_left),
        PixelAligned(draw_area.d_top),
        PixelAligned(draw_area.d_right),
        PixelAligned(draw_area.d_bottom)
    );

    size_t lineStart = 0, lineEnd = 0;
    String	currLine;

    while (lineEnd < text.length())
    {
        if ((lineEnd = text.find_first_of('\n', lineStart)) == String::npos)
            lineEnd = text.length();

        currLine = text.substr(lineStart, lineEnd - lineStart);
        lineStart = lineEnd + 1;	// +1 to skip \n char

        switch(fmt)
        {
        case LeftAligned:
            drawTextLine(currLine, Vector3(tmpDrawArea.d_left, y_base, z), clip_rect, colours, x_scale, y_scale, in_bInternal);
            thisCount = 1;
            y_base += getLineSpacing(y_scale);
            break;

        case RightAligned:
            drawTextLine(currLine, Vector3(tmpDrawArea.d_right - getTextExtent(currLine, x_scale), y_base, z), clip_rect, colours, x_scale, y_scale, in_bInternal);
            thisCount = 1;
            y_base += getLineSpacing(y_scale);
            break;

        case Centred:
            drawTextLine(currLine, Vector3(PixelAligned(tmpDrawArea.d_left + ((tmpDrawArea.getWidth() - getTextExtent(currLine, x_scale)) / 2.0f)), y_base, z), clip_rect, colours, x_scale, y_scale, in_bInternal);
            thisCount = 1;
            y_base += getLineSpacing(y_scale);
            break;

        case Justified:
            // new function in order to keep drawTextLine's signature unchanged
            drawTextLineJustified(currLine, draw_area, Vector3(tmpDrawArea.d_left, y_base, z), clip_rect, colours, x_scale, y_scale, in_bInternal);
            thisCount = 1;
            y_base += getLineSpacing(y_scale);
            break;

        case WordWrapLeftAligned:
            thisCount = drawWrappedText(currLine, tmpDrawArea, z, clip_rect, LeftAligned, colours, x_scale, y_scale);
            tmpDrawArea.d_top += thisCount * getLineSpacing(y_scale);
            break;

        case WordWrapRightAligned:
            thisCount = drawWrappedText(currLine, tmpDrawArea, z, clip_rect, RightAligned, colours, x_scale, y_scale);
            tmpDrawArea.d_top += thisCount * getLineSpacing(y_scale);
            break;

        case WordWrapCentred:
            thisCount = drawWrappedText(currLine, tmpDrawArea, z, clip_rect, Centred, colours, x_scale, y_scale);
            tmpDrawArea.d_top += thisCount * getLineSpacing(y_scale);
            break;

        case WordWrapJustified:
            // no change needed
            thisCount = drawWrappedText(currLine, tmpDrawArea, z, clip_rect, Justified, colours, x_scale, y_scale);
            tmpDrawArea.d_top += thisCount * getLineSpacing(y_scale);
            break;

        default:
            throw InvalidRequestException("Font::drawText - Unknown or unsupported TextFormatting value specified.");
        }

        lineCount += thisCount;

    }

    // should not return 0
    return ceguimax(lineCount, (size_t)1);
}


/*************************************************************************
    helper function for renderWrappedText to get next word of a string
*************************************************************************/
size_t Font::getNextWord(const String& in_string, size_t start_idx, String& out_string) const
{
    out_string = TextUtils::getNextWord(in_string, start_idx, TextUtils::DefaultWrapDelimiters);

    return out_string.length();
}


/*************************************************************************
    draws wrapped text
*************************************************************************/
size_t Font::drawWrappedText(const String& text, const Rect& draw_area, float z, const Rect& clip_rect, TextFormatting fmt, const ColourRect& colours, float x_scale, float y_scale)
{
    size_t	line_count = 0;
    Rect	dest_area(draw_area);
    float	wrap_width = draw_area.getWidth();

    String  whitespace = TextUtils::DefaultWhitespace;
    String	thisLine, thisWord;
    size_t	currpos = 0;

    d_origColour = colours;
    d_curColour = colours;

    // get first word.
    currpos += getNextWord(text, currpos, thisLine);

    // while there are words left in the string...
    while (String::npos != text.find_first_not_of(whitespace, currpos))
    {
        // get next word of the string...
        currpos += getNextWord(text, currpos, thisWord);

        // if the new word would make the string too long
        if ((getTextExtent(thisLine, x_scale) + getTextExtent(thisWord, x_scale)) > wrap_width)
        {
            // output what we had until this new word
            line_count += drawText(thisLine, dest_area, z, clip_rect, fmt, colours, x_scale, y_scale, true);

            // remove whitespace from next word - it will form start of next line
            thisWord = thisWord.substr(thisWord.find_first_not_of(whitespace));

            // reset for a new line.
            thisLine.clear();

            // update y co-ordinate for next line
            dest_area.d_top += getLineSpacing(y_scale);
        }

        // add the next word to the line
        thisLine += thisWord;
    }

    // Last line is left aligned
    TextFormatting last_fmt = (fmt == Justified ? LeftAligned : fmt);
    // output last bit of string
    line_count += drawText(thisLine, dest_area, z, clip_rect, last_fmt, colours, x_scale, y_scale, true);

    return line_count;
}


/*************************************************************************
    Draw a line of text.  No formatting is applied.
*************************************************************************/
void Font::drawTextLine(const String& text, const Vector3& position, const Rect& clip_rect, const ColourRect& colours, float x_scale, float y_scale, bool in_bInternal)
{
    Vector3	cur_pos(position);

    const FontGlyph* glyph;
    float base_y = position.d_y;

    // If not called recursively internally, we need to set the colors to the
    // user's choice.
    if(!in_bInternal) {
        d_origColour = colours;
        d_curColour = colours;
    }

    for (size_t c = 0; c < text.length();)
    {
        // If we encounter a tab character, we go to the next tab line.
        if(text[c] == '\t' && d_tabEnabled) {
            size_t pixel_now = (size_t)PixelAligned(cur_pos.d_x - position.d_x);
            cur_pos.d_x += (float)(d_tabWidth - (pixel_now % d_tabWidth));
        } else {
            // Do the action of all triggers we encounter here.
            BaseFontTrigger *pBFT = this->getTriggerHere(text, c);
            if(pBFT) {
                pBFT->doTriggerAction(d_curColour, d_origColour);
                c += pBFT->getEatenTextLen();
                continue;
            }

            glyph = getGlyphData(text[c]);

            if (glyph)
            {
                const Image* img = glyph->getImage();
                cur_pos.d_y = base_y - (img->getOffsetY() - img->getOffsetY() * y_scale);
                img->draw(cur_pos, glyph->getSize(x_scale, y_scale), clip_rect, d_curColour);
                cur_pos.d_x += glyph->getAdvance(x_scale);
            }
        }
        ++c;
    }
}


/*************************************************************************
    Draw a justified line of text.
*************************************************************************/
void Font::drawTextLineJustified (const String& text, const Rect& draw_area, const Vector3& position, const Rect& clip_rect, const ColourRect& colours, float x_scale, float y_scale, bool in_bInternal)
{
    Vector3	cur_pos(position);

    const FontGlyph* glyph;
    float base_y = position.d_y;
    size_t char_count = text.length();

    // Calculate the length difference between the justified text and the same text, left aligned
    // This space has to be shared between the space characters of the line
    float lost_space = getFormattedTextExtent(text, draw_area, Justified, x_scale) - getTextExtent(text, x_scale);

    // The number of spaces and tabs in the current line
    uint space_count = 0;
    size_t c;
    for (c = 0; c < char_count; ++c)
        if ((text[c] == ' ') || (text[c] == '\t'))
            ++space_count;

    // The width that must be added to each space character in order to transform the left aligned text in justified text
    float shared_lost_space = 0.0;
    if (space_count > 0)
        shared_lost_space = lost_space / (float)space_count;

    // If not called recursively internally, we need to set the colors to the
    // user's choice.
    if(!in_bInternal) {
        d_origColour = colours;
        d_curColour = colours;
    }

    for (c = 0; c < char_count; ++c)
    {
        // Do the action of all triggers we encounter here.
        BaseFontTrigger *pBFT = this->getTriggerHere(text, c);
        if(pBFT) {
            pBFT->doTriggerAction(d_curColour, d_origColour);
            c += pBFT->getEatenTextLen();
            continue;
        }

        glyph = getGlyphData(text[c]);

        if (glyph)
        {
            const Image* img = glyph->getImage();
            cur_pos.d_y = base_y - (img->getOffsetY() - img->getOffsetY() * y_scale);
            img->draw(cur_pos, glyph->getSize(x_scale, y_scale), clip_rect, d_curColour);
            cur_pos.d_x += glyph->getAdvance(x_scale);

            // That's where we adjust the size of each space character
            if ((text[c] == ' ') || (text[c] == '\t'))
                cur_pos.d_x += shared_lost_space;
        }
    }
}


/*************************************************************************
    Set the native resolution for this Font
*************************************************************************/
void Font::setNativeResolution (const Size& size)
{
    d_nativeHorzRes = size.d_width;
    d_nativeVertRes = size.d_height;

    // re-calculate scaling factors & notify images as required
    notifyScreenResolution (System::getSingleton ().getRenderer()->getSize ());
}


/*************************************************************************
    Notify the Font of the current (usually new) display resolution.
*************************************************************************/
void Font::notifyScreenResolution (const Size& size)
{
    d_horzScaling = size.d_width / d_nativeHorzRes;
    d_vertScaling = size.d_height / d_nativeVertRes;

    if (d_autoScale)
        updateFont ();
}


/*************************************************************************
    Return the horizontal pixel extent given text would be formatted to.
*************************************************************************/
float Font::getFormattedTextExtent(const String& text, const Rect& format_area, TextFormatting fmt, float x_scale)
{
    float lineWidth;
    float widest = 0;

    size_t lineStart = 0, lineEnd = 0;
    String	currLine;

    // If needed, we just operate on the text w/o the triggers.
    String sNoTrigText = this->removeAllTriggers(text, 0);

    while (lineEnd < sNoTrigText.length())
    {
        if ((lineEnd = sNoTrigText.find_first_of('\n', lineStart)) == String::npos)
        {
            lineEnd = sNoTrigText.length();
        }

        currLine = sNoTrigText.substr(lineStart, lineEnd - lineStart);
        lineStart = lineEnd + 1;	// +1 to skip \n char

        switch(fmt)
        {
        case Centred:
        case RightAligned:
        case LeftAligned:
            lineWidth = getTextExtent(currLine, x_scale);
            break;

        case Justified:
            // usually we use the width of the rect but we have to ensure the current line is not wider than that
            lineWidth = ceguimax(format_area.getWidth(), getTextExtent(currLine, x_scale));
            break;

        case WordWrapLeftAligned:
        case WordWrapRightAligned:
        case WordWrapCentred:
            lineWidth = getWrappedTextExtent(currLine, format_area.getWidth(), x_scale);
            break;

        case WordWrapJustified:
            // same as above
            lineWidth = ceguimax(format_area.getWidth(), getWrappedTextExtent(currLine, format_area.getWidth(), x_scale));
            break;

        default:
            throw InvalidRequestException("Font::getFormattedTextExtent - Unknown or unsupported TextFormatting value specified.");
        }

        if (lineWidth > widest)
        {
            widest = lineWidth;
        }

    }

    return widest;
}


/*************************************************************************
    returns extent of widest line of wrapped text.
*************************************************************************/
float Font::getWrappedTextExtent(const String& text, float wrapWidth, float x_scale)
{
    String  whitespace = TextUtils::DefaultWhitespace;
    String	thisWord;
    size_t	currpos;
    float	lineWidth, wordWidth;
    float	widest = 0;

    // If needed, we just operate on the text w/o the triggers.
    String sNoTrigText = this->removeAllTriggers(text, 0);

    // get first word.
    currpos = getNextWord (sNoTrigText, 0, thisWord);
    lineWidth = getTextExtent (thisWord, x_scale);

    // while there are words left in the string...
    while (String::npos != sNoTrigText.find_first_not_of (whitespace, currpos))
    {
        // get next word of the string...
        currpos += getNextWord (sNoTrigText, currpos, thisWord);
        wordWidth = getTextExtent (thisWord, x_scale);

        // if the new word would make the string too long
        if ((lineWidth + wordWidth) > wrapWidth)
        {
            if (lineWidth > widest)
                widest = lineWidth;

            // remove whitespace from next word - it will form start of next line
            thisWord = thisWord.substr (thisWord.find_first_not_of (whitespace));
            wordWidth = getTextExtent (thisWord, x_scale);

            // reset for a new line.
            lineWidth = 0;
        }

        // add the next word to the line
        lineWidth += wordWidth;
    }

    if (lineWidth > widest)
        widest = lineWidth;

    return widest;
}


void Font::rasterize (utf32 start_codepoint, utf32 end_codepoint)
{
    // do nothing by default
}


/*************************************************************************
    Writes an xml representation of this Font to \a out_stream.
*************************************************************************/
void Font::writeXMLToStream(XMLSerializer& xml_stream) const
{
    // output starting <Font ... > element
    xml_stream.openTag("Font")
        .attribute(FontNameAttribute, d_name)
        .attribute(FontFilenameAttribute, d_fileName);

    if (!d_resourceGroup.empty ())
        xml_stream.attribute (FontResourceGroupAttribute, d_resourceGroup);

    if (d_nativeHorzRes != DefaultNativeHorzRes)
        xml_stream.attribute(FontNativeHorzResAttribute, PropertyHelper::uintToString(static_cast<uint>(d_nativeHorzRes)));

    if (d_nativeVertRes != DefaultNativeVertRes)
        xml_stream.attribute(FontNativeVertResAttribute, PropertyHelper::uintToString(static_cast<uint>(d_nativeVertRes)));

    if (d_autoScale)
        xml_stream.attribute(FontAutoScaledAttribute, "True");

    writeXMLToStream_impl (xml_stream);

    // output closing </Font> element.
    xml_stream.closeTag();
}


} // End of  CEGUI namespace section
