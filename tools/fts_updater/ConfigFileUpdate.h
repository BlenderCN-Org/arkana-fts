#pragma once

#include "main.h"

class ConfigFileUpdate {
  private:
    char *m_pszBuf;
    char *m_p;

    char *getNextFile();
    char *getNextOption(char *cType);
    char *getOptionText();

  public:
     ConfigFileUpdate(void);
    ~ConfigFileUpdate(void);

    bool init(char *pszFile, netbuf * pConnection);
    bool update(void);
};
