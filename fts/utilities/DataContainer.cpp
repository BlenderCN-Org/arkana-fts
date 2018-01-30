/**
 * \file DataContainer.cpp
 * \author Pompei2
 * \date 01 May 2006
 * \brief This file contains the implementation of a raw data container.
 **/

#include <cstring>
#include <malloc.h>

#include "DataContainer.h"

using namespace FTS;

/// \return The fletcher32 checksum of the data in the container.
uint32_t FTS::DataContainer::fletcher32() const
{
    // Taken from wikipedia. Input only works on an array of uint16_t, thus
    // if our data has one byte too much, cut it off.

    // 0 if the length is pair (exactly enough bytes), 1 if we are missing 1 byte.
    uint64_t uiToCompare = this->getSize() % 2;

    uint64_t uiDataLen16Bit = this->getSize()/2;
    const uint16_t *pData16Bit = reinterpret_cast<const uint16_t*>(this->getData());

    uint32_t sum1 = 0xffff, sum2 = 0xffff;

    while(uiDataLen16Bit > uiToCompare) {
            uint64_t tlen = uiDataLen16Bit > 360 ? 360 : uiDataLen16Bit;
            uiDataLen16Bit -= tlen;
            do {
                    sum1 += *pData16Bit++;
                    sum2 += sum1;
            } while (--tlen);
            sum1 = (sum1 & 0xffff) + (sum1 >> 16);
            sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    // If we are missing one byte, add a zero.
    if(uiToCompare > 0) {
        sum1 += *(reinterpret_cast<const uint8_t*>(pData16Bit));
        sum2 += sum1;
        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    // Second reduction step to reduce sums to 16 bits.
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return sum2 << 16 | sum1;
}

FTS::RawDataContainer::RawDataContainer()
{
}

FTS::RawDataContainer::RawDataContainer(uint64_t in_uiSize)
    : m_uiSize(in_uiSize)
{
    if(in_uiSize == 0) {
        return;
    }
    m_pData = reinterpret_cast<uint8_t *>(malloc(in_uiSize + 1));
    memset(m_pData, 0, in_uiSize + 1);
}

FTS::RawDataContainer::RawDataContainer(const DataContainer &o)
{
    m_uiSize = o.getSize();
    m_pData = reinterpret_cast<uint8_t *>(malloc(o.getSize()+1));
    memcpy(m_pData, o.getData(), o.getSize());
    m_pData[m_uiSize] = 0;
}

FTS::RawDataContainer::RawDataContainer(const RawDataContainer &o)
{
    m_uiSize = o.getSize();
    m_pData = reinterpret_cast<uint8_t *>(malloc(o.getSize() + 1));
    memcpy(m_pData, o.getData(), o.getSize());
    m_pData[m_uiSize] = 0;
}

FTS::RawDataContainer::~RawDataContainer()
{
    delete m_pData;
    m_uiSize = 0;
}

uint8_t *FTS::RawDataContainer::getData()
{
    return m_pData;
}

const uint8_t *FTS::RawDataContainer::getData() const
{
    return m_pData;
}

uint64_t FTS::RawDataContainer::getSize() const
{
    return m_uiSize;
}

void FTS::RawDataContainer::grow(uint64_t in_uiAdditional)
{
    this->resize(this->getSize() + in_uiAdditional);
}

void FTS::RawDataContainer::resize(uint64_t in_uiNewSize)
{
    auto newBlock = reinterpret_cast<uint8_t *>(realloc(m_pData, in_uiNewSize+1));
    if(newBlock != nullptr) {
        m_pData = newBlock;
        if(in_uiNewSize > m_uiSize)
            memset(m_pData + m_uiSize, 0, in_uiNewSize - m_uiSize);
        m_uiSize = in_uiNewSize;
    } else {
        this->destroy();
    }
}

void FTS::RawDataContainer::destroy()
{
    delete m_pData;
    m_pData = nullptr;
    m_uiSize = 0;
}

RawDataContainer *FTS::RawDataContainer::copy() const
{
    RawDataContainer *pRet = new RawDataContainer(this->getSize());
    memcpy(pRet->getData(), this->getData(), this->getSize());
    return pRet;
}

void FTS::RawDataContainer::append(const ConstRawDataContainer& in_other)
{
    uint64_t oldSize = this->getSize();
    this->resize(oldSize + in_other.getSize());
    memcpy(this->getData() + oldSize, in_other.getData(), in_other.getSize());
}

FTS::ConstRawDataContainer::ConstRawDataContainer() : m_pData(nullptr), m_uiSize(0)
{
}

FTS::ConstRawDataContainer::ConstRawDataContainer(const DataContainer &in_o)
    : m_pData(in_o.getData())
    , m_uiSize(in_o.getSize())
{
}

FTS::ConstRawDataContainer::ConstRawDataContainer(const uint8_t *in_pData, uint64_t in_uiSize)
    : m_pData(in_pData)
    , m_uiSize(in_uiSize)
{
}

FTS::ConstRawDataContainer::ConstRawDataContainer(const void *in_pData, uint64_t in_uiSize)
    : m_pData(reinterpret_cast<const uint8_t *>(in_pData))
    , m_uiSize(in_uiSize)
{
}

FTS::ConstRawDataContainer::~ConstRawDataContainer()
{
}

const uint8_t *FTS::ConstRawDataContainer::getData() const
{
    return m_pData;
}

uint64_t FTS::ConstRawDataContainer::getSize() const
{
    return m_uiSize;
}

DataContainer *FTS::ConstRawDataContainer::copy() const
{
    return new ConstRawDataContainer(this->getData(), this->getSize());
}

void FTS::toSystemEndian(uint8_t *out_pBuff, uint64_t in_uiSize)
{
    if(!systemHasGoodEndian()) {
        // Rather use htons and htonl?

        // Swap the bytes around if needed.
        switch(in_uiSize) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            std::swap(out_pBuff[0], out_pBuff[1]);
            break;
        case 4:
            std::swap(out_pBuff[0], out_pBuff[3]);
            std::swap(out_pBuff[1], out_pBuff[2]);
            break;
        default:
            // Swap the bytes around (123 -> 321)
            for(size_t i = 0 ; i < in_uiSize / 2 ; i++) {
                std::swap(out_pBuff[i], out_pBuff[in_uiSize-i-1]);
            }
        }
    }
}

bool FTS::systemHasGoodEndian()
{
    // Note: GCC evaluates all this at compile-time and skips branches
    //       using this optimization. If other compilers (VC++) don't and this
    //       really becomes a bottleneck, take a look at boost/detail/endian.hpp
    union {
        uint32_t i;
        char c[4];
    } blabla = {0x01020304};

    return !(blabla.c[0] == 1);
}
