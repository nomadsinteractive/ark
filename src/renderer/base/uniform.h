#ifndef ARK_RENDERER_BASE_UNIFORM_H_
#define ARK_RENDERER_BASE_UNIFORM_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class Uniform {
public:
    enum Type {
        TYPE_I1,
        TYPE_F1,
        TYPE_F2,
        TYPE_F3,
        TYPE_F4,
        TYPE_I1V,
        TYPE_F1V,
        TYPE_F2V,
        TYPE_F3V,
        TYPE_F4V,
        TYPE_MAT3,
        TYPE_MAT3V,
        TYPE_MAT4,
        TYPE_MAT4V,
        TYPE_SAMPLER2D
    };

    Uniform(const String& name, Type type, const sp<Flatable>& flatable, const sp<Changed>& changed);
    DEFAULT_COPY_AND_ASSIGN(Uniform);

    const String& name() const;
    Type type() const;

    static Type toType(const String& declaredType);

    const sp<Flatable>& flatable() const;
    const sp<Changed>& notifier() const;

    String declaration() const;
    void notify() const;

private:
    String _name;
    Type _type;
    sp<Flatable> _flatable;
    sp<Changed> _notifier;
};

}

#endif
