#pragma once

#include <array>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/material_texture.h"
#include "graphics/base/rect.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API MaterialBundle {
public:
//  [[script::bindings::auto]]
    MaterialBundle(const Vector<sp<Material>>& materials = {});
    MaterialBundle(Table<String, sp<Material>> materials, std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures);

//  [[script::bindings::property]]
    const Vector<sp<Material>>& materials() const;

//  [[script::bindings::auto]]
    sp<Material> getMaterial(const String& name) const;
//  [[script::bindings::auto]]
    void addMaterial(String name, sp<Material> material);

    sp<Material> addMaterial(String name);

    Rect getMaterialUV(const String& name) const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<MaterialBundle> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        sp<MaterialBundle> build(const Scope& args) override;

    private:
        Vector<String> _names;
        Vector<sp<Builder<Material>>> _materials;

        Vector<MaterialTexture::Type> _types;
        Vector<sp<Builder<Texture>>> _textures;
    };

private:
    Table<String, sp<Material>> _materials;
    Map<String, RectI> _material_bounds;
    int32_t _width;
    int32_t _height;

    sp<TexturePacker> _texture_packers[MaterialTexture::TYPE_LENGTH];

    MaxRectsBinPack _max_rects_bin_pack;
};

}
