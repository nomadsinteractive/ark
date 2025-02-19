#pragma once

#include <bgfx/bgfx.h>

namespace ark::plugin::bgfx {

template<typename T> class Handle;

struct BgfxContext;

typedef Handle<::bgfx::VertexBufferHandle> StaticVertexBuffer;
typedef Handle<::bgfx::DynamicVertexBufferHandle> DynamicVertexBuffer;
typedef Handle<::bgfx::IndexBufferHandle> StaticIndexBuffer;
typedef Handle<::bgfx::DynamicIndexBufferHandle> DynamicIndexBuffer;
typedef Handle<::bgfx::IndirectBufferHandle> IndirectBuffer;

}
