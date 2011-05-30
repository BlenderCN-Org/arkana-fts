#pragma once
#include <dao.h>
#include "input/InputConstants.h"
#include "input/input.h"

class IHotkey 
{
public:
    virtual int exec() = 0 ;
};

class Hotkey 
{
public:
    Hotkey( int k, const char * function); 
    Hotkey( int k, IHotkey* obj, const char * function); 
    Hotkey( int k, IHotkey* obj); 
    void addModifier(Hotkey * obj) ;
    void remove() ;
private:
    FTS::InputCombo * m_pInputCombo;
    FTS::CommandBase* m_pCmd;
};

bool isKeyPressed(int k);
bool isMousePressed(int k);
unsigned int mouseX();
unsigned int mouseY();
