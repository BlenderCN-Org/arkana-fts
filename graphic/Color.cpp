#include "Color.h"

#include "main/Exception.h"

#include "dLib/dFile/dFile.h"

////////////////////////////////////////////
// Constructors and assignment operators. //
////////////////////////////////////////////

FTS::Color::Color(uint32_t col)
{
    m_col[0] = static_cast<float>((col >> 24) & 0xFF)/255.0f;
    m_col[1] = static_cast<float>((col >> 16) & 0xFF)/255.0f;
    m_col[2] = static_cast<float>((col >> 8) & 0xFF)/255.0f;
    m_col[3] = static_cast<float>(col & 0xFF)/255.0f;
}

FTS::Color::Color(float r, float g, float b, float a)
{
    m_col[0] = r;
    m_col[1] = g;
    m_col[2] = b;
    m_col[3] = a;
}

FTS::Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    m_col[0] = static_cast<float>(r)/255.0f;
    m_col[1] = static_cast<float>(g)/255.0f;
    m_col[2] = static_cast<float>(b)/255.0f;
    m_col[3] = static_cast<float>(a)/255.0f;
}

FTS::Color::Color(const FTS::Color& in_c)
{
    m_col[0] = in_c[0];
    m_col[1] = in_c[1];
    m_col[2] = in_c[2];
    m_col[3] = in_c[3];
}

FTS::Color::Color(const FTS::Color& in_c, float a)
{
    m_col[0] = in_c[0];
    m_col[1] = in_c[1];
    m_col[2] = in_c[2];
    m_col[3] = a;
}

FTS::Color::Color(const FTS::Color& in_c, uint8_t a)
{
    m_col[0] = in_c[0];
    m_col[1] = in_c[1];
    m_col[2] = in_c[2];
    m_col[3] = static_cast<float>(a)/255.0f;
}

FTS::Color::Color(const String& in_sCol)
{
    std::stringstream ss(in_sCol.str());
    char dummy; // Placeholder for ( , and )
    m_col[0] = 0.0;
    m_col[1] = 0.0;
    m_col[2] = 0.0;
    m_col[3] = 1.0;
    ss >> dummy >> m_col[0] >> dummy >> m_col[1] >> dummy >> m_col[2] >> dummy >> m_col[3] >> dummy;
}

const FTS::Color& FTS::Color::operator=(const FTS::Color& in_c)
{
    m_col[0] = in_c[0];
    m_col[1] = in_c[1];
    m_col[2] = in_c[2];
    m_col[3] = in_c[3];

    return *this;
}

///////////////////////////////////////
// Conversion methods and operators. //
///////////////////////////////////////

FTS::String FTS::Color::toString(unsigned int in_iDecimalPlaces) const
{
    return "("+String::nr(this->r(), in_iDecimalPlaces)+", "
              +String::nr(this->g(), in_iDecimalPlaces)+", "
              +String::nr(this->b(), in_iDecimalPlaces)+", "
              +String::nr(this->a(), in_iDecimalPlaces)+")";
}

FTS::Color::operator FTS::String() const
{
    return this->toString();
}

uint32_t FTS::Color::rgba() const
{
    return 0x01000000 * static_cast<uint32_t>(this->ir())
         + 0x00010000 * static_cast<uint32_t>(this->ig())
         + 0x00000100 * static_cast<uint32_t>(this->ib())
         + 0x00000001 * static_cast<uint32_t>(this->ia());
}

#ifdef D_USE_CEGUI
FTS::Color::Color(const CEGUI::colour& o)
{
    m_col[0] = o.getRed();
    m_col[1] = o.getGreen();
    m_col[2] = o.getBlue();
    m_col[3] = o.getAlpha();
}

FTS::Color::operator CEGUI::colour() const
{
    return CEGUI::colour(this->r(), this->g(), this->b(), this->a());
}

FTS::Color::operator CEGUI::ColourRect() const
{
    return CEGUI::ColourRect(this->operator CEGUI::colour());
}
#endif // D_USE_CEGUI

/////////////////////////////////////
// Accessors, getters and setters. //
/////////////////////////////////////

float& FTS::Color::operator[](unsigned int idx)
{
    if(idx > 3)
        throw FTS::NotExistException("Index " + String::nr(idx), "Color");

    return m_col[idx];
}

float FTS::Color::operator[](unsigned int idx) const
{
    if(idx > 3)
        throw FTS::NotExistException("Index " + String::nr(idx), "Color");

    return m_col[idx];
}

//////////////////////////////////////
// Vector interpolation operations. //
//////////////////////////////////////

FTS::Color FTS::Color::mix(const Color& c2, float alpha) const
{
    return Color(this->r() * (1.0f - alpha) + c2.r() * alpha,
                 this->g() * (1.0f - alpha) + c2.g() * alpha,
                 this->b() * (1.0f - alpha) + c2.b() * alpha,
                 this->a() * (1.0f - alpha) + c2.a() * alpha);
}

///////////////////////////
// Vector serialization. //
///////////////////////////

FTS::File& FTS::operator<<(File& f, const Color& c) {
    return f << c.r() << c.g() << c.b() << c.a();
}

FTS::File& FTS::operator>>(File& f, Color& c) {
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
    f >> r >> g >> b >> a;
    c.r(r).g(g).b(b).a(a);
    return f;
}

/// \TODO: I guess this should later be choosable per-map?
const std::vector<FTS::Color>& FTS::getPlayerColors()
{
    static std::vector<Color> ret;
    if(ret.empty()) {
        ret.push_back(Color(0x000000FF));
        ret.push_back(Color(0xFF0000FF));
        ret.push_back(Color(0x00FF00FF));
        ret.push_back(Color(0x0000FFFF));
        ret.push_back(Color(0xFFFF00FF));
        ret.push_back(Color(0xFF00FFFF));
        ret.push_back(Color(0x00FFFFFF));
        ret.push_back(Color(0xFFFFFFFF));
    }
    return ret;
}
