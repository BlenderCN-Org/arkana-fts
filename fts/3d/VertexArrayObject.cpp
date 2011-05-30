#include "VertexArrayObject.h"

FTS::VertexBufferObject::VertexBufferObject(const std::vector<float>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_FLOAT)
    , normalize(GL_FALSE) // Only useful for integer-like types.
    , stride(nComponents*sizeof(float))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
    glBufferData(GL_ARRAY_BUFFER, in_buf.size()*sizeof(float), &in_buf[0], in_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FTS::VertexBufferObject::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

void FTS::VertexBufferObject::unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

FTS::VertexBufferObject::~VertexBufferObject()
{
    glDeleteBuffers(1, &this->id);
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<int>& in_buf, GLint in_nComponents, GLenum in_usage, GLboolean in_normalize)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_INT)
    , normalize(in_normalize)
    , stride(nComponents*sizeof(int))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(int), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void FTS::ElementsBufferObject::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void FTS::ElementsBufferObject::unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FTS::ElementsBufferObject::~ElementsBufferObject()
{
    glDeleteBuffers(1, &this->id);
}

FTS::VertexArrayObject::VertexArrayObject()
{
    glGenVertexArrays(1, &this->id);
}

FTS::VertexArrayObject::~VertexArrayObject()
{
    glDeleteVertexArrays(1, &this->id);
}

bool FTS::VertexArrayObject::bind()
{
    glBindVertexArray(this->id);
    return true;
}

void FTS::VertexArrayObject::unbind()
{
    glBindVertexArray(0);
}
