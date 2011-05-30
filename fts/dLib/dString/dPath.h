#ifndef D_PATH_H
#define D_PATH_H

#include "dString.h"

namespace FTS {

class Path : public String {
protected:
    Path* cleanup();
    void escape();

public:
    Path();
    Path(const char* in_s);
    Path(const String& in_s);
    static Path wd();
    static Path datadir(const Path& in_sSubdir = "");
    static Path userdir(const Path& in_sSubdir = "");
    static Path getUserConfPath();
    static Path getScriptPath();
    virtual ~Path();

    static bool wouldEscapeFile(const String& in_sFile);
    static bool wouldEscapePath(const String& in_sFile);

    static size_t rootLength(const String& s);

    Path basename() const;
    Path ext() const;
    Path withoutExt() const;
    Path directory() const;
    Path sameDir(const Path& in_sNewFile) const;
    Path cdUp() const;

    String protocol() const;

    Path appendWithSeparator(const Path& right) const;
    Path operator+(const Path& other) const;

};

};

#endif // D_PATH_H
