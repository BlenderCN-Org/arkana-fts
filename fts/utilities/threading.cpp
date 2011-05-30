/**
 * \file threading.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file implements everything that has to do with threading.
 **/

#include "threading.h"

#if WINDOOF
#  include <windows.h>
#else
#  include <pthread.h>
#endif

using namespace FTS;

FTS::Mutex::Mutex()
{
#if WINDOOF
    m_pImpl = new HANDLE;
    *((HANDLE*)m_pImpl) = CreateMutex(NULL, FALSE, NULL);
#else
    m_pImpl = new pthread_mutex_t;
    pthread_mutex_init((pthread_mutex_t*)m_pImpl, NULL);
#endif
}

FTS::Mutex::~Mutex()
{
#if WINDOOF
    HANDLE* pImpl = reinterpret_cast<HANDLE*>(m_pImpl);
    CloseHandle(*pImpl);
#else
    pthread_mutex_t* pImpl = reinterpret_cast<pthread_mutex_t*>(m_pImpl);
    pthread_mutex_destroy(pImpl);
#endif
    SAFE_DELETE(pImpl);
    m_pImpl = NULL;
}

int FTS::Mutex::lock()
{
    int iRet = ERR_OK;

#if WINDOOF
    if(WAIT_OBJECT_0 != WaitForSingleObject(*((HANDLE*)m_pImpl), INFINITE))
        iRet = -1;
#else
    if(0 != pthread_mutex_lock((pthread_mutex_t*)m_pImpl))
        iRet = -1;
#endif

    return iRet;
}

int FTS::Mutex::unlock()
{
    int iRet = ERR_OK;

#if WINDOOF
    if(0 == ReleaseMutex(*((HANDLE*)m_pImpl)))
        iRet = -1;
#else
    if(0 != pthread_mutex_unlock((pthread_mutex_t*)m_pImpl))
        iRet = -1;
#endif

    return iRet;
}

FTS::Lock::Lock(Mutex& in_m)
    : m(in_m)
{
    m.lock();
}

FTS::Lock::~Lock()
{
    m.unlock();
}

 /* EOF */
