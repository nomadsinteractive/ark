#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/material_map.h"

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
    const sp<MaterialMap>& baseColor() const;
//  [[script::bindings::property]]
    void setBaseColor(sp<MaterialMap> map);
//  [[script::bindings::property]]
    const sp<MaterialMap>& normal() const;
//  [[script::bindings::property]]
    void setNormal(sp<MaterialMap> map);
//  [[script::bindings::property]]
    const sp<MaterialMap>& roughness() const;
//  [[script::bindings::property]]
    void setRoughness(sp<MaterialMap> map);
//  [[script::bindings::property]]
    const sp<MaterialMap>& metallic() const;
//  [[script::bindings::property]]
    void setMetallic(sp<MaterialMap> map);
//  [[script::bindings::property]]
    const sp<MaterialMap>& specular() const;
//  [[script::bindings::property]]
    void setSpecular(sp<MaterialMap> map);
//  [[script::bindings::property]]
    const sp<MaterialMap>& emission() const;
//  [[script::bindings::property]]
    void setEmission(sp<MaterialMap> map);

    const sp<Variable<Rect>>& uv() const;
    void setUV(sp<Variable<Rect>> uv);

    Rect toTextureUV() const;

//  [[script::bindings::auto]]
    const sp<MaterialMap>& getTexture(MaterialMap::Type type) const;

private:
    uint32_t _id;
    String _name;
    sp<Variable<Rect>> _uv;
    sp<MaterialMap> _textures[MaterialMap::TYPE_LENGTH];
};

}

