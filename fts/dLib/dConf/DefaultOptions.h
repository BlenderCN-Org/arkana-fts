#pragma once
#include "dLib/dString/dString.h"
#include <map>

namespace FTS {
    typedef std::map<String, String> Options;
    
    class DefaultOptions
    {
    public:
        virtual ~DefaultOptions(void);
        const Options& getDefaults() const {return m_defOpt; }
        void add(String name, String value) ;
        void add(String name, const char * value) ;
        void add(String name, int value) ;
        void add(String name, bool value) ;
        void add(String name, float value) ;
        
    protected:
        DefaultOptions();
    private :
        Options m_defOpt;
    public:
    };
}