#ifndef D_SHADER_H
#define D_SHADER_H

#include "utilities/Singleton.h"
#include "dLib/dString/dString.h"

#include "3d/opengl_wrapper.h"
#include "3d/math/Matrix.h"
#include "3d/VertexArrayObject.h"

#include <map>
#include <list>

namespace FTS {

class Color;
class Path;

// Implementation detail.
class CompiledShader;
class ShaderIncludeManager;

/// This class represents a fully-linked shader composed of a vertex shader,
/// a fragment shader and optionally a geometry shader.
class Program : public NonCopyable {
public:
    Program(CompiledShader* in_pVert, CompiledShader* in_pFrag, CompiledShader* in_pGeom, const String& in_sShaderName);
    virtual ~Program();

    bool hasVertexAttribute(const String& in_sAttribName) const;
    bool setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer);
    bool setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer, GLint in_nComponents, std::size_t in_offset);

    bool hasUniform(const String& in_sUniformName) const;
    bool setUniform(const String& in_sUniformName, float in_v);
    bool setUniform(const String& in_sUniformName, const Vector& in_v);
    bool setUniform(const String& in_sUniformName, const Color& in_c);
    bool setUniform(const String& in_sUniformName, const Quaternion& in_q);
    bool setUniform(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose = false);
    bool setUniform(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose = false);
    bool setUniformInverse(const String& in_sUniformName, const General4x4Matrix& in_mat, bool in_transpose = false);
    bool setUniformInverse(const String& in_sUniformName, const AffineMatrix& in_mat, bool in_transpose = false);
    bool setUniformSampler(const String& in_sUniformName, uint8_t in_iTexUnit);
    bool setUniformArrayElement(const String& in_sUniformName, uint16_t in_iArrayIdx, const Vector& in_v);
    bool setUniformArrayElement(const String& in_sUniformName, uint16_t in_iArrayIdx, const AffineMatrix& in_mat, bool in_transpose = false);

    void bind();
    static void unbind();

    GLuint id() const {return m_id;};
private:
    char convertTextNr(uint8_t nr) {assert(nr>=0 && nr<10); return char(nr)+'0';}
    GLuint m_id;   ///< Contains the OpenGL ID of the compiled shader.
    String m_sLog; ///< Might contain infos/warnings/errors about the shader.

    struct Attribute {
        String name;
        GLuint id;
        GLenum type;
        GLint size;

        Attribute() : id(0), type(GL_FLOAT), size(0) {};
    };
    std::map<String, Attribute> m_attribs;

    struct Uniform {
        String name;
        GLuint id;
        GLenum type;
        GLint size;

        // Contains the ids of array elements.
        std::vector<GLuint> arrayIds;

        Uniform() : id(0), type(GL_FLOAT), size(0) {};
    };
    std::map<String, Uniform> m_uniforms;

    // For optimization: do not re-bind the same shader:
    static GLuint m_uiCurrentlyBoundShaderId;
};

class ShaderCompileFlags;

class ShaderCompileFlag {
public:
    static const ShaderCompileFlag Lit;
    static const ShaderCompileFlag Textured;

    ShaderCompileFlag(const String& name);

    ShaderCompileFlags operator|(const ShaderCompileFlag&);

    inline operator std::string() const {return this->name().str();};
    String name() const;

private:
    String m_flag;
};

class ShaderCompileFlags {
public:
    ShaderCompileFlags();
    ShaderCompileFlags(const ShaderCompileFlag& flag);
    ShaderCompileFlags(const ShaderCompileFlag& flag, const ShaderCompileFlag& flag2);

    ShaderCompileFlags operator|(const ShaderCompileFlags&);

    const std::list<ShaderCompileFlag>& flags() const;
    String toString() const;
    bool empty() const;

private:
    std::list<ShaderCompileFlag> m_flags;
};

/// This is the shader manager that keeps track of all existing shaders,
/// creates new shaders, combines them and deletes them.\n
/// \note Right now, it keeps all compiled shaders in memory until the end,
///       this *might* be memory waste, but only time will tell.
class ShaderManager : public Singleton<ShaderManager> {
public:
    static const String DefaultVertexShader;
    static const String DefaultFragmentShader;
    static const String DefaultGeometryShader;

    ShaderManager();
    virtual ~ShaderManager();
    void clearCache();

    CompiledShader* compileShader(const Path& in_sFile, const ShaderCompileFlags& flags = ShaderCompileFlags(), String in_sShaderName = String::EMPTY);
    CompiledShader* compileShaderCode(const String& in_sShaderName, const String& in_sShaderContent, const ShaderCompileFlags& flags = ShaderCompileFlags());
    bool hasShader(const String& in_sShaderName, const ShaderCompileFlags& flags = ShaderCompileFlags());
    String getShaderSource(const String& in_sShaderName, const ShaderCompileFlags& flags = ShaderCompileFlags());
    void destroyShader(String in_sShaderName, const ShaderCompileFlags& flags = ShaderCompileFlags());

    Program* getOrLinkProgram(const String& in_sVertexShader = DefaultVertexShader, const String& in_sFragmentShader = DefaultFragmentShader, const String& in_sGeometryShader = DefaultGeometryShader, const ShaderCompileFlags& flags = ShaderCompileFlags());

private:
    std::map<String, CompiledShader*> m_compiledVertexShaders;
    std::map<String, CompiledShader*> m_compiledFragmentShaders;
    std::map<String, CompiledShader*> m_compiledGeometryShaders;
    std::map<String, Program*> m_linkedShaders;
    ShaderIncludeManager *m_pInclManager;

    CompiledShader* findShader(String in_sShaderName, const ShaderCompileFlags& flags);
    String buildShaderName(String baseName, const ShaderCompileFlags& flags);
    String buildProgramName(const String& vtxBaseName, const String& fragBaseName, const String& geomBaseName, const ShaderCompileFlags& flags);

    String m_sep;
    String m_optSep;
};

}; // namespace FTS

#endif // D_SHADER_H
