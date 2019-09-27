#include "renderer/impl/render_model/render_model_nine_patch.h"

#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "graphics/base/layer_context.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"

namespace ark {

RenderModelNinePatch::Item::Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : _paddings(patches)
{
    _x[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left()), textureWidth);
    _x[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + patches.left()), textureWidth);
    _x[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + patches.right()), textureWidth);
    _x[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.right()), textureWidth);

    _y[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top()), textureHeight);
    _y[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + patches.top()), textureHeight);
    _y[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + patches.bottom()), textureHeight);
    _y[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.bottom()), textureHeight);

    _paddings.setRight(bounds.width() - patches.right());
    _paddings.setBottom(bounds.height() - patches.bottom());
}

RenderModelNinePatch::RenderModelNinePatch(const RenderController& renderController, const document& manifest, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_NINE_PATCH))
{
    uint32_t textureWidth = static_cast<uint32_t>(_atlas->texture()->width());
    uint32_t textureHeight = static_cast<uint32_t>(_atlas->texture()->height());
    for(const document& node : manifest->children())
    {
        String name = node->name();
        const Rect paddings = Documents::ensureAttribute<Rect>(node, Constants::Attributes::NINE_PATCH_PADDINGS);
        if(name == "import")
        {
            for(const auto& i : _atlas->items()->indices())
                importAtlasItem(i.first, paddings, textureWidth, textureHeight);
        }
        else
        {
            int32_t type = Documents::getAttribute<int32_t>(node, Constants::Attributes::TYPE, 0);
            bool hasBounds = atlas->has(type);
            if(hasBounds)
                importAtlasItem(type, paddings, textureWidth, textureHeight);
            else
            {
                const Rect bounds = Rect::parse(node);
                _nine_patch_items.emplace(type, bounds, getPatches(paddings, bounds), textureWidth, textureHeight);
            }
        }
    }
}

sp<ShaderBindings> RenderModelNinePatch::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLE_STRIP, shader.renderController()->makeVertexBuffer(), _index_buffer->buffer());
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void RenderModelNinePatch::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController, snapshot._items.size());
}

void RenderModelNinePatch::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    DCHECK(snapshot._items.size() > 0, "LayerSnapshot has no RenderObjects");

    buf.vertices().setGrowCapacity(16 * snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void RenderModelNinePatch::load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    const Item& item = _nine_patch_items.at(snapshot._type);
    const Rect& paddings = item._paddings;
    const Rect paintRect(0, 0, std::max(paddings.left() + paddings.right(), snapshot._size.x()), std::max(paddings.top() + paddings.bottom(), snapshot._size.y()));
    float xData[4] = {paintRect.left(), paintRect.left() + paddings.left(), paintRect.right() - paddings.right(), paintRect.right()};
    float yData[4] = {paintRect.bottom(), paintRect.bottom() - paddings.top(), paintRect.top() + paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.nextVertex();
            buf.writePosition(xData[j], yData[i], 0);
            buf.writeTexCoordinate(item._x[j], item._y[i]);
        }
    }
}

Rect RenderModelNinePatch::getPatches(const Rect& paddings, const Rect& bounds) const
{
    return Rect(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
}

void RenderModelNinePatch::importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight)
{
    const Atlas::Item& item = _atlas->at(type);
    const Rect bounds(item.left() * textureWidth / 65536.0f - 0.5f, item.bottom() * textureHeight / 65536.0f - 0.5f,
                      item.right() * textureWidth / 65536.0f + 0.5f, item.top() * textureHeight / 65536.0f + 0.5f);
    _nine_patch_items.emplace(type, bounds, getPatches(paddings, bounds), textureWidth, textureHeight);
}

RenderModelNinePatch::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _atlas(factory.ensureBuilder<Atlas>(manifest)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> RenderModelNinePatch::BUILDER::build(const Scope& args)
{
    return sp<RenderModelNinePatch>::make(_resource_loader_context->renderController(), _manifest, _atlas->build(args));
}

}
