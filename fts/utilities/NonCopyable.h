#ifndef D_NONCOPYABLE_H
#define D_NONCOPYABLE_H

namespace FTS {

/// Inherit (publicly) from this class whenever you don't want your class to
/// ever be copied using the copy constructor or assignment operator.
/// \see PolymorphicCopyable
class NonCopyable
{
private:
    NonCopyable(const NonCopyable&);
    const NonCopyable& operator=(const NonCopyable&);
protected:
    NonCopyable() {};
    virtual ~NonCopyable() {};
};

}

#endif // D_NONCOPYABLE_H
