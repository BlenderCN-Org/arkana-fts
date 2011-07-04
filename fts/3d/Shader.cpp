#include "Shader.h"

#include "3d/3d.h"
#include "3d/math/Vector.h"
#include "3d/math/Quaternion.h"
#include "3d/math/Matrix.h"
#include "logging/logger.h"
#include "utilities/NonCopyable.h"
#include "main/Exception.h"
#include "dLib/dBrowse/dBrowse.h"
#include "utilities/utilities.h"
#include "graphic/Color.h"

#include <map>
#include <vector>
#include <set>
#include <dLib/dFile/dFile.h>

using namespace FTS;

const String FTS::ShaderManager::DefaultVertexShader = "Default.vert";
const String FTS::ShaderManager::DefaultFragmentShader = "Default.frag";
const String FTS::ShaderManager::DefaultGeometryShader = String::EMPTY;
extern const char* sErrorVert;
extern const char* sErrorFrag;

#define D_SHADERS_DIRNAME "Shaders"

namespace FTS {
/// Abstract base-class that is used to independently either use native OpenGL
/// #include feature if available (GL_ARB_shading_language_include) or use a
/// workaround.
class ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManager() {};
    /// default destructor
    virtual ~ShaderIncludeManager() {};

    /// Adds a file to the list of files being searched for when an #include is
    /// met.
    ///
    /// \param in_sFileName The name of the file to add (and used in the
    ///                     #include directive in the shaders).
    /// \param in_sContent The content of the file.
    ///
    /// \return true if successful, false if not.
    /// \see NamedStringARB in the OpenGL extension registry.
    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent) = 0;

    /// This just calls the compile shader process. It might make some other
    /// stuff before or after the compilation, to get the includes done.
    ///
    /// \param in_id The id of the shader.
    /// \param in_sShaderName The name of the shader, for diagnostic purposes.
    /// \param in_sSrc The source code of the shader.
    ///
    /// \return An empty string if nothing special happened. You still have to
    ///         call the usual OpenGL diagnostic stuff. If the string is not
    ///         empty, an error occured in the pre/post processing and the
    ///         error message is returned.
    virtual String compileShader(const GLuint in_id, const String& in_sShaderName, String in_sSrc) = 0;
};

std::size_t findVersionLineEnd(const String& code)
{
    // As the version string has to be the FIRST line, that's it!
    return code.find("\n");
}

String injectExtensionRequirement(const String& code, const String& name)
{
    if(name.empty())
        return code;
    
    std::size_t endOfVersionLine = findVersionLineEnd(code);
    String realCode = code.left(endOfVersionLine + 1);
    realCode += "#extension " + name + " : require\n";
    realCode += String(code, endOfVersionLine);
    return realCode;
}

/// This is the concrete ShaderIncludeManager that we want to use exclusively
/// in the future, as it uses the native OpenGL include extension.
class ShaderIncludeManagerNativeGL : public ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManagerNativeGL()
    {
        glNamedStringARB = (t_glNamedStringARB)FTS::glGetProcAddress("glNamedStringARB");
        glDeleteNamedStringARB = (t_glDeleteNamedStringARB)FTS::glGetProcAddress("glDeleteNamedStringARB");

        if(    !glNamedStringARB
            || !glDeleteNamedStringARB
            || !String((const char *)glGetString(GL_EXTENSIONS) ).contains("GL_ARB_shading_language_include"))
            throw NotExistException("GL_ARB_shading_language_include", "OpenGL extension");
    }

    /// default destructor
    virtual ~ShaderIncludeManagerNativeGL()
    {
        for(std::list<Path>::iterator i = m_names.begin() ; i != m_names.end() ; ++i) {
            glDeleteNamedStringARB(-1, i->c_str());
        }
    }

    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent)
    {
        // Clear it.
        glGetError();
        glNamedStringARB(SHADER_INCLUDE_ARB, -1, ("/" + in_sFileName).c_str(), -1, in_sContent.c_str());
        if(glGetError() != GL_NO_ERROR)
            return false;
        m_names.push_back(in_sFileName);

        return true;
    }

    virtual String compileShader(const GLuint in_id, const String&, String in_sSrc)
    {
        // We split up the source into:
        //  - the version line
        //  - the extension specifications
        //  - the option defines
        //  - all the rest of the sourcecode
        String newSrc = injectExtensionRequirement(in_sSrc, "GL_ARB_shading_language_include");
        const char *src = newSrc.c_str();
        glShaderSource(in_id, 1, &src, NULL);
        glCompileShader(in_id);
        return String::EMPTY;
    }
private:
    std::list<Path> m_names;

    static const GLenum SHADER_INCLUDE_ARB = 0x8DAE;

    typedef void (APIENTRYP t_glNamedStringARB)(GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar* string);
    typedef void (APIENTRYP t_glDeleteNamedStringARB)(GLint namelen, const GLchar* name);
    t_glNamedStringARB glNamedStringARB;
    t_glDeleteNamedStringARB glDeleteNamedStringARB;
};

/// This is the concrete ShaderIncludeManager that we will use most of the time
/// right now (april 2010) as there is no graphics card driver out yet that
/// supports the native OpenGL include extension. This kind-of does it by hand:
/// it just replaces the #include line with the file's source, also adding #line
/// and #file preprocessors in order to still get valid compiler messages.
class ShaderIncludeManagerWorkaround : public ShaderIncludeManager {
public:
    /// default constructor
    ShaderIncludeManagerWorkaround() {};
    /// default destructor
    virtual ~ShaderIncludeManagerWorkaround() {};

    virtual bool addIncludeFile(const Path& in_sFileName, const String& in_sContent)
    {
        m_names[in_sFileName] = in_sContent;
        return true;
    }

    virtual String compileShader(const GLuint in_id, const String& in_sShaderName, String in_sSrc)
    {
        try {
            size_t start = 0, end = 0, line = 0;
            String sName;

            std::set<String> namesDone;

            // We replace all #include lines until there is no more left.
            while(!(sName = this->findIncludeLine(in_sShaderName, in_sSrc, start, end, line)).empty()) {
                // Check if it is registered.
                if(m_names.find(sName) == m_names.end()) {
                    throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                        ": Can't find the include file \""+sName+"\""
                        "preprocessor: There should only be whitespace trailing the #include");
                }
                // As we do not preprocess the #ifdefs, we need to somehow avoid
                // cyclic (infinite) inclusion. We do this by keeping a list of
                // already included ones. This is not ideal but this is a huge
                // workaround anyways.
                if(namesDone.find(sName) != namesDone.end()) {
                    in_sSrc.replaceStr(start, end - start, "");
                    continue;
                }
                namesDone.insert(sName);

                // And now we gotta insert it into the source code string.
                String sCode = "#line 0";
                sCode += "\n" + m_names[sName] + "\n";
                //String sCode = "\n" + m_names[sName] + "\n";
                sCode += "#line " + String::nr(line);// + " " + String::nr(in_id);
                in_sSrc.replaceStr(start, end - start, sCode);
            }
        } catch(const ArkanaException& e) {
            return e.what();
        }

        const char *src = in_sSrc.c_str();
        glShaderSource(in_id, 1, &src, NULL);
        glCompileShader(in_id);
        return String::EMPTY;
    }
private:
    std::map<Path, String> m_names;

    /// This method finds a line of the form #include "bla" in the source string.
    ///
    /// \param in_sShaderName The name of the shader being parsed, for diagnostic.
    /// \param in_sSrc The source string where to search the include line in.
    /// \param out_start The position in the string where the # sign is found.
    /// \param out_end The position in the string right before the next newline.
    /// \param out_line The line number of the found #include.
    ///
    /// \note Both output parameters are left unchanged if there is an error in
    ///       the include line or no include line has been found.
    ///
    /// \return The string between the "" of the include line. Empty string if
    ///         no include line was found.
    ///
    /// \throw CorruptDataException If something's wrong with the include line.
    String findIncludeLine(const String& in_sShaderName, const String& in_sSrc, size_t& out_start, size_t& out_end, size_t& out_line)
    {
        size_t pos = (size_t)-1, line = 0;
        do {
            line++;
            pos++; // Go behind the newline.
        //for(size_t line = 1 ; pos != std::string::npos ; line++, pos = in_sSrc.find("\n", pos)) {
            // We need the '#' to be the first char of the line.
            if(in_sSrc.getCharAt(pos) != '#') {
                continue;
            }

            size_t start = pos;

            // now we might skip whitespace after the #.
            pos++;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            // check for the word "include".
            if(in_sSrc.mid(pos, 0).left(7) != "include")
                continue;

            // skip whitespace again.
            pos += 7;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            // From now on, anything else is an error, because we detected #include already.

            // get the name string.
            if(in_sSrc.getCharAt(pos) != '"') {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    " : line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: you must use the \" sign");
            }

            // Find the end of the string.
            pos++;
            size_t eos = in_sSrc.find("\"", pos);
            if(eos == std::string::npos) {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    ": line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: missing an ending \" sign");
            }

            // Extract the include string.
            String name = in_sSrc.mid(pos, in_sSrc.len()-eos);
            pos = eos;

            // Check if there are only trailing whitespace but nothing else.
            pos++;
            while(in_sSrc.getCharAt(pos) == ' ' || in_sSrc.getCharAt(pos) == '\t') pos++;

            if(in_sSrc.getCharAt(pos) != '\r' && in_sSrc.getCharAt(pos) != '\n') {
                throw CorruptDataException(in_sShaderName, "Shader preprocessor"
                    ": line "+String::nr(line)+": Syntax error in #include "
                    "preprocessor: There should only be whitespace trailing the #include");
            }

            out_start = start;
            out_end = pos;
            out_line = line;
            return name;
        } while((pos = in_sSrc.find("\n", pos)) != std::string::npos);

        return String::EMPTY;
    }

};

/// This is an internal class only to be used by the ShaderManager and nobody
/// else. The ShaderManager keeps a collection of these CompiledShader and
/// mixes them together (by linking) however the options state to.\n
class CompiledShader : public NonCopyable {
public:
    /// This compiles the shader of type \a in_type using the given sourcecode
    /// \a in_sSourceCode.
    ///
    /// \param in_sShaderName The name of the shader, for diagnostic purposes only.
    /// \param in_sSourceCode The source-code of the shader.
    /// \param in_type The type of the shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
    ///                GL_GEOMETRY_SHADER, ...)
    /// \param in_pIncMgr The include manager to use to compile the shader.
    ///
    /// \throws CorruptDataException When the shader cannot compile.
    CompiledShader(const String& in_sShaderName, const String& in_sSourceCode, GLuint in_type, ShaderIncludeManager* in_pIncMgr)
    {
        verifGL("Shader::~CompiledShader pre " + in_sShaderName);
        FTSMSGDBG("Shader::CompiledShader: Preparing to compile " + in_sShaderName, 2);
        m_id = glCreateShader(in_type);
        String sErr = in_pIncMgr->compileShader(m_id, in_sShaderName, in_sSourceCode);

        if(!sErr.empty()) {
            throw CorruptDataException(in_sShaderName, sErr);
        }

        // We always get the info log, it might contain some useful warnings!
        GLint loglen = 0;
        glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &loglen);
        std::vector<GLchar> pszLog(loglen);     // Deleted automatically.
        glGetShaderInfoLog(m_id, loglen, NULL, &pszLog[0]);
        m_sLog = &pszLog[0];

        if(!m_sLog.empty()) {
            FTSMSGDBG("Shader::CompiledShader: info-log of " + in_sShaderName + ":\n" + m_sLog, 2);
        }

        // If there was an error, we throw it as an exception.
        GLint bCompiled = GL_FALSE;
        glGetShaderiv(m_id, GL_COMPILE_STATUS, &bCompiled);
        if(bCompiled != GL_TRUE || m_id == 0) {
            throw CorruptDataException(in_sShaderName, m_sLog);
        }

        FTSMSGDBG("Shader::CompiledShader: done with " + in_sShaderName + "; id = " + String::nr(m_id), 2);
        verifGL("Shader::~CompiledShader post " + in_sShaderName);
    };

    virtual ~CompiledShader()
    {
        if(m_id != 0) {
            verifGL("Shader::~CompiledShader pre");
            FTSMSGDBG("Shader::CompiledShader: deleting " + String::nr(m_id), 2);
            glDeleteShader(m_id);
            verifGL("Shader::~CompiledShader");
        }
    };

    GLuint id() const {return m_id;};
private:
    GLuint m_id;   ///< Contains the OpenGL ID of the compiled shader.
    String m_sLog; ///< Might contain infos/warnings/errors about the shader.
};

} // namespace FTS

FTS::Shader::Shader(CompiledShader* in_pVert, CompiledShader* in_pFrag, CompiledShader* in_pGeom, const String& in_sShaderName)
{
    verifGL("Shader::ShaderLinker: start of " + in_sShaderName);
    FTSMSGDBG("Shader::ShaderLinker: Preparing to link " + in_sShaderName + " using: " + String::nr(in_pVert->id()) + ", " + String::nr(in_pFrag->id()) + (in_pGeom ? ", " + String::nr(in_pGeom->id()) : ""), 2);

    m_id = glCreateProgram();
    glAttachShader(this->id(), in_pVert->id());
    glAttachShader(this->id(), in_pFrag->id());
    if(in_pGeom)
        glAttachShader(this->id(), in_pGeom->id());

    glLinkProgram(this->id());

    // We always get the info log, it might contain some useful warnings!
    GLint loglen = 0;
    glGetProgramiv(this->id(), GL_INFO_LOG_LENGTH, &loglen);
    std::vector<GLchar> pszLog(loglen);     // Deleted automatically.
    glGetProgramInfoLog(this->id(), loglen, NULL, &pszLog[0]);
    m_sLog = &pszLog[0];

    if(!m_sLog.empty()) {
        FTSMSGDBG("Shader::ShaderLinker: info-log of " + in_sShaderName + ":\n" + m_sLog, 2);
    }

    // If there was an error, we throw it as an exception.
    GLint bLinked = GL_FALSE;
    glGetProgramiv(this->id(), GL_LINK_STATUS, &bLinked);
    if(bLinked != GL_TRUE || this->id() == 0) {
        throw CorruptDataException(in_sShaderName, m_sLog);
    }

    // Say that the fragment shader "out" variable "Color" is the output to the screen (0).
    glBindFragDataLocation(this->id(), 0, "Color");

    verifGL("Shader::ShaderLinker: link of " + in_sShaderName);
    FTSMSGDBG("Shader::ShaderLinker: done with " + in_sShaderName + "; got id = " + String::nr(this->id()), 2);

    // If all that worked, we query all attributes and all uniforms that are
    // available in the program and store their informations.

    // First the attributes:
    GLint nAttribs = 0, nLongestAttrib = 0;
    glGetProgramiv(this->id(), GL_ACTIVE_ATTRIBUTES, &nAttribs);
    glGetProgramiv(this->id(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &nLongestAttrib);

    for(GLint i = 0 ; i < nAttribs ; ++i) {
        Attribute a;
        std::vector<GLchar> name(nLongestAttrib);
        glGetActiveAttrib(this->id(), i, nLongestAttrib, NULL, &a.size, &a.type, &name[0]);
        a.name = &name[0];

        // Wow, it took me hours to find out that the id isn't forcedly i!
        a.id = glGetAttribLocation(this->id(), a.name.c_str());

        m_attribs[a.name] = a;
        FTSMSGDBG("Shader::ShaderLinker: found attrib " + a.name + " id = " + String::nr(a.id), 3);
    }

    // Then the uniforms:
    GLint nUniforms = 0, nLongestUniform = 0;
    glGetProgramiv(this->id(), GL_ACTIVE_UNIFORMS, &nUniforms);
    glGetProgramiv(this->id(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &nLongestUniform);

    for(GLint i = 0 ; i < nUniforms ; ++i) {
        GLsizei iActualLen = 0;
        Uniform u;
        std::vector<GLchar> name(nLongestUniform);
        glGetActiveUniform(this->id(), i, nLongestUniform, &iActualLen, &u.size, &u.type, &name[0]);
        u.name = &name[0];

        // Same story here as for the attribs... Crazy shit!
        u.id = glGetUniformLocation(this->id(), u.name.c_str());

        // In case it is an array, we get the id of every single array entry...
        for(GLint i = 0 ; i < u.size ; ++i) {
            u.arrayIds.push_back(glGetUniformLocation(this->id(), (u.name + "[" + String::nr(i) + "]").c_str()));
        }

        m_uniforms[u.name] = u;
        FTSMSGDBG("Shader::ShaderLinker: found uniform " + u.name + " id = " + String::nr(u.id), 3);
    }

    verifGL("Shader::ShaderLinker: end of " + in_sShaderName);
}

bool FTS::Shader::hasVertexAttribute(const String& in_sAttribName) const
{
    return m_attribs.find(in_sAttribName) != m_attribs.end();
}

/// This method binds a vertex buffer to a certain attribute of the shader.
/// \return true if the bind succeeded, false else.
bool FTS::Shader::setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer)
{
    auto i = m_attribs.find(in_sAttribName);
    if(i == m_attribs.end())
        return false;

    verifGL("Shader::setVertexAttribute("+in_sAttribName+") start");
    // Do some verifications
    if(in_buffer.type == GL_FLOAT) {
        switch(in_buffer.nComponents) {
        case 1:
            if(i->second.type != GL_FLOAT)
                return false;
            break;
        case 2:
            if(i->second.type != GL_FLOAT_VEC2)
                return false;
            break;
        case 3:
            if(i->second.type != GL_FLOAT_VEC3)
                return false;
            break;
        case 4:
            if(i->second.type != GL_FLOAT_VEC4)
                return false;
            break;
        default:
            return false;
        }
    }

    glEnableVertexAttribArray(i->second.id);
    in_buffer.bind();
    glVertexAttribPointer(i->second.id, in_buffer.nComponents, in_buffer.type, in_buffer.normalize, in_buffer.stride, NULL);

    /// \TODO: where to disable the vertex attribute?
    verifGL("Shader::setVertexAttribute("+in_sAttribName+") end");
    return true;
}

bool FTS::Shader::hasUniform(const String& in_sUniformName) const
{
    auto i = m_uniforms.find(in_sUniformName);
    return i != m_uniforms.end();
}

bool FTS::Shader::setUniform(const String& in_sUniformName, const Vector& in_v)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_v.array4f());
        break;
    default:
        verifGL("Shader::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Shader::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniform(const String& in_sUniformName, const Color& in_c)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_c.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_c.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_c.array4f());
        break;
    default:
        verifGL("Shader::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Shader::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniform(const String& in_sUniformName, const Quaternion& in_q)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniform("+in_sUniformName+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.id, 1, in_q.array4f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.id, 1, in_q.array4f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.id, 1, in_q.array4f());
        break;
    default:
        verifGL("Shader::setUniform("+in_sUniformName+") badend");
        return false;
    };

    verifGL("Shader::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniform(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniform("+in_sUniformName+") start");
    if(i->second.type != GL_FLOAT_MAT4) {
        verifGL("Shader::setUniform("+in_sUniformName+") badend");
        return false;
    }

    glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    verifGL("Shader::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniform(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniform("+in_sUniformName+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9f());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    } else {
        verifGL("Shader::setUniform("+in_sUniformName+") badend");
        return false;
    }

    verifGL("Shader::setUniform("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniformInverse(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniformInv("+in_sUniformName+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9fInverse());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16fInverse());
    } else {
        verifGL("Shader::setUniformInv("+in_sUniformName+") badend");
        return false;
    }

    verifGL("Shader::setUniformInv("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniformInverse(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniformInv("+in_sUniformName+") start");
    if(i->second.type != GL_FLOAT_MAT4) {
        verifGL("Shader::setUniformInv("+in_sUniformName+") badend");
        return false;
    }

    glUniformMatrix4fv(i->second.id, 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16fInverse());
    verifGL("Shader::setUniformInv("+in_sUniformName+") end");
    return true;
}

bool FTS::Shader::setUniformSampler(const String& in_sUniformName, uint8_t in_iTexUnit)
{
    String sUniformName = in_sUniformName + String::chr(convertTextNr(in_iTexUnit));
    auto i = m_uniforms.find(sUniformName);
    if(i == m_uniforms.end())
        return false;

    verifGL("Shader::setUniformSampler("+in_sUniformName+") start");
    if(i->second.type != GL_SAMPLER_2D){
        verifGL("Shader::setUniformSampler("+in_sUniformName+", "+String::nr(in_iTexUnit)+") badend");
        return false;
    }

    glUniform1i(i->second.id, (GLint)in_iTexUnit);
    verifGL("Shader::setUniformSampler("+in_sUniformName+", "+String::nr(in_iTexUnit)+") end");
    return true;
}

bool FTS::Shader::setUniformArrayElement(const String& in_sUniformName, uint16_t in_iArrayIdx, const Vector& in_v)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    if(in_iArrayIdx >= i->second.size) {
        FTSMSG("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+"): index out of bounds (max is "+String::nr(i->second.size)+")\n", MsgType::WarningNoMB);
        return false;
    }

    verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") start");
    switch(i->second.type) {
    case GL_FLOAT_VEC2:
        glUniform2fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC3:
        glUniform3fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array3f());
        break;
    case GL_FLOAT_VEC4:
        glUniform4fv(i->second.arrayIds[in_iArrayIdx], 1, in_v.array4f());
        break;
    default:
        verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") badend");
        return false;
    };

    verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") end");
    return true;
}

bool FTS::Shader::setUniformArrayElement(const String& in_sUniformName, uint16_t in_iArrayIdx, const AffineMatrix& in_mat, bool in_transpose)
{
    auto i = m_uniforms.find(in_sUniformName);
    if(i == m_uniforms.end())
        return false;

    if(in_iArrayIdx >= i->second.size) {
        FTSMSG("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+"): index out of bounds (max is "+String::nr(i->second.size)+")\n", MsgType::WarningNoMB);
        return false;
    }

    verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") start");
    if(i->second.type == GL_FLOAT_MAT3) {
        glUniformMatrix3fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array9f());
    } else if(i->second.type == GL_FLOAT_MAT4) {
        glUniformMatrix4fv(i->second.arrayIds[in_iArrayIdx], 1, in_transpose ? GL_TRUE : GL_FALSE, in_mat.array16f());
    } else {
        verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") badend");
        return false;
    }

    verifGL("Shader::setUniformArrayElement("+in_sUniformName+", "+String::nr(in_iArrayIdx)+") end");
    return true;
}

GLuint Shader::m_uiCurrentlyBoundShaderId = (GLuint)-1;

void FTS::Shader::bind()
{
    // Do not re-bind the same shader again.
    if(this->id() == m_uiCurrentlyBoundShaderId)
        return;

    verifGL("Shader::bind("+String::nr(this->id())+") start");
    glUseProgram(this->id());
    m_uiCurrentlyBoundShaderId = this->id();
    verifGL("Shader::bind("+String::nr(this->id())+") end");
}

void Shader::unbind()
{
    glUseProgram(0);
    m_uiCurrentlyBoundShaderId = (GLuint)-1;
}

FTS::Shader::~Shader()
{
    if(m_id != 0) {
        verifGL("Shader::~Shader("+String::nr(this->id())+") start");
        glDeleteProgram(this->id());
        verifGL("Shader::~Shader("+String::nr(this->id())+") end");
    }
}

FTS::ShaderManager::ShaderManager()
    : m_sep("|")
{
    String sInfo;

    // Get some interesting informations.
    GLint i = 0, i2 = 0, i3 = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &i);
    sInfo += "\nMaximum vertex attributes: " + String::nr(i) + " (min: 16)";
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &i);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &i2);
    glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &i3);
    sInfo += "\nMaximum vertex/fragment/combined uniform components: " + String::nr(i) + "/" + String::nr(i2) + "/" + String::nr(i3) + " (min: 16, w00t)";

    FTSMSGDBG("Initialised shader manager, some OpenGL informations: "+sInfo, 2);

    // We need that extension to work. This is only included in recent drivers!
    // Recent as of March/April 2010
    try {
        m_pInclManager = new ShaderIncludeManagerNativeGL();
    } catch(const ArkanaException&) {
        // If that extension is not available, we need to use our workaround.
        m_pInclManager = new ShaderIncludeManagerWorkaround();
    }

    // First, load every possible shader include files.
    Path sPath;
    PDBrowseInfo dbi = dBrowse_Open(Path::datadir(D_SHADERS_DIRNAME));
    while(!(sPath = dBrowse_GetNextWithWildcard(dbi, "*.shadinc")).empty()) {
        this->loadShader(sPath);
    }
    dBrowse_Close(dbi);

    /// \TODO What to do if those default shaders fail?? They actually shouldn't
    this->makeShader(ShaderManager::DefaultVertexShader, sErrorVert);
    this->makeShader(ShaderManager::DefaultFragmentShader, sErrorFrag);

    // Also link the default shaders already:
    this->getOrLinkShader(); // Default arguments are "Default.*".

    // And now we load every shader we can find in the folder and try to compile
    // it, so later on we can get a list of "working shaders".
    dbi = dBrowse_Open(Path::datadir(D_SHADERS_DIRNAME));
    while(!(sPath = dBrowse_GetNextWithWildcard(dbi, "*.vert")).empty()) {
        this->loadShader(sPath);
    }
    dBrowse_Close(dbi);

    dbi = dBrowse_Open(Path::datadir(D_SHADERS_DIRNAME));
    while(!(sPath = dBrowse_GetNextWithWildcard(dbi, "*.frag")).empty()) {
        this->loadShader(sPath);
    }
    dBrowse_Close(dbi);

    dbi = dBrowse_Open(Path::datadir(D_SHADERS_DIRNAME));
    while(!(sPath = dBrowse_GetNextWithWildcard(dbi, "*.geom")).empty()) {
        this->loadShader(sPath);
    }
    dBrowse_Close(dbi);
}

FTS::ShaderManager::~ShaderManager()
{
    FTSMSGDBG("Destroying shader manager", 2);

    // Delete the builtin shaders.
    SAFE_DELETE(m_compiledVertexShaders["Default.vert"]);
    SAFE_DELETE(m_compiledFragmentShaders["Default.frag"]);
    m_compiledVertexShaders.erase("Default.vert");
    m_compiledFragmentShaders.erase("Default.frag");

    // Everything should already be deleted by now.
    // That's why we print a warning for everything that is left.
    // Or not?

    for(auto i = m_compiledVertexShaders.begin() ; i != m_compiledVertexShaders.end() ; ++i) {
        //FTS18N("Shader_NotUnloaded_File", MsgType::Warning, i->first);

        SAFE_DELETE(i->second);
    }

    for(auto i = m_compiledFragmentShaders.begin() ; i != m_compiledFragmentShaders.end() ; ++i) {
        //FTS18N("Shader_NotUnloaded_File", MsgType::Warning, i->first);

        SAFE_DELETE(i->second);
    }

    for(std::map<String, CompiledShader*>::iterator i = m_compiledGeometryShaders.begin() ; i != m_compiledGeometryShaders.end() ; ++i) {
        //FTS18N("Shader_NotUnloaded_File", MsgType::Warning, i->first);

        SAFE_DELETE(i->second);
    }

    m_compiledVertexShaders.clear();
    m_compiledFragmentShaders.clear();
    m_compiledGeometryShaders.clear();

    SAFE_DELETE(m_pInclManager);
}

bool FTS::ShaderManager::loadShader(const Path& in_sFile, const String& in_sShaderName)
{
    String sShaderName = in_sShaderName.empty() ? in_sFile : in_sShaderName;

    try {
        // Already loaded?
        try {
            this->findShader(sShaderName);
            return true;
        } catch(...) { }

        return this->makeShader(sShaderName, File::open(Path::datadir(D_SHADERS_DIRNAME) + in_sFile, File::Read)->readstr());
    } catch(const ArkanaException& e) {
        e.show();
        return false;
    }
}

bool FTS::ShaderManager::makeShader(const String& in_sShaderName, const String& in_sShaderContent)
{
    // If the file is located within the shaders directory (or somewhere deeper)
    // we take out the whole shaders directory prefix so we kindo got "relative"
    // names to a kind of "include directory".
    String prettyShaderName = in_sShaderName;
    prettyShaderName.replaceStr(D_SHADERS_DIRNAME, "");

    // Already loaded?
    try {
        this->findShader(prettyShaderName);
        return true;
    } catch( NotExistException&) {
        // We expect this if the shader wasn't load.
    } catch(...) { 
        FTSMSG("Unknonwn exception makeShader {1}", FTS::MsgType::Warning, prettyShaderName);
    }

    try {
        if(Path(prettyShaderName).ext() == "vert") {
            m_compiledVertexShaders[prettyShaderName] = new CompiledShader(in_sShaderName, in_sShaderContent, GL_VERTEX_SHADER, m_pInclManager);
        } else if(Path(prettyShaderName).ext() == "frag") {
            m_compiledFragmentShaders[prettyShaderName] = new CompiledShader(in_sShaderName, in_sShaderContent, GL_FRAGMENT_SHADER, m_pInclManager);
        } else if(Path(prettyShaderName).ext() == "geom") {
            m_compiledGeometryShaders[prettyShaderName] = new CompiledShader(in_sShaderName, in_sShaderContent, GL_GEOMETRY_SHADER, m_pInclManager);
        } else if(Path(prettyShaderName).ext() == "shadinc") {
            m_pInclManager->addIncludeFile(prettyShaderName, in_sShaderContent);
        }

        return true;
    } catch(const ArkanaException& e) {
        e.show();
        // Just skip bogous shaders.

        return false;
    }
}

/// Find a compiled shader in my collection, by name, whatever kind of shader it be.
///
/// \return A pointer to the compiled shader.
///
/// \exceptions NotFoundException if the shader doesn't exist.
CompiledShader* FTS::ShaderManager::findShader(const String& in_sShaderName)
{
    auto i = m_compiledVertexShaders.find(in_sShaderName);
    if(i != m_compiledVertexShaders.end())
        return i->second;
    i = m_compiledFragmentShaders.find(in_sShaderName);
    if(i != m_compiledFragmentShaders.end())
        return i->second;
    i = m_compiledGeometryShaders.find(in_sShaderName);
    if(i != m_compiledGeometryShaders.end())
        return i->second;

    throw NotExistException(in_sShaderName);
}

bool FTS::ShaderManager::hasShader(const String& in_sShaderName)
{
    try {
        this->findShader(in_sShaderName);
        return true;
    } catch(...) {
        return false;
    }
}

String FTS::ShaderManager::getSource(const String& in_sShaderName)
{
    // We always get the info log, it might contain some useful warnings!
    GLint srclen = 0;
    glGetShaderiv(this->findShader(in_sShaderName)->id(), GL_SHADER_SOURCE_LENGTH, &srclen);
    GLint realsrclen = 0;
    std::vector<GLchar> pszSrc(srclen);     // Deleted automatically.
    glGetShaderSource(this->findShader(in_sShaderName)->id(), srclen, &realsrclen, &pszSrc[0]);
    return String(&pszSrc[0]);
}

void FTS::ShaderManager::destroyShader(const FTS::String& in_sShaderName)
{
    // If there is no such shader, we just ignore the request.
    if(!this->hasShader(in_sShaderName)) {
        FTS18N("Want to destroy the inexistent shader " + in_sShaderName, MsgType::Warning);
        return ;
    }

    // We definitely need to check if this shader has been used in linked shaders.
    // If this is the case, we destroy the linked shader too.
    // We do this because else it defeats the whole sense of destroying a shader.
    for(auto i = m_linkedShaders.begin() ; i != m_linkedShaders.end() ; ++i) {
        if(i->first.contains(in_sShaderName + m_sep) || i->first.contains(m_sep + in_sShaderName)) {
            delete i->second;
            m_linkedShaders.erase(i);
            break;
        }
    }

    // Finally, take out the shader itself. We already made sure above that this
    // line returns something valid.
    delete this->findShader(in_sShaderName);

    m_compiledVertexShaders.erase(in_sShaderName);
    m_compiledFragmentShaders.erase(in_sShaderName);
    m_compiledGeometryShaders.erase(in_sShaderName);
    verifGL("post-destroy shader " + in_sShaderName);
}

FTS::Shader* FTS::ShaderManager::getOrLinkShader(const String& in_sVertexShader, const String& in_sFragmentShader, const String& in_sGeometryShader)
{
    // Check if we got that one cached (linked) already?
    String sLinkedShaderName = (in_sVertexShader.empty() ? DefaultVertexShader : in_sVertexShader) + m_sep
                             + (in_sFragmentShader.empty() ? DefaultFragmentShader : in_sFragmentShader) + m_sep
                             + (in_sGeometryShader.empty() ? DefaultGeometryShader : in_sGeometryShader);
    auto iCached = m_linkedShaders.find(sLinkedShaderName);
    if(iCached != m_linkedShaders.end())
        return iCached->second;

    // If it's not been linked in this combination yet, do this now!
    // Use the default ones as fallbacks.
    CompiledShader* pVert = m_compiledVertexShaders[DefaultVertexShader];
    CompiledShader* pFrag = m_compiledFragmentShaders[DefaultFragmentShader];
    CompiledShader* pGeom = nullptr;

    auto i = m_compiledVertexShaders.find(in_sVertexShader);
    if(i != m_compiledVertexShaders.end()) {
        pVert = i->second;
    }
    i = m_compiledFragmentShaders.find(in_sFragmentShader);
    if(i != m_compiledFragmentShaders.end()) {
        pFrag = i->second;
    }
    i = m_compiledGeometryShaders.find(in_sGeometryShader);
    if(i != m_compiledGeometryShaders.end()) {
        pGeom = i->second;
    }

    // Link them. If they don't fit, replace it by the default ones linked.
    try {
        return m_linkedShaders[sLinkedShaderName] = new Shader(pVert, pFrag, pGeom, sLinkedShaderName);
    } catch(const CorruptDataException& e) {
        // Use the all-default shader in case of failure.

        // But if that one doesn't even exist, we're doomed!
        if(sLinkedShaderName == DefaultVertexShader+m_sep+DefaultFragmentShader+m_sep+DefaultGeometryShader) {
            e.show();
        }

        return m_linkedShaders[sLinkedShaderName] = m_linkedShaders[DefaultVertexShader+m_sep+DefaultFragmentShader+m_sep+DefaultGeometryShader];
    }
}
