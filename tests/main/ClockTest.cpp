

#include "dLib/aTest/TestHarness.h"

#include <chrono>
#include <thread>

#include "main/Clock.h"

using namespace FTS;

SUITE( ClockTests )

TEST_INSUITE( ClockTests, startTimeIsZero )
{
    Clock c;
    auto actual = c.getCurrentTime();
    auto expected = 0.0;
    CHECK_DOUBLES_EQUAL( expected, actual );

}

TEST_INSUITE( ClockTests, ticksPerSec )
{
    Clock c;
    c.tick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    c.tick();
    CHECK_EQUAL( 2, c.getTPS() );

    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    c.tick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    c.tick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    c.tick();
    CHECK_EQUAL( 5, c.getTPS() );

    std::this_thread::sleep_for( std::chrono::milliseconds( 1100 ) );
    c.tick();
    CHECK_EQUAL( 1, c.getTPS() );

}

TEST_INSUITE( ClockTests, deltaT )
{
    Clock c;

    c.tick();

    std::this_thread::sleep_for( std::chrono::milliseconds( 40 ) );

    c.tick();

    auto actual = c.getDeltaT();
    auto expected = 0.04;
    CHECK( ( expected - actual ) < 0.01 );

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    c.tick();

    actual = c.getDeltaT();
    expected = 0.5;
    CHECK( ( expected - actual ) < 0.02 );

}

TEST_INSUITE( ClockTests, fixDuration )
{
    Clock c;

    for ( int i = 0; i < 10; ++i )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
        c.tick();
    }

    auto actual = c.getDeltaT();
    auto expected = 0.01;
    CHECK( ( expected - actual ) < 0.01 );
    CHECK_EQUAL( 10, c.getTPS() );

}


TEST_INSUITE( ClockTests, currentTime )
{
    Clock c;

    c.tick();
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    c.tick();

    auto actual = c.getCurrentTime();
    auto expected = 0.01;
    CHECK( ( expected - actual ) < 0.01 );

    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    c.tick();
    actual = c.getCurrentTime();
    expected = 0.02;
    CHECK( ( expected - actual ) < 0.01 );

    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    c.tick();
    actual = c.getCurrentTime();
    expected = 0.03;
    CHECK( ( expected - actual ) < 0.01 );

    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    c.tick();
    actual = c.getCurrentTime();
    expected = 0.12;
    CHECK( ( expected - actual ) < 0.01 );
}

