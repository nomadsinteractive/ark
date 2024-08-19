#include "bgfx/impl/buffer/buffer_base.h"

#include "renderer/base/pipeline_descriptor.h"

namespace ark::plugin::bgfx {

namespace {

::bgfx::Attrib::Enum toAttribEnum(Attribute::Usage layoutType, uint32_t customAttrIdx)
{
    switch(layoutType)
    {
        case Attribute::USAGE_POSITION:
            return ::bgfx::Attrib::Position;
        case Attribute::USAGE_TEX_COORD:
            return ::bgfx::Attrib::TexCoord0;
        case Attribute::USAGE_COLOR:
            return ::bgfx::Attrib::Color0;
        case Attribute::USAGE_NORMAL:
            return ::bgfx::Attrib::Normal;
        case Attribute::USAGE_TANGENT:
            return ::bgfx::Attrib::Tangent;
        case Attribute::USAGE_BITANGENT:
            return ::bgfx::Attrib::Bitangent;
        default:
            break;
    }
    CHECK(customAttrIdx < 10, "Too many custom attributes");
    return static_cast<::bgfx::Attrib::Enum>(customAttrIdx < 3 ? ::bgfx::Attrib::Color1 + customAttrIdx : ::bgfx::Attrib::TexCoord1 + (customAttrIdx - 3));
}

::bgfx::AttribType::Enum toAttribType(Attribute::Type type)
{
    switch(type)
    {
        case Attribute::TYPE_BYTE:
            return ::bgfx::AttribType::Uint8;
        case Attribute::TYPE_FLOAT:
            return ::bgfx::AttribType::Float;
        case Attribute::TYPE_INTEGER:
            return ::bgfx::AttribType::Float;
        case Attribute::TYPE_SHORT:
            return ::bgfx::AttribType::Int16;
        case Attribute::TYPE_UBYTE:
            return ::bgfx::AttribType::Uint8;
        case Attribute::TYPE_USHORT:
            return ::bgfx::AttribType::Int16;
        default:
            break;
    }
    return ::bgfx::AttribType::Float;
}

}

BufferBase::BufferBase(Buffer::Type type, Buffer::Usage usage)
    : _type(type), _usage(usage)
{
}

Buffer::Type BufferBase::type() const
{
    return _type;
}

Buffer::Usage BufferBase::usage() const
{
    return _usage;
}

void BufferBase::setupVertexBufferLayout(::bgfx::VertexLayout& vertexLayout, const PipelineDescriptor& pipelineDescriptor)
{
    vertexLayout.begin();
    uint32_t customAttrIdx = 0;
    for(const auto& [k, v] : pipelineDescriptor.input()->getStreamLayout(0).attributes())
    {
        const ::bgfx::Attrib::Enum attribEnum = toAttribEnum(v.usage(), v.usage() == Attribute::USAGE_CUSTOM ? customAttrIdx++ : customAttrIdx);
        vertexLayout.add(attribEnum, v.length(), toAttribType(v.type()), v.normalized());
    }
    vertexLayout.end();
}

void BufferBase::shiftTexCoords(const ::bgfx::VertexLayout& vertexLayout, void* data, size_t numberOfVertex)
{
//TODO: Waiting. We're using Uint16 as uv coords' format in the vertex layout. Unfortunately Bgfx doesn't(or revoked) support this format. So we have to manually shift those uvs.
    if(vertexLayout.m_attributes[::bgfx::Attrib::TexCoord0] != std::numeric_limits<uint16_t>::max())
    {
        const uint16_t uvOffset = vertexLayout.m_offset[::bgfx::Attrib::TexCoord0];
        uint8_t* ptr = static_cast<uint8_t*>(data);
        for(size_t i = 0; i < numberOfVertex; ++i)
        {
            uint16_t* uvPtrU = reinterpret_cast<uint16_t*>(ptr + uvOffset);
            int16_t* uvPtr = reinterpret_cast<int16_t*>(uvPtrU);
            uvPtr[0] = uvPtrU[0] / 2;
            uvPtr[1] = uvPtrU[1] / 2;
            ptr += vertexLayout.m_stride;
        }
    }
}

}
