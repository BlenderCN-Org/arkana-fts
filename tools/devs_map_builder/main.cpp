#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

/* Header. */
#pragma pack(push,data)
#pragma pack(1)
struct SHeader {
    char id[4];
    unsigned short w;
    unsigned short h;
    char ts[5];
    float mult;
    unsigned short offs_q;
    unsigned short offs_t;
    unsigned short offs_u;
} header = { {
'F', 'T', 'S', 'T'}, 0, 0, {
'O', 'l', 'd', 'W', 'i'}, 1.0, 0, 0, 0};

#pragma pack(pop,data)

#define BLENDMASK 'a'

struct SQuad {
    short sHeight[25];
    bool bComplex;
};

int main(int argc, char *argv[])
{
    ifstream pInfoFile("map_info.conf");
    ifstream pComplexQuadsFile("map_complex_quads.conf");
    ifstream pQuadsMatrix("map_quads.conf");
    ifstream pTilesMatrix("map_tiles.conf");
    ifstream pUpperTilesMatrix("map_uppertiles.conf");

    int iOffset = 0;
    char c = '\0';

    /* Read the info. */
    pInfoFile >> header.w;
    pInfoFile >> header.h;
    pInfoFile >> header.mult;

    header.offs_q = 30;
    header.offs_t =
        header.offs_q + (header.w) * (header.h) * (sizeof(char) * 2 +
                                                   sizeof(short) * 4) + 1;
    header.offs_u =
        header.offs_t + (header.w+1) * (header.h+1) * sizeof(char) + 1;

    printf("Creating a %d x %d map ...     ", header.w, header.h);

    FILE *pFile = fopen("test.ftst", "w+b");

    if(!pFile) {
        printf("Error: can't write to output file '%s'\n", "test.ftst");
        return -1;
    }

    short *sHeights = new short[(header.w + 1) * (header.h + 1)];
    SQuad *s = new SQuad[(header.w) * (header.h)];
    char *lowerTiles = new char[(header.w + 1) * (header.h + 1)];
    char *upperTiles = new char[(header.w) * (header.h)];

    /* Read the heights from the input file. */
    printf("Done !\nRead the quads ...             ");
    for(int i = 0; i < (header.w + 1) * (header.h + 1); i++) {
        pQuadsMatrix >> sHeights[i];
    }

    /* Read the lowerTiles from the input file. */
    printf("Done !\nRead the lowerTiles ...        ");
    for(int i = 0; i < (header.w + 1) * (header.h + 1); i++) {
        pTilesMatrix >> lowerTiles[i];
    }

    /* Read the upperTiles from the input file. */
    printf("Done !\nRead the upperTiles ...        ");
    for(int i = 0; i < (header.w) * (header.h); i++) {
        int iID = 0;
        pUpperTilesMatrix >> iID;
        upperTiles[i] = (char)iID;
    }

    /* Create the quads structure. */
    printf("Done !\nCreate the quads structure ... ");
    for(int y = 0, i = 0; y < header.h; y++) {
        for(int x = 0; x < header.w; x++, i++) {
            s[i].bComplex = false;
            s[i].sHeight[0] = sHeights[y * (header.w + 1) + x];
            s[i].sHeight[1] = sHeights[y * (header.w + 1) + x + 1];
            s[i].sHeight[2] = sHeights[(y + 1) * (header.w + 1) + x];
            s[i].sHeight[3] = sHeights[(y + 1) * (header.w + 1) + x + 1];
        }
    }

    /* Read the complex quads now if there are. */
    printf("Done !\nReading the complex quads ... ");
    int iComplexQuads = 0;
    while(pComplexQuadsFile) {
        int ix, iy;
        pComplexQuadsFile >> ix;
        pComplexQuadsFile >> iy;

        if(!pComplexQuadsFile)
            break;

        SQuad *pq = &s[iy * header.w + ix];

        for(int j = 0 ; j < 25 ; j++)
            pComplexQuadsFile >> pq->sHeight[j];
        pq->bComplex = true;
        printf("x ");
        iComplexQuads++;
    }

    // Re-calculate the offset of the lowerTiles.
    header.offs_t =
        header.offs_q + (header.w) * (header.h) * (sizeof(char) * 2 +
                                                   sizeof(short) * 4) + 1
        + (sizeof(short) * 21) * iComplexQuads; // The complex quads take more place.

    // And the offset of the upperTiles.
    header.offs_u = header.offs_t + (header.w+1) * (header.h+1) * sizeof(char);

    printf("Done !\nWriting the header ...         ");
    fwrite(&header, sizeof(SHeader), 1, pFile);

    for(int i = 0; (size_t) i < (size_t) header.offs_q - sizeof(SHeader);
        i++) {
        fwrite(&c, sizeof(char), 1, pFile);
    }

    printf("Done !\nWriting the quads ...          ");
    for(int y = 0, i = 0; y < header.h; y++) {
        for(int x = 0; x < header.w; x++, i++) {
            // The flags.
            c = s[i].bComplex ? 1 : 0;
            fwrite(&c, sizeof(char), 1, pFile);
            c = BLENDMASK;
            fwrite(&c, sizeof(char), 1, pFile);
            fwrite(s[i].sHeight, sizeof(short), s[i].bComplex ? 25 : 4, pFile);
        }
    }

    printf("Done !\nWriting the lowerTiles ...     ");
    c = '\0';
    for(int i = 0;
        (size_t) i <
        (size_t) header.offs_t - ((size_t) header.offs_q +
                                  (header.w) * (header.h) * (sizeof(char) * 2 +
                                                             sizeof(short)* 4)+
                                  (sizeof(short) * 21) * iComplexQuads); i++) {
        fwrite(&c, sizeof(char), 1, pFile);
    }

    fwrite(lowerTiles, sizeof(char), (header.w + 1) * (header.h + 1), pFile);

    printf("Done !\nWriting the upperTiles ...     ");
    c = '\0';
    for(int i = 0;
        (size_t) i <
        (size_t) header.offs_u - (header.offs_t + (header.w+1) * (header.h+1) * sizeof(char)); i++) {
        fwrite(&c, sizeof(char), 1, pFile);
    }

    fwrite(upperTiles, sizeof(char), (header.w) * (header.h), pFile);

    fflush(pFile);
    fclose(pFile);
    printf("Done !\n");
    system("pause");
    return 0;
}
