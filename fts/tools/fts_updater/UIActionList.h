#pragma once

#define UIA_BUFSIZE  128
#define UIA_MAXPARAM 256

#define UIA_NONE	0
#define UIA_ADDLIST	1
#define UIA_SETPROGRESS	2
#define UIA_ADDPROGRESS	3
#define UIA_SET_DL_MAX	4
#define UIA_SET_DL_CUR	5

#define UIA_DEBUG 0

class CUIActionList {
  private:
    int m_piActions[UIA_BUFSIZE];
    char m_ppszParams[UIA_BUFSIZE][UIA_MAXPARAM];

    int m_iCurrent;

    bool m_bLocked;
    bool m_bWaiting;

  public:
     CUIActionList(void);
    ~CUIActionList(void);

    bool Add(int in_iAction, char *in_pszParam, bool in_bReplace = false);
    bool RemoveHead(void);
    bool GetHead(int &out_iAction, char out_pszParam[UIA_MAXPARAM]);

    bool BlockingAdd(int in_iAction, char *in_pszParam, bool in_bReplace =
                     false);
    bool BlockingRemoveHead(void);
    bool BlockingGetHead(int &out_iAction,
                         char out_pszParam[UIA_MAXPARAM]);

    int getNbr(void) {
        return m_iCurrent;
    };
};
