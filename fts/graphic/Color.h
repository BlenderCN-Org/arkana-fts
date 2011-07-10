#ifndef D_COLOR_H
#define D_COLOR_H

#include "main/main.h" // int types.
#include "dLib/dString/dString.h"

#ifdef D_USE_CEGUI
#  include <CEGUIcolour.h>
#  include <CEGUIColourRect.h>
#endif

#include <vector>

namespace FTS {
    class Color;
    class File;

    //////////////////////////
    // Color serialization. //
    //////////////////////////
    /// Serializes a color to a file. Use it like this: f << c1 << c2 << etc.
    /// \param f The file to write the color to.
    /// \param c The color to write to the file.
    /// \return a reference to the file to allow chaining.
    File& operator<<(File& f, const Color& c);
    /// Extracts a color from a file. Use it like this: f >> c1 >> c2 >> etc.
    /// \param f The file to read the color from.
    /// \param c The color to write the read values to.
    /// \return a reference to the file to allow chaining.
    File& operator>>(File& f, Color& c);

/// This class represents a color in RGBA color space. The color can be
/// interpreted both as a 32 bit color with each value ranging from 0 to 255 or
/// as a HD color with each value being a floating point ranging from 0 to 1.
class Color
{
    friend File& operator<<(File& f, const Color& v);
    friend File& operator>>(File& f, Color& v);
public:
    ////////////////////////////////////////////
    // Constructors and assignment operators. //
    ////////////////////////////////////////////

    /// Creates a color from a 32 bit rgba value.
    /// \param col The rgba value to use, for example 0x00FF00FF is green.
    Color(uint32_t col = 0x000000FF);
    /// Construct a color.
    Color(float r, float g, float b, float a = 1.0);
    /// Construct a color.
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    /// Copies a color.
    /// \param in_c The color to be copied.
    Color(const Color& in_c);
    /// Copies a color but gives a different alpha value.
    /// \param in_c The color to be copied.
    /// \param a The alpha value to be used for the new color.
    Color(const Color& in_c, float a);
    /// Copies a color but gives a different alpha value.
    /// \param in_c The color to be copied.
    /// \param a The alpha value to be used for the new color.
    Color(const Color& in_c, uint8_t a);
    /// Constructs a color from a string that has to be in the same format as
    /// the toString method returns.
    /// \param in_sCol A string in the same format as the toString method returns.
    Color(const String& in_sCol);
    /// Copies a color.
    /// \param in_c The color to be copied.
    /// \return a const reference to myself that might be used as a rvalue.
    const Color& operator=(const Color& in_c);

    ///////////////////////////////////////
    // Conversion methods and operators. //
    ///////////////////////////////////////

    /// \return A read-only array of three floats holding the values of the
    ///         three components (RGB) of this color.
    inline const float *array3f() const {return m_col;};
    /// \return A read-only array of four floats holding the values of the
    ///         four components (RGBA) of this color.
    inline const float *array4f() const {return m_col;};

    /// \return A string-representation of the color.
    /// \param in_iDecimalPlaces The amount of numbers to print behind the dot.
    String toString(unsigned int in_iDecimalPlaces = 2) const;
    /// \return A string-representation of the vector.
    operator String() const;

    /// \return the color as a 32-bit RGBA color.
    uint32_t rgba() const;

#ifdef D_USE_CEGUI
    Color(const CEGUI::colour&);
    operator CEGUI::colour() const;
    operator CEGUI::ColourRect() const;
#endif // D_USE_CEGUI

    /////////////////////////////////////
    // Accessors, getters and setters. //
    /////////////////////////////////////

    /// \return The red component of the color, in a range from 0 to 1.
    inline float r() const {return m_col[0];}
    /// \return The green component of the color, in a range from 0 to 1.
    inline float g() const {return m_col[1];}
    /// \return The blue component of the color, in a range from 0 to 1.
    inline float b() const {return m_col[2];}
    /// \return The alpha component of the color, in a range from 0 to 1.
    inline float a() const {return m_col[3];}
    /// \return The red component of the color, in a range from 0 to 255.
    inline uint8_t ir() const {return static_cast<uint8_t>(m_col[0] * 255.0f);}
    /// \return The green component of the color, in a range from 0 to 255.
    inline uint8_t ig() const {return static_cast<uint8_t>(m_col[1] * 255.0f);}
    /// \return The blue component of the color, in a range from 0 to 255.
    inline uint8_t ib() const {return static_cast<uint8_t>(m_col[2] * 255.0f);}
    /// \return The alpha component of the color, in a range from 0 to 255.
    inline uint8_t ia() const {return static_cast<uint8_t>(m_col[3] * 255.0f);}

    /// \param r The new red component to give to this color.
    inline Color& r(float r) {m_col[0] = r; return *this;}
    /// \param g The new green component to give to this color.
    inline Color& g(float g) {m_col[0] = g; return *this;}
    /// \param b The new blue component to give to this color.
    inline Color& b(float b) {m_col[0] = b; return *this;}
    /// \param a The new alpha component to give to this color.
    inline Color& a(float a) {m_col[0] = a; return *this;}

    /// Access the elements of this color.
    /// \param idx The index of the element of this color. This may only be a
    ///            value between 0 and 3.
    /// \throws NotExistException if \a idx is >3.
    float& operator[](unsigned int idx);
    /// Access the elements of this color in read-only.
    /// \param idx The index of the element of this color. This may only be a
    ///            value between 0 and 3.
    /// \throws NotExistException if \a idx is >3.
    float operator[](unsigned int idx) const;

    //////////////////////////////////////
    // Vector interpolation operations. //
    //////////////////////////////////////

    /// Linear interpolation between this and c2. This is like the GLSL mix function.
    /// \param c2 The other color with which to interpolate.
    /// \param alpha The position inbetween. 0.0f results in this, 1.0f results in \a c2.
    /// \return A color resulting from the linear interpolation of this and \a c2, with factor \a alpha
    Color mix(const Color& c2, float alpha) const;

    /// Linear interpolation between this and c2. This is like the GLSL mix function.
    /// \param c2 The other color with which to interpolate.
    /// \param alpha The position inbetween. 0 results in this, 255 results in \a c2.
    /// \return A color resulting from the linear interpolation of this and \a c2, with factor \a alpha
    inline Color mix(const Color& c2, uint8_t alpha) const {return this->mix(c2, static_cast<float>(alpha)/255.0f);};

private:
    float m_col[4];
};

const std::vector<Color>& getPlayerColors();

} // namespace FTS

#endif // D_COLOR_H
