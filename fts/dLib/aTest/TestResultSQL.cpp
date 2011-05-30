#define _CRT_SECURE_NO_WARNINGS

#include "TestResultSQL.h"
#include "Failure.h"
#include "Test.h"
#include "TestSuite.h"
#include <iostream>
#include <iomanip>

TestResultSQL::TestResultSQL(std::ostream& out, const std::string& in_sProjectName,
                             bool in_bKeepOld,
                             const std::string& in_sProjectsTbl,
                             const std::string& in_sSuitesTbl,
                             const std::string& in_sCasesTbl,
                             const std::string& in_sFailuresTbl)
    : m_sProjectsTbl(in_sProjectsTbl)
    , m_sSuitesTbl(in_sSuitesTbl)
    , m_sCasesTbl(in_sCasesTbl)
    , m_sFailuresTbl(in_sFailuresTbl)
    , m_bKeepOld(in_bKeepOld)
    , m_out(out)
    , m_sProjectName(in_sProjectName)
{
}

TestResultSQL::~TestResultSQL()
{
}

std::string TestResultSQL::now() const
{
    time_t rawtime;
    time(&rawtime);

    struct tm* gmt_utc = gmtime(&rawtime);

    std::ostringstream datetime;
    datetime.fill('0');
    datetime << std::setw(4) << 1900 + gmt_utc->tm_year << "-"
             << std::setw(2) << gmt_utc->tm_mon << "-"
             << std::setw(2) << gmt_utc->tm_mday << " "
             << std::setw(2) << gmt_utc->tm_hour << ":"
             << std::setw(2) << gmt_utc->tm_min << ":"
             << std::setw(2) << gmt_utc->tm_sec;

    return datetime.str();
}

std::string TestResultSQL::wsToUnderscore(const std::string& s) const
{
    std::string sNew(s);
    size_t sPos = sNew.find_first_of(" \t\n");
    while(sPos != std::string::npos) {
        sNew.replace(sPos, 1, "_");
        sPos = sNew.find_first_of(" \t\n");
    }

    return sNew;
}

void TestResultSQL::startTests ()
{
    TestResult::startTests();

    // Write some big comment and the table creation code.
    m_out << "-----------------------------------------------" << std::endl
          << "-- automatically generated SQL output script --" << std::endl
          << "--                                           --" << std::endl
          << "" << std::endl
          << "--------------------------------------------------------" << std::endl
          << "-- Create the projects tables if they don't exist yet --" << std::endl
          << "" << std::endl
          << "create table if not exists `" << m_sProjectsTbl << "` (" << std::endl
          << "  `id` int unsigned not null auto_increment," << std::endl
          << "  `name` varchar(255)  not null," << std::endl
          << "  `starttime` datetime not null," << std::endl
          << "  `stoptime` datetime not null," << std::endl
          << "  primary key (`id`)" << std::endl
          << ") character set utf8 COLLATE utf8_general_ci;" << std::endl
          << "" << std::endl
          << "create table if not exists `" << m_sSuitesTbl << "` (" << std::endl
          << "  `id` int unsigned not null auto_increment," << std::endl
          << "  `name` varchar(255) not null," << std::endl
          << "  `parent` int unsigned not null," << std::endl
          << "  `parent_issuite` boolean default false," << std::endl
          << "  `starttime` datetime not null," << std::endl
          << "  `stoptime` datetime not null," << std::endl
          << "  primary key (`id`)" << std::endl
          << ") character set utf8 COLLATE utf8_general_ci;" << std::endl
          << "" << std::endl
          << "create table if not exists `" << m_sCasesTbl << "` (" << std::endl
          << "  `id` int unsigned not null auto_increment," << std::endl
          << "  `name` varchar(255) not null," << std::endl
          << "  `parent` int unsigned not null," << std::endl
          << "  `parent_issuite` boolean default false," << std::endl
          << "  `failures` int unsigned default 0," << std::endl
          << "  `starttime` datetime not null," << std::endl
          << "  `stoptime` datetime not null," << std::endl
          << "  primary key (`id`)" << std::endl
          << ") character set utf8 COLLATE utf8_general_ci;" << std::endl
          << "" << std::endl
          << "create table if not exists `" << m_sFailuresTbl << "` (" << std::endl
          << "  `id` int unsigned not null auto_increment," << std::endl
          << "  `case` int unsigned not null," << std::endl
          << "  `reason` text not null," << std::endl
          << "  primary key (`id`)" << std::endl
          << ") character set utf8 COLLATE utf8_general_ci;" << std::endl;

    // Now insert the project into the table, removing older instances of it if desired.
    m_out << "" << std::endl
          << "-------------------" << std::endl
          << "-- test projects --" << std::endl
          << "" << std::endl;

    if(!m_bKeepOld) {
        m_out << "-- replace the old tests" << std::endl
              << "delete from `" << m_sProjectsTbl << "`" << std::endl
              << "    where `name`='ftsarc';" << std::endl;
    }
    m_out << "-- add the new tests" << std::endl
          << "insert into `" << m_sProjectsTbl << "` (`id`, `name`, `starttime`)" << std::endl
          << "    values (null, '" << m_sProjectName << "', '" << now() << "');" << std::endl
          << "set @projectID = LAST_INSERT_ID();" << std::endl
          << "" << std::endl
          << "" << std::endl
          << "----------------------------------" << std::endl
          << "-- test data (suites and cases) --" << std::endl
          << "" << std::endl;
}

void TestResultSQL::endTests ()
{
    TestResult::endTests();

    // Now we can update the stoptime of the project.
    m_out << "update `" << m_sProjectsTbl << "` set `stoptime`='" << now() << "'" << std::endl
          << "    where `id`= @projectID;" << std::endl;

/*    m_out << testCount << " tests run" << std::endl;
    if (failureCount > 0)
        m_out << "****** There were " << failureCount << " failures.";
    else
        m_out << "There were no test failures.";
    m_out << "(time: " << secondsElapsed << " s)" << std::endl;*/
}

void TestResultSQL::startSuite(const TestSuite& suite)
{
    TestResult::startSuite(suite);
//     m_out << "startSuite(" << suite.name() << ")" << std::endl;

    if(!m_bKeepOld) {
        m_out << "delete from `" << m_sSuitesTbl << "`" << std::endl
              << "    where `name`='" << suite.name() << "';" << std::endl;
    }

    m_out << "insert into `" << m_sSuitesTbl << "` (`id`, `name`, `parent`, `parent_issuite`, `starttime`)" << std::endl;
    if(suite.registrar() == NULL)
        m_out << "    values (0, '" << suite.name() << "', @projectID, false, '" << now() << "');" << std::endl;
    else
        m_out << "    values (0, '" << suite.name() << "', @" << wsToUnderscore(suite.registrar()->name()) << "ID, true, '" << now() << "');" << std::endl;
    m_out << "set @" << wsToUnderscore(suite.name()) << "ID = LAST_INSERT_ID();" << std::endl
          << "" << std::endl;
}

void TestResultSQL::endSuite(const TestSuite& suite)
{
    // Now we can update the stoptime of the suite.
    m_out << "update `" << m_sSuitesTbl << "` set `stoptime`='" << now() << "'" << std::endl
          << "    where `id`= @" << wsToUnderscore(suite.name()) << "ID;" << std::endl
          << "" << std::endl;

//     m_out << "endSuite(" << suite.name() << ")" << std::endl;
    TestResult::endSuite(suite);
}

void TestResultSQL::startTest (const Test& test)
{
    TestResult::startTest(test);
//     m_out << "startTest(" << test.name() << ")" << std::endl;

    if(!m_bKeepOld) {
        // Delete all its old failures first.
        m_out << "delete from `" << m_sFailuresTbl << "`" << std::endl
              << "    where `case`=(select `id` from " << m_sCasesTbl << " where name='" << test.name() << "');" << std::endl;
        m_out << "delete from `" << m_sCasesTbl << "`" << std::endl
              << "    where `name`='" << test.name() << "';" << std::endl;
    }

    m_out << "insert into `" << m_sCasesTbl << "` (`id`, `name`, `parent`, `parent_issuite`, `starttime`)" << std::endl;
    if(test.registrar() == NULL)
        m_out << "    values (0, '" << test.name() << "', @projectID, false, '" << now() << "');" << std::endl;
    else
        m_out << "    values (0, '" << test.name() << "', @" << wsToUnderscore(test.registrar()->name()) << "ID, true, '" << now() << "');" << std::endl;
    m_out << "    set @" << wsToUnderscore(test.name()) << "ID = LAST_INSERT_ID();" << std::endl
          << "" << std::endl;
}

void TestResultSQL::testWasRun (const Test& test)
{
    // Now we can update the stoptime of the case.
    m_out << "update `" << m_sCasesTbl << "` set `stoptime`='" << now() << "'" << std::endl
          << "    where `id`= @" << wsToUnderscore(test.name()) << "ID;" << std::endl
          << "" << std::endl;

    TestResult::testWasRun(test);
//     m_out << "testWasRun(" << test.name() << ")" << std::endl;
}

void TestResultSQL::testWasSkipped (const Test& test)
{
    // Now we can update the stoptime of the case.
    m_out << "update `" << m_sCasesTbl << "` set `stoptime`='" << now() << "'" << std::endl
          << "    where `id`= @" << wsToUnderscore(test.name()) << "ID;" << std::endl
          << "" << std::endl;

    TestResult::testWasSkipped(test);
//     m_out << "testWasRun(" << test.name() << ")" << std::endl;
}

void TestResultSQL::addFailure (const Test& test, const Failure & failure)
{
    TestResult::addFailure(test, failure);
//     m_out << failure << std::endl;

    std::stringstream ss;
    ss << failure;
    std::string sFailure = ss.str();

    // Prefix any ' or backslash by a backslash to escape it.
    size_t sPos = sFailure.find_first_of("'\\");
    while(sPos != std::string::npos) {
        sFailure.insert(sPos, 1, '\\');
        if(sPos+2 > sFailure.length())
            sPos = std::string::npos;
        else
            sPos = sFailure.find_first_of("'\\", sPos+2);
    }

    // Create the failure entry.
    m_out << "insert into `" << m_sFailuresTbl << "` (`id`, `case`, `reason`)" << std::endl
          << "    values (0, @" << wsToUnderscore(test.name()) << "ID, '" << sFailure << "');" << std::endl;

    // Increment the case's failure count.
    m_out << "update `" << m_sCasesTbl << "` SET `failures`=`failures`+1 WHERE `id`=@" << wsToUnderscore(test.name()) << "ID;" << std::endl
          << "" << std::endl;
}
