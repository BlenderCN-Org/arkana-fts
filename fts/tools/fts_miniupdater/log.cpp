#include "log.h"
#include "main.h"

void vprint(char *pszmsg, ...)
{
    va_list li;
    FILE *pF = NULL;

    va_start(li, pszmsg);

    pF = fopen("minupdate.log", "a+");
    vfprintf(pF, pszmsg, li);
    vfprintf(stdout, pszmsg, li);

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

    pF = fopen("minupdate.log", "a+");
    vfprintf(pF, pszmsg, li);
    fclose(pF);

    va_end(li);
    return;
}

void verror(char *pszmsg)
{
    FILE *pF = NULL;
    char *pszErr = strerror(errno);

    perror(pszmsg);

    pF = fopen("minupdate.log", "a+");
    fprintf(pF, "ERR: ");
    fprintf(pF, "%s: %s\n", pszmsg, pszErr);
    fclose(pF);

    return;
}
