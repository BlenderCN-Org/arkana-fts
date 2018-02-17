#ifndef D_POLYMORPHICCOPYABLE_H
#define D_POLYMORPHICCOPYABLE_H

#include "utilities/NonCopyable.h"

namespace FTS {

/// Inherit (publicly) from this class whenever you want your class to have a
/// copy method. This is
/// \see NonCopyable
class PolymorphicCopyable : public NonCopyable
{
protected:
    PolymorphicCopyable() {};
    virtual ~PolymorphicCopyable() {};
public:
    /// Inheriters should overwrite this method and implement it to return a
    /// copy of their own instance.
    virtual PolymorphicCopyable* copy() const = 0;
};

}

#endif // D_POLYMORPHICCOPYABLE_H
