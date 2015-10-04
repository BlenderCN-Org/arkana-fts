#include <dao.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hotkey.h"
#include "DaoFunction.h"
#include "dao_snd.h"
#include "DaoVm.h"
#include "utilities/command.h"
#include "input/input.h"
#include "dLib/dString/dString.h"


int IHotkey::exec()
{
    throw "int IHotkey::getVar() / Not implemented. Subclass has to override";
}

namespace KeyType {
    enum Enum {
        Key, SpecialKey, MouseButton, MouseScroll, NoKeyType
    };
};

KeyType::Enum getKeyType(int k)
{
    if( k < FTS::Key::Last ) {
        return KeyType::Key;
    } else if ( k < FTS::SpecialKey::NoSpecial ) {
        return KeyType::SpecialKey;
    } else if ( k < FTS::MouseButton::NoButton) {
        return KeyType::MouseButton;
    } else if ( k < FTS::MouseScroll::NoScroll ) {
        return KeyType::MouseScroll;
    } else {
        return KeyType::NoKeyType;
    }

}

FTS::InputCombo* createInputCombo( const FTS::String& name, int key, FTS::CommandBase* pCmd)
{
    switch( getKeyType(key) ) {
    case KeyType::Key:
        return new FTS::InputCombo(name, (FTS::Key::Enum)key, pCmd);
    case KeyType::SpecialKey:
        return new FTS::InputCombo(name, (FTS::SpecialKey::Enum)key, pCmd);
    case KeyType::MouseButton:
        return new FTS::InputCombo(name, (FTS::MouseButton::Enum)key, pCmd);
    case KeyType::MouseScroll:
        return new FTS::InputCombo(name, (FTS::MouseButton::Enum)key, pCmd);
    default:
        assert(false); // can't be here
        return nullptr;
    }
}
class DaoCmdFunction : public FTS::CommandBase
{
public:
    DaoCmdFunction( const char * func)
    {
        m_funcName = func;
    }
    bool exec()
    {
        FTS::DaoFunctionCall<> f(m_funcName);
        f();
        return true;
    }
private:
    FTS::String m_funcName;
};

class DaoCmdMethod : public FTS::CommandBase
{
public:
    DaoCmdMethod(IHotkey* obj, const char * func)
    {
        m_funcName = func;
        m_obj = obj;
    }
    bool exec()
    {
        FTS::DaoVm::getSingleton().callObjectMethod(((DaoCxx_IHotkey*)m_obj)->cdata, m_funcName.c_str()) ;
        return true;
    }
private:
    FTS::String m_funcName;
    IHotkey * m_obj;
};

class DaoCommand : public FTS::CommandBase
{
public:
    DaoCommand( IHotkey* obj)
    {
        m_obj = obj;
    }
    bool exec()
    {
        m_obj->exec();
        return true;
    }
private:
    IHotkey * m_obj;
};

Hotkey::Hotkey(int k, const char * function) 
{
    m_pCmd = new DaoCmdFunction(function);
    m_pInputCombo = createInputCombo("Dao/" + FTS::String::nr((uint64_t)this), k, m_pCmd);
    FTS::InputManager::getSingleton().add(m_pInputCombo);
}
Hotkey::Hotkey( int k, IHotkey* obj, const char * function)
{
    m_pCmd = new DaoCmdMethod(obj, function);
    m_pInputCombo = createInputCombo("Dao/" + FTS::String::nr((uint64_t)this), k, m_pCmd);
    FTS::InputManager::getSingleton().add(m_pInputCombo);
}
Hotkey::Hotkey( int k, IHotkey* obj)
{
    m_pCmd = new DaoCommand(obj);
    m_pInputCombo = createInputCombo("Dao/" + FTS::String::nr((uint64_t)this), k, m_pCmd);
    FTS::InputManager::getSingleton().add(m_pInputCombo);
}

void Hotkey::addModifier(Hotkey * obj) 
{
    FTS::InputManager::getSingleton().detachShortcut(obj->m_pInputCombo->getName());
    m_pInputCombo->addModifier(obj->m_pInputCombo);
}

void Hotkey::remove() 
{
    FTS::InputManager::getSingleton().delShortcut(m_pInputCombo->getName());
}

bool isKeyPressed(int k)
{
    KeyType::Enum kt = getKeyType(k);
    if( kt == KeyType::Key ) 
        return FTS::InputManager::getSingleton().isKeyPressed((FTS::Key::Enum)k);
    else 
        return FTS::InputManager::getSingleton().isKeyPressed((FTS::SpecialKey::Enum)k);
}
bool isMousePressed(int k)
{
    return FTS::InputManager::getSingleton().isMousePressed((FTS::MouseButton::Enum)k);
}
unsigned int mouseX()
{
    return FTS::InputManager::getSingleton().getMouseX();
}
unsigned int mouseY()
{
    return FTS::InputManager::getSingleton().getMouseY();
}
