#include "main.h"

/* Config file reading/writing functions. */

bool confExistsInt(char *in_pszFile, char *in_pszName)
{
    int iLen = 0, i = 0, bIs = 0, iSign = 1;
    char *pBuf = NULL, *p = NULL;
    FILE *pFile = NULL;

    bool iRet = false;

    if(!in_pszFile || !in_pszName) {
        return iRet;
    }

    if(NULL == (pFile = fopen(in_pszFile, "rb"))) {
        return iRet;
    }

    iLen = FLength(in_pszFile, 'b');

    if(NULL == (p = pBuf = (char *)calloc(iLen + 1, 1)))
        return iRet;

    if(fread(pBuf, iLen, 1, pFile) < 1) {
        return iRet;
    }
    pBuf[iLen] = '\0';

    while(*p) {
        if(*p == '#') {
            while(*p != '\n' && *p != '\0')
                p++;
            p++;
            continue;
        } else if(*p == in_pszName[0]) {
            for(bIs = 1, i = 0; i < (int)strlen(in_pszName); i++, p++) {
                if(*p != in_pszName[i]) {
                    bIs = 0;
                    break;
                }
            }

            if(p[1] != '=' && !isspace(p[1]))
                bIs = false;

            if(bIs) {
                while(isspace(*p))
                    p++;

                if(*p++ != '=') {
                    SAFE_FREE(pBuf);
                    return iRet;
                }

                while(isspace(*p))
                    p++;

                if(*p == '-') {
                    iSign = -1;
                    p++;
                    while(isspace(*p))
                        p++;
                } else if(*p == '+') {
                    iSign = 1;
                    p++;
                    while(isspace(*p))
                        p++;
                }

                if(isdigit(*p))
                    iRet = true;
                else {
                    SAFE_FREE(pBuf);
                    return iRet;
                }

                fclose(pFile);
                SAFE_FREE(pBuf);
                return iRet;
            } else {
                while(*p != '\n' && *p != '\0')
                    p++;
            }
        }
        while(*p != '\n' && *p != '\0')
            p++;
        p++;
    }

    fclose(pFile);
    SAFE_FREE(pBuf);

    return iRet;
}

// In this function we say errors using printf, because this function is used to get
// the language, and the error messages themselves. So if we use internationalised
// error messages here, we would get infinite recursion.
bool confExistsStr(char *in_pszFile, char *in_pszName)
{
    int iLen = 0, i = 0, bIs = 0, iStrLen = 0;
    char *pBuf = NULL, *p = NULL, *pOld = NULL;
    FILE *pFile = NULL;

    if(!in_pszFile || !in_pszName) {
        goto return_err;
    }

    if(NULL == (pFile = fopen(in_pszFile, "rb"))) {
        goto return_err;
    }

    iLen = FLength(in_pszFile, 'b');

    if(NULL == (p = pBuf = (char *)calloc(iLen + 1, 1)))
        goto return_err;

    if(fread(pBuf, iLen, 1, pFile) < 1) {
        goto return_err;
    }
    pBuf[iLen] = '\0';

    while(*p) {
        if(*p == '#') {
            while(*p != '\n' && *p != '\0')
                p++;
            p++;
            continue;
        } else if(*p == in_pszName[0]) {
            for(bIs = 1, i = 0; i < (int)strlen(in_pszName); i++, p++) {
                if(*p != in_pszName[i]) {
                    bIs = 0;
                    break;
                }
            }

            if(p[1] != '=' && !isspace(p[1]))
                bIs = false;

            if(bIs) {
                while(isspace(*p))
                    p++;

                if(*p++ != '=') {
                    SAFE_FREE(pBuf);
                    goto return_err;
                }

                while(isspace(*p))
                    p++;

                if(*p++ != '"') {
                    SAFE_FREE(pBuf);
                    goto return_err;
                }
                pOld = p;

                while(*p++ != '"')
                    iStrLen++;
                p = pOld;

                fclose(pFile);
                SAFE_FREE(pBuf);

                return true;
            } else {
                while(*p != '\n' && *p != '\0')
                    p++;
            }
        }
        while(*p != '\n' && *p != '\0')
            p++;
        p++;
    }

    fclose(pFile);
    SAFE_FREE(pBuf);

  return_err:
    return false;
}

bool confExistsBool(char *in_pszFile, char *in_pszName)
{
    int iLen = 0, i = 0, bIs = 0;
    char *pBuf = NULL, *p = NULL;
    FILE *pFile = NULL;

    bool bRet = false;

    if(!in_pszFile || !in_pszName) {
        return bRet;
    }

    if(NULL == (pFile = fopen(in_pszFile, "rb"))) {
        return bRet;
    }

    iLen = FLength(in_pszFile, 'b');

    if(NULL == (p = pBuf = (char *)calloc(iLen + 1, 1)))
        return bRet;

    if(fread(pBuf, iLen, 1, pFile) < 1) {
        return bRet;
    }
    pBuf[iLen] = '\0';

    while(*p) {
        if(*p == '#') {
            while(*p != '\n' && *p != '\0')
                p++;
            p++;
            continue;
        } else if(*p == in_pszName[0]) {
            for(bIs = 1, i = 0; i < (int)strlen(in_pszName); i++, p++) {
                if(*p != in_pszName[i]) {
                    bIs = 0;
                    break;
                }
            }

            if(p[1] != '=' && !isspace(p[1]))
                bIs = false;

            if(bIs) {
                while(isspace(*p))
                    p++;

                if(*p++ != '=') {
                    SAFE_FREE(pBuf);
                    return bRet;
                }

                while(isspace(*p))
                    p++;

                switch (*p++) {
                case 't':
                case 'T':
                    bRet = true;
                    break;
                case 'f':
                case 'F':
                    bRet = true;
                    break;
                default:
                    return bRet;
                }

                fclose(pFile);
                SAFE_FREE(pBuf);
                return bRet;
            } else {
                while(*p != '\n' && *p != '\0')
                    p++;
            }
        }
        while(*p != '\n' && *p != '\0')
            p++;
        p++;
    }

    fclose(pFile);
    SAFE_FREE(pBuf);

    return bRet;
}

 /* EOF */
