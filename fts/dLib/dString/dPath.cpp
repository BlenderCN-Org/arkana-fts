#include "dPath.h"
#include <experimental/filesystem>
#include <cstdlib>

using namespace FTS;
namespace fs = std::experimental::filesystem;

FTS::Path::Path()
    : String()
{
}

FTS::Path FTS::Path::wd()
{
    auto cwd = std::experimental::filesystem::current_path();
    FTS::Path ret(cwd.string());
    return ret;
}

FTS::Path FTS::Path::datadir(const Path& in_sSubdir)
{
    /// \TODO Probably place the game's data under /usr/share/arkana-fts/... on linux?
    return FTS::Path(DATA) + in_sSubdir;
}

FTS::Path FTS::Path::userdir(const Path& in_sSubdir)
{
    Path path;
#if defined(_MSC_VER)
    path = std::getenv ( "APPDATA" );
    path = path + Path ( "arkana-fts" ) ;
#else
    path = std::getenv ( "HOME" );
    path = path + Path ( ".arkana-fts" ) ;
#endif
    return path + in_sSubdir;
}

FTS::Path FTS::Path::getUserConfPath()
{
    return userdir("Confs");
}

Path Path::getScriptPath()
{
    return datadir("Scripts");
}

FTS::Path::Path(const char* in_s)
    : String(in_s)
{
    this->cleanup();
}

FTS::Path::Path(const String& in_s)
    : String(in_s)
{
    this->cleanup();
}

FTS::Path::Path(const std::string& in_s)
    : String(in_s)
{
    this->cleanup();
}

FTS::Path::~Path()
{
}

/// Cleans up the current path.
/** This cleans up the path. That means it first escapes the path and the file,
 *  then it will remove some nasty things in the path like /./ and so on.
 *  It will also remove the beginning ./ or .\ if there are.
 *  Additionally, it will remove any trailing (back)slashes and points.
 *
 * \return The cleaned string.
 *
 * \TODO: also clean out things like ./c/d/../a/\ would become c/a
 *
 * \author Pompei2
 */
Path* Path::cleanup()
{
    size_t oldLen = 0;
    // Cleanup some bad stuff first ...
    this->replaceStr("\\", FTS_DIR_SEPARATOR);

    do {
        oldLen = this->len();
        this->replaceStr("/./", FTS_DIR_SEPARATOR);
        this->replaceStr("//", FTS_DIR_SEPARATOR);
    } while(this->len() < oldLen);

//     if(this->isEmpty())
//         *this = ".";

    // Removes all beginning ./
//     while(this->ncmp("./", 2)) {
        // Remove these two first characters.
//         this->removeChar(0).removeChar(0);
//     }

    // And the directory '.' becomes nothing.
//     if(*this == ".")
//         this->removeChar(0);

    // Removes all ending /.
//     while(this->right(2).ncmp("/.", 2) && this->len() > Path::rootLength(*this)) {
        // Remove these two first characters.
//         this->removeChar(-1).removeChar(-1);
//     }

    // Also, remove every trailing slash! Except the ones that are part of the
    // root, we want to keep them.
    this->replaceStr(Path::rootLength(*this), -1, this->mid(Path::rootLength(*this), 0).trimRight("/"));

    // Now replace all invalid characters by an underscore.
    this->escape();

    // TODO: Now we could clean out the ".." we find ...

    return this;
}

/// Returns escaped path.
/** This replaces all special signs that can't be in pathnames with an underscore.
 *
 * \return The escaped string.
 *
 * Example: \n \code
 *       String esc = Path::escapePath("/us:r/s?hare/a*a/blub/dat?a.co*nf");
 *       // esc is now "/us:r/s_hare/a_a/blub/dat_a.co_nf" \endcode
 *
 * \author Pompei2
 */
void Path::escape()
{
    // Here, we allow the : symbol because we use it to refer to files that
    // are located inside an archive for example, or because we use it for the
    // protocol too, for example in http://blabla.
    for(std::string::size_type i = 0; i < this->len(); i++) {
        if(   this->getCharAt(i) == '*'
           || this->getCharAt(i) == '?'
           || this->getCharAt(i) == '"'
           || this->getCharAt(i) == '<'
           || this->getCharAt(i) == '>'
           || this->getCharAt(i) == '|'
          ) {
            this->operator[](i) = '_';
        }
    }
}

/// Returns whether the filename would be escaped or not.
/** This looks if this filename would be escaped by the escapeFile method.
 *
 * \param in_sPath The filename to check.
 *
 * \return true if the filename would escape, false if it's a valid filename.
 *
 * \author Pompei2
 */
bool Path::wouldEscapeFile(const String& in_sFile)
{
    for(std::string::size_type i = 0; i < in_sFile.len(); i++) {
        if(    in_sFile.getCharAt(i) == ':'
            || in_sFile.getCharAt(i) == '*'
            || in_sFile.getCharAt(i) == '?'
            || in_sFile.getCharAt(i) == '"'
            || in_sFile.getCharAt(i) == '<'
            || in_sFile.getCharAt(i) == '>'
            || in_sFile.getCharAt(i) == '/'
            || in_sFile.getCharAt(i) == '\\'
            || in_sFile.getCharAt(i) == '|'
        ) {
            return true;
        }
    }

    return false;
}

/// Returns whether the pathname would be escaped.
/** This looks if this pathname would be escaped by the escapePath function.
 *
 * \param in_sPath The name of the path.
 *
 * \return true if the path would escape, false if it's a correct path name.
 *
 * \author Pompei2
 */
bool Path::wouldEscapePath(const String& in_sPath)
{
    for(size_t i = 0; i < in_sPath.len(); i++) {
        if(   in_sPath.getCharAt(i) == ':'
           || in_sPath.getCharAt(i) == '*'
           || in_sPath.getCharAt(i) == '?'
           || in_sPath.getCharAt(i) == '"'
           || in_sPath.getCharAt(i) == '<'
           || in_sPath.getCharAt(i) == '>'
           || in_sPath.getCharAt(i) == '|'
          ) {
            // Exception: a colon (':') is allowed as second character on windows
            if(i == 1 && in_sPath.getCharAt(1) == ':' && isalpha(in_sPath.getCharAt(0)))
                continue;

            return true;
        }
    }

    return false;
}

size_t Path::rootLength(const String& s)
{
    auto p = fs::path(s.c_str());
    return p.root_path().string().size();
}

/// returns the base name of this path (like the Linux command).
/** Gives you the last component of a path. Works just like the Linux base name
 *  command.
 *
 * \return a new path, containing the base name.
 *
 * Example:\n \code
 *  Path path("c:\\Program Files\\FTS\\data.conf");
 *  FTSMSGDBG("reading file: "+path.basename());\endcode
 *
 * This will print "reading file: data.conf" as debug info.
 *
 * \author Pompei2
 */
Path Path::basename() const
{
    auto p = fs::path(this->c_str());
    return p.filename().string();
}

/// Returns a string containing the extension of the filename this string represents.
/** This creates a new string which contains everything that is behind the last
 *  point of this string. If this string is a filename, \a ext will give the
 *  file's extension back, hence the name of this method.
 *
 * \Note All leading and trailing points in the base name are removed!
 *
 * \return The extension of the file (w/o the dot). If there is no dot, returns
 *         the empty string.
 *
 * Example: \n \code
 *       Path p("c:/Program Files/F.T.S/.data.conf.");
 *       Path e = p.ext( );
 *       // e is now "conf" \endcode
 *
 * \author Pompei2
 */
Path Path::ext() const
{
    auto p = fs::path(this->c_str());
    auto ext = p.extension().string();
    // remove the point.
    return ext.substr(1);
}

/// Returns a string similar to this one but without the extension.
/** This creates a new string which contains exactly the same text as this
 *  except that the extension is cut off, that means the last point and all
 *  that follows isn't in the returned string.
 *
 * \return The path to the file, w/o extension. Note that if there is no dot in
 *         the string, the full string is returned! If the only dot in the
 *         is the first character and there is no other dot, the full string is
 *         returned (to be compatible with Unix hidden file names).
 *
 * Example: \n \code
 *       Path p("c:\\Program Files\\FTS\\data.conf");
 *       Path core = p.basename( ).noExt( );
 *       // core is now "data" \endcode
 *
 * \author Pompei2
 */
Path Path::withoutExt() const
{
    auto p = fs::path(this->c_str());
    return p.replace_extension("").string();
}

/// Returns a path containing only the path to the directory this file resides in.
/** This creates a new string which contains the same as this, but without the filename.
 *  That means the path to the current file.
 *
 * \return The path name.
 *
 * Example: \n \code
 *       Path p1("/usr/share/FTS/data.conf");
 *       Path p2("C:/bla");
 *       String dir1 = p1.directory();
 *       String dir3 = p2.directory();
 *       // dir1 is now "/usr/share/FTS/" and dir3 is "C:/" \endcode
 *
 * \note If the name contains no path but only the file, either "." or "./" is
 *       returned, depending on the value of \a in_bTrailing.
 *
 * \author Pompei2
 */
Path Path::directory() const
{
    size_t lastSlash = this->str().find_last_of(FTS_DIR_SEPARATOR);

    if(lastSlash == std::string::npos) {
        return Path(".");
    }

    if(lastSlash == 0) {
        return Path("/");
    }

    return this->left(lastSlash);
}

/// Returns a path to the file in_pszNewFile, but in the same directory as me.
/** This creates a new string which contains a path to a file. This file is
 *  located in the same directory as the file that this string contains, but
 *  has the name \a in_sNewFile
 *
 * \param in_sNewFile The filename you want in this directory.
 *
 * \return The new string.
 *
 * Example: \n \code
 *       Path p1( "/usr/share/fts/blub/data.conf" );
 *       Path other = p.sameDir( "/bla/blub.conf" );
 *       // other is now "/usr/share/fts/blub/blub.conf" \endcode
 *
 * \author Pompei2
 */
Path Path::sameDir(const Path &in_sNewFile) const
{
    return this->directory().appendWithSeparator(in_sNewFile.basename());
}

/// Goes up one directory.
/** This creates a new string which contains a directory one above this.
 *  If this is the root, it stays the root. If this is empty or a dot,
 *  it means that this is the current directory ; it will be determined and
 *  it will go one level up.
 *
 * \return The path to the directory one above.
 *
 * \author Pompei2
 */
Path Path::cdUp() const
{
    // If this is the root, it stays the root.
    if(Path::rootLength(*this) == this->len())
        return *this;

    Path curr(*this);

    // If we are in the app's directory, determine it.
    if(*this == "." || this->empty()) {
        curr = Path::wd();
    }

    // To do this, we remove the last component of the path.
    return curr.directory();
}

/** This creates a new string which contains the protocol of this path.
 *  The protocol is whatever comes from the beginning of the path up to
 *  the "://" symbol, not including the latter. If not indicated, the protocol
 *  defaults to "file".
 *
 * \return The protocol.
 *
 * Example: \n \code
 *       Path p1( "http://www.arkana-fts.org/bla/blub/data.conf" );
 *       Path p2( "file:///bla/blub.conf" );
 *       Path p3( "C:/bla/blub.conf" );
 *       // p1.protocol() gives "http", p2.protocol() gives "file" and p3.protocol gives "file" \endcode
 *
 * \author Pompei2
 */
String Path::protocol() const
{
    size_t s = this->find("://");
    if(s == (size_t)-1) {
        return "file";
    } else {
        return this->left(s);
    }
}

Path Path::appendWithSeparator(const Path& right) const
{
    if(this->empty())
        return right;
    // convert to string in order to avoid infinite recursion because
    // of the overloaded + operator.
    return String(*this) + FTS_DIR_SEPARATOR + String(right);
}

Path Path::operator+(const Path& other) const
{
    return this->appendWithSeparator(other);
}
