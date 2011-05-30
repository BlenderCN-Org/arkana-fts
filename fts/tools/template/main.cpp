#define TOOLNAME "__TEMPLATE__"
#define TOOLVERSIONSTR "0.1"

#include "../toolcompat.h"
// #include "graphic/image.h"

using namespace FTS;
using namespace FTSTools;

void usage()
{
// EXAMPLE
//     FTSMSG("Usage: {1} [-o FMT] [FILE_1 FILE_2 ... FILE_N]", MsgType::Raw, TOOLNAME);
//     FTSMSG("Converts all given files from their original format into the Arkana-FTS Image format.", MsgType::Raw);
//     FTSMSG("Options:", MsgType::Raw);
//     FTSMSG("  -o FMT choose the output format FMT. FMT is the number in parantesis in front of", MsgType::Raw);
//     FTSMSG("         the output format as listed below. For example: -o 01", MsgType::Raw);
//     FTSMSG("         If none is specified, the first in the list is taken as a default", MsgType::Raw);
//     FTSMSG("Supported input formats:", MsgType::Raw);
//     for(int iFormat = 0 ; informats[iFormat] != NULL ; iFormat++) {
//         FTSMSG("  * " + informats[iFormat]->getDescription(), MsgType::Raw);
//     }
//     FTSMSG("Supported output formats:", MsgType::Raw);
//     for(int iFormat = 0 ; outformats[iFormat] != NULL ; iFormat++) {
//         FTSMSG("  ("+String::nr(iFormat,2)+") " + outformats[iFormat]->getDescription(), MsgType::Raw);
//     }
}

int main(int argc, char *argv[])
{
    // Init the logging system.
    new MinimalLogger;
    FTSMSG("Arkana-FTS {1} Version {2}\n", MsgType::Raw, TOOLNAME, TOOLVERSIONSTR);

    if(argc <= 1) {
        usage();
        return 0;
    }

//     if(argv[1][0] == '-'){
        // Output format specified.
//         if(argv[1][1] == 'o') {
//         } else {
//             usage();
//             return 0;
//         }
//     }

    return 0;
}
