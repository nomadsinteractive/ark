#pragma once

#include <array>
#include <mutex>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/material.h"
#include "graphics/base/material_texture.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API MaterialBundle {
public:

    struct Initializer {
        Table<String, sp<Material>> _materials;
        Map<String, sp<Bitmap>> _images;
        std::mutex _mutex;

        std::pair<sp<Material>, bool> ensureMaterial(String materialName) {
            const auto synchronized = _materials.threadSynchronize(_mutex);
            if(const auto iter = _materials.find(materialName); iter != _materials.end())
                return {iter->second, false};

            sp<Material> material = sp<Material>::make(_materials.size(), materialName);
            _materials.push_back(std::move(materialName), material);
            return {material, true};
        }
    };

    MaterialBundle(const Vector<sp<Material>>& materials = {});
    MaterialBundle(Table<String, sp<Material>> materials, Map<String, sp<Bitmap>> images);

    const Vector<sp<Material>>& materials() const;
    const Map<String, sp<Bitmap>>& images() const;

    const std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH>& textures() const;

    sp<Material> getMaterial(const String& name) const;
    Rect getMaterialUV(const String& name) const;

private:
    struct Stub;
    class VariableMaterialUV;

    void initialize();

private:
    ImmutableTable<String, sp<Material>> _materials;
    Map<String, sp<Bitmap>> _images;

    std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> _textures;
    sp<Stub> _stub;
};

}
