#ifndef D_DB_H
#  define D_DB_H

#  include "server.h"
#  include "utilities/threading.h"
#  include "dLib/dString/dString.h"

#include <list>

namespace FTSSrv2 {

class DataBase {
private:
    MYSQL *m_pSQL;

    FTS::Mutex m_mutex;

    static FTS::String m_psTblUsrFields[DSRV_TBL_USR_COUNT];
    static FTS::String m_psTblChansFields[DSRV_TBL_CHANS_COUNT];
    static FTS::String m_psTblChanOpsFields[DSRV_VIEW_CHANOPS_COUNT];
    static FTS::String m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_COUNT];

public:
    DataBase();
    virtual ~DataBase();

    int init();
    int free(MYSQL_RES *&out_pRes);

#  ifdef D_DSTRING
    bool query(MYSQL_RES *&out_pRes, const FTS::String & in_sStr);
    FTS::String escape(const FTS::String & in_sStr);
    FTS::String getError();
    int storedFunctionInt(const FTS::String & in_sFunc, const FTS::String & in_sArgs);
    bool storedProcedure(MYSQL_RES *&out_pRes, const FTS::String & in_sProc, const FTS::String & in_sArgs);
#  endif

    int buildupDataBase();

    bool query(MYSQL_RES *&out_pRes, const char *in_pszQuery, ...);

    int storedFunctionInt(const char *in_pszFunc, const char *in_pszArgs);
    bool storedProcedure(MYSQL_RES *&out_pRes, const char *in_pszProc, const char *in_pszArgs);

    // Singleton-like stuff.
    static int initUniqueDB();
    static DataBase *getUniqueDB();
    static int deinitUniqueDB();

    inline static FTS::String TblUsrField(int i) {if(i >= DSRV_TBL_USR_COUNT) return FTS::String::EMPTY; else return m_psTblUsrFields[i];};
    inline static FTS::String TblChansField(int i) {if(i >= DSRV_TBL_CHANS_COUNT) return FTS::String::EMPTY; else return m_psTblChansFields[i];};
    inline static FTS::String TblChanOpsField(int i) {if(i >= DSRV_VIEW_CHANOPS_COUNT) return FTS::String::EMPTY; else return m_psTblChanOpsFields[i];};
    inline static FTS::String TblFeedbackField(int i) {if(i >= DSRV_TBL_FEEDBACK_COUNT) return FTS::String::EMPTY; else return m_psTblFeedbackFields[i];};
};

} // namespace FTSSrv2

#endif                          /* D_DB_H */
