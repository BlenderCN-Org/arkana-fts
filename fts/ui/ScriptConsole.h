/**
 * \author kby
 * \date 22 August 2010
 * \brief This is the Script console interface file.
 **/

#ifndef FTS_SCRIPTINGCONSOLE_H
#define FTS_SCRIPTINGCONSOLE_H

#include "utilities/command.h"
#include "input/InputConstants.h"
#include "dLib/dString/dString.h"
#include <vector>
#include <memory>

namespace CEGUI {
    class Window;
}

namespace FTS {
    
    class ScriptConsole
    {
    public:
        ScriptConsole();
        virtual ~ScriptConsole(void);
        static Key::Enum getHotKey() { return Key::F9; }
    private:
        CEGUI::Window* m_pRoot;
    };

    class OpenScriptConsoleCmd : public CommandBase {
    public:
        OpenScriptConsoleCmd() {};
        bool exec() ;
    };

    class CloseScriptConsoleCmd : public CommandBase {
    public:
        CloseScriptConsoleCmd(ScriptConsole* in_scriptConsole) : m_pScriptConsole(in_scriptConsole) {};
        bool exec() ;
    private:
        ScriptConsole* m_pScriptConsole;
    };
    class ScriptConsoleHistoryCmd;
    class ScriptConsoleHistory;
    class ScriptConsoleEnterCmd : public CommandBase {
    public:
        ScriptConsoleEnterCmd(const String& in_controlName, std::shared_ptr<ScriptConsoleHistory> in_pHistory);
        ~ScriptConsoleEnterCmd() {
            m_pHistory.reset();
        }
        bool exec() ;
    private:
        bool hasContinues(const String& sMessage);
        String m_controlName;
        std::vector<String> m_cmdLines;
        std::shared_ptr<ScriptConsoleHistory> m_pHistory;
    };

    class ScriptConsoleHistoryCmd : public CommandBase {
    public:
        ScriptConsoleHistoryCmd(const String& in_controlName, const Key::Enum in_key, std::shared_ptr<ScriptConsoleHistory> in_pHistory);
        ~ScriptConsoleHistoryCmd() {
            m_pHistory.reset();
        }
        bool exec() ;
    private:
        String m_controlName;
        std::shared_ptr<ScriptConsoleHistory> m_pHistory;
        Key::Enum m_key;
    };

    class ScriptConsoleHistory {
    public:
        ScriptConsoleHistory();
        void add(const String& in_cmdLine);
        String next();
        String previous();
    private:
        std::vector<String> m_history;
        int m_position;

    };
}

#endif
