#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/material_texture.h"

namespace ark {

class ARK_API Material {
public:
    Material(uint32_t id, String name, bitmap baseColor = nullptr, bitmap normal = nullptr, bitmap roughness = nullptr, bitmap metallic = nullptr, bitmap specular = nullptr);

//  [[script::bindings::property]]
    uint32_t id() const;
//  [[script::bindings::property]]
    void setId(uint32_t id);
//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    const sp<MaterialTexture>& baseColor() const;
//  [[script::bindings::property]]
    const sp<MaterialTexture>& normal() const;
//  [[script::bindings::property]]
    const sp<MaterialTexture>& roughness() const;
//  [[script::bindings::property]]
    const sp<MaterialTexture>& metallic() const;
//  [[script::bindings::property]]
    const sp<MaterialTexture>& specular() const;
//  [[script::bindings::property]]
    const sp<MaterialTexture>& emission() const;

//  [[script::bindings::auto]]
    const sp<MaterialTexture>& getTexture(MaterialTexture::Type type) const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Material> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        sp<Material> build(const Scope& args) override;

    private:
        SafeBuilder<Bitmap> _base_color;
        SafeBuilder<Bitmap> _normal;
        SafeBuilder<Bitmap> _roughness;
        SafeBuilder<Bitmap> _metallic;
        SafeBuilder<Bitmap> _specular;
    };

private:
    uint32_t _id;
    String _name;
    sp<MaterialTexture> _textures[MaterialTexture::TYPE_LENGTH];
};

}

