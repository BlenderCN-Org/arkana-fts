#define TOOLNAME "ftsblends"
#define TOOLVERSIONSTR "0.1"

#include "../toolcompat.h"
#include "dLib/dCompressor/dCompressor.h"
#include "graphic/image.h"

#include <cmath>
#include <cstdlib>
#include <algorithm>

using namespace FTS;

#define RAND(min, max) (int)(((double)rand() / (double)RAND_MAX) * (double)(max - min + 1) + (double)(min))

void usage()
{
// EXAMPLE
    FTSMSG("Usage: {1} WIDTH HEIGHT RANDOM\n", MsgType::Raw, TOOLNAME);
    FTSMSG("Creates an uncompressed arkana-fts image file that represents a valid blendmask of size WIDTH x HEIGHT.\n", MsgType::Raw);
    FTSMSG("Options:\n", MsgType::Raw);
    FTSMSG("  WIDTH  The width of the blendmask to create.\n", MsgType::Raw);
    FTSMSG("  HEIGHT The height of the blendmask to create.\n", MsgType::Raw);
    FTSMSG("  RANDOM The amount of random noise signal to create.\n", MsgType::Raw);
}

bool verify(uint8_t *pData, uint16_t w, uint16_t h, bool in_bSTFU = false)
{
    // Verify the buffer.
    int biggestDiff = 0;
    uint64_t errorAccum = 0;
    for(uint16_t y = 0 ; y < h ; ++y) {
        for(uint16_t x = 0 ; x < w ; ++x) {
            uint16_t sum = (uint16_t)pData[x*4+y*w*4+0] + (uint16_t)pData[x*4+y*w*4+1]
                         + (uint16_t)pData[x*4+y*w*4+2] + (uint16_t)pData[x*4+y*w*4+3];
            int16_t diff = sum - 255;
            biggestDiff = std::max(biggestDiff, std::abs(diff));
            errorAccum += std::abs(diff);
        }
    }

    if(!in_bSTFU) {
        String sMsg("Total error accumulation = {1}\n"
                     "Biggest error = {2}\n");
        FTSMSG(sMsg, MsgType::Raw, String::nr(errorAccum), String::nr(biggestDiff));
    }

    return errorAccum == 0;
}

bool randomFix(uint8_t *pData, uint16_t w, uint16_t h)
{
    FTSMSG("Suppressing errors using random distribution.\n", MsgType::Raw);
    // Suppress the error randomly.
    for(uint16_t y = 0 ; y < h ; ++y) {
        for(uint16_t x = 0 ; x < w ; ++x) {
            uint16_t sum = (uint16_t)pData[x*4+y*w*4+0] + (uint16_t)pData[x*4+y*w*4+1]
                            + (uint16_t)pData[x*4+y*w*4+2] + (uint16_t)pData[x*4+y*w*4+3];

            int16_t diff = sum - 255;
            if(diff == 0)
                continue;

            // Do we need to add or subtract values?
            int op = 1;
            if(diff > 0)
                op = -1;

            // Add or subtract random values.
            for(int16_t i = 0 ; i < -op*diff ; ++i) {
                int h = 0;
                // Do not add to 255 or subtract from 0.
                do {
                    // Random(0, 3)
                    h = RAND(0,3);
                } while((op == 1 && pData[x*4+y*w*4+h] == 255) ||
                        (op == -1 && pData[x*4+y*w*4+h] == 0));
                pData[x*4+y*w*4+h] += op;
            }
        }
    }

    return true;
}

bool fixAlpha(uint8_t *pData, uint16_t w, uint16_t h)
{
    for(uint16_t y = 0 ; y < h ; ++y) {
        for(uint16_t x = 0 ; x < w ; ++x) {
            pData[x*4+y*w*4+3]=255-pData[x*4+y*w*4+3];
        }
    }

    return true;
}

bool evenDistribution(uint8_t *pData, uint16_t w, uint16_t h, uint16_t iRandomRange)
{
    for(uint16_t y = 0 ; y < h ; ++y) {
        for(uint16_t x = 0 ; x < w ; ++x) {
            uint16_t uiMinXRand = std::min(iRandomRange, x);
            uint16_t uiMaxXRand = std::min(iRandomRange, (uint16_t)(w-x-1));
            uint16_t uiMinYRand = std::min(iRandomRange, y);
            uint16_t uiMaxYRand = std::min(iRandomRange, (uint16_t)(h-y-1));
            double dX = (double)x + RAND(-uiMinXRand, uiMaxXRand);
            double dY = (double)y + RAND(-uiMinYRand, uiMaxYRand);
            double dW = (double)w;
            double dH = (double)h;
            double distTL = std::sqrt(dX*dX          +dY*dY);
            double distTR = std::sqrt((dW-dX)*(dW-dX)+dY*dY);
            double distBL = std::sqrt(dX*dX          +(dH-dY)*(dH-dY));
            double distBR = std::sqrt((dW-dX)*(dW-dX)+(dH-dY)*(dH-dY));

            double dSum = distTL + distTR + distBL + distBR;

            double dWeightTL = 0.0;
            double dWeightTR = 0.0;
            double dWeightBL = 0.0;
            double dWeightBR = 0.0;

            if(distTL == 0.0) {
                dWeightTL = 1.0;
            } else if(distTR == 0.0) {
                dWeightTR = 1.0;
            } else if(distBL == 0.0) {
                dWeightBL = 1.0;
            } else if(distBR == 0.0) {
                dWeightBR = 1.0;
            } else {
                dWeightTL = dSum / distTL;
                dWeightTR = dSum / distTR;
                dWeightBL = dSum / distBL;
                dWeightBR = dSum / distBR;

                // Normalize.
                double dNorm = dWeightTL + dWeightTR + dWeightBL + dWeightBR;
                dWeightTL /= dNorm;
                dWeightTR /= dNorm;
                dWeightBL /= dNorm;
                dWeightBR /= dNorm;
            }

            pData[x*4+y*w*4+0] = (uint8_t)(std::ceil(255.0f*dWeightTL));
            pData[x*4+y*w*4+1] = (uint8_t)(std::ceil(255.0f*dWeightTR));
            pData[x*4+y*w*4+2] = (uint8_t)(255.0f*dWeightBR);
            pData[x*4+y*w*4+3] = (uint8_t)(255.0f*dWeightBL);
        }
    }

    return true;
}

bool gradientDistribution(uint8_t *pData, uint16_t w, uint16_t h, uint16_t iRandomRange)
{
    static const uint16_t uiBorder = 5;
    double dXStep = 1.0/(double)w;
    double dYStep = 1.0/(double)h;
    for(uint16_t x = 0 ; x < w ; ++x) {
        for(uint16_t y = 0 ; y < h ; ++y) {
            uint16_t uiMinXRand = std::min(iRandomRange, (uint16_t)(x));
            uint16_t uiMaxXRand = std::min(iRandomRange, (uint16_t)(w-(x)-1));
            uint16_t uiMinYRand = std::min(iRandomRange, (uint16_t)(y));
            uint16_t uiMaxYRand = std::min(iRandomRange, (uint16_t)(h-(y)-1));

            // This will help us have the border clean of randomness.

//             if(x < uiBorder || w-x < uiBorder || y < uiBorder || h-y < uiBorder)
//                 uiMinXRand = uiMaxXRand = uiMinYRand = uiMaxYRand = 0;

            uint16_t xDist = std::abs(x - uiBorder);
            uint16_t yDist = std::abs(y - uiBorder);
            uint16_t xwDist = std::abs((w-x-1) - uiBorder);
            uint16_t yhDist = std::abs((h-y-1) - uiBorder);
            if(x < uiBorder && y < uiBorder) {
                uiMinXRand = uiMaxXRand = 0;
                uiMinYRand = uiMaxYRand = 0;
            } else if((w-x-1) < uiBorder && (h-y-1) < uiBorder) {
                uiMinXRand = uiMaxXRand = 0;
                uiMinYRand = uiMaxYRand = 0;
            } else if(x < uiBorder && (h-y-1) < uiBorder) {
                uiMinXRand = uiMaxXRand = 0;
                uiMinYRand = uiMaxYRand = 0;
            } else if((w-x-1) < uiBorder && y < uiBorder) {
                uiMinXRand = uiMaxXRand = 0;
                uiMinYRand = uiMaxYRand = 0;
            } else if(x < uiBorder) {
                uiMinXRand = uiMaxXRand = std::min((uint16_t)(uiBorder - xDist), iRandomRange);
                uiMinYRand = uiMaxYRand = 0;
            } else if(y < uiBorder) {
                uiMinYRand = uiMaxYRand = std::min((uint16_t)(uiBorder - yDist), iRandomRange);
                uiMinXRand = uiMaxXRand = 0;
            } else if((w-x-1) < uiBorder) {
                uiMinXRand = uiMaxXRand = std::min((uint16_t)(uiBorder - xwDist), iRandomRange);
                uiMinYRand = uiMaxYRand = 0;
            } else if((h-y-1) < uiBorder) {
                uiMinYRand = uiMaxYRand = std::min((uint16_t)(uiBorder - yhDist), iRandomRange);
                uiMinXRand = uiMaxXRand = 0;
            }

//             xDist = std::abs((w-x-1) - uiBorder);
//             yDist = std::abs((h-y-1) - uiBorder);
//             if((w-x-1) < uiBorder && y >= uiBorder && x >= uiBorder) {
//                 uiMinXRand = uiMaxXRand = std::min((uint16_t)(uiBorder - xDist), iRandomRange);
//                 uiMinYRand = uiMaxYRand = 0;
//             }
//             if((h-y-1) < uiBorder && x >= uiBorder && y >= uiBorder) {
//                 uiMinYRand = uiMaxYRand = std::min((uint16_t)(uiBorder - yDist), iRandomRange);
//                 uiMinXRand = uiMaxXRand = 0;
//             }

            // first, create a gradient r->g in x direction.
            double dXPos = (double)(x + RAND(-uiMinXRand, uiMaxXRand)) * dXStep;
            double dYPos = (double)(y + RAND(-uiMinYRand, uiMaxYRand)) * dYStep;

            pData[x*4+y*w*4+0] = (uint8_t)((1.0 - dXPos)*(1.0-dYPos) * 255.0);
            pData[x*4+y*w*4+1] = (uint8_t)((dXPos)*(1.0-dYPos) * 255.0);
            pData[x*4+y*w*4+2] = (uint8_t)((dXPos)*(dYPos) * 255.0);
            pData[x*4+y*w*4+3] = (uint8_t)((1.0 - dXPos)*(dYPos) * 255.0);
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    // Init the logging system.
    new FTSTools::MinimalLogger;
    new CompressorFactory;
    FTSMSG("Arkana-FTS {1} Version {2}\n", MsgType::Raw, TOOLNAME, TOOLVERSIONSTR);

    if(argc <= 3) {
        usage();
        return 0;
    }

    uint16_t w = atoi(argv[1]);
    uint16_t h = atoi(argv[2]);
    uint16_t r = atoi(argv[3]);

    uint8_t *pData = new uint8_t[w*h*4];

    gradientDistribution(pData, w, h, r);

//     if(!verify(pData, w, h)) {
//         randomFix(pData, w, h);
        verify(pData, w, h);
//     }

    fixAlpha(pData, w, h);
    ImageFormat img;
    img.createFromData(w, h, reinterpret_cast<uint32_t *>(pData));
    img.save("out.png");

    return 0;
}
