#ifndef D_DOUGH_H
#define D_DOUGH_H

#include <ostream>

namespace PeetsaSize {
    enum Enum {
        Big,
        Medium,
        Kid,
    };
}

class Dough {
    PeetsaSize::Enum m_size;

public:
    Dough(const PeetsaSize::Enum& size)
        : m_size(size)
    { }

    virtual ~Dough() {};

    inline PeetsaSize::Enum getSize() const {return m_size;}
    virtual bool operator==(const Dough& o) const {return m_size == o.m_size;}
};

std::ostream& operator<< (std::ostream& os, const Dough& me);

class ThinItalianDough : public Dough {
public:
    ThinItalianDough(const PeetsaSize::Enum& size)
        : Dough(size)
    { }

    virtual ~ThinItalianDough() {};
};

class PizzaHutDough : public Dough {
    unsigned m_calories;
public:
    PizzaHutDough(const PeetsaSize::Enum& size, unsigned calories)
        : Dough(size)
        , m_calories(calories)
    { }

    virtual ~PizzaHutDough() {};

    unsigned getCalories() const {return m_calories;}

    // Note that the operator== from derived classes needs to have the same
    // signature as the operator== of its base-class.
    virtual bool operator==(const Dough& o) const {
        const PizzaHutDough* ph = dynamic_cast<const PizzaHutDough*>(&o);
        if(!ph)
            return false;
        return Dough::operator==(o) && m_calories == ph->m_calories;
    }
};

std::ostream& operator<< (std::ostream& os, const PizzaHutDough& me);

#endif // D_DOUGH_H
