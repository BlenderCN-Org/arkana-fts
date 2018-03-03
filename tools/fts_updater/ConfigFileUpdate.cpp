#include "ConfigFileUpdate.h"
#include "main.h"

ConfigFileUpdate::ConfigFileUpdate(void)
{
    m_pszBuf = NULL;
    m_p = NULL;
}

ConfigFileUpdate::~ConfigFileUpdate(void)
{
    SAFE_FREE(m_pszBuf);
    m_p = NULL;
}

char *ConfigFileUpdate::getNextFile()
{
    unsigned long int i = 0;
    char *pszFName = NULL;
    char *p = m_p;

    if(*p == '\0' || *p == '\t')
        return NULL;

    while(*p != '\n' && *p != '\0')
        p++;

    i = (unsigned long int)p - (unsigned long int)m_p;
    pszFName = (char *)calloc(i + 1, sizeof(char));
    if(!pszFName)
        verror("error");

    snprintf(pszFName, i, "%s", m_p);

    m_p += i + 1;

    return pszFName;
}

char *ConfigFileUpdate::getNextOption(char *cType)
{
    unsigned long int i = 0;
    char *pszOption = NULL;
    char *p = m_p;

    if(*m_p != '\t')
        return NULL;

    m_p++;
    p++;
    while(!isspace(*p) && *p != '\n' && *p != '\0')
        p++;

    i = (unsigned long int)p - (unsigned long int)m_p;
    pszOption = (char *)calloc(i + 1, sizeof(char));
    if(!pszOption)
        verror("error");

    snprintf(pszOption, i, "%s", m_p);

    /* Now read the space and the type specifying charachter. */
    i += 2;
    *cType = p[1];

    m_p += i + 1;

    return pszOption;
}

char *ConfigFileUpdate::getOptionText()
{
    unsigned long int iSize = 0;
    char *pszOptionT = NULL;
    char *p = m_p;
    int iLines = 1;
    int j = 0;

    if(m_p[0] != '\t' && m_p[1] != '\t')
        return NULL;

    while(*p != '\0') {
        if(*p == '\n') {
            /* Stop reading as soon as a newline isn't followed by two tabs anymore. */
            if(p[1] != '\t' || p[2] != '\t') {
                break;
            }
            iLines++;
        }
        p++;
    }

    /* Here we alloc enough memory to hold the entire text,
     * withoute the two leading tabs of each line.
     */
    iSize = (unsigned long int)p - (unsigned long int)m_p;
    pszOptionT = (char *)calloc((iSize - 2 * iLines) + 1, sizeof(char));
    if(!pszOptionT)
        verror("error");

    /* line per line, write the text into the memory. */
    p = m_p;
    for(int i = 0; i < iLines; i++) {
        /* Skip the two tabs. */
        p += 2;

        /* copy the line. */
        while(*p != '\n' && *p != '\0') {
            pszOptionT[j] = *p;
            p++;
            j++;
        }
        pszOptionT[j] = '\n';
        p++;
        j++;
    }

    pszOptionT[(iSize - 2 * iLines)] = '\0';
    m_p += iSize + 1;

    return pszOptionT;
}

bool ConfigFileUpdate::init(char *pszFile, netbuf * pConnection)
{
    m_pszBuf = FtpFileToBuf(pszFile, pConnection, 't');
    if(!m_pszBuf)
        return false;

    m_p = m_pszBuf;

    return true;
}

bool ConfigFileUpdate::update(void)
{
    char *pszFile = NULL;
    char *pszOpt = NULL;
    char *pszOptT = NULL;
    char cType = 0;
    bool bExists = false;

    if(!m_pszBuf)
        return false;

    while((pszFile = this->getNextFile())) {
        while((pszOpt = this->getNextOption(&cType))) {
            pszOptT = this->getOptionText();

            switch (cType) {
            case 'i':
                bExists = confExistsInt(pszFile, pszOpt);
                break;
            case 's':
                bExists = confExistsStr(pszFile, pszOpt);
                break;
            case 'b':
                bExists = confExistsBool(pszFile, pszOpt);
                break;
            default:
                bExists = false;
            }

            if(!bExists) {
                vprint("Adding configuration '%s' to file '%s'\n", pszOpt,
                       pszFile);
                /* Add this option at the end of the file. */
                mkdir_if_needed(pszFile, true);
                FILE *pF = fopen(pszFile, "a+");

                if(pF) {
                    fwrite(pszOptT, sizeof(char), strlen(pszOptT), pF);
                    fclose(pF);
                } else {
                    char pszErr[1024];

                    snprintf(pszErr, 1024,
                             "error opening file '%s' with write access",
                             pszFile);
                    verror(pszErr);
                }
            } else {
                vprint("Skipping configuration '%s' in file '%s'\n",
                       pszOpt, pszFile);
            }

            SAFE_FREE(pszOpt);
            SAFE_FREE(pszOptT);
        }
        SAFE_FREE(pszFile);
    }

    return true;
}
