#include "dLib/dConf/DefaultOptions.h"

namespace FTS {
DefaultOptions::DefaultOptions(void)
{
}


DefaultOptions::~DefaultOptions(void)
{
}

void DefaultOptions::add(String name, String value) 
{
    m_defOpt[name] = value;
}

void DefaultOptions::add(String name, const char * value) 
{
    m_defOpt[name] = value;
}

void DefaultOptions::add(String name, int value) 
{
    add(name, String::nr(value));
}
void DefaultOptions::add(String name, bool value) 
{
    add(name, String::b(value));
}
void DefaultOptions::add(String name, float value) 
{
    add(name, String::nr(value));
}

}