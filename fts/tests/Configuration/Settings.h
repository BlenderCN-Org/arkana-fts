#pragma once
#include "dLib/dConf/DefaultOptions.h"

namespace FTS {
    class Settings : public DefaultOptions
    {
    public:
        Settings(const String& in_DefaultLang);
        ~Settings(void);
    };

}