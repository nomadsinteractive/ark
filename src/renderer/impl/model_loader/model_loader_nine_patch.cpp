#include "renderer/impl/model_loader/model_loader_nine_patch.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/texture.h"

namespace ark {

ModelLoaderNinePatch::Item::Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
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

ModelLoaderNinePatch::ModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas)
    : ModelLoader(RenderModel::RENDER_MODE_TRIANGLE_STRIP, makeUnitModel()), _atlas(atlas)
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

Model ModelLoaderNinePatch::load(int32_t type)
{
    return Model();
}

sp<Model> ModelLoaderNinePatch::makeUnitModel()
{
    return sp<Model>::make(nullptr, makeVertices(Rect(0, 0, 1.0f, 1.0f)), nullptr, nullptr, nullptr, V3(1.0f));
}

sp<Array<V3>> ModelLoaderNinePatch::makeVertices(const Rect& bounds)
{
    return sp<Array<V3>::Fixed<4>>::make(std::initializer_list<V3>({{bounds.left(), bounds.top(), 0}, {bounds.left(), bounds.bottom(), 0},
                                                                    {bounds.right(), bounds.top(), 0}, {bounds.right(), bounds.bottom(), 0}}));
}

void ModelLoaderNinePatch::importAtlasItem(int32_t type, const Rect& paddings, uint32_t textureWidth, uint32_t textureHeight)
{
    const Atlas::Item& item = _atlas->at(type);
    const Rect bounds(item.ux() * textureWidth / 65536.0f, item.vy() * textureHeight / 65536.0f,
                      item.vx() * textureWidth / 65536.0f, item.uy() * textureHeight / 65536.0f);
    _nine_patch_items.emplace(type, bounds, getPatches(paddings, bounds), textureWidth, textureHeight);
}

Rect ModelLoaderNinePatch::getPatches(const Rect& paddings, const Rect& bounds) const
{
    return Rect(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
}

ModelLoaderNinePatch::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderNinePatch::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderNinePatch>::make(_manifest, _atlas->build(args));
}

}
