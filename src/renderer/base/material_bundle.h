#ifndef ARK_RENDERER_BASE_MATERIAL_BUNDLE_H_
#define ARK_RENDERER_BASE_MATERIAL_BUNDLE_H_

#include <array>
#include <map>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
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
    MaterialBundle(const sp<ResourceLoaderContext>& resourceLoaderContext, std::map<String, sp<Material>> materials, std::array<sp<Texture>, MaterialTexture::TYPE_LENGTH> textures);

    sp<Material> getMaterial(const String& name) const;
    Rect getMaterialUV(const String& name) const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<MaterialBundle> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<MaterialBundle> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        std::vector<String> _names;
        std::vector<sp<Builder<Material>>> _materials;

        std::vector<MaterialTexture::Type> _types;
        std::vector<sp<Builder<Texture>>> _textures;
    };

private:
    std::map<String, sp<Material>> _materials;
    std::map<String, RectI> _material_bounds;
    int32_t _width;
    int32_t _height;

    sp<TexturePacker> _texture_packers[MaterialTexture::TYPE_LENGTH];

    MaxRectsBinPack _max_rects_bin_pack;
};

}

#endif
