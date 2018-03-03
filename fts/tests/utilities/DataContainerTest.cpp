#include "dLib/aTest/TestHarness.h"
#include "utilities/DataContainer.h"
#include <cstring>

using namespace FTS;

SUITE( DataContainerTests )

TEST_INSUITE( DataContainerTests, createWithSize )
{
    RawDataContainer container(10);
    CHECK_EQUAL( 10, container.getSize() );
    CHECK(container.getData() != nullptr);
}

TEST_INSUITE(DataContainerTests, createWithOutSize)
{
    RawDataContainer container;
    CHECK_EQUAL(0, container.getSize());
    CHECK(container.getData() == nullptr);
}

TEST_INSUITE(DataContainerTests, copy)
{
    RawDataContainer container(10);
    CHECK_EQUAL(10, container.getSize());
    CHECK(container.getData() != nullptr);
    char testdata[] = "123456789";
    memcpy(container.getData(), testdata, 9);
    
    auto c2 = container.copy();
    CHECK_EQUAL(10, c2->getSize()); 
    CHECK(memcmp( testdata, c2->getData(), 9) == 0);
    CHECK(container.getData() != c2->getData());
    delete c2;
}

TEST_INSUITE(DataContainerTests, assign)
{
    RawDataContainer container(2);
    auto c2 = container;
    CHECK_EQUAL(2, container.getSize());
    CHECK(container.getData() != nullptr);

    CHECK(container.getData() != c2.getData());
}

TEST_INSUITE(DataContainerTests, copy_ctor)
{
    RawDataContainer container(10);
    char testdata[] = "123456789";
    memcpy(container.getData(), testdata, 9);

    auto c2(container);
    CHECK_EQUAL(10, c2.getSize());
    CHECK(memcmp(testdata, c2.getData(), 9) == 0);
    CHECK(container.getData() != c2.getData());
    //CHECK(false);
}

TEST_INSUITE(DataContainerTests, ctor_with_base_container_type)
{
    RawDataContainer container(10);
    char testdata[] = "123456789";
    memcpy(container.getData(), testdata, 9);
    auto base = dynamic_cast<DataContainer*>(&container);
    RawDataContainer c2(*base);
    CHECK_EQUAL(10, c2.getSize());
    CHECK(memcmp(testdata, c2.getData(), 9) == 0);
    CHECK(container.getData() != c2.getData());
}

TEST_INSUITE(DataContainerTests, grow)
{
    RawDataContainer container(10);
    container.grow(5);
    CHECK_EQUAL(15, container.getSize());
    CHECK(container.getData() != nullptr);
}

TEST_INSUITE(DataContainerTests, grow_null)
{
    RawDataContainer container(10);
    container.grow(0);
    CHECK_EQUAL(10, container.getSize());
    CHECK(container.getData() != nullptr);
}

TEST_INSUITE(DataContainerTests, grow_very_big)
{
    RawDataContainer container(10);
    container.grow(-1);
    CHECK_EQUAL(10, container.getSize());
    CHECK(container.getData() != nullptr);
}

TEST_INSUITE(DataContainerTests, append)
{
    RawDataContainer container(5);
    char testdata[] = "1234567890";
    memcpy(container.getData(), testdata, 5);
    RawDataContainer container2(5);
    memcpy(container2.getData(), &testdata[5], 5);

    container.append(container2);
    CHECK_EQUAL(10, container.getSize());
    CHECK(container.getData() != nullptr);
    CHECK(memcmp(testdata, container.getData(), 10) == 0);
}

TEST_INSUITE(DataContainerTests, append_empty)
{
    RawDataContainer container(5);
    char testdata[] = "1234567890";
    memcpy(container.getData(), testdata, 5);
    RawDataContainer container2;

    container.append(container2);
    CHECK_EQUAL(5, container.getSize());
    CHECK(container.getData() != nullptr);
    CHECK(memcmp(testdata, container.getData(), 5) == 0);
}

TEST_INSUITE(DataContainerTests, resize_shrink)
{
    RawDataContainer container(10);
    char testdata[] = "1234567890";
    memcpy(container.getData(), testdata, 10);
    container.resize(5);
    CHECK_EQUAL(5, container.getSize());
    CHECK(container.getData() != nullptr);
    CHECK(memcmp(testdata, container.getData(), 5) == 0);
}

TEST_INSUITE(DataContainerTests, resize_same)
{
    RawDataContainer container(10);
    char testdata[] = "1234567890";
    memcpy(container.getData(), testdata, 10);
    container.resize(10);
    CHECK_EQUAL(10, container.getSize());
    CHECK(container.getData() != nullptr);
    CHECK(memcmp(testdata, container.getData(), 10) == 0);
}

TEST_INSUITE(DataContainerTests, resize_grow)
{
    RawDataContainer container(10);
    char testdata[] = "1234567890";
    memcpy(container.getData(), testdata, 10);
    container.resize(15);
    CHECK_EQUAL(15, container.getSize());
    CHECK(container.getData() != nullptr);
    CHECK(memcmp(testdata, container.getData(), 10) == 0);
}
