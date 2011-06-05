#pragma once

// Please always include 3d.h, it is safer.

#include "main/defines.h"

#define NO_SDL_GLEXT
#define __glext_h_ // We define extensions ourselves below.
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

// X11 is soo sucky that they #define None 0. The word None is commonly used
// so we undefine it here in order to avoid a disaster.
#if !WINDOOF
#  ifdef None
#    undef None
#  endif
#endif

// OpenGL 3.2 is not well supported yet, it is a hassle to make it work and
// only SDL trunk, GLFW is not good enough yet (no deadkeys for example), so
// we have to wait until version 3.0, that will be a while.

// In the meantime, we just get our function pointers ourselves and we do not
// create a forward context yet.

// Also, we define the extensions we want to use ourselves, because some
// platforms (Read: windows) do not include a glext.h by default...

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

// OpenGL 1.3
#define GL_TEXTURE0 0x84C0
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
GLAPI void APIENTRY glActiveTexture (GLenum texture);
// Deprecated in OpenGL 3.2
typedef void (APIENTRYP PFNGLMULTITEXCOORD2FPROC) (GLenum target, GLfloat s, GLfloat t);
GLAPI void APIENTRY glMultiTexCoord2f (GLenum target, GLfloat s, GLfloat t);

// OpenGL 1.5
#include <stddef.h>
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4

typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
GLAPI void APIENTRY glBindBuffer(GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
GLAPI void APIENTRY glDeleteBuffers(GLsizei n, const GLuint *buffers);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
GLAPI void APIENTRY glGenBuffers(GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
GLAPI void APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

// OpenGL 2.0
typedef char GLchar;

#define GL_MAX_VERTEX_ATTRIBS               0x8869
#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_VERTEX_SHADER                    0x8B31
#define GL_COMPILE_STATUS                   0x8B81
#define GL_LINK_STATUS                      0x8B82
#define GL_INFO_LOG_LENGTH                  0x8B84
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS  0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS    0x8B4A
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_FLOAT_VEC2                       0x8B50
#define GL_FLOAT_VEC3                       0x8B51
#define GL_FLOAT_VEC4                       0x8B52
#define GL_FLOAT_MAT3                       0x8B5B
#define GL_FLOAT_MAT4                       0x8B5C
#define GL_SAMPLER_2D                       0x8B5E
#define GL_ACTIVE_UNIFORMS                  0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH        0x8B87
#define GL_SHADER_SOURCE_LENGTH             0x8B88
#define GL_ACTIVE_ATTRIBUTES                0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH      0x8B8A

typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
GLAPI void APIENTRY glCompileShader (GLuint shader);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
GLAPI void APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
GLAPI void APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
GLAPI GLuint APIENTRY glCreateShader (GLenum type);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
GLAPI void APIENTRY glDeleteShader (GLuint shader);
typedef void (APIENTRYP PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GLAPI void APIENTRY glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
GLAPI GLuint APIENTRY glCreateProgram();
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
GLAPI void APIENTRY glDeleteProgram (GLuint program);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
GLAPI void APIENTRY glAttachShader(GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
GLAPI void APIENTRY glLinkProgram(GLuint program);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
GLAPI void APIENTRY glUseProgram(GLuint program);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GLAPI void APIENTRY glGetProgramInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
GLAPI void APIENTRY glGetProgramiv (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GLAPI void APIENTRY glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GLAPI void APIENTRY glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
GLAPI GLint APIENTRY glGetAttribLocation (GLuint program, const GLchar *name);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
GLAPI GLint APIENTRY glGetUniformLocation (GLuint program, const GLchar *name);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
GLAPI void APIENTRY glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
GLAPI void APIENTRY glEnableVertexAttribArray(GLuint index);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
GLAPI void APIENTRY glDisableVertexAttribArray(GLuint index);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
GLAPI void APIENTRY glUniform1i (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY glUniform4fv(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY glUniform3fv(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
GLAPI void APIENTRY glUniform2fv(GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
GLAPI void APIENTRY glUniform1f(GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GLAPI void APIENTRY glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

// OpenGL 3.0
#define GL_NUM_EXTENSIONS                 0x821D
typedef const GLubyte * (APIENTRYP PFNGLGETSTRINGIPROC) (GLenum name, GLuint index);
GLAPI const GLubyte * APIENTRY glGetStringi (GLenum name, GLuint index);
typedef void (APIENTRYP PFNGLBINDFRAGDATALOCATIONPROC) (GLuint program, GLuint color, const GLchar *name);
GLAPI void APIENTRY glBindFragDataLocation(GLuint program, GLuint color, const GLchar* name);

// OpenGL 3.1
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31

// OpenGL 3.2
#define GL_GEOMETRY_SHADER                0x8DD9

// GL_ARB_vertex_array_object
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
GLAPI void APIENTRY glBindVertexArray(GLuint arr);
typedef void (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
GLAPI void APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
GLAPI void APIENTRY glGenVertexArrays(GLsizei n, GLuint *arrays);
