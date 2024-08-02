#include "bgfx/impl/buffer/buffer_base.h"

namespace ark::plugin::bgfx {

uint64_t BufferBase::id()
{
    return _usage == Buffer::USAGE_STATIC ? std::get<StaticVertexBuffer>(_handle).id() : std::get<DynamicVertexBuffer>(_handle).id();
}

ResourceRecycleFunc BufferBase::recycle()
{
    return _usage == Buffer::USAGE_STATIC ? std::get<StaticVertexBuffer>(_handle).recycle() : std::get<DynamicVertexBuffer>(_handle).recycle();
}

}
