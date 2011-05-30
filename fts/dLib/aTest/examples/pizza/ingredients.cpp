#include "ingredients.h"

std::ostream& operator<< (std::ostream& os, const Ingredient& me)
{
    return os << "I'm an igredient, but what?";
}

std::ostream& operator<< (std::ostream& os, const Cheese& me)
{
    return os << "Some good cheese with " << me.getMilk()*100.0 << "% milk";
}

