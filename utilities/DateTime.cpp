/**
* \file utilities.cpp
* \author Pompei2
* \date unknown (very old)
* \brief This file implements general functions.
**/
#include <ctime>
#include "DateTime.h"

#include "dLib/dString/dTranslation.h"

using namespace FTS;

DateTime FTS::DateTime::EMPTY;

FTS::DateTime::DateTime()
{
    m_iYear = 1;
    m_ucMonth = 1;
    m_ucDay = 1;
    m_ucHour = 0;
    m_ucMinute = 0;
    m_ucSecond = 0;
    m_usMs = 0;
    m_bIsSet = false;
}

FTS::DateTime::DateTime( int in_iYear, unsigned char in_ucMonth, unsigned char in_ucDay )
{
    this->setYear( in_iYear );
    this->setMonth( in_ucMonth );
    this->setDay( in_ucDay );
    m_ucHour = 0;
    m_ucMinute = 0;
    m_ucSecond = 0;
    m_usMs = 0;
    m_bIsSet = true;
}

FTS::DateTime::DateTime( unsigned char in_ucHour, unsigned char in_ucMinute,
                         unsigned char in_ucSecond, unsigned short in_usMs )
{
    m_iYear = 1;
    m_ucMonth = 1;
    m_ucDay = 1;
    this->setHour( in_ucHour );
    this->setMinute( in_ucMinute );
    this->setSecond( in_ucSecond );
    this->setMs( in_usMs );
    m_bIsSet = true;
}

FTS::DateTime::DateTime( int in_iYear, unsigned char in_ucMonth,
                         unsigned char in_ucDay, unsigned char in_ucHour,
                         unsigned char in_ucMinute, unsigned char in_ucSecond,
                         unsigned short in_usMs )
{
    this->setYear( in_iYear );
    this->setMonth( in_ucMonth );
    this->setDay( in_ucDay );
    this->setHour( in_ucHour );
    this->setMinute( in_ucMinute );
    this->setSecond( in_ucSecond );
    this->setMs( in_usMs );
    m_bIsSet = true;
}

String FTS::DateTime::toInternStr() const
{
    return String::nr( this->getYear() ) + "-" + String::nr( this->getMonth() ) + "-" + String::nr( this->getDay() )
        + " " + String::nr( this->getHour() ) + ":" + String::nr( this->getMinute() ) + ":" + String::nr( this->getSecond() ) + ":" + String::nr( this->getMs() );
}

int FTS::DateTime::fromInternStr( const String &in_str )
{
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0;
    int count = sscanf( in_str.c_str(), "%d%d%d %d:%d:%d:%d", &i, &j, &k, &l, &m, &n, &o );
    if ( count == 7 )
    {
        this->setYear( i );
        this->setMonth( -j );
        this->setDay( -k );
        this->setHour( l );
        this->setMinute( m );
        this->setSecond( n );
        this->setMs( o );
    }
    else if ( count == 6 )
    {
        this->setYear( i );
        this->setMonth( -j );
        this->setDay( -k );
        this->setHour( l );
        this->setMinute( m );
        this->setSecond( n );
        this->setMs( 0 );
    }
    else
    {
        assert( false );
    }
    return ERR_OK;
}

String FTS::DateTime::toStr() const
{
    struct tm time;
    const char *psz = NULL;

    time.tm_sec = this->getSecond();
    time.tm_min = this->getMinute();
    time.tm_hour = this->getHour();
    time.tm_mday = this->getDay();
    time.tm_mon = this->getMonth();
    time.tm_year = /*this->getYear()*/100;
    time.tm_wday = 0;
    time.tm_yday = 0;
    time.tm_isdst = -1;

    // Calculate the wday and yday.
    mktime( &time );
    psz = asctime( &time );

    // Ignore the year.
    String s( psz, 0, 20 );

    // Add the year again, but don't use a negative number, prefer using a suffix.
    s += String::nr( std::abs( this->getYear() ) );
    if ( this->getYear()<0 )
    {
        Translation trans( "ui" );
        s += trans.get( "General_ac" );
    }

    return s;
}
