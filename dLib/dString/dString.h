#ifndef D_STRING_H
#define D_STRING_H

/* Why do my own ? because I like reinvent the wheel ... maybe i'll write my own OpenGL hehe ... no. */

#include "main.h"
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#ifdef D_STRING_STD_ONLY
#  ifdef D_USE_CEGUI
#    undef D_USE_CEGUI
#  endif
#endif

#ifdef D_USE_CEGUI
#  include <CEGUIString.h>
#endif

/// Whether in a wildcard expression a questionmark ('?') can be no match or not.
#  define D_QUEST_CAN_BE_NONE	1

#include "main.h"
#include <typeinfo>
#include <string>

namespace FTS {

class String {
private:
    std::string m_s;

    template <typename T>
    static String fromSpecialNumber(T number);
    void fromStringWithLength(const char* in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength);
    int getByteCount(const char* in_pszString, std::string::size_type in_utf8Len);
    const char* advance(const char* in_pszString, std::string::size_type in_utf8Len) const;
    typedef std::tuple<std::string::size_type, std::string::size_type> StringSize;
    StringSize strlen() const;
public:
    static const String sWhiteSpace;

    /// Creates an empty string
    String();

    // We need those three for optimization.
    String(const char* in_pszString);
    String(const unsigned char* in_pszString);
    String(const String& in_sString);
    String(const std::string& in_sString);

    /// Construct the string with a maximum length, only starting from the
    /// \a in_iStart th character.
    /// Creates a string based on a C-string, but with a maximum length and
    /// starting only from a specified character index. If the index is higher
    /// then then string length, the created string is empty.
    ///
    /// \param in_pszString The C-String to use.
    /// \param in_iStart    The (0-based) index of the first character to copy.
    /// \param in_iLength   The maximum length of the string.
    String(const char *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    /// Construct the string with a maximum length
    /// Creates a string based on a C-string, but with a maximum length.
    ///
    /// \param in_pszString The C-String to use.
    /// \param in_iLength   The maximum length of the string.
    String(const int8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    /// Construct the string with a maximum length
    /// Creates a string based on a C-string, but with a maximum length.
    ///
    /// \param in_pszString The C-String to use.
    /// \param in_iLength   The maximum length of the string.
    String(const uint8_t *in_pszString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    /// Construct the string with a maximum length
    /// Creates a string based on another string, but with a maximum length.
    ///
    /// \param in_sString The String to use.
    /// \param in_iLength The maximum length of the string.
    String(const String& in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
    /// Construct the string with a maximum length
    /// Creates a string based on another string, but with a maximum length.
    ///
    /// \param in_sString The String to use.
    /// \param in_iLength The maximum length of the string.
    String(const std::string &in_sString, std::string::size_type in_iStart, std::string::size_type in_iLength = (std::string::size_type)-1);
#ifdef D_USE_CEGUI
    String(const CEGUI::String &in_sString, std::string::size_type in_iStart = 0, std::string::size_type in_iLength = (std::string::size_type)-1);
#endif

    /// Creates a string containing 1 char.
    /// Creates a string based on one single charachter.
    ///
    /// \param in_cChar The charachter wich to copy in the string.
    static String chr(unsigned char in_cChar);
    static String nr(const int8_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint8_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int16_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint16_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int32_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint32_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const int64_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const uint64_t& o, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::dec);
    static String nr(const float& o, std::streamsize in_iPrecision = -1, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::fixed | std::ios::right);
    static String nr(const double& o, std::streamsize in_iPrecision = -1, std::streamsize in_iWidth = 0, char in_cFill = ' ', std::ios_base::fmtflags in_fmtfl = std::ios::fixed | std::ios::right);
    static String b(const bool& o);

    /// Very inefficient but useful method to generate a random string following a
    /// pattern.
    ///
    /// \param in_sPattern The pattern used to generate the random string.
    ///                    If this pattern is empty, a random hex string of length 6
    ///                    is generated.
    /// \param gen A functor to generate random numbers that takes two integers and
    ///            returns a random integer between them.
    ///
    /// \note Everytime a '#' is encountered in the pattern, a random uppercase hex
    ///       digit (0-9,A-f) will be inserted in the result.\n
    ///       Every backslash '\' (except the last character of the string) will be
    ///       ignored and the next character will be used without interpretation.
    ///       Use this for escaping special characters: \\# becomes #, \\\\ becomes \.
    ///
    /// \return a random string (following the given pattern).
    template <class Generator>
    static String random(const String& in_sPattern, Generator gen);

    virtual ~String();

    /// Returns a string consisting of only the first \a in_iLength characters of
    /// this one.
    ///
    /// \param in_iLength The number of characters to take from the left of this string.
    ///
    /// \return A string consisting of the first \a in_iLenght characters of this one.
    String left(std::string::size_type in_iLength) const;

    /// Returns a string consisting of only the last \a in_iLength characters of
    /// this one.
    ///
    /// \param in_iLength The number of characters to take from the right of this string.
    ///
    /// \return A string consisting of the last \a in_iLenght characters of this one.
    String right(std::string::size_type in_iLength) const;

    /// Returns a string consisting of only the middle part of this one. So you can
    /// cut-off something on the left and something on the right.
    ///
    /// \param in_iLengthLeft The number of characters to cut-off on the left.
    /// \param in_iLengthRight The number of characters to cut-off on the right.
    ///
    /// \note If \a in_iLengthRight is 0, then it will take the string from
    ///       \a in_iLengthLeft until the end of the string.
    ///
    /// \return A string consisting of the characters in the middle of this string.
    String mid(std::string::size_type in_iLengthLeft, std::string::size_type in_iLengthRight) const;

    // Hex string <-> data conversion.

    /// Converts one byte into a two-letters-hex-string.
    /// Creates a string that contains two letters that represent the hex value
    /// of the given number.
    ///
    /// \param in_cNumber The number to create the string with.
    /// \param in_bCaps Shall the alphabeticals in the string be capitals or not?
    ///
    /// \return The string containing the number.
    ///
    /// \example Let in_cNumber be 0x6A, in_bCaps be true, then the string returned will be "6A"
    static String sfromHex(uint8_t in_cNumber, bool in_bCaps = true);
    /// Converts a one-or-two-letters-hex-string into a byte.
    /// Creates one byte using a string that contains two letters that represent
    /// the hex value of the given byte.
    ///
    /// \param in_sString The string to get the byte from.
    /// \param out_bSuccess Was the conversion successfull or not?
    ///
    /// \return The data byte.
    ///
    /// \example Let in_sString be "6A", then the returned byte will be 106
    ///
    /// \note If the string contains more then 2 chars, only the two first are taken.\n
    ///      A leading "0x" or similar in the string will make the conversion fail.\n
    ///      \n
    static uint8_t byteFromHex(const String &in_sString, bool &out_bSuccess);
    /// Construct a hexadecimal string from some arbitrary data.
    /// Creates a string that contains the data, stored as a hexadecimal string.
    /// That means that every byte from the data is stored into this string, using
    /// hexadecimal numbers.
    ///
    /// \param in_pData The data to convert into a hex string.
    /// \param in_uiLen The length of the data in bytes. This may also be 0, meaning
    ///                that the data is zero-terminated (stops at the first byte that is 0).
    /// \param in_bCaps Shall the alphabeticals in the string be capitals or not?
    ///
    /// \return The string containing the data, stored using hexadecimal numbers.
    static String hexFromData(const void *in_pData, uint32_t in_uiLen = 0, bool in_bCaps = true);
    /// Constructs data from an hexadecimal string.
    /// Creates data from the hexadecimal string.
    /// That means that every two-signs in the string will be converted into a byte
    /// that corresponds to the hexadecimal represented by the two signs.
    ///
    /// \return The data (allocated automatically). This has to be freed (delete [])
    ///        by the user !\n
    ///        NULL on failure.
    ///
    /// \note If there is any non-hex sign in the string, NULL will be returned.
    uint8_t *dataFromHex() const;

    // Functions that format the string itself.
    String fmt(const String &in_sArg1 = String::EMPTY,
                const String &in_sArg2 = String::EMPTY,
                const String &in_sArg3 = String::EMPTY,
                const String &in_sArg4 = String::EMPTY,
                const String &in_sArg5 = String::EMPTY,
                const String &in_sArg6 = String::EMPTY,
                const String &in_sArg7 = String::EMPTY,
                const String &in_sArg8 = String::EMPTY,
                const String &in_sArg9 = String::EMPTY
               ) const;
    String fmtRemoveEmpty(const String &in_sArg1 = String::EMPTY,
                           const String &in_sArg2 = String::EMPTY,
                           const String &in_sArg3 = String::EMPTY,
                           const String &in_sArg4 = String::EMPTY,
                           const String &in_sArg5 = String::EMPTY,
                           const String &in_sArg6 = String::EMPTY,
                           const String &in_sArg7 = String::EMPTY,
                           const String &in_sArg8 = String::EMPTY,
                           const String &in_sArg9 = String::EMPTY
                          ) const;

    /// Wether the string is empty or not.
    /// Returns a boolean specifying wether the string is empty or not.
    /// It is empty if it hasn't been initialised AND if it's "".
    ///
    /// \return true if the string is empty, else false.
    bool empty() const;

    /// get the length of the string, in characters.
    /// This function returns the length of the string in characters.
    ///
    /// \return the length of the string in characters, 0 if it is empty.
    std::string::size_type len() const;
    /// get the length of the string in characters.
    /// This function returns the length of the string in characters.
    ///
    /// \return the length of the string in characters, 0 if it is empty.
    int lenInt() const;
    /// get the length of the string in bytes.
    /// This function returns the amout of bytes this string takes in memory.
    ///
    /// \return the size of the string in bytes, 0 if it is empty.
    std::string::size_type byteCount() const;

    /// Get the C-String (read-only).
    /// Get the C-string that builds this string, in read-only mode.
    ///
    /// \return the C-string.
    const char *c_str() const;
    /// Get a copy of this string as a C++ std::string.
    ///
    /// \return A copy of this string as a C++ std::string.
    std::string str() const;

#ifdef D_USE_CEGUI
private:
    /// Get the CEGUI-String.
    /// Get the CEGUI-string pendant to this string. A copy is returned.
    ///
    /// \return A CEGUI-style copy of this string.
    CEGUI::String cegui() const;
public:
    /// Get the CEGUI-String.
    /// Get the CEGUI-string pendant to this string. A copy is returned.
    ///
    /// \return A CEGUI-style copy of this string.
    inline operator CEGUI::String() const {return this->cegui();};
#endif

    /// Remove Nth character from a string.
    /// Removes the \a in_ulIndex the character from the string.
    ///
    /// \param in_ulIndex The index of the character you want to delete.
    ///
    /// \return *this.
    ///
    /// \note If in_ulIndex is 0, the first character is deleted (it is 0-based).
    /// \note If in_ulIndex is bigger then the length of the string, the last
    ///      character of the string is deleted.
    ///
    ///Example: \n \code
    ///      String str( "HelloO, World !" );
    ///      str.removeChar( 5 )
    ///      // str is now "Hello, World !"\endcode
    String& removeChar(std::string::size_type in_ulIndex);
    /// Adds Nth character to a sring.
    /// Adds the character \a in_cChar at the \a in_ulIndex th place of the string.
    ///
    /// \param in_ulIndex The index where to add the character. (If higher then the
    ///                  length of the string, the character is added at the end.)
    /// \param in_cChar   The character you want to add.
    ///
    /// \return this.
    ///
    /// \Note If \a in_ulIndex is 0, the charachter is added at the beginning. (0-based)
    ///
    ///Example: \n \code
    ///      String str( "ello, World" );
    ///      str.addChar( 0, 'H' )
    ///      str.addChar( 99, '!' )
    ///      // str is now "Hello, World!"\endcode
    String& addChar(std::string::size_type in_ulIndex, unsigned char in_cChar);
    /// Replace a part of a string with another one.
    /// Replaces the part of the string that goes from \a in_ulIndex to
    /// \a in_ulIndex + \a in_ulLenght with \a in_pszNew.
    ///
    /// \param in_ulIndex  The index of the first charachter to replace (begins at 0).
    /// \param in_ulLenght The length of the string to replace.
    /// \param in_pszNew   The string with wich to replace the part index -> length.
    ///
    /// \return this.
    ///
    ///Example: \n \code
    ///      String str( "Hello, Awful World !" );
    ///      str.replaceStr( 7, 5, "Beautifull" );
    ///      // str is now "Hello, Beautifull World !"
    /// \endcode
    String& replaceStr(std::string::size_type in_ulIndex, std::string::size_type in_ulLenght,
                       const char *in_pszNew);
    /// Replace a part of a string with another one.
    /// Replaces the part of the string that goes from \a in_ulIndex to
    /// \a in_ulIndex + \a in_ulLenght with \a in_sNew.
    ///
    /// \param in_ulIndex  The index of the first charachter to replace (begins at 0).
    /// \param in_ulLenght The length of the string to replace.
    /// \param in_sNew     The string with wich to replace the part index -> length.
    ///
    /// \return this.
    ///
    ///Example: \n \code
    ///      String str( "Hello, Awful World !" );
    ///      str.replaceStr( 7, 5, "Beautifull" );
    ///      // str is now "Hello, Beautifull World !"
    /// \endcode
    String& replaceStr(std::string::size_type in_ulIndex, std::string::size_type in_ulLenght,
                       const String & in_sNew);
    /// Replace all occurrences of \a in_sNeedle with \a in_sNew.
    /// Replaces all occurrences of \a in_sNeedle found in this string by
    /// \a in_sNew.
    ///
    /// \param in_sNeedle  The string to replace with \a in_ulLenght.
    /// \param in_ulLenght The string that replaces \a in_sNeedle.
    ///
    /// \return this.
    String& replaceStr(const String & in_sNeedle,
                       const String & in_sNew);

    /// Returns (a copy of) the lowercase version of this string.
    /// this creates a string that is a copy of this one and then makes it all lowercase.
    ///
    /// \return The lowercase copy of this string.
    /// \warning This function is half UTF-8 aware! It will probably only work.
    ///          for pure ASCII characters a-z only.
    String lower() const;

    /// Returns (a copy of) the uppercase version of this string.
    /// this creates a string that is a copy of this one and then makes it all uppercase.
    ///
    /// \return The uppercase copy of this string.
    /// \warning This function is half UTF-8 aware! It will probably only work.
    ///          for pure ASCII characters a-z only.
    String upper() const;

    /// Creates a copy of the string that has removes all leading characters.
    /// This removes all characters included in \a in_sWhat that are at the front
    /// of the string (in a copy of that string).\n
    /// The original string does not get touched at all.
    ///
    /// \param in_sWhat A list of all characters that shall be trimmed (removed).
    ///
    /// \return the new, trimmed string
    /// \warning This function is not UTF-8 aware yet! If \a in_sWhat is meant to
    ///          contain any multibyte character, it will fail.
    ///          When \a getCharAt will be UTF-8 aware, this will probably be too.
    String trimLeft(const String &in_sWhat = sWhiteSpace) const;

    /// Creates a copy of the string that has removes all trailing characters.
    /// This removes all characters included in \a in_sWhat that are at the back
    /// of the string (in a copy of that string).\n
    /// The original string does not get touched at all.
    ///
    /// \param in_sWhat A list of all characters that shall be trimmed (removed).
    ///
    /// \return the new, trimmed string
    /// \warning This function is not UTF-8 aware yet! If \a in_sWhat is meant to
    ///          contain any multibyte character, it will fail.
    ///          When \a getCharAt will be UTF-8 aware, this will probably be too.
    String trimRight(const String &in_sWhat = sWhiteSpace) const;

    /// Creates a copy of the string that has removes all leading and trailing characters.
    /// This removes all characters included in \a in_sWhat that are at the front and
    /// at the back of the string (in a copy of that string).
    /// The original string does not get touched at all.
    ///
    /// \param in_sWhat A list of all characters that shall be trimmed (removed).
    ///
    /// \return the new, trimmed string
    /// \warning This function is not UTF-8 aware yet! If \a in_sWhat is meant to
    ///          contain any multibyte character, it will fail.
    ///          When \a getCharAt will be UTF-8 aware, this will probably be too.
    String trim(const String &in_sWhat = sWhiteSpace) const;

    /// Searches for a sub-string in this string.
    /// Searches for a sub-string in this string and returns the (0-based) index
    /// of the first occurence found.
    ///
    /// \param in_pszNeedle The string you want to search for.
    /// \param in_nIdx      The index where to begin the search (0 = search the whole string).
    ///
    /// \return If the string was found, the character index (0-based) of the first occurence of it.
    /// \return If the string wasn't found, std::string::npos.
    ///
    /// \note searching for the empty string will always return std::string::npos.
    std::string::size_type find(const char *in_pszNeedle, std::string::size_type in_nIdx = 0) const;

    /// Searches for a sub-string in this string.
    /// Searches for a sub-string in this string and returns the (0-based) index
    /// of the first occurence found.
    ///
    /// \param in_sNeedle The string you want to search for.
    /// \param in_nIdx    The index where to begin the search (0 = search the whole string).
    ///
    /// \return If the string was found, the character index (0-based) of the first occurence of it.
    /// \return If the string wasn't found, -1.
    std::string::size_type find(const String & in_sNeedle, std::string::size_type in_nIdx = 0) const;

    /// Compares this string to another, ignoring the case.
    /// This compares the current string with the string \a in_pszOther, ignoring
    /// case. If they are the same (ignoring case), true is returned, else false.
    ///
    /// \param in_pszOther The string you want to compare to.
    ///
    /// \return True if they are the same (ignoring case), false else.
    bool ieq(const char *in_pszOther) const;

    /// Compares this string to another, ignoring the case.
    /// This compares the current string with the string \a in_pszOther, ignoring
    /// case. If they are the same (ignoring case), true is returned, else false.
    ///
    /// \param in_sOther The string you want to compare to.
    ///
    /// \return True if they are the same (ignoring case), false else.
    bool ieq(const String & in_sOther) const;

    /// Acts like strncmp == 0
    /// Compare the first in_nChars charachters of in_pszNeedle with the first
    /// in_nChars charachters of this, if they all equal, returns true, else it
    /// returns false. If in_nChars is 0, the amount of chars to compare will be
    /// equal to the amount of char in_pszNeedle or this (if lower) has.
    ///
    /// \param in_pszNeedle The string you want to compare to.
    /// \param in_nChars    The number of charachters to compare, or 0 to compare the
    ///                    whole \a in_pszNeedle string.
    ///
    /// \return If the string beginning matches, true.
    /// \return If the string beginning doesn't match, false.
    bool neq(const char *in_pszNeedle, std::string::size_type in_nChars = (std::string::size_type)-1) const;

    /// Acts like strncmp == 0
    /// Compare the first in_nChars charachters of in_pszNeedle with the first
    /// in_nChars charachters of this, if they all equal, returns true, else it
    /// returns false. If in_nChars is 0, the amount of chars to compare will be
    /// equal to the amount of char in_pszNeedle or this (if lower) has.
    ///
    /// \param in_sNeedle  The string you want to compare to.
    /// \param in_nChars   The number of charachters to compare, or 0 to compare the whole string.
    ///
    /// \return If the string beginning matches, true.
    /// \return If the string beginning doesn't match, false.
    bool neq(const String & in_sNeedle, std::string::size_type in_nChars = (std::string::size_type)-1) const;

    /// Acts like strncmp == 0, but ignoring the case.
    /// Compare the first in_nChars charachters of in_pszNeedle with the first
    /// in_nChars charachters of this, if they all equal (ignoring case), returns true, else it
    /// returns false. If in_nChars is 0, the amount of chars to compare will be
    /// equal to the amount of char in_pszNeedle or this (if lower) has.
    ///
    /// \param in_pszNeedle The string you want to compare to.
    /// \param in_nChars    The number of charachters to compare, or 0 to compare the whole needle string.
    ///
    /// \return If the string beginning matches, true.
    /// \return If the string beginning doesn't match, false.
    bool nieq(const char *in_pszNeedle, std::string::size_type in_nChars = (std::string::size_type)-1) const;

    /// Acts like strncmp == 0, but ignoring the case.
    /// Compare the first in_nChars charachters of in_pszNeedle with the first
    /// in_nChars charachters of this, if they all equal (ignoring case), returns true, else it
    /// returns false. If in_nChars is 0, the amount of chars to compare will be
    /// equal to the amount of char in_pszNeedle or this (if lower) has.
    ///
    /// \param in_sNeedle The string you want to compare to.
    /// \param in_nChars  The number of charachters to compare, or 0 to compare the whole string.
    ///
    /// \return If the string beginning matches, true.
    /// \return If the string beginning doesn't match, false.
    bool nieq(const String & in_sNeedle, std::string::size_type in_nChars = (std::string::size_type)-1) const;

    /// Looks wether a sub-string is present in this string.
    ///
    /// \param in_pszNeedle The string you want to search for.
    ///
    /// \return If the string is present true, else false.
    ///
    /// \note the empty string is contained nowhere.
    bool contains(const char *in_pszNeedle) const;
    /// Looks wether a sub-string is present in this string.
    ///
    /// \param in_sNeedle The string you want to search for.
    ///
    /// \return If the string is present true, else false.
    bool contains(const String & in_sNeedle) const;

    /// Matches a pattern onto this string. The pattern is not a regular expression!
    /// It can have * (a star), that means to match any string of any length,
    /// including nothing ; it can have a question mark, that means to match any
    /// single character (or nothing if D_QUEST_CAN_BE_NONE is defined as 1) and
    /// it can have a range of characters using the syntax [a-z] or [xah].
    ///
    /// \param in_pszPat The pattern to try to match.
    ///
    /// \return true if this matches the pattern, false if not or if there is a
    ///         syntax error in the pattern.
    bool matchesPattern(const char *in_pszPat) const;

    /// Matches a pattern onto this string. The pattern is not a regular expression!
    /// It can have * (a star), that means to match any string of any length,
    /// including nothing ; it can have a question mark, that means to match any
    /// single character (or nothing if D_QUEST_CAN_BE_NONE is defined as 1) and
    /// it can have a range of characters using the syntax [a-z] or [xah].
    ///
    /// \param in_sPat The pattern to try to match.
    ///
    /// \return true if this matches the pattern, false if not or if there is a
    ///         syntax error in the pattern.
    bool matchesPattern(const String& in_sPat) const;

    /// Splits the string at every occurrence of \a in_sSplit and stores the
    /// substrings into the output iterator \a out. The container either needs
    /// to be big enough, or you have to use something like \a std::inserter
    /// to construct an output iterator that inserts new elements into the
    /// container.
    ///
    /// \param out The output iterator to write the substrings to.
    /// \param in_sSplit the delimiter string to use for splitting.
    template <class OutputIterator>
    void split(OutputIterator out, const String& in_sSplit = " ") const
    {
        std::string::size_type lastpos = 0;
        for(std::string::size_type pos = this->find(in_sSplit) ; pos != std::string::npos ; pos = this->find(in_sSplit, pos+1)) {
            if(pos <= lastpos) {
                *out++ = String::EMPTY;
            } else {
                *out++ = String(*this, lastpos, pos-lastpos);
            }
            lastpos = pos + in_sSplit.len();
        }

        // Now we may still need to get the last part.
        if(lastpos < this->len()) {
            *out = String(*this, lastpos, this->len() - lastpos);
        }
    }

    // A whole bunch of overloaded operators.
    String& operator =(const char *in_pszString);
    String& operator =(const String & in_sString);

    void operator +=(const char *in_pszString);
    void operator +=(const String & in_sString);

    String operator +(const char *in_pszString) const;
    String operator +(const String & in_sString) const;

    bool operator ==(const char *in_pszString) const;
    bool operator ==(const String & in_sString) const;

    bool operator !=(const char *in_pszString) const;
    bool operator !=(const String & in_sString) const;

    bool operator !() const;

    bool operator <(const char *in_pszString) const;
    bool operator <(const String & in_sString) const;

    /// \throw std::out_of_range if \a in_iIndex is bigger than the string.
    /// \warning This function is not UTF-8 aware yet! Its index is in bytes
    ///          and it may break a character and return garbage.
    unsigned char& operator [] (std::string::size_type in_iIndex);
    /// \return 0 on a bad index.
    /// \warning This function is not UTF-8 aware yet! Its index is in bytes
    ///          and it may break a character and return garbage.
    unsigned char getCharAt(std::string::size_type in_iIndex) const;

    static String EMPTY;
        
    template<typename T> T toExactly() const
    {
        T ret;
        std::istringstream i(m_s);
        char c;
        if (!(i >> ret) || i.get(c))
            throw std::bad_cast();

        return ret;
    }

    template<typename T> T& to(T& t) const 
    { 
        std::istringstream i(m_s);
        i >> t;
        return t;
    };

    template<typename T> T& toExactly(T& t) const { t = this->toExactly<T>(); return t; }
};

template<> bool& String::to<bool>(bool& t) const; 
template<> bool String::toExactly<bool>() const;

}; // namespace FTS;

bool operator ==(const char *in_pszString, const FTS::String& in_sString);
bool operator !=(const char *in_pszString, const FTS::String& in_sString);
bool operator ==(const std::string& in_s, const FTS::String& in_sString);
bool operator !=(const std::string& in_s, const FTS::String& in_sString);
bool operator <(const char *in_pszString, const FTS::String& in_sString);
FTS::String operator +(const char *in_pszString, const FTS::String & in_sString);
std::ostream& operator<< (std::ostream& os, const FTS::String& me);

    /// Use this as a comparator for the strings (check if str1 is less then str2).
struct cstring_ltcomp {
  bool operator() (const FTS::String& lstr, const FTS::String& rstr) const
  {return lstr<rstr;}
};

    /// Use this as a comparator for the strings (check if str1 is equal to str2).
struct cstring_eqcomp {
  bool operator() (const FTS::String& lhs, const FTS::String& rhs) const
  {return lhs==rhs;}
};

template <class Generator>
FTS::String FTS::String::random(const String& in_sPattern, Generator gen)
{
    // Empty pattern means 6-digit random hex.
    String sResult(in_sPattern.empty() ? "######" : in_sPattern);

    for(std::string::size_type i = 0 ; i < sResult.len() ; ++i) {
        switch(sResult.getCharAt(i)) {
        case '#':
        {
            // This has to be replaced by a random uppercase hex number.
            uint8_t num = static_cast<uint8_t>(gen(0, 0xF));
            sResult[i] = (num < 0xA ? ('0'+num) : ('A'+(num-0xA)));
            break;
        }
        case '\\':
            // If there is a character after the backslash, we just remove the backslash
            // and skip the next character w/o interpreting it.
            if(i+1 < sResult.len()) {
                sResult.removeChar(i);
            }
        default: break;
        }
    }

    return sResult;
}

/// Joins a "collection of things that can construct a string" into a string
/// with glue string in between.
///
/// \param begin The start iterator of the collection. It will be dereferenced and
///              passed to a string constructor in order to transform it to a string.
/// \param end The iterator one past the last one of the collection.
/// \param glue The glue string that is placed in between each entry of the collection.
///
/// \return a string consisting of the collection's items with glue in between them.
template <class InputIterator>
FTS::String join(InputIterator begin, InputIterator end, const FTS::String& glue = " ")
{
    FTS::String ret;
    while(begin != end) {
        ret += FTS::String(*begin);
        if(++begin != end) {
            ret += glue;
        }
    }

    return ret;
}

/// Joins a "collection of things that can construct a string" into a string
/// with glue before and after each element.
///
/// \param before The string that will be placed in front of every element.
/// \param begin The start iterator of the collection. It will be dereferenced and
///              passed to a string constructor in order to transform it to a string.
/// \param end The iterator one past the last one of the collection.
/// \param after The string that will be placed behind every element.
///
/// \return a string consisting of the collection's items with glue before and after each of them.
template <class InputIterator>
FTS::String join(const FTS::String& before, InputIterator begin, InputIterator end, const FTS::String& after = "")
{
    return before + join(begin, end, after + before) + after;
}

#endif                          /* D_STRING_H */

 /* EOF */
