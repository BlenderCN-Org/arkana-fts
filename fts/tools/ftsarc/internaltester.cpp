#include "internaltester.h"
#include "logging/logger.h"
#include "dLib/dArchive/dArchive.h"
#include "utilities/StreamedDataContainer.h"

using namespace FTSArc;
using namespace FTS;

InternalTester::InternalTester()
{
}

InternalTester::~InternalTester()
{
}

int InternalTester::execute()
{
    FTSMSG("Read stream tests\n"
           "=================\n");

    FTSMSG("Creating empty pre-sized data container:       ");
    RawDataContainer rdc(58);
    if(rdc.getSize() != 58) {
        FTSMSG("FAIL\n");
        return -1;
    }
    FTSMSG("OK\n");

    FTSMSG("Filling data container with data:              ");
    for(uint64_t i = 0 ; i < rdc.getSize() ; i++) {
        rdc.getData()[i] = i;
    }
    FTSMSG("OK\n");

    FTSMSG("Creating streamed read data container from it: ");
    StreamedConstDataContainer scdc(new ConstRawDataContainer(rdc));
    if(scdc.eod()) {
        FTSMSG("FAIL\n");
        return -2;
    }
    FTSMSG("OK\n");

    FTSMSG("Reading arbitrary 8-bit chunk:                 ");
    scdc.setCursorPos(5);
    if(scdc.readi8() != 5) {
        FTSMSG("FAIL\n");
    }
    scdc.setCursorPos(0);
    FTSMSG("OK\n");

    FTSMSG("Reading signed 8-bit chunk:                    ");
    if(scdc.readi8() != 0) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading unsigned 8-bit chunk:                  ");
    if(scdc.readui8() != 1) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading signed 16-bit chunk:                   ");
    if(scdc.readi16() != 770) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading unsigned 16-bit chunk:                 ");
    if(scdc.readui16() != 1284) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading signed 32-bit chunk:                   ");
    if(scdc.readi32() != 151521030) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading unsigned 32-bit chunk:                 ");
    if(scdc.readui32() != 218893066) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading signed 64-bit chunk:                   ");
    if(scdc.readi64() != 1518859942647303950) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading unsigned 64-bit chunk:                 ");
    if(scdc.readui64() != 2097581325351917334) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading 32-bit floating point chunk:           ");
    if(scdc.readf() - 5.42512919e-19 >= 0.1e-19) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading 64-bit floating point chunk:           ");
    if(scdc.readd() - 2.008636e-110 >= 0.1e-110) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Reading 128-bit floating point chunk:          ");
    FTSMSG("CHECK NOT IMPLEMENTED\n");

    FTSMSG("Write stream tests\n"
           "==================\n");

    StreamedDataContainer sdc;

    FTSMSG("Write & Read back signed 8-bit chunk:          ");
    sdc.insert(static_cast<int8_t>(-2));
    sdc.moveCursor(-1);
    if(sdc.readi8() != -2) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back unsigned 8-bit chunk:        ");
    sdc.insert(static_cast<uint8_t>(252u));
    sdc.moveCursor(-1);
    if(sdc.readui8() != 252u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back signed 16-bit chunk:         ");
    sdc.insert(static_cast<int16_t>(-3));
    sdc.moveCursor(-2);
    if(sdc.readi16() != -3) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back unsigned 16-bit chunk:       ");
    sdc.insert(static_cast<uint16_t>(64574u));
    sdc.moveCursor(-2);
    if(sdc.readui16() != 64574u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back signed 32-bit chunk:         ");
    sdc.insert(static_cast<int32_t>(-4));
    sdc.moveCursor(-4);
    if(sdc.readi32() != -4) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back unsigned 32-bit chunk:       ");
    sdc.insert(static_cast<uint32_t>(4194957299u));
    sdc.moveCursor(-4);
    if(sdc.readui32() != 4194957299u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back signed 64-bit chunk:         ");
    sdc.insert(static_cast<int64_t>(-5));
    sdc.moveCursor(-8);
    if(sdc.readi64() != -5) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back unsigned 64-bit chunk:       ");
    sdc.insert(static_cast<uint64_t>(17446746073709651614u));
    sdc.moveCursor(-8);
    if(sdc.readui64() != 17446746073709651614u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back 32-bit floating point chunk: ");
    sdc.insert(static_cast<float>(-1.1f));
    sdc.moveCursor(-4);
    if(sdc.readf() != -1.1f) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back 64-bit floating point chunk: ");
    sdc.insert(static_cast<double>(-1.1));
    sdc.moveCursor(-8);
    if(sdc.readd() != -1.1) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Write & Read back 128-bit floating point chunk:");
    FTSMSG("CHECK NOT IMPLEMENTED\n");

    FTSMSG("Insertion of an unsigned 32-bit chunk:  ");
    sdc.setCursorPos(2);
    sdc.insert(static_cast<uint32_t>(4294967294u));
    sdc.moveCursor(-4);
    if(sdc.readui32() != 4294967294u) {
        FTSMSG("FAIL\n");
    }
    if(sdc.readi16() != -3) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Prepending of an unsigned 32-bit chunk: ");
    sdc.setCursorPos(0);
    sdc.insert(static_cast<uint32_t>(4294967293u));
    sdc.moveCursor(-4);
    if(sdc.readui32() != 4294967293u) {
        FTSMSG("FAIL\n");
    }
    if(sdc.readi8() != -2) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back signed 8-bit chunk:          ");
    sdc.overwrite(static_cast<int8_t>(-22));
    sdc.moveCursor(-1);
    if(sdc.readi8() != -22) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back unsigned 8-bit chunk:        ");
    sdc.overwrite(static_cast<uint8_t>(152u));
    sdc.moveCursor(-1);
    if(sdc.readui8() != 152u) {
        FTSMSG("FAIL\n");
    }
    sdc.moveCursor(+4);
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back signed 16-bit chunk:         ");
    sdc.overwrite(static_cast<int16_t>(-3));
    sdc.moveCursor(-2);
    if(sdc.readi16() != -3) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back unsigned 16-bit chunk:       ");
    sdc.overwrite(static_cast<uint16_t>(64574u));
    sdc.moveCursor(-2);
    if(sdc.readui16() != 64574u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back signed 32-bit chunk:         ");
    sdc.overwrite(static_cast<int32_t>(-4));
    sdc.moveCursor(-4);
    if(sdc.readi32() != -4) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back unsigned 32-bit chunk:       ");
    sdc.overwrite(static_cast<uint32_t>(4194957299u));
    sdc.moveCursor(-4);
    if(sdc.readui32() != 4194957299u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back signed 64-bit chunk:         ");
    sdc.overwrite(static_cast<int64_t>(-5));
    sdc.moveCursor(-8);
    if(sdc.readi64() != -5) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back unsigned 64-bit chunk:       ");
    sdc.overwrite(static_cast<uint64_t>(17446746073709651614u));
    sdc.moveCursor(-8);
    if(sdc.readui64() != 17446746073709651614u) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back 32-bit floating point chunk: ");
    sdc.overwrite(static_cast<float>(-1.1f));
    sdc.moveCursor(-4);
    if(sdc.readf() != -1.1f) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back 64-bit floating point chunk: ");
    sdc.overwrite(static_cast<double>(-1.1));
    sdc.moveCursor(-8);
    if(sdc.readd() != -1.1) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    FTSMSG("Overwrite & Read back 128-bit floating point chunk:");
    FTSMSG("CHECK NOT IMPLEMENTED\n");

    FTSMSG("Overwrite+Insert & Read back 64-bit floating point chunk: ");
    sdc.moveCursor(-4);
    sdc.overwrite(static_cast<double>(-1.23456789));
    sdc.moveCursor(-8);
    if(sdc.readd() != -1.23456789) {
        FTSMSG("FAIL\n");
    }
    FTSMSG("OK\n");

    return ERR_OK;
}
