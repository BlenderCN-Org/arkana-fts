#ifndef D_MAIN_H
#define D_MAIN_H

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

#endif

