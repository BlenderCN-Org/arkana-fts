#ifndef D_SAUCE_H
#define D_SAUCE_H

#include <ostream>

class Sauce {
    double m_liters;

public:
    Sauce(double liters)
        : m_liters(liters)
    { }

    virtual ~Sauce() {};

    inline double getVolume() const {return m_liters;}
    virtual bool operator==(const Sauce& o) const {return m_liters == o.m_liters;}
};

std::ostream& operator<< (std::ostream& os, const Sauce& me);

class TomatoeSauce : public Sauce {
public:
    TomatoeSauce(double liters)
        : Sauce(liters)
    { }

    virtual ~TomatoeSauce() {};
};

namespace ChiliHotness {
    enum Enum {
        European = 0,
        Hot,
        VeryHot,
    };
};

class HotChiliSauce : public Sauce {
    ChiliHotness::Enum m_hotness;
public:
    HotChiliSauce(const ChiliHotness::Enum& hot, double liters)
        : Sauce(liters)
        , m_hotness(hot)
    { }

    virtual ~HotChiliSauce() {};

    ChiliHotness::Enum getHotness() const {return m_hotness;}

    // Note that the operator== from derived classes needs to have the same
    // signature as the operator== of its base-class.
    virtual bool operator==(const Sauce& o) const {
        const HotChiliSauce* ps = dynamic_cast<const HotChiliSauce*>(&o);
        if(!ps)
            return false;
        return Sauce::operator==(o) && m_hotness == ps->getHotness();
    }
};

std::ostream& operator<< (std::ostream& os, const HotChiliSauce& me);

#endif // D_SAUCE_H
