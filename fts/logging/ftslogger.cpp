#include "main/main.h"
#include <CEGUI.h>

#include "logging/ftslogger.h"
#include "ui/ui.h"
#include "ui/ui_commands.h"
#include "input/input.h"
#include "utilities/utilities.h"
#include "dLib/dFile/dFile.h"
#include "dLib/dConf/configuration.h"
#include <ctime>

using namespace FTS;

DefaultLogger::DefaultLogger()
    : m_pLogFile(NULL)
    , m_bLastDbg(false)
    , m_iGDLL(0)
    , m_lastMessageType(MsgType::Raw)
    , m_iLastDbgLv(1)
    , m_nLastMessageRepeats(0)
    , m_bIsInTranslation(false)
    , m_bCEGUIReady(false)
    , m_bSuppressNextDlg(false)
    , m_bSuppressOnlyNextDlg(false)
    , m_bMute(false)
{
#if WINDOOF
    system(("rmdir /Q /S \"" + Path::userdir("Logfiles").str() + "\"").c_str());
#else
    system(("rm -R \"" + Path::userdir("Logfiles").str() + "\"").c_str());
#endif
    // If the log dir doesn't exist, create it.
    FileUtils::mkdirIfNeeded(Path::userdir("Logfiles"), false);

    Configuration conf("conf.xml", ArkanaDefaultSettings());
    m_iGDLL = conf.getInt("DebugLevel");

    if(!m_pLogFile) {
        // Create a filename consisting of date and time.
        struct tm *newtime;
        time_t long_time;

        time(&long_time);
        newtime = localtime(&long_time);        // What a struggle to just get the time and date.

        String sOnlyName = String::nr(newtime->tm_mday, 2) + "-" +
                           String::nr(newtime->tm_mon + 1, 2) + "-" +
                           String::nr(newtime->tm_year + 1900, 4) + "_" +
                           String::nr(newtime->tm_hour, 2) + "-" +
                           String::nr(newtime->tm_min, 2) + "-" +
                           String::nr(newtime->tm_sec, 2) + ".log";
        String sFileName = Path::userdir("Logfiles") + Path(sOnlyName);

        // Create the log file.
        m_pLogFile = fopen(sFileName.c_str(), "w+");
        if(!m_pLogFile) {
            printf("HORROR: COULD NOT CREATE THE LOGFILE '%s': '%s' !\n",
                   sFileName.c_str(), strerror(errno));
        } else {
            fprintf(m_pLogFile, "=====================\n");
            fprintf(m_pLogFile, "== Logging started ==\n");
            fprintf(m_pLogFile, "=====================\n\n");
        }
    }
}

DefaultLogger::~DefaultLogger()
{
    SAFE_FCLOSE(m_pLogFile);
}

int DefaultLogger::loadConfig()
{
    return ERR_OK;
}

/// Sets the Global Debug Level Limit.
/** This sets the Global Debug Level Limit. See the \c MsgType::Enum enum and
 *  the \c message function if you don't know what it is.
 *
 * \param value: The new value for the GDLL
 *
 * \return ERR_OK
 *
 * \author Pompei2
 */
int DefaultLogger::setGDLL(int in_iValue)
{
    if(in_iValue < 1) {
        m_iGDLL = 1;
    } else {
        m_iGDLL = in_iValue;
    }
    Configuration conf ("conf.xml", ArkanaDefaultSettings());

    conf.set("DebugLevel", m_iGDLL);
    conf.save();
    return ERR_OK;
}

/// Returns the Global Debug Level Limit.
/** This returns the Global Debug Level Limit. See the \c MsgType::Enum enum and
 *  the \c message function if you don't know what it is.
 *
 * \return the GDLL
 *
 * \author Pompei2
 */
int DefaultLogger::getGDLL()
{
    return m_iGDLL;
}

String DefaultLogger::formatMessage(const String &in_sMsg,
                                    const MsgType::Enum& in_Gravity,
                                    const String &in_sArg1,
                                    const String &in_sArg2,
                                    const String &in_sArg3,
                                    const String &in_sArg4,
                                    const String &in_sArg5,
                                    const String &in_sArg6,
                                    const String &in_sArg7,
                                    const String &in_sArg8,
                                    const String &in_sArg9
                                   ) const
{
    return in_sMsg.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                       in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String DefaultLogger::formatMessageDbg(const String &in_sMsg, int in_iDbgLv,
                                       const String &in_sArg1,
                                       const String &in_sArg2,
                                       const String &in_sArg3,
                                       const String &in_sArg4,
                                       const String &in_sArg5,
                                       const String &in_sArg6,
                                       const String &in_sArg7,
                                       const String &in_sArg8,
                                       const String &in_sArg9
                                      ) const
{
    return in_sMsg.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4, in_sArg5,
                       in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String DefaultLogger::formatI18nMessage(const String &in_sMsgID,
                                        const MsgType::Enum& in_Gravity,
                                        const String &in_sArg1,
                                        const String &in_sArg2,
                                        const String &in_sArg3,
                                        const String &in_sArg4,
                                        const String &in_sArg5,
                                        const String &in_sArg6,
                                        const String &in_sArg7,
                                        const String &in_sArg8,
                                        const String &in_sArg9
                                       ) const
{
    String sErrMsg;

    // This avoids infinite recursion.
    if(m_bIsInTranslation) {
        sErrMsg = in_sMsgID + "(" + in_sArg1 + ","
                                  + in_sArg2 + ","
                                  + in_sArg3 + ","
                                  + in_sArg4 + ","
                                  + in_sArg5 + ","
                                  + in_sArg6 + ","
                                  + in_sArg7 + ","
                                  + in_sArg8 + ","
                                  + in_sArg9 + ")";
        return sErrMsg;
    } else
        m_bIsInTranslation = true;

    // Get the translated string.
    sErrMsg = getTranslatedString(in_sMsgID, "messages");
    if( sErrMsg.empty() ) {
        sErrMsg = "Missing translation for the error message " + in_sMsgID;
    }
    // Return, inserting the pieces at their place.
    m_bIsInTranslation = false;
    return sErrMsg.fmt(in_sArg1, in_sArg2, in_sArg3, in_sArg4,
                       in_sArg5, in_sArg6, in_sArg7, in_sArg8, in_sArg9);
}

String DefaultLogger::formatI18nMessageDbg(const String &in_pszMsgID, int in_iDbgLv,
                                           const String &in_sArg1,
                                           const String &in_sArg2,
                                           const String &in_sArg3,
                                           const String &in_sArg4,
                                           const String &in_sArg5,
                                           const String &in_sArg6,
                                           const String &in_sArg7,
                                           const String &in_sArg8,
                                           const String &in_sArg9
                                          ) const
{
    return this->formatI18nMessage(in_pszMsgID, MsgType::Raw, in_sArg1, in_sArg2,
                                   in_sArg3, in_sArg4, in_sArg5, in_sArg6,
                                   in_sArg7, in_sArg8, in_sArg9);
}

/// Write a translated message.
/** Writes a internationalized message into the console.
 *
 * \param in_pszMsgID: The message's ID
 * \param in_Gravity:  The message's gravity (like in \c message )
 * \param ...:         Additional params like they're passed to \c message
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \Note: This works like this: The message is searched in the current
 *        language file (by searching for ID = "Text") and then it's
 *        read from the file and written trough the \c message method.
 *        This means that the message can contain printf style chars
 *        (%s, %d, ...) but then \b the \b ... \b params \b have \b to
 *        \b be \b set \b right \b !
 *
 * \author Pompei2
 */
int DefaultLogger::i18nMessage(const String &in_sMsgID, const MsgType::Enum& in_Gravity,
                               const String &in_sArg1, const String &in_sArg2,
                               const String &in_sArg3, const String &in_sArg4,
                               const String &in_sArg5, const String &in_sArg6,
                               const String &in_sArg7, const String &in_sArg8,
                               const String &in_sArg9
                              )
{
    return this->doMessage(this->formatI18nMessage(in_sMsgID, in_Gravity, in_sArg1,
                                                   in_sArg2, in_sArg3, in_sArg4,
                                                   in_sArg5, in_sArg6, in_sArg7,
                                                   in_sArg8, in_sArg9),
                           in_Gravity);
}

/// Write a translated message.
/** Writes a internationalized message into the console.
 *
 * \param in_pszMsgID: The message's ID
 * \param in_Gravity:  The message's gravity (like in \c message )
 * \param ...:         Additional params like they're passed to \c message
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \Note: This works like this: The message is searched in the current
 *        language file (by searching for ID = "Text") and then it's
 *        read from the file and written trough the \c message method.
 *        This means that the message can contain printf style chars
 *        (%s, %d, ...) but then \b the \b ... \b params \b have \b to
 *        \b be \b set \b right \b !
 *
 * \author Pompei2
 */
int DefaultLogger::i18nMessageDbg(const String &in_sMsgID, int in_iDbgLv,
                                  const String &in_sArg1, const String &in_sArg2,
                                  const String &in_sArg3, const String &in_sArg4,
                                  const String &in_sArg5, const String &in_sArg6,
                                  const String &in_sArg7, const String &in_sArg8,
                                  const String &in_sArg9
                                 )
{
    return this->doMessage(this->formatI18nMessageDbg(in_sMsgID, in_iDbgLv, in_sArg1,
                                                      in_sArg2, in_sArg3, in_sArg4,
                                                      in_sArg5, in_sArg6, in_sArg7,
                                                      in_sArg8, in_sArg9),
                           MsgType::Raw, in_iDbgLv);
}

/// Send messages to the user.
/** This functions shows the user a message, if the message is shown
 *  by a popup during the game or only written in the console, or even
 *  just ignored, depends on the kind of message and user settings.
 *
 * \param in_pszMsg The text you want to show to the user.
 * \param in_Gravity The message's gravity (error, warning, ...).
 *                    See the MsgType::Enum enum for possible values here.
 * \param ... If FTS_DEBUG is set, the next param \b MUST be the debug level (int).
 *            This number is in the range 1 -> 5.\n
 *            The following params are treated like printf ones.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \note If you write a debug message, the first of the optional params (...)
 *       \b must be the debug level (an integer from 1 to 5).\n
 *       If the Debug level is bigger than the global debug level limit (gdll),
 *       the message is ignored and not printed to the screen.\n
 *       Level 1 debug messages are reserved for "doing ...", followed by a
 *       "done" or a "failed".\n
 *       Level 5 debug messages are reserved for Alloc/QuiteAlloc, MyFree and
 *       MyRealloc.\n
 *       Level 2, level 3 and level 4 debug messages are free for you t use.\n
 *       Possible values for string formating are the same as for printf.
 *
 * \author Pompei2
 **/
int DefaultLogger::message(const String &in_sMsg, const MsgType::Enum& in_Gravity,
                           const String &in_sArg1, const String &in_sArg2,
                           const String &in_sArg3, const String &in_sArg4,
                           const String &in_sArg5, const String &in_sArg6,
                           const String &in_sArg7, const String &in_sArg8,
                           const String &in_sArg9
                          )
{
    return this->doMessage(this->formatMessage(in_sMsg, in_Gravity, in_sArg1,
                                               in_sArg2, in_sArg3, in_sArg4,
                                               in_sArg5, in_sArg6, in_sArg7,
                                               in_sArg8, in_sArg9),
                           in_Gravity);
}

/// Send messages to the user.
/** This functions shows the user a message, if the message is shown
 *  by a popup during the game or only written in the console, or even
 *  just ignored, depends on the kind of message and user settings.
 *
 * \param in_pszMsg The text you want to show to the user.
 * \param in_Gravity The message's gravity (error, warning, ...).
 *                    See the MsgType::Enum enum for possible values here.
 * \param ... If FTS_DEBUG is set, the next param \b MUST be the debug level (int).
 *            This number is in the range 1 -> 5.\n
 *            The following params are treated like printf ones.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \note If you write a debug message, the first of the optional params (...)
 *       \b must be the debug level (an integer from 1 to 5).\n
 *       If the Debug level is bigger than the global debug level limit (gdll),
 *       the message is ignored and not printed to the screen.\n
 *       Level 1 debug messages are reserved for "doing ...", followed by a
 *       "done" or a "failed".\n
 *       Level 5 debug messages are reserved for Alloc/QuiteAlloc, MyFree and
 *       MyRealloc.\n
 *       Level 2, level 3 and level 4 debug messages are free for you t use.\n
 *       Possible values for string formating are the same as for printf.
 *
 * \author Pompei2
 **/
int DefaultLogger::messageDbg(const String &in_sMsg, int in_iDbgLv,
                              const String &in_sArg1, const String &in_sArg2,
                              const String &in_sArg3, const String &in_sArg4,
                              const String &in_sArg5, const String &in_sArg6,
                              const String &in_sArg7, const String &in_sArg8,
                              const String &in_sArg9
                             )
{
    return this->doMessage(this->formatMessageDbg(in_sMsg, in_iDbgLv, in_sArg1,
                                                  in_sArg2, in_sArg3, in_sArg4,
                                                  in_sArg5, in_sArg6, in_sArg7,
                                                  in_sArg8, in_sArg9),
                           MsgType::Raw, in_iDbgLv);
}

/// Just like message, but takes a \c va_list as argument instead of \c ...
/** See \c message for help.
 *
 * \return If successfull: ERR_OK
 * \return If failed:      An error code <0
 *
 * \author Pompei2
 **/
int DefaultLogger::doMessage(const String &in_sMsg, const MsgType::Enum& in_Gravity, int in_iDbgLv)
{
    String sMessage;

    if(in_sMsg.empty()) {
        FTS18N("InvParam", MsgType::Error, "Logger::vaMessage");
        return -1;
    }

    // If there is an error before comes the DONE message, write the FAILED message.
    if(m_bLastDbg && (in_Gravity == MsgType::Error || in_Gravity == MsgType::Horror))
        this->failConsoleMessage();

    // If this is a debug message that has debug level 5, only logg this if
    // the verbosity option is set to 5!
    // That means this is a really spammed message.
    if(in_iDbgLv >= 5 && !(this->getGDLL() <= 5))
        return ERR_OK;

    /* Write some prefix. */
    switch (in_Gravity) {
    case MsgType::Warning:
    case MsgType::WarningNoMB:
        sMessage = "FTS: Warning: ";
        ConsAttr(D_CHANGEFG, D_BLUE);
        break;
    case MsgType::Error:
        sMessage = "FTS: Error: ";
        ConsAttr(D_CHANGEFG, D_RED);
        break;
    case MsgType::Horror:
        sMessage = "FTS: HORROR: ";
        ConsAttr(D_CHANGEFG, D_DARKRED);
        break;
    case MsgType::Raw:
        if(in_iDbgLv > 1) {
            for(int i = 0 ; i < in_iDbgLv ; i++)
                sMessage += "  ";
        } else if(in_iDbgLv == 1) {
            sMessage = " - ";
        }
        break;
    case MsgType::GoodMessage:
        ConsAttr(D_CHANGEFG, D_DARKGREEN);
        break;
    case MsgType::Message:
    case MsgType::MessageNoMB:
        break;
    default:
        FTS18N("InvParam", MsgType::Error, "CUI::Message");
        return -1;
    }

    /* Then comes the main part of the message. */
    sMessage += in_sMsg;

    /* And finally the postfix. */
    switch (in_Gravity) {
    case MsgType::Warning:
    case MsgType::WarningNoMB:
        sMessage += "\n";
        break;
    case MsgType::Error:
        sMessage += " !\n";
        break;
    case MsgType::Horror:
        sMessage += " !!!\nThis is often a programmer error.\n";
        break;
    case MsgType::Raw:
        if(in_iDbgLv > 1 && in_iDbgLv < 5)
            sMessage += ".\n";
        break;
    case MsgType::Message:
    case MsgType::MessageNoMB:
    default:
        break;
    }

    if(in_iDbgLv != 5) {
#if 0
        // If it's the same message again, don't display it, just
        // count the repeats it has.
        // But only if it happened to be the same within 1 second.
        double timeSinceLast = m_lastMessageTime.measure();
        if(m_sLastMessage == sMessage && m_lastMessageType == in_Gravity && timeSinceLast < 1.0 ) {
            m_nLastMessageRepeats++;
            m_lastMessageTime.reset();
            return ERR_OK;
        } else if(m_nLastMessageRepeats > 0) {
            // If it is not the same as before, but the message
            // before has been repeated several times, say how
            // often it has been repeated.
            uint64_t nReps = m_nLastMessageRepeats;
            m_nLastMessageRepeats = 0;
            if(in_iDbgLv <= getGDLL()) {
                if(m_lastMessageType == MsgType::Raw && m_iLastDbgLv >= 1 && m_iLastDbgLv <= 5) {
                    FTSMSGDBG("The last message has been repeated "+String::nr(nReps)+" times", m_iLastDbgLv);
                } else {
                    FTSMSG("The last message has been repeated "+String::nr(nReps)+" times", m_lastMessageType);
                }
            }
        }
#endif
        m_sLastMessage = sMessage;
        m_lastMessageType = in_Gravity;
        if(m_lastMessageType == MsgType::Raw)
            m_iLastDbgLv = in_iDbgLv;
    }

    /* Now that we have our message ready, write it to log file and console when needed. */
    if(in_iDbgLv != 5 && m_pLogFile) {
        /* In the file, we write the time/date too. */
        struct tm *newtime;
        time_t long_time;

        time(&long_time);
        newtime = localtime(&long_time);        // What a struggle to just get the time and date.

        fprintf(m_pLogFile, "%.2d-%.2d-%.4d_%.2d-%.2d-%.2d -- ",
                newtime->tm_mday, newtime->tm_mon + 1,
                newtime->tm_year + 1900, newtime->tm_hour, newtime->tm_min,
                newtime->tm_sec);
        // In the logfile, we add a \n if there is none.
        if(sMessage.right(1) != "\n")
            fprintf(m_pLogFile, "%s\n", sMessage.c_str());
        else
            fprintf(m_pLogFile, "%s", sMessage.c_str());
        fflush(m_pLogFile);
    }

    if(in_iDbgLv <= getGDLL()) {
        if(!m_bMute) {
            printf("%s", sMessage.c_str());
            fflush(stdout);
        }
        m_bLastDbg = (in_Gravity == MsgType::Raw) && (in_iDbgLv == 1);
        ConsAttr(D_NORMAL);
        m_lastMessageTime.reset();
    }

    createWindow(in_Gravity, sMessage);

    return ERR_OK;
}

int DefaultLogger::doneConsoleMessage()
{
    // Allow a done to follow only a debug message, nothing else.
    if(!m_bLastDbg)
        return -2;

    if(!m_bMute) {
        ConsAttr(D_CHANGEFG, D_DARKGREEN);
        printf("Done");
        fflush(stdout);
        ConsAttr(D_NORMAL);
        putchar('\n');
    }

    if(m_pLogFile)
        fprintf(m_pLogFile, "Done\n");

    return ERR_OK;
}

int DefaultLogger::failConsoleMessage()
{
    // Allow a fail to follow only a debug message, nothing else.
    if(!m_bLastDbg)
        return -2;

    if(!m_bMute) {
        ConsAttr(D_CHANGEFG, D_DARKRED);
        printf("Failed");
        fflush(stdout);
        ConsAttr(D_NORMAL);
        putchar('\n');
    }

    if(m_pLogFile)
        fprintf(m_pLogFile, "Failed");

    return ERR_OK;
}

int DefaultLogger::createWindow(MsgType::Enum in_Gravity, const String &in_sMessage)
{
    String sTitle;

    // Check if CEGUI is ready to create a messagebox?
    if(!m_bCEGUIReady)
        return ERR_OK;

    // Only display warnings, errors, messages, goodmessage and horrors !
    if(in_Gravity == MsgType::Warning ||
       in_Gravity == MsgType::Error ||
       in_Gravity == MsgType::Message ||
       in_Gravity == MsgType::GoodMessage ||
       in_Gravity == MsgType::Horror
      ) {
        // Do we want to suppress the next window?
        if(m_bSuppressNextDlg) {
            // Clear huh?
            if(m_bSuppressOnlyNextDlg)
                this->stopSuppressingDlgs();
            return ERR_OK;
        }

        new DefaultLoggerDlg(in_Gravity, in_sMessage);
    }

    return ERR_OK;
}

uint64_t DefaultLoggerDlg::m_iCurrentID = 0;

/// Default constructor
/** This is the default constructor that creates the dialog, sets
 *  up all callbacks etc.
 *
 * \param in_Gravity The gravity of the message.
 * \param in_sMessage The message itself.
 *
 * \author Pompei2
 */
DefaultLoggerDlg::DefaultLoggerDlg(MsgType::Enum in_Gravity, const String &in_sMessage)
    : m_pRoot(NULL)
{
    CEGUI::Window *pLastActive = GUI::getSingleton().getActiveWin();
    m_sLastActive = pLastActive ? pLastActive->getName() : "None";
    m_bLastActiveWasModal = pLastActive ? pLastActive->getModalState() : false;

    try {
        // Load the window. Do it manually to avoid infinite recursion if there
        // is an error during loading of the window !
        Configuration conf ("conf.xml", ArkanaDefaultSettings());

        String sWindowName = "dlg_message." + conf.get("Language") + ".layout";
        m_pRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout(sWindowName);
        String sTitle = m_pRoot->getText();

        /* Set the correct title and text color. */
        switch (in_Gravity) {
        case MsgType::Warning:
            m_pRoot->getChild("dlg_message/Message")->setProperty("TextColours", "tl:FFFF3300 tr:FFFF3300 bl:FFFF6600 br:FFFF6600");
            sTitle = getTranslatedString("Msg_Warn", "ui");
            break;
        case MsgType::Error:
            m_pRoot->getChild("dlg_message/Message")->setProperty("TextColours", "tl:FFFF0000 tr:FFFF0000 bl:FFFF3333 br:FFFF3333");
            sTitle = getTranslatedString("Msg_Err", "ui");
            break;
        case MsgType::Horror:
            m_pRoot->getChild("dlg_message/Message")->setProperty("TextColours", "tl:FFBB0000 tr:FFBB0000 bl:FFFF0000 br:FFFF0000");
            sTitle = getTranslatedString("Msg_Horr", "ui");
            break;
        case MsgType::GoodMessage:
            m_pRoot->getChild("dlg_message/Message")->setProperty("TextColours", "tl:FF00BB00 tr:FF00BB00 bl:FF00FF00 br:FF00FF00");
            break;
        default:
            break;
        }
        m_pRoot->setText(sTitle);

        // Setup the text and the callbacks.
        CEGUI::Window *wOk = m_pRoot->getChild("dlg_message/Ok");
        m_pRoot->getChild("dlg_message/Message")->setText(in_sMessage);
        m_pRoot->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked,
                                FTS_SUBS(DefaultLoggerDlg::cbOk));
        wOk->subscribeEvent(CEGUI::PushButton::EventClicked,
                            FTS_SUBS(DefaultLoggerDlg::cbOk));

        // Change the names of all widgets, so we can display more dialogs at the same time.
        String sName = m_pRoot->getName() + " " + String::nr(m_iCurrentID);
        CEGUI::WindowManager::getSingleton().renameWindow(m_pRoot, sName);
        for(size_t i = 0; i < m_pRoot->getChildCount(); i++) {
            CEGUI::Window * child = m_pRoot->getChildAtIdx(i);
            // Avoid the tilebar, closebuttons etc.
            if(child->getName().find("__auto_") != CEGUI::String::npos)
                continue;
            sName = child->getName() + " " + String::nr(m_iCurrentID);
            CEGUI::WindowManager::getSingleton().renameWindow(child, sName);
        }
        m_iCurrentID++;

        // Show it.
        m_pRoot->setVisible(true);
        CEGUI::System::getSingleton().getGUISheet()->addChildWindow(m_pRoot);
        m_pRoot->setModalState(true);
        m_pRoot->activate();
        m_pRoot->setAlwaysOnTop(true);
        GUI::getSingleton().setActiveWidget(wOk);

    } catch(CEGUI::Exception & e) {
        delete this;
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
        return ;
    }

    try {
        // Add a keyboard shortcut to close this messagebox.
        InputManager *pMgr = InputManager::getSingletonPtr();
        ActiveWindowCheckCmd *pCond = NULL;
        CallbackCommand *pCbCmd = NULL;

        // One for the return key.
        pCond = new ActiveWindowCheckCmd(m_pRoot->getName());
        pCbCmd = new CallbackCommand(CEGUI::Event::Subscriber(&DefaultLoggerDlg::cbOk, this));
        pMgr->add(String(m_pRoot->getName()) + "1", SpecialKey::Enter,
                  new ConditionalCommand(pCond, pCbCmd));

        // And one for the escape key.
        pCond = new ActiveWindowCheckCmd(m_pRoot->getName());
        pCbCmd = new CallbackCommand(CEGUI::Event::Subscriber(&DefaultLoggerDlg::cbOk, this));
        pMgr->add(String(m_pRoot->getName()) + "2", Key::Escape,
                  new ConditionalCommand(pCond, pCbCmd));
    } catch (std::bad_alloc &ba) {
        // TODO check if we can log a message here or what to do else in such a rare situation.
        // If one of the new throws an exception. Just log it and return. This shouldn't happen,
        // but to gracefully return and not to crash tell the problem and live w/ some missing
        // commands.
        Logger::getSingleton().i18nMessage("Memory allocation failed in ftslogger.cpp DefaultLoggerDlg(): {1} ", MsgType::Horror, ba.what());
    }
}

/// Default destructor
/** This is the default destructor that deletes the dialog
 *  and maybe gives the modal state back to the parent.
 *
 * \author Pompei2
 */
DefaultLoggerDlg::~DefaultLoggerDlg()
{
    if(m_pRoot) {
        // Now that the dialog gets closed, we can remove both shortcuts from the system.
        InputManager::getSingleton().delShortcut(String(m_pRoot->getName()) + "1");
        InputManager::getSingleton().delShortcut(String(m_pRoot->getName()) + "2");
    }

    try {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_pRoot);
        if(m_sLastActive != "None" && m_bLastActiveWasModal) {
            try {
                CEGUI::WindowManager::getSingleton().getWindow(m_sLastActive)->setModalState(true);
            } catch(CEGUI::Exception &) {
                ;
            }
        }
    } catch(CEGUI::Exception & e) {
        FTS18N("CEGUI", MsgType::Error, e.getMessage());
    }
}

/// Closes the dialog. \param in_ea unused. \return true
bool DefaultLoggerDlg::cbOk(const CEGUI::EventArgs & in_ea)
{
    delete this;
    return true;
}
