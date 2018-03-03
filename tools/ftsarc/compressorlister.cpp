#include "compressorlister.h"
#include "logging/logger.h"
#include "dLib/dCompressor/dCompressor.h"

using namespace FTSArc;
using namespace FTS;

CompressorLister::CompressorLister()
{
}

CompressorLister::~CompressorLister()
{
}

int CompressorLister::execute()
{
    FTSMSG("These are all the compressors currently available:\n");

    std::list< std::pair<String, String> >lComps = CompressorFactory::getSingletonPtr()->list();
    std::list< std::pair<String, String> >::iterator i;
    for(i = lComps.begin() ; i != lComps.end() ; ++i) {
        FTSMSG(" * {1} ({2})\n", FTS::MsgType::Raw, i->first, i->second);
    }

    return ERR_OK;
}
