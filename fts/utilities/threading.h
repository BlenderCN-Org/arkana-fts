/**
 * \file threading.cpp
 * \author Pompei2
 * \date unknown (very old)
 * \brief This file declares everything that has to do with threading.
 **/

#ifndef D_THREADING_H
#define D_THREADING_H

#ifdef D_COMPILES_SERVER
#  include "server.h"
#else
#  include "main.h"
#endif

#if WINDOOF
/// TODO: implement a real thread function that is multi os.
#  define D_THREADFCT void
#  define D_THREADRET
#  define d_threadStart( fct, arg ) { beginthread( (fct), 0, (arg) ); }

#else
/// TODO: implement a real thread function that is multi os.
#  include <pthread.h>
#  define D_THREADFCT void *
#  define D_THREADRET NULL
#  define d_threadStart( fct, arg ) { pthread_t _a_b_thread1_c_; pthread_create( &_a_b_thread1_c_, NULL, (fct), (arg) ); }
#endif

namespace FTS {

class Lock;

class Mutex {
    friend class Lock;
protected:
    void *m_pImpl;

    int lock();
    int unlock();
public:
    Mutex();
    virtual ~Mutex();
};

class Lock {
protected:
    Mutex& m;
public:
    Lock(Mutex& m);
    ~Lock();
};

} // namespace FTS

#endif /* D_THREADING_H */

 /* EOF */
