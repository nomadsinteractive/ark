#include "renderer/base/gl_shader_input.h"

#include "core/base/string.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

void GLShaderInput::addAttribute(String name, GLAttribute attribute)
{
    if(_streams.find(attribute.divisor()) == _streams.end())
        _streams[attribute.divisor()] = Stream(attribute.divisor());

    _streams[attribute.divisor()].addAttribute(std::move(name), std::move(attribute));
}

std::vector<std::pair<uint32_t, GLBuffer>> GLShaderInput::makeInstancedArrays(GLResourceManager& resourceManager) const
{
    std::vector<std::pair<uint32_t, GLBuffer>> instancedArrays;
    for(auto iter : _streams)
    {
        uint32_t divisor = iter.first;
        if(divisor != 0)
            instancedArrays.push_back(std::make_pair(divisor, resourceManager.makeDynamicArrayBuffer()));
    }
    return instancedArrays;
}

void GLShaderInput::bind(GraphicsContext&, GLProgram& program, uint32_t divisor) const
{
    const auto iter = _streams.find(divisor);
    DCHECK(iter != _streams.end(), "Shader has no %d divided attribute stream", divisor);
    const Stream& stream = iter->second;
    for(auto iter : stream.attributes())
    {
        const GLProgram::Attribute& glAttribute = program.getAttribute(iter.second.name());
        iter.second.setVertexPointer(glAttribute.location(), stream.stride());
    }
}

const GLShaderInput::Stream& GLShaderInput::getStream(uint32_t divisor) const
{
    const auto iter = _streams.find(divisor);
    DCHECK(iter != _streams.end(), "GLShaderInput has no stream(%d)", divisor);
    return iter->second;
}

const GLAttribute&GLShaderInput::getAttribute(const String& name, uint32_t divisor) const
{
    return getStream(divisor).getAttribute(name);
}

int32_t GLShaderInput::getAttributeOffset(const String& name, uint32_t divisor) const
{
    return getStream(divisor).getAttributeOffset(name);
}

GLShaderInput::Stream::Stream()
    : _divisor(0), _stride(0)
{
}

GLShaderInput::Stream::Stream(uint32_t divisor)
    : _divisor(divisor), _stride(0)
{
}

uint32_t GLShaderInput::Stream::stride() const
{
    return _stride;
}

const std::unordered_map<String, GLAttribute>& GLShaderInput::Stream::attributes() const
{
    return _attributes;
}

void GLShaderInput::Stream::addAttribute(String name, GLAttribute attribute)
{
    DCHECK(_attributes.find(name) == _attributes.end(), "Attribute \"%s\" has been added already", name.c_str());
    attribute.setOffset(_stride);
    _stride += attribute.size();
    _attributes.insert(std::make_pair(std::move(name), std::move(attribute)));
}

const GLAttribute& GLShaderInput::Stream::getAttribute(const String& name) const
{
    const auto iter = _attributes.find(name);
    DCHECK(iter != _attributes.end(), "Stream(%d) has no attribute \"%s\"", name.c_str());
    return iter->second;
}

int32_t GLShaderInput::Stream::getAttributeOffset(const String& name) const
{
    const auto iter = _attributes.find(name);
    return iter != _attributes.end() ? iter->second.offset() : -1;
}

void GLShaderInput::Stream::align()
{
    uint32_t mod = _stride % sizeof(GLfloat);
    if(mod != 0)
        _stride += (sizeof(GLfloat) - mod);
}

}