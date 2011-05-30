#include "server_log.h"
#include "db.h"
#include "mysql_config.h"
#include <mysql/errmsg.h>

using namespace FTS;
using namespace FTSSrv2;

FTS::String FTSSrv2::DataBase::m_psTblUsrFields[DSRV_TBL_USR_COUNT];
FTS::String FTSSrv2::DataBase::m_psTblChansFields[DSRV_TBL_CHANS_COUNT];
FTS::String FTSSrv2::DataBase::m_psTblChanOpsFields[DSRV_VIEW_CHANOPS_COUNT];
FTS::String FTSSrv2::DataBase::m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_COUNT];

FTSSrv2::DataBase::DataBase()
{
    m_pSQL = NULL;

    // Fill the arrays with the table names.
    m_psTblUsrFields[DSRV_TBL_USR_ID]="id";
    m_psTblUsrFields[DSRV_TBL_USR_NICK]="nickname";
    m_psTblUsrFields[DSRV_TBL_USR_PASS_MD5]="password";
    m_psTblUsrFields[DSRV_TBL_USR_PASS_SHA]="passwordSHA";
    m_psTblUsrFields[DSRV_TBL_USR_MAIL]="e_mail";
    m_psTblUsrFields[DSRV_TBL_USR_JABBER]="jabber";
    m_psTblUsrFields[DSRV_TBL_USR_CONTACT]="contact";
    m_psTblUsrFields[DSRV_TBL_USR_FNAME]="f_name";
    m_psTblUsrFields[DSRV_TBL_USR_NAME]="name";
    m_psTblUsrFields[DSRV_TBL_USR_BDAY]="bday";
    m_psTblUsrFields[DSRV_TBL_USR_SEX]="sex";
    m_psTblUsrFields[DSRV_TBL_USR_CMT]="comment";
    m_psTblUsrFields[DSRV_TBL_USR_LOCATION]="location";
    m_psTblUsrFields[DSRV_TBL_USR_IP]="ip";
    m_psTblUsrFields[DSRV_TBL_USR_LOCATION]="location";
    m_psTblUsrFields[DSRV_TBL_USR_SIGNUPD]="signup_date";
    m_psTblUsrFields[DSRV_TBL_USR_LASTON]="last_online";
    m_psTblUsrFields[DSRV_TBL_USR_WEEKON]="week_online";
    m_psTblUsrFields[DSRV_TBL_USR_TOTALON]="total_online";
    m_psTblUsrFields[DSRV_TBL_USR_WINS]="wins";
    m_psTblUsrFields[DSRV_TBL_USR_LOOSES]="looses";
    m_psTblUsrFields[DSRV_TBL_USR_DRAWS]="draws";
    m_psTblUsrFields[DSRV_TBL_USR_CLAN]="clan";
    m_psTblUsrFields[DSRV_TBL_USR_FLAGS]="flags";

    m_psTblChansFields[DSRV_TBL_CHANS_ID]="id";
    m_psTblChansFields[DSRV_TBL_CHANS_NAME]="name";
    m_psTblChansFields[DSRV_TBL_CHANS_MOTTO]="motto";
    m_psTblChansFields[DSRV_TBL_CHANS_ADMIN]="admin";
    m_psTblChansFields[DSRV_TBL_CHANS_PUBLIC]="public";

    m_psTblChanOpsFields[DSRV_VIEW_CHANOPS_NICK]="nickname";
    m_psTblChanOpsFields[DSRV_VIEW_CHANOPS_CHAN]="channel";

    m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_ID]="ID";
    m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_NICK]="nickname";
    m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_MSG]="feedback";
    m_psTblFeedbackFields[DSRV_TBL_FEEDBACK_WHEN]="when";
}

FTSSrv2::DataBase::~DataBase()
{
    if(m_pSQL) {
        mysql_close(m_pSQL);
        m_pSQL = NULL;
    }
}

int FTSSrv2::DataBase::init()
{
    if(NULL == (m_pSQL = mysql_init(NULL))) {
        FTSMSG("[ERROR] MySQL init: "+this->getError(), MsgType::Error);
        return -1;
    }

    char bTrue = 1;
    mysql_options(m_pSQL, MYSQL_OPT_RECONNECT, &bTrue);
    mysql_options(m_pSQL, MYSQL_OPT_COMPRESS, &bTrue);

    if(NULL == mysql_real_connect(m_pSQL, DSRV_MYSQL_HOST, DSRV_MYSQL_USER,
                                  DSRV_MYSQL_PASS, DSRV_MYSQL_DB, 0, NULL,
                                  CLIENT_MULTI_STATEMENTS)) {
        FTSMSG("[ERROR] MySQL connect: "+this->getError(), MsgType::Error);
        mysql_close(m_pSQL);
        m_pSQL = NULL;
        return -2;
    }

    if(0 != mysql_set_character_set(m_pSQL, "utf8"))
        FTSMSG("[ERROR] MySQL characterset: "+this->getError()+" .. ignoring", MsgType::Error);

    this->buildupDataBase();

    // Call our stored procedure that initialises the database.
    // We don't care about the result, there is none.
    MYSQL_RES *pRes;
    this->storedProcedure(pRes, "init", "");
    FTSSrv2::DataBase::getUniqueDB()->free(pRes);

    return 0;
}

int FTSSrv2::DataBase::free(MYSQL_RES *&out_pRes)
{
    // Free the current result set first:
    if(out_pRes) {
        mysql_free_result(out_pRes);
    }
    out_pRes = NULL;

    // And then every other if there are some more from multiple queries:
    while(mysql_next_result(m_pSQL) == 0) {
        MYSQL_RES *pRes = mysql_store_result(m_pSQL);
        if(pRes) {
            mysql_free_result(pRes);
        }

        // We don't care about errors.
    }

    // done.
    m_mutex.unlock();
    return ERR_OK;
}

#ifdef D_DSTRING
bool FTSSrv2::DataBase::query(MYSQL_RES *&out_pRes, const FTS::String & in_sStr)
{
    return this->query(out_pRes, in_sStr.c_str(), NULL);
}

FTS::String FTSSrv2::DataBase::escape(const FTS::String & in_sStr)
{
    return in_sStr.mysqlEscaped(m_pSQL);
}

int FTSSrv2::DataBase::storedFunctionInt(const FTS::String & in_sFunc, const FTS::String & in_sArgs)
{
    return this->storedFunctionInt(in_sFunc.c_str(), in_sArgs.c_str());
}

bool FTSSrv2::DataBase::storedProcedure(MYSQL_RES *&out_pRes, const FTS::String & in_sProc, const FTS::String & in_sArgs)
{
    return this->storedProcedure(out_pRes, in_sProc.c_str(), in_sArgs.c_str());
}
#endif

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !! DONT FORGET to call this->free on the returned value when it's != NULL !!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !! when finished to work with it, cause it then gets unblocked for other threads to execute queries. !! //
// !! Also, DONT FORGET TO END THIS function's parameter list with a NULL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !! Also, note that only the last query result is kept !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
bool FTSSrv2::DataBase::query(MYSQL_RES *&out_pRes, const char *in_pszQuery, ...)
{
    out_pRes = NULL;

    va_list args;
    const char *pszArg = in_pszQuery;
    char *pszQuery = NULL;

    int n = 0, i = 0;

    // Build the complete query string.
    va_start(args, in_pszQuery);
    while(pszArg) {
        i = (int)strlen(pszArg);
        pszQuery = (char *)realloc(pszQuery, n + i);
        memcpy(&pszQuery[n], pszArg, i);
        n += i;
        pszArg = (char *)va_arg(args, char *);
    }
    va_end(args);

    // Add a trailing zero.
    pszQuery = (char *)realloc(pszQuery, n + 1);
    pszQuery[n] = '\0';

    // If the result has to be freed, this mutex only gets unlocked when calling free.
    m_mutex.lock();

    // Execute the query.
    if(0 != mysql_query(m_pSQL, pszQuery)) {
        FTSMSG("[ERROR] MySQL query\nQuery string: "+String(pszQuery)+"\nError: "+this->getError(), MsgType::Error);
        ::free(pszQuery);
        m_mutex.unlock();

        unsigned int error = mysql_errno(m_pSQL);
        if(error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR || error == CR_SERVER_LOST_EXTENDED) {
            FTSMSG("Server lost, bye bye.\n", MsgType::Error);
            exit(0);
        }
        return false;
    }
    ::free(pszQuery);

    out_pRes = mysql_store_result(m_pSQL);
    if(out_pRes == NULL && mysql_field_count(m_pSQL) != 0) {
        // Errors occured during the query. (connection ? too large result ?)
        FTSMSG("[ERROR] MySQL field count\nQuery string: "+String(pszQuery)+"\nError: "+this->getError(), MsgType::Error);
        return false;
    }

    return true;

#if 0
    int status = 0;
    do {
        MYSQL_RES *pRes = mysql_store_result(m_pSQL);
        if(pRes) {
            if(mysql_more_results(m_pSQL)) {
                // If there are more, skip this one.
                mysql_free_result(pRes);
                // More results to come.
            } else if(out_pRes != NULL) {
                // If there are no more, store this one.
                *out_pRes = pRes;
                return true; // That was the last result.
            } else /* out_pRes is NULL, do not lock the mutex! */ {
                mysql_free_result(pRes);
                m_mutex.unlock();
                return true; // That was the last result.
            }
        } else {
            if(mysql_field_count(m_pSQL) == 0) {
                // Was a query that did not return results (insert/update/...)
                if(!mysql_more_results(m_pSQL)) {
                    m_mutex.unlock();
                    return true;
                }
            } else {
                // Errors occured during the query. (connection ? too large result ?)
                FTSMSG("[ERROR] MySQL field count\nQuery string: "+String(pszQuery)+"\nError: "+this->getError(), MsgType::Error);
                m_mutex.unlock();
                return false;
            }
        }

        // more results? -1 = no, >0 = error, 0 = yes (keep looping)
        status = mysql_next_result(m_pSQL);
        if(status > 0) {
            FTSMSG("[ERROR] MySQL next result\nQuery string: "+String(pszQuery)+"\nError: "+this->getError(), MsgType::Error);
            m_mutex.unlock();
            return false;
        }
    } while(status == 0);

    // Code should never come here!
    m_mutex.unlock();
    return false;
#endif
}

int FTSSrv2::DataBase::storedFunctionInt(const char *in_pszFunc, const char *in_pszArgs)
{
    MYSQL_RES *pRes = NULL;
    MYSQL_ROW row = NULL;

    // There was an error hum hum ...
    if(!this->query(pRes, "SELECT `"DSRV_MYSQL_DB"`.`", in_pszFunc, "` ( ", in_pszArgs, " )", NULL)) {
        return -1;
    }

    // How can it happen that a stored function returns nothing ??
    if(NULL == (row = mysql_fetch_row(pRes))) {
        this->free(pRes);
        FTSMSG("[ERROR] MySQL fetch stored function "+String(in_pszFunc)+"\nArguments: "+String(in_pszArgs)+"\nError: "+this->getError(), MsgType::Error);
        return -1;
    }

    int iRet = atoi(row[0]);
    this->free(pRes);
    return iRet;
}

bool FTSSrv2::DataBase::storedProcedure(MYSQL_RES *&out_pRes, const char *in_pszProc, const char *in_pszArgs)
{
    return this->query(out_pRes, "CALL `"DSRV_MYSQL_DB"`.`", in_pszProc, "` ( ", in_pszArgs, " )", NULL);
}

FTS::String FTSSrv2::DataBase::getError()
{
    return FTS::String(mysql_error(m_pSQL));
}

static FTSSrv2::DataBase *g_pTheDatabase = NULL;

int FTSSrv2::DataBase::initUniqueDB()
{
    g_pTheDatabase = new FTSSrv2::DataBase();
    if(ERR_OK != g_pTheDatabase->init()) {
        SAFE_DELETE(g_pTheDatabase);
        return -1;
    }

    return ERR_OK;
}

FTSSrv2::DataBase *FTSSrv2::DataBase::getUniqueDB()
{
    return g_pTheDatabase;
}

int FTSSrv2::DataBase::deinitUniqueDB()
{
    SAFE_DELETE(g_pTheDatabase);
    return ERR_OK;
}
