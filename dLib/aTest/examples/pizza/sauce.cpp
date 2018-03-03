#include "sauce.h"

std::ostream& operator<< (std::ostream& os, const Sauce& me)
{
    return os << me.getVolume() << " liters of sauce";
}

std::ostream& operator<< (std::ostream& os, const HotChiliSauce& me)
{
    switch(me.getHotness()) {
    case ChiliHotness::European: return os << dynamic_cast<const Sauce&>(me) << " with sweet chilis";
    case ChiliHotness::Hot: return os << dynamic_cast<const Sauce&>(me) << ", quite hot";
    case ChiliHotness::VeryHot: return os << dynamic_cast<const Sauce&>(me) << ", very hot!";
    }

    return os;
}

