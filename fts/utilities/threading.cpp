/**
 * \file threading.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements everything that has to do with threading.
 **/

#include "threading.h"

FTS::Mutex::Mutex()
{
}

FTS::Mutex::~Mutex()
{
}

void FTS::Mutex::lock()
{
    m_mtx.lock();
}

void FTS::Mutex::unlock()
{
    m_mtx.unlock();
}

FTS::Lock::Lock(FTS::Mutex& in_m)
    : m(in_m)
{
    m.lock();
}

FTS::Lock::~Lock()
{
    m.unlock();
}

 /* EOF */
