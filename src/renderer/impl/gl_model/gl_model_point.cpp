#include "renderer/impl/gl_model/gl_model_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/opengl/util/gl_index_buffers.h"

namespace ark {

GLModelPoint::GLModelPoint(const sp<Atlas>& atlas)
    : RenderModel(RENDER_MODE_POINTS), _atlas(atlas)
{
}

void GLModelPoint::initialize(ShaderBindings& bindings)
{
    bindings.bindGLTexture(_atlas->texture());
}

void GLModelPoint::start(ModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());
    buf.setIndices(GLIndexBuffers::makeGLBufferSnapshot(resourceManager, GLBuffer::NAME_POINTS, layerContext._items.size()));
}

void GLModelPoint::load(ModelBuffer& buf, int32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    buf.nextVertex();
    buf.writePosition(0, 0, 0);
    buf.writeTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
}

GLModelPoint::BUILDER::BUILDER(BeanFactory& factory, const document manifest)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<RenderModel> GLModelPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelPoint>::make(_atlas->build(args));
}

}
