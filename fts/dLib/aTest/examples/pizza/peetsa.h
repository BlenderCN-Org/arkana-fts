#ifndef D_PEETSA_H
#define D_PEETSA_H

#include <exception>
#include <list>

#include "dough.h"
#include "sauce.h"
#include "ingredients.h"

class Peetsa {
    Dough* m_dough;
    Sauce* m_sauce;
    std::list<Ingredient*> m_ingredients;

public:
    Peetsa(Dough *d) : m_dough(d), m_sauce(NULL) {
        if(m_dough == NULL)
            throw std::exception();
    }

    virtual ~Peetsa() {
        if(m_dough)
            delete m_dough;
        if(m_sauce)
            delete m_sauce;
        while(this->countIngredients() > 0) {
            delete m_ingredients.front();
            m_ingredients.pop_front();
        }
    }
    Dough& getDough() {return *m_dough;}

    Peetsa& addSauce(Sauce* s) {
        if(!s)
            throw "no sauce!";
        if(m_sauce)
            delete m_sauce;
        m_sauce = s;
        return *this;
    }
    Sauce& getSauce() {if(!m_sauce) throw "no sauce!"; return *m_sauce;}

    Peetsa& addIngredient(Ingredient* i) {
        if(!i)
            throw "no ingredient!";
        m_ingredients.push_back(i);
    }
    size_t countIngredients() {return m_ingredients.size();}
};

#endif // D_PEETSA_H
