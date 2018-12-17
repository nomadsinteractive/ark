#ifndef ARK_RENDERER_BASE_UNIFORM_H_
#define ARK_RENDERER_BASE_UNIFORM_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

class Uniform {
public:
    enum Type {
        UNIFORM_I1,
        UNIFORM_F1,
        UNIFORM_F2,
        UNIFORM_F3,
        UNIFORM_F4,
        UNIFORM_I1V,
        UNIFORM_F1V,
        UNIFORM_F2V,
        UNIFORM_F3V,
        UNIFORM_F4V,
        UNIFORM_MAT4,
        UNIFORM_MAT4V,
    };

    Uniform(const String& name, Type type, const sp<Flatable>& flatable, const sp<Changed>& changed, const sp<RenderController>& renderController);
    Uniform(const Uniform& other);

    const String& name() const;
    Type type() const;
    const sp<Flatable>& flatable() const;
    const sp<Changed>& notifier() const;

    String declaration() const;
    void notify() const;

private:
    void synchronize(const sp<RenderController>& renderController);

private:
    String _name;
    Type _type;
    sp<Flatable> _flatable;
    sp<Changed> _notifier;
};

}

#endif
