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

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<int>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_INT)
    , stride(nComponents*sizeof(int))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(int), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<short>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_SHORT)
    , stride(nComponents*sizeof(short))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(short), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<unsigned int>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_UNSIGNED_INT)
    , stride(nComponents*sizeof(unsigned int))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(unsigned int), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<unsigned short>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_UNSIGNED_SHORT)
    , stride(nComponents*sizeof(unsigned short))
{
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(unsigned short), &in_buf[0], in_usage);
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
