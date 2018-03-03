#ifndef D_INGREDIENTS_H
#define D_INGREDIENTS_H

#include <ostream>

class Ingredient {
public:
    Ingredient() {}
    virtual ~Ingredient() {}
    virtual bool operator==(const Ingredient& o) const {return true;}
};

std::ostream& operator<< (std::ostream& os, const Ingredient& me);

class Cheese : public Ingredient {
    float m_fMilk;
public:
    Cheese(float fPercentMilk)
        : m_fMilk(fPercentMilk)
    {
        if(m_fMilk < 0.5f) {
            throw "not enough milk! At least get 50% milk!";
        }
        if(m_fMilk > 1.0f) {
            throw "howdy! more then 100% milk in your cheese!?";
        }
    }

    virtual ~Cheese() {}

    float getMilk() const {return m_fMilk;}
    virtual bool operator==(const Ingredient& o) const {
        const Cheese* pc = dynamic_cast<const Cheese*>(&o);
        if(!pc)
            return false;
        return Ingredient::operator==(o) && m_fMilk == pc->m_fMilk;
    }
};

std::ostream& operator<< (std::ostream& os, const Cheese& me);

#endif // D_INGREDIENTS_H
