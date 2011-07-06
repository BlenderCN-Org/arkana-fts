#ifndef D_WORKAROUNDS_H
#define D_WORKAROUNDS_H

////////////////////////////////////////////////////////////////////////////////
// Adding nullptr to older versions of GCC
////////////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ == 5)

class nullptr_t {
public:
    template<class T>        // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...

    template<class C, class T> // or any type of null
    operator T C::*() const    // member pointer...
    { return 0; }

private:
    void operator&() const;    // whose address can't be taken
};

const nullptr_t nullptr = {};

template<class T>
bool operator == (T t, nullptr_t) { return !t; }
template<class T>
bool operator == (nullptr_t, T t) { return !t; }
template<class T>
bool operator != (T t, nullptr_t) { return !!t; }
template<class T>
bool operator != (nullptr_t, T t) { return !!t; }

#endif // GCC 4.5 nullptr workaround

#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 5)
const                        // this is a const object...
class {
public:
    template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...

    template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }

private:
    void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif // GCC < 4.5 nullptr workaround

////////////////////////////////////////////////////////////////////////////////
// Letting KDevelop understand std::shared_ptr and other C++0x stuff.
////////////////////////////////////////////////////////////////////////////////
#ifdef IN_KDEVELOP_PARSER

#  define __GXX_EXPERIMENTAL_CXX0X__
#  define _GLIBCXX_BEGIN_NAMESPACE(x) namespace x {
#  define _GLIBCXX_END_NAMESPACE }

#  include <memory>

struct Foo {
    void doIt() {};
};

shared_ptr<Foo> pFoo1;

std::shared_ptr<Foo> pFoo;
pFoo->doIt();
// typedef

#endif // IN_KDEVELOP_PARSER

#endif // D_WORKAROUNDS_H
