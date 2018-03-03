
#ifndef MYTESTCLASS_H
#define MYTESTCLASS_H

#include <ostream>

class MyTestClass
{
    friend std::ostream& operator<< (std::ostream&, const MyTestClass&);
    int s_myOwnVar;
public:
    MyTestClass();
    MyTestClass(int i);
    ~MyTestClass();

	bool UseBadPointer() const;
    bool DivideByZero() const;
    void ThrowException() const;
    void ThrowUserException() const;

    // My requirements for easy testing:
    bool operator==(const MyTestClass&) const;

    static int s_currentInstances;
    static int s_instancesCreated;
    static int s_maxSimultaneousInstances;
};

std::ostream& operator<< (std::ostream&, const MyTestClass&);

#endif
