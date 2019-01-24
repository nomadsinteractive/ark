#include "renderer/impl/render_model/render_model_nine_patch.h"

#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "graphics/base/layer_context.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/index_buffers.h"

namespace ark {

GLModelNinePatch::Item::Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
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

GLModelNinePatch::GLModelNinePatch(const document& manifest, const sp<Atlas>& atlas)
    : _atlas(atlas)
{
    uint32_t textureWidth = static_cast<uint32_t>(_atlas->texture()->width());
    uint32_t textureHeight = static_cast<uint32_t>(_atlas->texture()->height());
    for(const document& node : manifest->children("render-object"))
    {
        int32_t type = Documents::getAttribute<int32_t>(node, Constants::Attributes::TYPE, 0);
        bool hasBounds = atlas->has(type);
        if(hasBounds)
        {
            const Atlas::Item& item = atlas->at(type);
            const Rect bounds(item.left() * textureWidth / 65536.0f - 0.5f, item.bottom() * textureHeight / 65536.0f - 0.5f,
                              item.right() * textureWidth / 65536.0f + 0.5f, item.top() * textureHeight / 65536.0f + 0.5f);
            _nine_patch_items.emplace(type, bounds, getPatches(node, bounds), textureWidth, textureHeight);
        }
        else
        {
            const Rect bounds = Rect::parse(node);
            _nine_patch_items.emplace(type, bounds, getPatches(node, bounds), textureWidth, textureHeight);
        }
    }
}

sp<ShaderBindings> GLModelNinePatch::makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout)
{
    const sp<ShaderBindings> bindings = sp<ShaderBindings>::make(RENDER_MODE_TRIANGLE_STRIP, renderController, pipelineLayout);
    bindings->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelNinePatch::start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext)
{
    DCHECK(layerContext._items.size() > 0, "LayerContext has no RenderObjects");

    buf.vertices().setGrowCapacity(16 * layerContext._items.size());
    buf.setIndices(IndexBuffers::snapshot(buf.indexBuffer(), renderController.resourceManager(), Buffer::NAME_NINE_PATCH, layerContext._items.size()));
}

void GLModelNinePatch::load(ModelBuffer& buf, int32_t type, const V& size)
{
    const Rect paintRect(0, 0, size.x(), size.y());
    const Item& item = _nine_patch_items.at(type);

    const Rect& paddings = item._paddings;
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

Rect GLModelNinePatch::getPatches(const document& doc, const Rect& bounds) const
{
    const Rect pad = Documents::ensureAttribute<Rect>(doc, Constants::Attributes::NINE_PATCH_PADDINGS);
    return Rect(pad.left(), pad.top(), bounds.width() - pad.right(), bounds.height() - pad.bottom());
}

GLModelNinePatch::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _atlas(factory.ensureBuilder<Atlas>(manifest))
{
}

sp<RenderModel> GLModelNinePatch::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelNinePatch>::make(_manifest, _atlas->build(args));
}

}
