#ifndef D_SHADER_H
#define D_SHADER_H

#include "utilities/Singleton.h"
#include "dLib/dString/dString.h"

#include "3d/opengl_wrapper.h"
#include "3d/math/Matrix.h"
#include "3d/VertexArrayObject.h"

#include <map>
#include <vector>

namespace FTS {

class Color;
class Path;

// Implementation detail.
class CompiledShader;
class ShaderIncludeManager;

/// This class represents a fully-linked shader composed of a vertex shader,
/// a fragment shader and optionally a geometry shader.
class Shader : public NonCopyable {
public:
    Shader(CompiledShader* in_pVert, CompiledShader* in_pFrag, CompiledShader* in_pGeom, const String& in_sShaderName);
    virtual ~Shader();

    bool hasVertexAttribute(const String& in_sAttribName) const;
    bool setVertexAttribute(const String& in_sAttribName, const VertexBufferObject& in_buffer);

    bool hasUniform(const String& in_sUniformName) const;
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

    bool loadShader(const Path& in_sFile, const String& in_sShaderName = String::EMPTY);
    bool makeShader(const String& in_sShaderName, const String& in_sShaderContent);
    bool hasShader(const String& in_sShaderName);
    String getSource(const String& in_sShaderName);
    void destroyShader(const String& in_sShaderName);

    Shader* getOrLinkShader(const String& in_sVertexShader = DefaultVertexShader, const String& in_sFragmentShader = DefaultFragmentShader, const String& in_sGeometryShader = DefaultGeometryShader);

private:
    std::map<String, CompiledShader*> m_compiledVertexShaders;
    std::map<String, CompiledShader*> m_compiledFragmentShaders;
    std::map<String, CompiledShader*> m_compiledGeometryShaders;
    std::map<String, Shader*> m_linkedShaders;
    ShaderIncludeManager *m_pInclManager;

    CompiledShader* findShader(const String& in_sShaderName);

    String m_sep;
};

}; // namespace FTS

#endif // D_SHADER_H
