#ifndef ARK_GRAPHICS_BASE_MATERIAL_H_
#define ARK_GRAPHICS_BASE_MATERIAL_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"


namespace ark {

class Material {
public:
    Material(bitmap baseColor = nullptr, bitmap normal = nullptr, bitmap roughness = nullptr, bitmap metallic = nullptr, bitmap specular = nullptr);

    const sp<VariableWrapper<bitmap>>& baseColor() const;
    void setBaseColor(bitmap baseColor) const;

    const sp<VariableWrapper<bitmap>>& normal() const;
    void setNormal(bitmap normal) const;

    const sp<VariableWrapper<bitmap>>& roughness() const;
    void setRoughness(bitmap roughness) const;

    const sp<VariableWrapper<bitmap>>& metallic() const;
    void setMetallic(bitmap metallic) const;

    const sp<VariableWrapper<bitmap>>& specular() const;
    void setSpecular(bitmap specular) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Material> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<Material> build(const Scope& args) override;

    private:
        sp<Builder<Bitmap>> makeBitmapBuilder(BeanFactory& beanFactory, const document& manifest);

    private:
        SafePtr<Builder<Bitmap>> _base_color;
        SafePtr<Builder<Bitmap>> _normal;
        SafePtr<Builder<Bitmap>> _roughness;
        SafePtr<Builder<Bitmap>> _metallic;
        SafePtr<Builder<Bitmap>> _specular;
    };

private:
    sp<VariableWrapper<bitmap>> _base_color;
    sp<VariableWrapper<bitmap>> _normal;
    sp<VariableWrapper<bitmap>> _roughness;
    sp<VariableWrapper<bitmap>> _metallic;
    sp<VariableWrapper<bitmap>> _specular;
};

}

#endif
