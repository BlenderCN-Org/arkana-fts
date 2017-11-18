#ifndef D_BROWSE_H
#define D_BROWSE_H


#include "main.h"

#include <vector>

#include "dLib/dString/dString.h"

std::vector<FTS::String> dBrowse(const FTS::String &in_sDir, const FTS::String &wc = FTS::String::EMPTY);

#endif

 /* EOF */
