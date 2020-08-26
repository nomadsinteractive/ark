#ifndef ARK_GRAPHICS_BASE_MATERIAL_BUNDLE_H_
#define ARK_GRAPHICS_BASE_MATERIAL_BUNDLE_H_

#include <map>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API MaterialBundle {
public:
    MaterialBundle(std::map<String, sp<Material>> materials);

    sp<Material> getMaterial(const String& name) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<MaterialBundle> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<MaterialBundle> build(const Scope& args) override;

    private:
        std::vector<String> _names;
        std::vector<sp<Builder<Material>>> _materials;
    };

private:
    std::map<String, sp<Material>> _materials;
};

}

#endif
