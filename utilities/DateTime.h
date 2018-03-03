/**
* \file parse.h
* \author Pompei2
* \date 01 May 2006
* \brief This file contains Date and Time handling.
* \remark Moved in from the utilities header file.
**/



#ifndef FTS_DATETIME_H
#define FTS_DATETIME_H

#include "dLib/dString/dString.h"

namespace FTS {

class DateTime
{
private:
    int m_iYear;
    unsigned char m_ucMonth;
    unsigned char m_ucDay;
    unsigned char m_ucHour;
    unsigned char m_ucMinute;
    unsigned char m_ucSecond;
    unsigned short m_usMs;

    bool m_bIsSet;
public:
    DateTime();
    DateTime( int in_iYear, unsigned char in_ucMonth,
              unsigned char in_ucDay );
    DateTime( unsigned char in_ucHour, unsigned char in_ucMinute,
              unsigned char in_ucSecond, unsigned short in_usMs = 0 );
    DateTime( int in_iYear, unsigned char in_ucMonth,
              unsigned char in_ucDay, unsigned char in_ucHour,
              unsigned char in_ucMinute, unsigned char in_ucSecond,
              unsigned short in_usMs = 0 );

    inline ~DateTime() {};

    inline int            getYear() const { return this->isSet() ? m_iYear : 0; };
    inline unsigned char  getMonth() const { return this->isSet() ? m_ucMonth : 0; };
    inline unsigned char  getDay() const { return this->isSet() ? m_ucDay : 0; };
    inline unsigned char  getHour() const { return this->isSet() ? m_ucHour : 0; };
    inline unsigned char  getMinute() const { return this->isSet() ? m_ucMinute : 0; };
    inline unsigned char  getSecond() const { return this->isSet() ? m_ucSecond : 0; };
    inline unsigned short getMs() const { return this->isSet() ? m_usMs : 0; };
    inline bool isSet() const { return m_bIsSet; };
    inline void empty() { m_iYear = 0; m_ucMonth = m_ucDay = m_ucHour = m_ucMinute = m_ucSecond = 0; m_usMs = 0; m_bIsSet = false; };

    inline void setYear( int value ) { m_iYear = std::abs( value ) < 2147483641 ? value : 1; m_bIsSet = std::abs( value ) < 2147483641; };
    inline void setMonth( unsigned char value ) { if ( value > 0 && value < 13 ) { m_ucMonth = value; m_bIsSet = true; } };
    inline void setDay( unsigned char value ) { if ( value > 0 && value < 32 ) { m_ucDay = value; m_bIsSet = true; } };
    inline void setHour( unsigned char value ) { if ( value > 0 && value < 25 ) { m_ucHour = value; m_bIsSet = true; } };
    inline void setMinute( unsigned char value ) { if ( value > 0 && value < 60 ) { m_ucMinute = value; m_bIsSet = true; } };
    inline void setSecond( unsigned char value ) { if ( value > 0 && value < 60 ) { m_ucSecond = value; m_bIsSet = true; } };
    inline void setMs( unsigned short value ) { if ( value > 0 && value < 1000 ) { m_usMs = value; m_bIsSet = true; } };

    int fromInternStr( const FTS::String &in_str );
    FTS::String toInternStr() const;
    FTS::String toStr() const;

    static DateTime EMPTY;
};

}
#endif
