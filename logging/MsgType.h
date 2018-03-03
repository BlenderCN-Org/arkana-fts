#ifndef D_MSG_TYPE_H
#define D_MSG_TYPE_H

namespace FTS {

namespace MsgType {
    /// The types of messages you can send.
    /** The types of messages you can send via CFTS::Message or better using macros
    *  like the FTSMSG one.
    **/
    enum Enum {
        /** Print out a warning. That is when something strange occurs but
        *  The game should continue running.
        **/
        Warning,

        /** Print out a warning without displaying a messagebox. That is when
        *  something strange occurs but the game should continue running
        *  without disturbing the player with a messagebox.
        **/
        WarningNoMB,

        /** Prints out an error message, that means something that is fatal
        *  to the normal game so it can't continue.
        **/
        Error,

        /** HORRORs are (catched) developer errors or very fatal (and rare ?)
        *  errors. They can be, for example, invalid parameters to a function.
        **/
        Horror,

        /// This is a positive massage that should be displayed in green within a dialogbox.
        GoodMessage,
        /// This is just a simple message that gets also displayed in a dialogbox.
        Message,
        /// This is just a simple message that gets displayed in the console and logfile.
        MessageNoMB,
        /// This is just a simple message that gets displayed only in console.
        Raw
    };
};

} // namespace FTS;

#endif // D_MSG_TYPE_H
