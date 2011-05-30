#include "dough.h"

std::ostream& operator<< (std::ostream& os, const Dough& me)
{
    switch(me.getSize()) {
    case PeetsaSize::Big: return os << "Big pizza dough";
    case PeetsaSize::Medium: return os << "Medium pizza dough";
    case PeetsaSize::Kid: return os << "Pizza dough for kiddies";
    }

    return os;
}

std::ostream& operator<< (std::ostream& os, const PizzaHutDough& me)
{
    return os << dynamic_cast<const Dough&>(me) << " with " << me.getCalories() << " calories";
}

