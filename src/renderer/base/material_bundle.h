#pragma once

#include <array>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/material_texture.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API MaterialBundle {
public:
//  [[script::bindings::auto]]
    MaterialBundle(const Vector<sp<Material>>& materials = {});

//  [[script::bindings::property]]
    const Vector<sp<Material>>& materials() const;
//  [[script::bindings::property]]
    const Map<String, sp<Bitmap>>& images() const;
    Map<String, sp<Bitmap>>& images();

//  [[script::bindings::property]]
    const std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>& textures() const;

//  [[script::bindings::auto]]
    sp<Material> getMaterial(const String& name) const;
//  [[script::bindings::auto]]
    void addMaterial(String name, sp<Material> material);

//  [[script::bindings::auto]]
    void update();

    Rect getMaterialUV(const String& name) const;

private:
    struct Stub;
    class VariableMaterialUV;

private:
    Table<String, sp<Material>> _materials;
    Map<String, sp<Bitmap>> _images;

    std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> _textures;
    sp<Stub> _stub;
};

}
