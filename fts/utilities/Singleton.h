#ifndef D_SINGLETON_H
#define D_SINGLETON_H

#include "main/main.h" // Only for nullptr workaround...
#include "main/Exception.h"
#include "utilities/NonCopyable.h"

#include <memory>

#ifdef __GNUC__
#  define SINGLETON_NAME __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && __STDC_VERSION__  >= 19901L
#  define SINGLETON_NAME __func__
#else
#  define SINGLETON_NAME __FUNCTION__
#endif

namespace FTS {

class SingletonNotExistException : public NotExistException {
public:
    SingletonNotExistException(const String& in_sClassName) throw();
};

class SingletonAlreadyExistException : public AlreadyExistException {
public:
    SingletonAlreadyExistException(const String& in_sClassName) throw();
};

/** Really nice singleton base-class originally written by Andrew Dai,
 *  Copyright (C) 2004 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *  GNU GPLv2 licensed.
 *
 */
template <typename T>
class Singleton : public NonCopyable {
public:
    virtual ~Singleton()
    {
        m_pSingleton = nullptr;
    }

    static T& getSingleton()
    {
        if(m_pSingleton == nullptr)
            throw SingletonNotExistException(__FUNCTION__);

        return *m_pSingleton;
    }

    static T* getSingletonPtr()
    {
        return m_pSingleton;
    }

protected:
    Singleton(T* in_p)
    {
        if(m_pSingleton)
            throw SingletonAlreadyExistException(__FUNCTION__);

        m_pSingleton = in_p;
    }

    // Use this constructor only when the derived class is only deriving from Singleton
    Singleton()
    {
        if(m_pSingleton)
            throw SingletonAlreadyExistException(__FUNCTION__);

        m_pSingleton = (T*) (this);
    }

private:
    /// The singleton object itself.
    static T* m_pSingleton;
};

template <typename T>
T* Singleton<T>::m_pSingleton = nullptr;

/// Lazy-Singleton class: A singleton that does lazy-initializes and destroys
/// itself. That means you don't need to create it with new, neither do you
/// need to delete it.
///
/// To use this class, inherit from it publicly.
/// \note This class does not <b>disallow</b> direct construction or deletition
///       as you might still want to do that. If you directly initialize it for
///       example, it will not re-initialize it later.
template <typename U>
class LazySingleton : public NonCopyable {
public:
    static U& getSingleton()
    {
        if(m_pSingleton.get() == nullptr)
            m_pSingleton.reset(new U());

        return *m_pSingleton;
    }

    static U* getSingletonPtr()
    {
        if(!m_pSingleton)
            m_pSingleton.reset(new U());

        return m_pSingleton.get();
    }

    typedef std::unique_ptr<U> Ptr;

protected:
    LazySingleton() {
    };
    virtual ~LazySingleton() {
        // Would cause infinite recursion:
        //if(m_pSingleton)
        //    m_pSingleton.reset();
    };

private:
    /// The singleton object itself.
    static std::unique_ptr<U> m_pSingleton;

    friend class std::unique_ptr<U>;
};

template <typename U>
std::unique_ptr<U> LazySingleton<U>::m_pSingleton;

}; // namespace FTS

#endif // D_SINGLETON_H
