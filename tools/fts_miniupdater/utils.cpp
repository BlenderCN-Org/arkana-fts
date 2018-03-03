#include "main.h"

/* This function is taken from a gcc manual page example and adapted to FTS. */
char *MyAllocSPrintf(const char *fmt, ...)
{
    va_list ap;
    char *p = NULL;
    int size = 2, n = 0;

    if(NULL == (p = (char *)malloc(size)))
        return NULL;
    while(1) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);
#if WINDOOF
        /* If that worked, return the string. */
        if(n > -1 && n < size - 1)
            return p;
        /* Else try again with more space. */
        if(n > -1)
            size += 1;
        else                    /* n <= -1 */
            size *= 2;
#else
        /* If that worked, return the string. */
        if(n > -1 && n < size)
            return p;
        /* Else try again with more space. */
        if(n > -1)
            size = n + 1;
        else
            size *= 2;
#endif
        if(NULL == (p = (char *)realloc(p, size)))
            return NULL;
    }
}


long FLength(char *in_pszFileName, char mode)
{
    FILE *pFile;
    long length = 0L;

    if(!in_pszFileName) {
        vperror("invalid parameter to FLength\n");
        return -1L;
    }

    if(mode == 'b') {
        pFile = fopen(in_pszFileName, "rb");
        if(pFile == NULL)
            return -1L;

        fseek(pFile, 0L, SEEK_END);
        length = ftell(pFile);
        fclose(pFile);
    } else {
        pFile = fopen(in_pszFileName, "rt");
        if(pFile == NULL)
            return -1L;
        char c;

        while((c = fgetc(pFile)) != EOF) {
            length++;
            if(c == '\r') {
                if((c = fgetc(pFile)) == EOF)
                    break;
                if(c != '\n')
                    length++;
            }
        }
    }

    return length;
}

char *__replaceStr(char *in_pszBase, unsigned long in_ulIndex,
                   unsigned long in_ulLenght, char *in_pszNew)
{
    char *pszResult = NULL, *pszTemp = NULL;

    if(in_pszBase == NULL) {
        vperror("InvParam: __replaceStr\n");
        return NULL;
    }
    if(strlen(in_pszBase) < in_ulIndex || in_ulIndex < 0) {
        vperror("InvParam: __replaceStr\n");
        return NULL;
    }

    if(in_ulIndex == 0) {
        pszResult =
            MyAllocSPrintf("%s%s", in_pszNew, &in_pszBase[in_ulLenght]);
        return pszResult;
    }

    if(NULL == (pszTemp = (char *)calloc(in_ulIndex + 1, sizeof(char)))) {
        vperror("out of memory !\n");
        return NULL;
    }

    snprintf(pszTemp, in_ulIndex, "%s", in_pszBase);
    pszTemp[in_ulIndex] = '\0';

    pszResult = MyAllocSPrintf("%s%s%s", pszTemp, in_pszNew,
                               &in_pszBase[in_ulIndex + in_ulLenght]);

    if(pszTemp)
        free(pszTemp);
    pszTemp = NULL;
    return pszResult;
}

int replaceStr(char **out_ppszBase, unsigned long in_ulIndex,
               unsigned long in_ulLenght, char *in_pszNew)
{
    char *pszTemp = MyAllocSPrintf("%s", *out_ppszBase);

    if(*out_ppszBase)
        free(*out_ppszBase);
    *out_ppszBase = NULL;

    *out_ppszBase =
        __replaceStr(pszTemp, in_ulIndex, in_ulLenght, in_pszNew);

    if(pszTemp)
        free(pszTemp);
    pszTemp = NULL;
    return ERR_OK;
}

char *basename(char *name)
{
    char *base = NULL;

#ifdef WINDOOF
    /* Skip over the disk name in MSDOS pathnames. */
    if(isalpha(name[0]) && name[1] == ':')
        name += 2;
#endif

    for(base = name; *name; name++) {
        if(FTS_IS_DIR_SEPARATOR(*name) && name[1] != '\0') {
            base = name + 1;
        }
    }

    return base;
}

bool mkdir_if_needed(char *path, bool bWithFile)
{
    struct stat buf;
    char *pszPath = NULL;
    char *pszCmd = NULL;
    int len = 0;
    int ret = 0;

    if(bWithFile) {
        char *pszBasename = basename(path);
        int i = (int)strlen(path);
        int j = (int)strlen(pszBasename);

        len = i - j;

        if(FTS_IS_DIR_SEPARATOR(path[len - 1]))
            len--;
    } else {
        len = (int)strlen(path);

        if(FTS_IS_DIR_SEPARATOR(path[len - 1]))
            len--;
    }

    pszPath = (char *)calloc(len + 1, sizeof(char));
    strncpy(pszPath, path, len);

    // Ignore the current and upper directory (. and ..).
    if(!strcmp(pszPath, ".") || !strcmp(pszPath, "..") ||
       !strcmp(pszPath, ".\\") || !strcmp(pszPath, "..\\") ||
       !strcmp(pszPath, "./") || !strcmp(pszPath, "../"))
        return true;

    //mkdir( pszPath );

    if((ret = stat(pszPath, &buf)) == 0) {
        free(pszPath);
        return true;
    }
#if WINDOOF
    pszCmd = MyAllocSPrintf("mkdir \"%s\"", pszPath);

    char *p = pszCmd;

    while(p = strchr(pszCmd, '/'))
        *p = '\\';
#else
    pszCmd = MyAllocSPrintf("mkdir -p %s", pszPath);
#endif

    system(pszCmd);

    free(pszPath);
    free(pszCmd);

    return true;
}

char *FileToBuf(char *pszFile, char mode)
{
    FILE *pFile = NULL;
    char *pszBuf = NULL;
    int iSize = 0;
    int i = 0;

    // Get the size of the file to know how mutch to read.
    if(-1 == (iSize = FLength(pszFile, mode))) {
        vperror("Error getting the file size of '%s'\n", pszFile);
        return NULL;
    }
    // Alloc the buffer to hold the file.
    if(NULL == (pszBuf = (char *)calloc(iSize + 1, sizeof(char)))) {
        vperror("out of memory\n");
        return NULL;
    }
    // Try to open the remote version info file.
    if(mode == 'b')
        pFile = fopen(pszFile, "rb");
    else
        pFile = fopen(pszFile, "rt");

    if(NULL == pFile) {
        vperror("could not open the file '%s'\n", pszFile);
        return NULL;
    }
    // Get the content of the remote version info file.
    if((i = (int)fread(pszBuf, sizeof(char), iSize, pFile)) != iSize) {
        vperror("could not read the file '%s'\n", pszFile);
        return NULL;
    }
    fclose(pFile);
    pszBuf[iSize] = '\0';

    return pszBuf;
}

char *FtpFileToBuf(char *pszFile, netbuf * pConnection, char mode)
{
    netbuf *pFile = NULL;
    char *pszBuf = NULL;
    char *p = NULL;
    int iSize = 0;

    vprint("entering FtpFileToBuf( %s, connection, %c );\n", pszFile,
           mode);

    // Get the size of the file to know how mutch to transfer.
    vprint("\tFtpSize .. ");
    if(FtpSize(pszFile, &iSize, FTPLIB_BINARY, pConnection) == 0) {
        vperror("Error getting the file size of '%s': '%s'\n", pszFile,
                FtpLastResponse(pConnection));
        return NULL;
    }
    vprint("done (%d)\n", iSize);

    // Alloc the buffer to hold the file.
    if(NULL == (pszBuf = (char *)calloc(iSize + 1, sizeof(char)))) {
        vperror("out of memory\n");
        return NULL;
    }
    // Try to open the remote version info file.
    vprint("\tFtpAccess .. ");
    if(FtpAccess
       (pszFile, FTPLIB_FILE_READ, FTPLIB_BINARY, pConnection,
        &pFile) == 0) {
        free(pszBuf);
        vperror("could not open the file '%s': '%s'\n", pszFile,
                FtpLastResponse(pConnection));
        return NULL;
    }
    vprint("done\n");

    // Get the content of the file into the buffer.
    int i = 0;
    int iRead = 0;

    do {
        vprint("\tFtpRead( &pszBuf[%d], %d, pFile ) .. ", iRead, BUFSIZE);
        // A this simple, normal, for human beings correct looking ... function
        // does seem to have problems on XP (when it should gelt let's say the 5 last bytes,
        // and says its buffer is 1024, it doesn't get 5 bytes, but 0 ! if its buffer is 5, the same.
        // But if it asks for 4 bytes, and the for the last one, everything seems to work fine ...
        // Voodoo XP ...
        // i = FtpRead( &pszBuf[iRead], BUFSIZE, pFile );
        if(BUFSIZE < (iSize - iRead)) {
            i = FtpRead(&pszBuf[iRead], BUFSIZE, pFile);
        } else {
            int j = iSize - iRead - 1;

            if(j == 0)
                j = 1;
            i = FtpRead(&pszBuf[iRead], j, pFile);
        }
        if(i == -1 || i == 0) {
            vperror("could not read the file '%s': '%s'\n", pszFile,
                    FtpLastResponse(pConnection));
            return false;
        }
        //i = FtpRead( &pszBuf[iRead], BUFSIZE, pFile );
        //if( i == -1 ) {
        //      free( pszBuf );
        //      FtpClose( pFile );
        //      vperror( "could not read the file '%s': '%s'\n", pszFile, FtpLastResponse( pConnection ) );
        //      return NULL;
        //}
        vprint("done (%d)\n", i);
        iRead += i;

    } while(iRead < iSize);
    vprint("\tFtpClose .. ");
    FtpClose(pFile);
    vprint("done\n");
    pszBuf[iSize] = '\0';

    // In binary mode, we stop here. In text mode we'll convert \r\n into \n only.
    if(mode == 'b')
        return pszBuf;

    // Here, we replace the special signs with the corresponding chars.
    vprint("\tConvert nl .. ");
    while(NULL != (p = strchr(pszBuf, '\r'))) {
        if(p[1]) {
            if(p[1] == '\n') {
                replaceStr(&pszBuf, (int)(strlen(pszBuf) - strlen(p)), 2,
                           "\n");
            }
        }
    }
    vprint("done\n");

    vprint("leaving FtpFileToBuf( %s, connection, %c );\n", pszFile, mode);
    return pszBuf;
}

int ReadInfo(char *pList, char *pszFile, unsigned long *ulSize,
             unsigned long *ulCRC32)
{
    int iRead = 0;
    int i = 0;

    for(i = 0; pList[i] != '\t'; i++) ;

    memcpy(pszFile, pList, i);
    pszFile[i] = '\0';

    sscanf(&pList[i + 1], "%lu\t%lu%n", ulSize, ulCRC32, &iRead);

    // +2 is because +1 for the tab between filename and size and +1 for the \n at the end.
    return iRead + i + 2;
}
