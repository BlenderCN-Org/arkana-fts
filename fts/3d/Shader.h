#ifndef D_SHADER_H
#define D_SHADER_H

#include "utilities/Singleton.h"
#include "dLib/dString/dString.h"

#include "3d/opengl_wrapper.h"
#include "3d/Math.h"
#include "3d/VertexArrayObject.h"

#include <map>
#include <list>

namespace FTS {

class Color;
class Path;

// Implementation detail.
class CompiledShader;
typedef std::unique_ptr<CompiledShader> CompiledShaderPtr;
class ShaderIncludeManager;

/// This class represents a fully-linked shader composed of a vertex shader,
/// a fragment shader and optionally a geometry shader.
class Program : public NonCopyable {
public:
    struct Attribute {
        String name;
        GLuint id;
        GLenum type;
        GLint size;

        Attribute() : id(0), type(GL_FLOAT), size(0) {};
    };

    struct Uniform {
        String name;
        GLuint id;
        GLenum type;
        GLint size;

        // Contains the ids of array elements.
        std::vector<GLuint> arrayIds;

        Uniform() : id(0), type(GL_FLOAT), size(0) {};
    };

    Program(const CompiledShader& in_pVert, const CompiledShader& in_pFrag, const String& in_sShaderName);
    Program(const CompiledShader& in_pVert, const CompiledShader& in_pFrag, const CompiledShader& in_pGeom, const String& in_sShaderName);
    virtual ~Program();

    const Attribute& vertexAttribute(const String& in_sAttribName) const;
    bool hasVertexAttribute(const String& in_sAttribName) const;
    bool setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer);
    bool setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer, GLint in_nComponents, std::size_t in_offset);

    const Uniform& uniform(const String& in_sUniformName) const;
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
    bool setUniformArrayElementInverse(const String& in_sUniformName, uint16_t in_iArrayIdx, const AffineMatrix& in_mat, bool in_transpose = false);

    void bind();
    static void unbind();

    GLuint id() const {return m_id;};
private:
    char convertTextNr(uint8_t nr) {assert(nr>=0 && nr<10); return char(nr)+'0';}
    void init(const String& in_sShaderName);

    GLuint m_id;   ///< Contains the OpenGL ID of the compiled shader.
    String m_sLog; ///< Might contain infos/warnings/errors about the shader.

    std::map<String, Attribute> m_attribs;
    std::map<String, Uniform> m_uniforms;

    // For optimization: do not re-bind the same shader:
    static GLuint m_uiCurrentlyBoundShaderId;
};

class ShaderCompileFlags;

class ShaderCompileFlag {
public:
    static const ShaderCompileFlag Lit;
    static const ShaderCompileFlag Textured;
    static const ShaderCompileFlag SkeletalAnimated;

    ShaderCompileFlag(const String& name);
    ShaderCompileFlag(const String& name, const String& value);

    ShaderCompileFlags operator|(const ShaderCompileFlag&) const;

    operator std::string() const;
    String name() const;
    String value() const;

private:
    String m_flag;
    String m_value;
};

class ShaderCompileFlags {
public:
    ShaderCompileFlags();
    ShaderCompileFlags(const ShaderCompileFlag& flag);
    ShaderCompileFlags(const ShaderCompileFlag& flag, const ShaderCompileFlag& flag2);

    ShaderCompileFlags operator|(const ShaderCompileFlag&) const;
    ShaderCompileFlags operator|(const ShaderCompileFlags&) const;
    void operator|=(const ShaderCompileFlag&);
    void operator|=(const ShaderCompileFlags&);

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
    //void clearCache();

    void loadShader(const Path& in_sFile, String in_sShaderName = String::EMPTY);
    void loadShaderCode(String in_sShaderName, String in_sShaderContent);
    bool hasShader(const String& in_sShaderName);
    void unloadShader(const String& in_sShaderName);
    String getCompiledShaderSource(const String& in_sShaderName, const ShaderCompileFlags& flags = ShaderCompileFlags());

    Program* getOrLinkProgram(const String& in_sVertexShader = DefaultVertexShader, const String& in_sFragmentShader = DefaultFragmentShader, const String& in_sGeometryShader = DefaultGeometryShader, const ShaderCompileFlags& flags = ShaderCompileFlags());
    void destroyProgramsUsing(const String& in_sShaderName);

protected:
    CompiledShaderPtr compileShader(const String& in_sShaderName, const ShaderCompileFlags& flags = ShaderCompileFlags());
    Program* getDefaultProgram();
    String buildShaderName(String baseName);
    String buildProgramName(const String& vtxBaseName, const String& fragBaseName, const String& geomBaseName, const ShaderCompileFlags& flags);

private:
    std::map<String, String> m_ShaderSources;
    std::map<String, Program*> m_linkedShaders;
    ShaderIncludeManager *m_pInclManager;

    String m_sep;
    String m_optSep;
};

}; // namespace FTS

#endif // D_SHADER_H
