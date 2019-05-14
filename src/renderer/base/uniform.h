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
        TYPE_NONE,
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

    Uniform(const String& name, Type type, const sp<Flatable>& flatable, const sp<Notifier>& notifier, int32_t binding = -1);
    DEFAULT_COPY_AND_ASSIGN(Uniform);

    const String& name() const;
    Type type() const;

    static Type toType(const String& declaredType);

    String getDeclaredType() const;

    const sp<Flatable>& flatable() const;
    void setFlatable(const sp<Flatable>& flatable);

    void setNotifier(const sp<Notifier>& notifier);
    bool dirty() const;

    int32_t binding() const;
    void setBinding(int32_t binding);

    String declaration(const String& descriptor) const;
    void notify() const;

private:
    String _name;
    Type _type;
    sp<Flatable> _flatable;
    sp<Notifier> _notifier;
    sp<Boolean> _dirty_flag;
    int32_t _binding;
};

}

#endif
