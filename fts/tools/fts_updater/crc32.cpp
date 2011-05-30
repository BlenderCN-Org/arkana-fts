#include "main.h"
#include "dLib/dBrowse/dBrowse.h"

char *ignore_list[] = { "./3d",
    "./3rd_party",
    "./Data/Confs",             // Don't know how to handle it yet ...
    "./Data/Dev",
    "./Debug",
    "./doc",
    "./graphic",
    "./input",
    "./main",
    "./myLib",
    "./Release",
    "./tmp",
    "./tools",
    "./ui",
    "./utilities",
    //                        "./fts_updater.exe",
    "./v.i",
    NULL
};

int crc_list_make(FILE * pF, char *pszRootDir)
{
    unsigned long ulCRC32 = 0;
    unsigned long ulFSize = 0;
    PDBrowseInfo dbi = NULL;
    char *pszF;

    dbi = dBrowse_Open(pszRootDir);

    while((pszF = dBrowse_GetNext(dbi))) {
        // Ignore the current and upper directory (. and ..).
        if(!strcmp(pszF, ".") || !strcmp(pszF, "..") ||
           !strcmp(pszF, ".\\") || !strcmp(pszF, "..\\") ||
           !strcmp(pszF, "./") || !strcmp(pszF, "../"))
            continue;

        // This is the whole path to the current file or directory.
        char *complete_file_path =
            MyAllocSPrintf("%s%s", pszRootDir, pszF);

        // look if we gotta ignore this one or not.
        for(int i = 0; ignore_list[i]; i++) {
            if(!strcmp(complete_file_path, ignore_list[i])) {
                goto next;
            }
        }

        // If this one is a directory, recurse into it.
        if(dBrowse_GetType(dbi) == DB_DIR) {
            char *c =
                MyAllocSPrintf("%s%s" FTS_DIR_SEPARATOR, pszRootDir, pszF);
            crc_list_make(pF, c);
            if(c)
                free(c);
        }
        // Skip everything that is not a file now.
        if(dBrowse_GetType(dbi) != DB_FILE)
            goto next;

        // Calculate the CRC32 of the file and get its size.
        if((ulCRC32 = CRC32_Fichier(complete_file_path, &ulFSize)) == 0)
            return -1;

        // Write it all to the file.
        fprintf(pF, "%s\t%lu\t%lu\n", complete_file_path, ulFSize,
                ulCRC32);

      next:
        if(complete_file_path)
            free(complete_file_path);
    }

    return 0;
}

unsigned long crc_list_get_total_size(char *pszList)
{
    char pszBuf[255];
    unsigned long ulSize = 0, ulCRC32 = 0, ulSizeTotal = 0;
    int i = 0, iRead = 0;

    do {
        i = ReadInfo(&pszList[iRead], pszBuf, &ulSize, &ulCRC32);
        ulSizeTotal += ulSize;
        iRead += i;
    } while(pszList[iRead] != '\0');

    return ulSizeTotal;
}

// !! Originally written by Capa6T, found at
// http://www.cppfrance.com/codes/CRC-32BITS-COMPATIBLE-WINZIP_24351.aspx
// modified by me but left the original comments.

// *****************************************************************
// Permet de creer la table de refeence CRC32 compatible Winzip en swappant
// les bits. Swap bit 0 avec bit 7, bit 1 avec bit 6, bit 2 avec bit 5 etc etc...
// *****************************************************************
unsigned long Reflect(unsigned long ref, char ch)
{
    unsigned long value = 0;

    for(int i = 1; i < (ch + 1); i++) {
        if(ref & 1)
            value |= 1 << (ch - i);
        ref >>= 1;
    }
    return value;
}

// ******************************************************************
// Renvoi la Valeur CRC32 d'un Fichier (0 en cas d'erreur)
// J'en profite pour renvoyer la taille du fichier (1 pierre 2 coups)
// c'est utile pour le projet que je developpe actuellemnt...desole ;)
// PS: vu sur http://www.createwindow.com/programming/crc32/
// ******************************************************************
unsigned long CRC32_Fichier(char *NomFichier, unsigned long *TailleFichier)
{
    unsigned long CRC32 = 0;    // Valeur de retour de la fonction
    unsigned long crc32_table[256];     // Table identique a celle utilisee dans
    unsigned long ulPolynomial = 0x04c11db7;    // Winzip etc...
    FILE *Hfich;                // Handle du fichier
    unsigned long cmpt;         // Compteur d'octets pour lecture fichier
    unsigned char *octet, *poct;        // octets lus

    // Debut
    *TailleFichier = 0;         // Bon OK sa sert a rien ;) c plus propre c tout
    // Ouverture Fichier. Si Echec alors Fonction Renvoi 0 et sort immediatement
    if(!(Hfich = fopen(NomFichier, "rb")))
        return 0;
    // Recupere Taille du Fichier. Si Echec alors Fonction Renvoi 0 et sort immediatement
    if((*TailleFichier = FLength(NomFichier, 'b')) == (unsigned long)-1L)
        return 0;
    // Generation table de reference CRC32
    for(int i = 0; i <= 0xFF; i++) {
        crc32_table[i] = Reflect(i, 8) << 24;
        for(int j = 0; j < 8; j++)
            crc32_table[i] =
                (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ?
                                         ulPolynomial : 0);
        crc32_table[i] = Reflect(crc32_table[i], 32);
    }
    // Allocation Memoire si pas possible Retour avec CRC32=0
    cmpt = (*TailleFichier);
    if((octet = (unsigned char *)malloc(cmpt + 1)) == NULL) {
        fclose(Hfich);          // Fermeture du Fichier
        return 0;               // Retourne 0 => Erreur
    }
    // Lecture Complete du Fichier en memoire allouee precedement
    if(fread(octet, sizeof(unsigned char), cmpt, Hfich) < cmpt) {       // Erreur M.... !
        free(octet);            // Liberation memoire
        fclose(Hfich);          // Fermeture du Fichier
        return 0;               // Retourne 0 => Erreur
    }
    // Calcul du CRC32 du Fichier
    CRC32 = 0xFFFFFFFF;         // Depart avec tous les bits a 1
    poct = octet;
    while(cmpt--)               // Tant qu'on a pas atteind la fin de fichier
        CRC32 = (CRC32 >> 8) ^ crc32_table[(CRC32 & 0xFF) ^ *poct++];
    // Fermeture du Fichier
    fclose(Hfich);
    free(octet);                // Liberation memoire
    return CRC32 ^ 0xFFFFFFFF;
}
