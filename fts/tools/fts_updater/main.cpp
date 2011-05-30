#include "main.h"
#include "setup.h"
#include "UIActionList.h"
#include "ConfigFileUpdate.h"

/* FIRST AT ALL: sorry for the bad code, i wanted to write the
 * sdl, opengl and cegui part quickly cauz i already did all this
 * for FTS ... so no comment plz i just copy/pasted and adjusted a bit :)
 *
 * AND i just wanted to write a nice program, not a nice/quick/proper code ;)
 *
 * Pompei2
 */

bool onBtnVerif(const CEGUI::EventArgs & ea);
bool onBtnUpdate(const CEGUI::EventArgs & ea);
bool onBtnFTS(const CEGUI::EventArgs & ea);
bool onBtnQuit(const CEGUI::EventArgs & ea);
bool onBtnMkList(const CEGUI::EventArgs & ea);
bool onChkVerbose(const CEGUI::EventArgs & ea);

D_THREADFCT threadMkList(void *);
D_THREADFCT threadUpdate(void *);

void main_loop(void);
void render_gui(void);

CUIActionList *g_pActions = NULL;

CEGUI::Window * g_rootWin = NULL;
netbuf *g_connection = NULL;
bool g_updateable = false;
bool g_bVerbose = true;
int g_version[4];

int main(int argc, char *argv[])
{
    bool bAdminMode = false;

    // first of all, read the version info.
    FILE *pVF = fopen("v.i", "r");

    if(!pVF) {
        verror("Unable to open the version info file \"v.i\" !\n");
        g_version[0] = g_version[1] = g_version[2] = g_version[3];
    } else {
        fscanf(pVF, "%d.%d.%d.%d", &g_version[0], &g_version[1],
               &g_version[2], &g_version[3]);
        fclose(pVF);
    }

    // empty the logfile.
    pVF = fopen("update.log", "w+");
    if(pVF)
        fclose(pVF);

    // And look if we run in admin mode ?
    if(argc > 1) {
        if(!strcmp(argv[1], "-admin"))
            bAdminMode = true;
        else if(!strcmp(argv[1], "-h")) {
            printf("-admin for admin mode.\n");
            exit(0);
        }
    }

    g_pActions = new CUIActionList();

    init(bAdminMode);
    Window *w = g_rootWin->getChild("vers");

    // Associate the verify button to its handler.
    PushButton *pb = static_cast < PushButton * >(w->getChild("btnCheck"));

    pb->subscribeEvent(PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&onBtnVerif));

    // Associate the update button to its handler.
    pb = static_cast < PushButton * >(w->getChild("btnUpdate"));
    pb->subscribeEvent(PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&onBtnUpdate));

    // Associate the launch FTS button to its handler.
    pb = static_cast < PushButton * >(w->getChild("btnFTS"));
    pb->subscribeEvent(PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&onBtnFTS));

    // Associate the Quit button to its handler.
    pb = static_cast < PushButton * >(w->getChild("btnQuit"));
    pb->subscribeEvent(PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&onBtnQuit));

    // Associate the Make list button to its handler.
    pb = static_cast < PushButton * >(w->getChild("btnMkList"));
    pb->subscribeEvent(PushButton::EventClicked,
                       CEGUI::Event::Subscriber(&onBtnMkList));

    // Associate the Verbose checkbox to its handler.
    pb = static_cast < PushButton * >(w->getChild("chkVerbose"));
    pb->subscribeEvent(Checkbox::EventCheckStateChanged,
                       CEGUI::Event::Subscriber(&onChkVerbose));

    main_loop();

    delete g_pActions;

    if(g_connection)
        FtpQuit(g_connection);
    SDL_Quit();

    return 0;
}

void main_loop(void)
{
    unsigned long ulLoops = 0;
    float fTotal = 1.0f;
    bool must_quit = false;
    char *p = NULL;

    int iAction = 0;
    char pszParam[UIA_MAXPARAM];

    ProgressBar *pb = NULL;
    ListboxTextItem *lti = NULL;
    Listbox *lb = NULL;

    // get "run-time" in seconds
    double last_time_pulse =
        0.001 * static_cast < double >(dGetTicks());

    while(!must_quit) {
        /* Update some stats. */
        ulLoops++;

        /* Do all remaining actions. */
        while(g_pActions->BlockingGetHead(iAction, pszParam)) {
            // This means we did all actions.
            if(iAction == UIA_NONE)
                break;

            // We got this, so remove it from the stack.
            g_pActions->BlockingRemoveHead();

            switch (iAction) {
            case UIA_ADDLIST:
                // Add an item to the loglist.
                lti = new ListboxTextItem(pszParam);
                lb = static_cast <
                    Listbox *
                    >(g_rootWin->getChild("vers")->getChild("FList"));
                lb->addItem(lti);
                lb->ensureItemIsVisible(lti);
                break;
            case UIA_SETPROGRESS:
                // Set the progressbar.
                pb = static_cast <
                    ProgressBar *
                    >(g_rootWin->getChild("vers")->getChild("proFile"));
                pb->setProgress((float)atof(pszParam));
                break;
            case UIA_ADDPROGRESS:
                // Add a value to the progressbar.
                pb = static_cast <
                    ProgressBar *
                    >(g_rootWin->getChild("vers")->getChild("proFile"));
                pb->adjustProgress((float)atof(pszParam));
                break;
            case UIA_SET_DL_MAX:
                // Set the total bytes to download.
                fTotal = (float)atof(pszParam);
                // And adapt the label.
                p = MyAllocSPrintf("Total: %.2f MB",
                                   atof(pszParam) / (1024.0f * 1024.0f));
                g_rootWin->getChild("vers")->getChild("lblMax")->
                    setText(p);
                SAFE_FREE(p);
                break;
            case UIA_SET_DL_CUR:
                // Set the currently downloaded bytes.
                pb = static_cast <
                    ProgressBar *
                    >(g_rootWin->getChild("vers")->getChild("proTotal"));
                pb->setProgress((float)atof(pszParam) / fTotal);
                // And adapt the label.
                p = MyAllocSPrintf("Downloaded: %.2f MB",
                                   atof(pszParam) / (1024.0f * 1024.0f));
                g_rootWin->getChild("vers")->getChild("lblComp")->
                    setText(p);
                SAFE_FREE(p);
                break;
            default:
                vperror("UI Action not yet implemented: %d %s\n", iAction,
                        pszParam);
                break;
            }
        }

        inject_input(must_quit);
        inject_time_pulse(last_time_pulse);
        render_gui();
    }
}

bool DownloadFile(char *pszSrc, char *pszDest, netbuf * pConnection)
{
    netbuf *pFile = NULL;
    char pszTmp[UIA_MAXPARAM];
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

    // Create an entry in the listbox and reset the progressbar.
    snprintf(pszTmp, UIA_MAXPARAM, "%s (%d kB)", pszSrc, iSize / 1024);
    g_pActions->BlockingAdd(UIA_ADDLIST, pszTmp);
    g_pActions->BlockingAdd(UIA_SETPROGRESS, "0");

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

        // Update the progressbar.
        snprintf(pszTmp, UIA_BUFSIZE, "%g", (float)iRead / (float)iSize);
        g_pActions->Add(UIA_SETPROGRESS, pszTmp);
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

bool onBtnVerif(const CEGUI::EventArgs & ea)
{
    char *pszBuf = NULL;
    int iVersion[4];

    g_updateable = false;

    /* First of all connect to the remote server. */
    if(!connect_to_server())
        return true;

    /* Read the content of the remote version info file. */
    if(NULL == (pszBuf = FtpFileToBuf("v.i", g_connection, 'a')))
        return true;

    /* Extract the version information. */
    sscanf(pszBuf, "%d.%d.%d.%d", &iVersion[0], &iVersion[1], &iVersion[2],
           &iVersion[3]);

    Window *w = g_rootWin->getChild("vers");

    w->getChild("serv_ver")->setText(pszBuf);
    SAFE_FREE(pszBuf);

    /* Already gots the actual version, skip further tests. */
    long vRemote =
        FTS_MK_VERSION_LONG(iVersion[0], iVersion[1], iVersion[2],
                            iVersion[3]);
    long vLocal =
        FTS_MK_VERSION_LONG(g_version[0], g_version[1], g_version[2],
                            g_version[3]);

    if(vRemote <= vLocal) {
        g_pActions->BlockingAdd(UIA_ADDLIST,
                                "You got the newest version !");
        return true;
    }

    g_pActions->BlockingAdd(UIA_ADDLIST,
                            "There is a new version avaible !");
    return g_updateable = true;
}

bool onBtnUpdate(const CEGUI::EventArgs & ea)
{
    if(!g_updateable) {
        // verify if there is an update avaible.
        onBtnVerif(ea);
    } else {
        // First of all connect to the remote server.
        if(!connect_to_server())
            return true;
    }

    if(!g_updateable)
        return true;

    vprint("\n================");
    vprint("\n= Begin update =");
    vprint("\n================\n");
    d_threadStart(threadUpdate, NULL);

    return true;
}

bool onBtnFTS(const CEGUI::EventArgs & ea)
{
    spawnp_sync("fts.exe", "fts.exe");
    exit(0);
    return true;
}

bool onBtnQuit(const CEGUI::EventArgs & ea)
{
    exit(0);
    return true;
}

bool onBtnMkList(const CEGUI::EventArgs & ea)
{
    d_threadStart(threadMkList, NULL);

    return true;
}

bool onChkVerbose(const CEGUI::EventArgs & ea)
{
    const WindowEventArgs evWindow =
        static_cast < const WindowEventArgs & >(ea);
    //evWindow.window->setText( "BlaBla" );
    Checkbox *cb = static_cast < Checkbox * >(evWindow.window);

    g_bVerbose = cb->isSelected();

    return true;
}

D_THREADFCT threadUpdate(void *)
{
    char *crc_list_remot = NULL;

    unsigned long ulRCRC32 = 0;
    unsigned long ulRSize = 0;
    unsigned long ulLSize = 0;
    unsigned long ulTmp = 0;
    unsigned long ulTot = 0;
    char pszFile[MAX_PATH], *p = NULL;
    int iRead = 0;
    int i = 0;

    vprint("entering threadUpdate\n");

    PushButton *pb1 =
        static_cast <
        PushButton * >(g_rootWin->getChild("vers")->getChild("btnUpdate"));
    PushButton *pb2 =
        static_cast <
        PushButton * >(g_rootWin->getChild("vers")->getChild("btnCheck"));
    pb1->setEnabled(false);
    pb2->setEnabled(false);

    if(!connect_to_server()) {
        pb1->setEnabled(true);
        pb2->setEnabled(true);
        return D_THREADRET;
    }
    // Read the content of the remote file list.
    if(NULL ==
       (crc_list_remot = FtpFileToBuf(D_FILE_LIST, g_connection, 'a'))) {
        pb1->setEnabled(true);
        pb2->setEnabled(true);
        return D_THREADRET;
    }
    // Calculate the total bytes to download in worst case.
    p = MyAllocSPrintf("%lu", crc_list_get_total_size(crc_list_remot));
    g_pActions->BlockingAdd(UIA_SET_DL_MAX, p);
    SAFE_FREE(p);

    g_pActions->BlockingAdd(UIA_ADDLIST,
                            "Searching for updateable files ...");
    do {
        i = ReadInfo(&crc_list_remot[iRead], pszFile, &ulRSize, &ulRCRC32);
        vprint("Verif file: '%s' ... ", pszFile);
        ulLSize = FLength(pszFile, 'b');
        if(ulLSize == (unsigned int)-1L) {
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
        p = MyAllocSPrintf("%lu", ulTot);
        g_pActions->BlockingAdd(UIA_SET_DL_CUR, p);
        SAFE_FREE(p);
        iRead += i;
    } while(crc_list_remot[iRead] != '\0');

    g_pActions->BlockingAdd(UIA_ADDLIST,
                            "Now updating the configuration files ...");
    ConfigFileUpdate cfu;

    cfu.init("v.i3", g_connection);
    cfu.update();

    g_pActions->BlockingAdd(UIA_ADDLIST,
                            "Successfully updated FTS ! Thank you !");
    SAFE_FREE(crc_list_remot);
    pb1->setEnabled(true);
    pb2->setEnabled(true);

    vprint("leaving threadUpdate\n");

    return D_THREADRET;
}

D_THREADFCT threadMkList(void *)
{
    g_pActions->Add(UIA_SETPROGRESS, "0");
    g_pActions->Add(UIA_ADDLIST, "Beginning creation of CRC32 list ...");

    /* Create a list file of all files with their CRC32 sum and their size. */
    FILE *pF = fopen(D_FILE_LIST, "w+");

    crc_list_make(pF, FTSDIR);
    fclose(pF);

    g_pActions->Add(UIA_SETPROGRESS, "1.0");
    g_pActions->Add(UIA_ADDLIST, "Done the CRC32 list " D_FILE_LIST " !");

    return D_THREADRET;
}
