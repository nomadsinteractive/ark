#include "bgfx/impl/buffer/buffer_base.h"

#include "renderer/base/pipeline_descriptor.h"

namespace ark::plugin::bgfx {

namespace {
    ::bgfx::Attrib::Enum toAttribEnum(Attribute::LayoutType layoutType, uint32_t customAttrIdx)
    {
        switch(layoutType)
        {
            case Attribute::LAYOUT_TYPE_POSITION:
                return ::bgfx::Attrib::Position;
            case Attribute::LAYOUT_TYPE_TEX_COORD:
                return ::bgfx::Attrib::TexCoord0;
            case Attribute::LAYOUT_TYPE_COLOR:
                return ::bgfx::Attrib::Color0;
            case Attribute::LAYOUT_TYPE_NORMAL:
                return ::bgfx::Attrib::Normal;
            case Attribute::LAYOUT_TYPE_TANGENT:
                return ::bgfx::Attrib::Tangent;
            case Attribute::LAYOUT_TYPE_BITANGENT:
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
    ASSERT(usage == Buffer::USAGE_STATIC || usage == Buffer::USAGE_DYNAMIC);
}

Buffer::Type BufferBase::type() const
{
    return _type;
}

Buffer::Usage BufferBase::usage() const
{
    return _usage;
}

void BufferBase::setupVertexBufferLayout(::bgfx::VertexLayout& vertexBufLayout, const PipelineDescriptor& pipelineDescriptor)
{
    vertexBufLayout.begin();
    uint32_t customAttrIdx = 0;
    for(const auto& [k, v] : pipelineDescriptor.input()->getStreamLayout(0).attributes())
    {
        const ::bgfx::Attrib::Enum attribEnum = toAttribEnum(v.layoutType(), v.layoutType() == Attribute::LAYOUT_TYPE_CUSTOM ? customAttrIdx++ : customAttrIdx);
        vertexBufLayout.add(attribEnum, v.length(), toAttribType(v.type()), v.normalized());
    }
    vertexBufLayout.end();
}

}
