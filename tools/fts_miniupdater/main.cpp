#include "main.h"
#include "setup.h"

/* FIRST AT ALL: sorry for the bad code, i wanted to write the
 * sdl, opengl and cegui part quickly cauz i already did all this
 * for FTS ... so no comment plz i just copy/pasted and adjusted a bit :)
 *
 * AND i just wanted to write a nice program, not a nice/quick/proper code ;)
 *
 * Pompei2
 */

void main_loop(void);
void threadUpdate(void);

netbuf *g_connection = NULL;

int main(int argc, const char *argv[])
{
    // empty the logfile.
    FILE *pVF = fopen("minupdate.log", "w+");

    if(pVF)
        fclose(pVF);

    printf("Updating the FTS Updater and other critical files ...");
    init();
    threadUpdate();

    if(g_connection)
        FtpQuit(g_connection);

    /* All is done, so start FTS normally next time. */
    if((pVF = fopen("startup.sys", "w+"))) {
        fprintf(pVF, "%d", 1);
        fflush(pVF);
        fclose(pVF);
    }
    printf("Done.\n");

    /* If some argument was given, execute it. */
    if(argc > 1) {
        spawnv_async(argv[1], &argv[1]);
    }
    return 0;
}

bool DownloadFile(char *pszSrc, char *pszDest, netbuf * pConnection)
{
    netbuf *pFile = NULL;
    char *pszBuf = NULL;
    int iSize = 0;

    vprint("entering DownloadFile( %s, %s, connection );\n", pszSrc,
           pszDest);

    // Get the size of the file to know how mutch to transfer.
    vprint("\tFtpSize .. ");
    if(FtpSize(pszSrc, &iSize, FTPLIB_BINARY, pConnection) == 0) {
        vperror("Error getting the file size of '%s': '%s'\n", pszSrc,
                FtpLastResponse(pConnection));
        return false;
    }
    vprint("done (%d)\n", iSize);

    // Alloc the buffer to hold the file.
    if(NULL == (pszBuf = (char *)calloc(iSize + 1, sizeof(char)))) {
        vperror("out of memory\n");
        return false;
    }
    // Try to open the remote version info file.
    vprint("\tFtpAccess .. ");
    if(FtpAccess
       (pszSrc, FTPLIB_FILE_READ, FTPLIB_BINARY, pConnection,
        &pFile) == 0) {
        vperror("could not open the file '%s': '%s'\n", pszSrc,
                FtpLastResponse(pConnection));
        return false;
    }
    vprint("done\n");

    // Get the content of the remote version info file.
    int iRead = 0, i = 0;

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
            vperror("could not read the file '%s': '%s'\n", pszSrc,
                    FtpLastResponse(pConnection));
            return false;
        }
        vprint("done (%d)\n", i);
        iRead += i;

    } while(iRead < iSize);
    vprint("\tFtpClose .. ");
    FtpClose(pFile);
    vprint("done\n");
    pszBuf[iSize] = '\0';

    mkdir_if_needed(pszDest, true);
    FILE *pF = fopen(pszDest, "w+b");

    if(pF) {
        fwrite(pszBuf, sizeof(char), iSize, pF);
        fclose(pF);
    } else {
        char pszErr[1024];

        snprintf(pszErr, 1024, "error opening file '%s' with write access",
                 pszDest);
        verror(pszErr);
        return true;
    }

    vprint("leaving DownloadFile( %s, %s, connection );\n", pszSrc,
           pszDest);
    return true;
}

void threadUpdate(void)
{
    char *crc_list_remot = NULL;

    unsigned long ulRCRC32 = 0;
    unsigned long ulRSize = 0;
    unsigned long ulLSize = 0;
    unsigned long ulTmp = 0;
    unsigned long ulTot = 0;
    char pszFile[255];
    int iRead = 0;
    int i = 0;

    vprint("entering threadUpdate\n");

    if(!connect_to_server()) {
        return;
    }
    // Read the content of the remote file list.
    if(NULL ==
       (crc_list_remot = FtpFileToBuf(D_FILE_LIST, g_connection, 'a'))) {
        return;
    }

    do {
        i = ReadInfo(&crc_list_remot[iRead], pszFile, &ulRSize, &ulRCRC32);
        vprint("Verif file: '%s' ... ", pszFile);
        ulLSize = FLength(pszFile, 'b');
        if(ulLSize == (unsigned long)-1L) {
            // Files Is new.
            DownloadFile(&pszFile[2], pszFile, g_connection);
            vprint("Updated\n", pszFile);
        } else if(ulLSize != ulRSize) {
            // Files Differ.
            DownloadFile(&pszFile[2], pszFile, g_connection);
            vprint("Updated\n", pszFile);
        } else if(CRC32_Fichier(pszFile, &ulTmp) != ulRCRC32) {
            // Files Differ too.
            DownloadFile(&pszFile[2], pszFile, g_connection);
            vprint("Updated\n", pszFile);
        } else {
            vprint("OK\n");
        }

        // Add this file to the handled ones.
        ulTot += ulRSize;
        iRead += i;
    } while(crc_list_remot[iRead] != '\0');

    SAFE_FREE(crc_list_remot);

    vprint("leaving threadUpdate\n");
}
