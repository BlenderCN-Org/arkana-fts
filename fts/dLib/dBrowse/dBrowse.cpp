#include "dBrowse.h"
#include "dLib/dString/dString.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

/*! Returns the list of file names w/o path
 *\param[in] in_sDir    The directory to browse.
 *\param[in] ec         Wildcard specifier.
 *\return a vector of all file names (name and extension)
 */
std::vector<FTS::String> dBrowse(const FTS::String &in_sDir, const FTS::String &wc)
{
    std::vector<FTS::String> ret;
    auto pattern = wc;
    if(wc == FTS::String::EMPTY) {
        pattern = "*";
    }
    for(auto& p : fs::directory_iterator(in_sDir.c_str())) {
        FTS::String name = p.path().filename().generic_string();
        if(name.matchesPattern(pattern)) {
            ret.push_back(name);
        }
    }
    return ret;
}
