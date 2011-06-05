#include "3d/3d.h"

#include "dLib/dString/dString.h"
#include "main/Exception.h"

GLAPI void APIENTRY glActiveTexture (GLenum texture)
{
    static PFNGLACTIVETEXTUREPROC proc = (PFNGLACTIVETEXTUREPROC)FTS::glGetProcAddress("glActiveTexture");
    if(!proc) { throw FTS::NotExistException("OpenGL Multitexturing extension", "Your way too old OpenGL drivers!"); }

    return proc(texture);
}

GLAPI void APIENTRY glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t)
{
    static PFNGLMULTITEXCOORD2FPROC proc = (PFNGLMULTITEXCOORD2FPROC)FTS::glGetProcAddress("glMultiTexCoord2f");
    if(!proc) { throw FTS::NotExistException("OpenGL Multitexturing extension", "Your way too old OpenGL drivers!"); }

    return proc(target, s, t);
}

GLAPI void APIENTRY glCompileShader (GLuint shader)
{
    static PFNGLCOMPILESHADERPROC proc = (PFNGLCOMPILESHADERPROC)FTS::glGetProcAddress("glCompileShader");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader);
}

GLAPI void APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
    static PFNGLSHADERSOURCEPROC proc = (PFNGLSHADERSOURCEPROC)FTS::glGetProcAddress("glShaderSource");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, count, string, length);
}

GLAPI void APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    static PFNGLGETSHADERINFOLOGPROC proc = (PFNGLGETSHADERINFOLOGPROC)FTS::glGetProcAddress("glGetShaderInfoLog");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params)
{
    static PFNGLGETSHADERIVPROC proc = (PFNGLGETSHADERIVPROC)FTS::glGetProcAddress("glGetShaderiv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, pname, params);
}

GLAPI GLuint APIENTRY glCreateShader (GLenum type)
{
    static PFNGLCREATESHADERPROC proc = (PFNGLCREATESHADERPROC)FTS::glGetProcAddress("glCreateShader");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(type);
}

GLAPI void APIENTRY glDeleteShader (GLuint shader)
{
    static PFNGLDELETESHADERPROC proc = (PFNGLDELETESHADERPROC)FTS::glGetProcAddress("glDeleteShader");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader);
}

GLAPI void APIENTRY glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    static PFNGLGETSHADERSOURCEPROC proc = (PFNGLGETSHADERSOURCEPROC)FTS::glGetProcAddress("glGetShaderSource");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, bufSize, length, source);
}

GLAPI GLuint APIENTRY glCreateProgram()
{
    static PFNGLCREATEPROGRAMPROC proc = (PFNGLCREATEPROGRAMPROC)FTS::glGetProcAddress("glCreateProgram");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc();
}

GLAPI void APIENTRY glDeleteProgram (GLuint program)
{
    static PFNGLDELETEPROGRAMPROC proc = (PFNGLDELETEPROGRAMPROC)FTS::glGetProcAddress("glDeleteProgram");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program);
}

GLAPI void APIENTRY glAttachShader(GLuint program, GLuint shader)
{
    static PFNGLATTACHSHADERPROC proc = (PFNGLATTACHSHADERPROC)FTS::glGetProcAddress("glAttachShader");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, shader);
}

GLAPI void APIENTRY glLinkProgram(GLuint program)
{
    static PFNGLLINKPROGRAMPROC proc = (PFNGLLINKPROGRAMPROC)FTS::glGetProcAddress("glLinkProgram");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program);
}

GLAPI void APIENTRY glUseProgram(GLuint program)
{
	static PFNGLUSEPROGRAMPROC proc = (PFNGLUSEPROGRAMPROC)FTS::glGetProcAddress("glUseProgram");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

	proc(program);
}

GLAPI void APIENTRY glGetProgramInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    static PFNGLGETPROGRAMINFOLOGPROC proc = (PFNGLGETPROGRAMINFOLOGPROC)FTS::glGetProcAddress("glGetProgramInfoLog");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetProgramiv (GLuint shader, GLenum pname, GLint *params)
{
    static PFNGLGETPROGRAMIVPROC proc = (PFNGLGETPROGRAMIVPROC)FTS::glGetProcAddress("glGetProgramiv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(shader, pname, params);
}

GLAPI void APIENTRY glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    static PFNGLGETACTIVEATTRIBPROC proc = (PFNGLGETACTIVEATTRIBPROC)FTS::glGetProcAddress("glGetActiveAttrib");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, index, bufSize, length, size, type, name);
}

GLAPI void APIENTRY glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    static PFNGLGETACTIVEUNIFORMPROC proc = (PFNGLGETACTIVEUNIFORMPROC)FTS::glGetProcAddress("glGetActiveUniform");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, index, bufSize, length, size, type, name);
}

GLAPI GLint APIENTRY glGetAttribLocation (GLuint program, const GLchar *name)
{
    static PFNGLGETATTRIBLOCATIONPROC proc = (PFNGLGETATTRIBLOCATIONPROC)FTS::glGetProcAddress("glGetAttribLocation");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, name);
}

GLAPI GLint APIENTRY glGetUniformLocation (GLuint program, const GLchar *name)
{
    static PFNGLGETUNIFORMLOCATIONPROC proc = (PFNGLGETUNIFORMLOCATIONPROC)FTS::glGetProcAddress("glGetUniformLocation");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, name);
}

GLAPI const GLubyte * APIENTRY glGetStringi (GLenum name, GLuint index)
{
    static PFNGLGETSTRINGIPROC proc = (PFNGLGETSTRINGIPROC)FTS::glGetProcAddress("glGetStringi");
    if(!proc) { throw FTS::NotExistException("OpenGL 3 functions", "Your way too old OpenGL drivers!"); }

    return proc(name, index);
}

GLAPI void APIENTRY glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name)
{
    static PFNGLBINDFRAGDATALOCATIONPROC proc = (PFNGLBINDFRAGDATALOCATIONPROC)FTS::glGetProcAddress("glBindFragDataLocation");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(program, color, name);
}

GLAPI void APIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
    static PFNGLVERTEXATTRIBPOINTERPROC proc = (PFNGLVERTEXATTRIBPOINTERPROC)FTS::glGetProcAddress("glVertexAttribPointer");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(index, size, type, normalized, stride, pointer);
}

GLAPI void APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    static PFNGLBINDBUFFERPROC proc = (PFNGLBINDBUFFERPROC)FTS::glGetProcAddress("glBindBuffer");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(target, buffer);
}

GLAPI void APIENTRY glEnableVertexAttribArray(GLuint index)
{
    static PFNGLENABLEVERTEXATTRIBARRAYPROC proc = (PFNGLENABLEVERTEXATTRIBARRAYPROC)FTS::glGetProcAddress("glEnableVertexAttribArray");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(index);
}

GLAPI void APIENTRY glDisableVertexAttribArray(GLuint index)
{
    static PFNGLDISABLEVERTEXATTRIBARRAYPROC proc = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)FTS::glGetProcAddress("glDisableVertexAttribArray");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(index);
}

GLAPI void APIENTRY glUniform1i(GLint location, GLint v0)
{
    static PFNGLUNIFORM1IPROC proc = (PFNGLUNIFORM1IPROC)FTS::glGetProcAddress("glUniform1i");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, v0);
}

GLAPI void APIENTRY glUniform4fv(GLint location, GLsizei count, const GLfloat* value)
{
    static PFNGLUNIFORM4FVPROC proc = (PFNGLUNIFORM4FVPROC)FTS::glGetProcAddress("glUniform4fv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, count, value);
}

GLAPI void APIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    static PFNGLUNIFORM3FVPROC proc = (PFNGLUNIFORM3FVPROC)FTS::glGetProcAddress("glUniform3fv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, count, value);
}

GLAPI void APIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
    static PFNGLUNIFORM2FVPROC proc = (PFNGLUNIFORM2FVPROC)FTS::glGetProcAddress("glUniform2fv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, count, value);
}

GLAPI void APIENTRY glUniform1f(GLint location, GLfloat v0)
{
    static PFNGLUNIFORM1FPROC proc = (PFNGLUNIFORM1FPROC)FTS::glGetProcAddress("glUniform1f");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, v0);
}

GLAPI void APIENTRY glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    static PFNGLUNIFORMMATRIX4FVPROC proc = (PFNGLUNIFORMMATRIX4FVPROC)FTS::glGetProcAddress("glUniformMatrix4fv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, count, transpose, value);
}

GLAPI void APIENTRY glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    static PFNGLUNIFORMMATRIX3FVPROC proc = (PFNGLUNIFORMMATRIX3FVPROC)FTS::glGetProcAddress("glUniformMatrix3fv");
    if(!proc) { throw FTS::NotExistException("Modern OpenGL Shaders", "Your way too old OpenGL drivers!"); }

    return proc(location, count, transpose, value);
}
/*
GLAPI void APIENTRY glBindBuffer(GLenum target, GLuint buffer)
{
    PFNGLBINDBUFFERPROC proc = (PFNGLBINDBUFFERPROC)FTS::glGetProcAddress("glBindBuffer");
    return proc(target, buffer);
}
*/
GLAPI void APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    static PFNGLDELETEBUFFERSPROC proc = (PFNGLDELETEBUFFERSPROC)FTS::glGetProcAddress("glDeleteBuffers");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(n, buffers);
}

GLAPI void APIENTRY glGenBuffers(GLsizei n, GLuint *buffers)
{
    static PFNGLGENBUFFERSPROC proc = (PFNGLGENBUFFERSPROC)FTS::glGetProcAddress("glGenBuffers");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(n, buffers);
}

GLAPI void APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    static PFNGLBUFFERDATAPROC proc = (PFNGLBUFFERDATAPROC)FTS::glGetProcAddress("glBufferData");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(target, size, data, usage);
}

GLAPI void APIENTRY glBindVertexArray(GLuint arr)
{
    static PFNGLBINDVERTEXARRAYPROC proc = (PFNGLBINDVERTEXARRAYPROC)FTS::glGetProcAddress("glBindVertexArray");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(arr);
}

GLAPI void APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
    static PFNGLDELETEVERTEXARRAYSPROC proc = (PFNGLDELETEVERTEXARRAYSPROC)FTS::glGetProcAddress("glDeleteVertexArrays");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(n, arrays);
}

GLAPI void APIENTRY glGenVertexArrays(GLsizei n, GLuint *arrays)
{
    static PFNGLGENVERTEXARRAYSPROC proc = (PFNGLGENVERTEXARRAYSPROC)FTS::glGetProcAddress("glGenVertexArrays");
    if(!proc) { throw FTS::NotExistException("OpenGL Vertex Array Objects", "Your way too old OpenGL drivers!"); }

    return proc(n, arrays);
}
