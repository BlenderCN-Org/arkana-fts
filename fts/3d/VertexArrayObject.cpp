#include "VertexArrayObject.h"

FTS::VertexBufferObject::VertexBufferObject(const std::vector<float>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_FLOAT)
    , normalize(GL_FALSE) // Only useful for integer-like types.
    , stride(nComponents*sizeof(float))
{
    verifGL("VertexBufferObject::VertexBufferObject start");
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
    glBufferData(GL_ARRAY_BUFFER, in_buf.size()*sizeof(float), &in_buf[0], in_usage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    verifGL("VertexBufferObject::VertexBufferObject end");
}

void FTS::VertexBufferObject::bind() const
{
    verifGL("VertexBufferObject::bind start");
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
    verifGL("VertexBufferObject::bind end");
}

void FTS::VertexBufferObject::unbind()
{
    verifGL("VertexBufferObject::unbind start");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    verifGL("VertexBufferObject::unbind end");
}

FTS::VertexBufferObject::~VertexBufferObject()
{
    verifGL("VertexBufferObject::~VertexBufferObject start");
    glDeleteBuffers(1, &this->id);
    verifGL("VertexBufferObject::~VertexBufferObject end");
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<int>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_INT)
    , stride(nComponents*sizeof(int))
{
    verifGL("ElementsBufferObject::ElementsBufferObject start");
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(int), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    verifGL("ElementsBufferObject::ElementsBufferObject end");
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<short>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_SHORT)
    , stride(nComponents*sizeof(short))
{
    verifGL("ElementsBufferObject::ElementsBufferObject start");
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(short), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    verifGL("ElementsBufferObject::ElementsBufferObject end");
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<unsigned int>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_UNSIGNED_INT)
    , stride(nComponents*sizeof(unsigned int))
{
    verifGL("ElementsBufferObject::ElementsBufferObject start");
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(unsigned int), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    verifGL("ElementsBufferObject::ElementsBufferObject end");
}

FTS::ElementsBufferObject::ElementsBufferObject(const std::vector<unsigned short>& in_buf, GLint in_nComponents, GLenum in_usage)
    : id(0)
    , nComponents(in_nComponents)
    , type(GL_UNSIGNED_SHORT)
    , stride(nComponents*sizeof(unsigned short))
{
    verifGL("ElementsBufferObject::ElementsBufferObject start");
    glGenBuffers(1, &this->id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in_buf.size()*sizeof(unsigned short), &in_buf[0], in_usage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    verifGL("ElementsBufferObject::ElementsBufferObject stop");
}

void FTS::ElementsBufferObject::bind() const
{
    verifGL("ElementsBufferObject::bind start");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
    verifGL("ElementsBufferObject::bind end");
}

void FTS::ElementsBufferObject::unbind()
{
    verifGL("ElementsBufferObject::unbind start");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    verifGL("ElementsBufferObject::unbind end");
}

FTS::ElementsBufferObject::~ElementsBufferObject()
{
    verifGL("ElementsBufferObject::~ElementsBufferObject start");
    glDeleteBuffers(1, &this->id);
    verifGL("ElementsBufferObject::~ElementsBufferObject end");
}

FTS::VertexArrayObject::VertexArrayObject()
{
    verifGL("VertexArrayObject::VertexArrayObject start");
    glGenVertexArrays(1, &this->id);
    verifGL("VertexArrayObject::VertexArrayObject end");
}

FTS::VertexArrayObject::~VertexArrayObject()
{
    verifGL("VertexArrayObject::~VertexArrayObject start");
    glDeleteVertexArrays(1, &this->id);
    verifGL("VertexArrayObject::~VertexArrayObject end");
}

bool FTS::VertexArrayObject::bind()
{
    verifGL("VertexArrayObject::bind start");
    glBindVertexArray(this->id);
    verifGL("VertexArrayObject::bind end");
    return true;
}

void FTS::VertexArrayObject::unbind()
{
    verifGL("VertexArrayObject::unbind start");
    glBindVertexArray(0);
    verifGL("VertexArrayObject::unbind end");
}
