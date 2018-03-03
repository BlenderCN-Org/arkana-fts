/**
 * \file ScriptConsole.cpp
 * \author Klaus Beyer
 * \date 24 August 2010
 * \brief This file implements the scripting console which can be opened to for testing the Dao interface and/or
 * change value of the scripting environment or add something.
 *
 **/

#include "ScriptConsole.h"
#include "main/main.h"
#include "main/Exception.h"
#include "ui/ui.h"
#include "logging/logger.h"
#include "ui/cegui_items/simple_list_item.h"
#include "CEGUI.h"
#include "scripting/DaoVm.h"

namespace FTS {
    ScriptConsole::ScriptConsole(void)
        : m_pRoot(nullptr)
    {
        m_pRoot = GUI::getSingleton().loadLayout("script_console", true);
        if(m_pRoot == nullptr)
            throw ErrorAlreadyShownException();
        try {
            CEGUI::Window* pWin = CEGUI::WindowManager::getSingleton().getWindow("script_console/edInput");
            GUI::getSingleton().setActiveWidget(pWin);
            InputManager *pMgr = InputManager::getSingletonPtr();
            std::shared_ptr<ScriptConsoleHistory> pHistory(new ScriptConsoleHistory()) ;
            ScriptConsoleEnterCmd* pCmd = new ScriptConsoleEnterCmd("script_console/edInput",pHistory);
            pMgr->add("Script Enter", SpecialKey::Enter, pCmd);
            CommandBase* pCmdKeyArrowUp = new ScriptConsoleHistoryCmd("script_console/edInput", Key::ArrowUp, pHistory);
            pMgr->add("Script History prev", Key::ArrowUp, pCmdKeyArrowUp);
            CommandBase* pCmdKeyArrowDown = new ScriptConsoleHistoryCmd("script_console/edInput", Key::ArrowDown, pHistory);
            pMgr->add("Script History next", Key::ArrowDown, pCmdKeyArrowDown);
        } catch (CEGUI::Exception &ex) {
            FTS18N("CEGUI", MsgType::Error, ex.getMessage());
        }

    }


    ScriptConsole::~ScriptConsole(void)
    {
        try {
            CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
            InputManager *pMgr = InputManager::getSingletonPtr();
            pMgr->delShortcut("Script Enter");
            pMgr->delShortcut("Script History prev");
            pMgr->delShortcut("Script History next");
        } catch(CEGUI::Exception &) {
        }
    }

    bool OpenScriptConsoleCmd::exec()
    {
        ScriptConsole* console = new ScriptConsole();
        InputManager *pMgr = InputManager::getSingletonPtr();
        pMgr->delShortcut("Script Console");
        pMgr->add("Script Console", ScriptConsole::getHotKey(), new CloseScriptConsoleCmd(console), false);
        return true;
    }

    bool CloseScriptConsoleCmd::exec()
    {
        delete m_pScriptConsole;

        InputManager *pMgr = InputManager::getSingletonPtr();
        pMgr->delShortcut("Script Console");
        pMgr->add("Script Console", ScriptConsole::getHotKey(), new OpenScriptConsoleCmd(), false);

        return true;
    }

    ScriptConsoleEnterCmd::ScriptConsoleEnterCmd(const String& in_controlName, std::shared_ptr<ScriptConsoleHistory> in_pHistory)
        : m_controlName(in_controlName)
        , m_pHistory(in_pHistory)
    {
        DaoVm::getSingleton().clearOutput();
    };

    bool ScriptConsoleEnterCmd::exec()
    {
        String sMessage;

        try {
            // Get the content of the specified window.
            CEGUI::Window *pW = CEGUI::WindowManager::getSingleton()
                .getWindow(m_controlName);
            sMessage = pW->getText();
            // Clear it if needed and give it the focus.
            pW->setText("");
            pW->activate();
            FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "script_console/lbOutput");
            (new SimpleListItem("> " + sMessage))->addAsLast(pLB);
            if( hasContinues(sMessage) ) {
                m_cmdLines.push_back(sMessage.trimRight("..."));
                return true;
            }
            String statement;
            for( auto i = m_cmdLines.cbegin() ; i != m_cmdLines.cend() ; ++i ) {
                statement += *i  + "\n";
            }
            statement += sMessage + "\n";
            m_pHistory->add(statement);
            DaoVm::getSingleton().execute(statement);
            m_cmdLines.clear();
            String output = DaoVm::getSingleton().getOutput();
            if( !output.empty() ) {
                (new SimpleListItem(output))->addAsLast(pLB);
                DaoVm::getSingleton().clearOutput();
            }
            return true;
        } catch(CEGUI::Exception & e) {
            FTS18N("CEGUI", MsgType::Error, e.getMessage());
            return false;
        } catch( ScriptEvalException & e) {
            FTSGetConvertWinMacro(CEGUI::Listbox, pLB, "script_console/lbOutput");
            (new SimpleListItem(String("> ") + String(e.what())))->addAsLast(pLB);
            String output = DaoVm::getSingleton().getOutput();
            if( !output.empty() ) {
                (new SimpleListItem(output))->addAsLast(pLB);
                DaoVm::getSingleton().clearOutput();
            }
            return true;
        }
    }
    bool ScriptConsoleEnterCmd::hasContinues(const String& sMessage)
    {
        return sMessage.matchesPattern("*...");
    }
    ScriptConsoleHistoryCmd::ScriptConsoleHistoryCmd(const FTS::String& in_controlName, const Key::Enum in_key, std::shared_ptr<ScriptConsoleHistory> in_pHistory)
    : m_controlName(in_controlName)
    , m_pHistory(in_pHistory)
    , m_key(in_key)
    {
    }
    bool ScriptConsoleHistoryCmd::exec()
    {
        // Get the content of the specified window.
        CEGUI::Window *pW = CEGUI::WindowManager::getSingleton()
        .getWindow(m_controlName);
        String text;
        if(m_key == Key::ArrowUp) {
            text = m_pHistory->previous();
        } else {
            text = m_pHistory->next();
        }
        pW->setText(text);
        return true;
    }


    ScriptConsoleHistory::ScriptConsoleHistory()
        : m_position(0)
    {
        m_history.push_back(String::EMPTY);
    }

    void ScriptConsoleHistory::add( const String& in_cmdLine )
    {
        String toStore = in_cmdLine.trimRight(String("\n"));
        if( toStore.empty() ) {
            return;
        }
        m_history.push_back(toStore);
        m_position = m_history.size() ;
    }

    FTS::String ScriptConsoleHistory::next()
    {
        ++m_position;
        if( m_position >= m_history.size() ) {
            m_position = 0 ;
        }
        return m_history[m_position];
    }

    FTS::String ScriptConsoleHistory::previous()
    {
        --m_position;
        if( m_position < 0 ) {
            m_position = m_history.size() - 1 ;
        }
        return m_history[m_position];

    }

}