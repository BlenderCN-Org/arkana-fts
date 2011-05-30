#ifndef D_VERTEX_ARRAY_OBJECT_H
#define D_VERTEX_ARRAY_OBJECT_H

#include "3d/3d.h"

#include <vector>

namespace FTS {

struct VertexBufferObject {
    GLuint id;
    GLint nComponents;
    GLenum type;
    GLboolean normalize;
    GLsizei stride;

    VertexBufferObject(const std::vector<float>& in_buf, GLint in_nComponents, GLenum in_usage = GL_STATIC_DRAW);
    virtual ~VertexBufferObject();

    void bind() const;
    static void unbind();
};

struct ElementsBufferObject {
    GLuint id;
    GLint nComponents;
    GLenum type;
    GLboolean normalize;
    GLsizei stride;

    ElementsBufferObject(const std::vector<int>& in_buf, GLint in_nComponents, GLenum in_usage = GL_STATIC_DRAW, GLboolean in_normalize = GL_FALSE);
    virtual ~ElementsBufferObject();

    void bind() const;
    static void unbind();
};

struct VertexArrayObject {
    GLuint id;

    VertexArrayObject();
    virtual ~VertexArrayObject();

    // Those two are non-const to show that as soon as you bind a vao, it starts recording.
    bool bind();
    static void unbind();
};

}; // namespace FTS

#endif // D_VERTEX_ARRAY_OBJECT_H
