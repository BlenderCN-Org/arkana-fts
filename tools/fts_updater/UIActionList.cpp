#include "UIActionList.h"
#include "main.h"

CUIActionList::CUIActionList(void)
{
    for(int i = 0; i < UIA_BUFSIZE; i++) {
        m_piActions[i] = UIA_NONE;
        memset(m_ppszParams[i], 0, UIA_MAXPARAM);
    }

    m_iCurrent = 0;

    m_bLocked = false;
    m_bWaiting = false;
}

CUIActionList::~CUIActionList(void)
{
    for(int i = 0; i < UIA_BUFSIZE; i++) {
        m_piActions[i] = UIA_NONE;
        memset(m_ppszParams[i], 0, UIA_MAXPARAM);
    }

    m_iCurrent = 0;

    m_bLocked = true;
    m_bWaiting = false;
}

bool CUIActionList::Add(int in_iAction, char *in_pszParam,
                        bool in_bReplace)
{
#if UIA_DEBUG
    vprint("\t\tAdd( %d, %s, %s ) (%d of %d) .. ", in_iAction, in_pszParam,
           in_bReplace ? "true" : "false", m_iCurrent, UIA_BUFSIZE);
#endif
    if(m_iCurrent >= UIA_BUFSIZE && !in_bReplace) {
        //vprint( "UIActions list full !\n" );
        vprint("list full\n");
        return false;
    }

    if(m_bLocked) {
#if UIA_DEBUG
        vprint("locked\n");
#endif
        return false;
    }
    m_bLocked = true;

    /* If the array is full, we can either ignore this request or ignore the head. */
    if(m_iCurrent >= UIA_BUFSIZE) {
        //vprint( "UIActions list full !\n" );
        if(in_bReplace)
            RemoveHead();
    }

    m_piActions[m_iCurrent] = in_iAction;

    if(strlen(in_pszParam) >= UIA_MAXPARAM) {
#if UIA_DEBUG
        vprint
            ("Warning: following parameter to the ui actions list is too long:\n\t'%s'\n",
             in_pszParam);
#endif
        strncpy(m_ppszParams[m_iCurrent], in_pszParam, UIA_MAXPARAM);
    } else {
        strncpy(m_ppszParams[m_iCurrent], in_pszParam,
                strlen(in_pszParam));
    }
    m_ppszParams[m_iCurrent][strlen(m_ppszParams[m_iCurrent])] = '\0';

    m_iCurrent++;
    m_bLocked = false;
#if UIA_DEBUG
    vprint("done\n");
#endif
    return true;
}

bool CUIActionList::RemoveHead(void)
{
    if(m_bLocked)
        return false;
    m_bLocked = true;

    for(int i = 0; i < m_iCurrent; i++) {
        // Empty the current one.
        m_piActions[i] = UIA_NONE;
        memset(m_ppszParams[i], 0, UIA_MAXPARAM);

        // This means we are at the last one, just empty it.
        if(i >= UIA_BUFSIZE - 1)
            break;

        // Fill the current one with the content of the next one.
        m_piActions[i] = m_piActions[i + 1];
        m_piActions[i + 1] = UIA_NONE;
        strncpy(m_ppszParams[i], m_ppszParams[i + 1], UIA_MAXPARAM);
        memset(m_ppszParams[i + 1], 0, UIA_MAXPARAM);
    }

    m_iCurrent--;
    if(m_iCurrent < 0)
        m_iCurrent = 0;
    m_bLocked = false;
    return true;
}

bool CUIActionList::GetHead(int &out_iAction,
                            char out_pszParam[UIA_MAXPARAM])
{
    if(m_bLocked)
        return false;
    m_bLocked = true;

    out_iAction = m_piActions[0];
    strcpy(out_pszParam, m_ppszParams[0]);

    m_bLocked = false;
    return true;
}

/* CARE ! THIS IS KNOWN TO NOT WORK !! I HADN'T THE TIME YET TO CORRECT THE ISSUE:
 * when adding to a full list, it seems to hang forever ... just in release mode ... fuck ^^
 */
bool CUIActionList::BlockingAdd(int in_iAction, char *in_pszParam,
                                bool in_bReplace)
{
#if UIA_DEBUG
    vprint("\t\tBlockingAdd( %d, %s, %s ) (%d of %d) .. ", in_iAction,
           in_pszParam, in_bReplace ? "true" : "false", m_iCurrent,
           UIA_BUFSIZE);
#endif
    while(!Add(in_iAction, in_pszParam, in_bReplace)) ;
#if UIA_DEBUG
    vprint("done\n");
#endif

    return true;
}

bool CUIActionList::BlockingRemoveHead(void)
{
    while(!RemoveHead()) ;

    return true;
}

bool CUIActionList::BlockingGetHead(int &out_iAction,
                                    char out_pszParam[UIA_MAXPARAM])
{
    while(!GetHead(out_iAction, out_pszParam)) ;

    return true;
}
