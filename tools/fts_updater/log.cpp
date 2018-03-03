#include "log.h"
#include "main.h"
#include "UIActionList.h"

void vprint(char *pszmsg, ...)
{
    va_list li;
    FILE *pF = NULL;

    va_start(li, pszmsg);

    if(g_bVerbose) {
        vprintf(pszmsg, li);
        fflush(stdout);
    }

    pF = fopen("update.log", "a+");
    vfprintf(pF, pszmsg, li);

    fflush(pF);
    fclose(pF);
    va_end(li);

    return;
}

void vperror(char *pszmsg, ...)
{
    va_list li;
    FILE *pF = NULL;

    if(pszmsg == NULL)
        pszmsg = "huh ?";

    va_start(li, pszmsg);
    vfprintf(stderr, pszmsg, li);

    pF = fopen("update.log", "a+");
    vfprintf(pF, pszmsg, li);
    fflush(stdout);
    fflush(pF);
    fclose(pF);

    if(g_pActions) {
        char pszTmp[UIA_MAXPARAM];

        vsnprintf(pszTmp, UIA_MAXPARAM, pszmsg, li);
        g_pActions->Add(UIA_ADDLIST, pszTmp, true);
    }

    va_end(li);
    return;
}

void verror(char *pszmsg)
{
    FILE *pF = NULL;
    char *pszErr = strerror(errno);

    perror(pszmsg);
    fflush(stdout);

    pF = fopen("update.log", "a+");
    fprintf(pF, "ERR: ");
    fprintf(pF, "%s: %s\n", pszmsg, pszErr);
    fflush(pF);
    fclose(pF);

    if(g_pActions) {
        char pszTmp[UIA_MAXPARAM];

        snprintf(pszTmp, UIA_MAXPARAM, "%s: %s", pszmsg, pszErr);
        g_pActions->Add(UIA_ADDLIST, pszTmp, true);
    }

    return;
}
