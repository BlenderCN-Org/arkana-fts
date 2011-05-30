#include "setup.h"

void exit_err(int i)
{
#if WINDOOF
    system("pause");
#endif
    exit(i);
}

void init(void)
{
    /* Init FTP. */
    FtpInit();
}

bool connect_to_server(void)
{
    /* Say something. */
    vprint("Connecting to %s ... ", D_HOST);

    if(g_connection != NULL)
        return true;

    /* Connect to the server ^^ */
    if(FtpConnect(D_HOST, &g_connection) == 0) {
        vperror("could not connect to host '%s': '%s'\n", D_HOST,
                FtpLastResponse(g_connection));
        return false;
    }

    /* Login to the remote server. */
    if(FtpLogin(D_USER, D_PASS, g_connection) == 0) {
        vperror("could not connect user '%s': '%s'\n", D_USER,
                FtpLastResponse(g_connection));
        return false;
    }

    /* Go to the FTS directory. */
    if(FtpChdir("mupd", g_connection) == 0) {
        vperror("could not change to directory '%s': '%s'\n", "mupd",
                FtpLastResponse(g_connection));
        return false;
    }

    /* Say something again. */
    vprint("connected !\n", D_HOST);

    return true;
}
