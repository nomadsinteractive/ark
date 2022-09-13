#ifndef ARK_RENDERER_BASE_UNIFORM_H_
#define ARK_RENDERER_BASE_UNIFORM_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Uniform {
public:
    enum Type {
        TYPE_NONE,
        TYPE_I1,
        TYPE_I2,
        TYPE_I3,
        TYPE_I4,
        TYPE_F1,
        TYPE_F2,
        TYPE_F3,
        TYPE_F4,
        TYPE_I1V,
        TYPE_I2V,
        TYPE_I3V,
        TYPE_I4V,
        TYPE_F1V,
        TYPE_F2V,
        TYPE_F3V,
        TYPE_F4V,
        TYPE_MAT3,
        TYPE_MAT3V,
        TYPE_MAT4,
        TYPE_MAT4V,
        TYPE_SAMPLER2D,
        TYPE_STRUCT,
        TYPE_COUNT
    };

    Uniform(const String& name, const String& declaredType, Type type, size_t size, uint32_t length, const sp<Input>& flatable, int32_t binding = -1);
    Uniform(const String& name, const String& type, uint32_t length, const sp<Input>& flatable, int32_t binding = -1);
    Uniform(const String& name, Type type, uint32_t length, const sp<Input>& flatable, int32_t binding = -1);
    DEFAULT_COPY_AND_ASSIGN(Uniform);

    const String& name() const;
    Type type() const;
    uint32_t length() const;

    size_t size() const;

    static Type toType(const String& declaredType);
    static String toDeclaredType(Type type);
    static uint32_t getComponentSize(Type type);

    const String& declaredType() const;

    const sp<Input>& input() const;
    void setInput(const sp<Input>& flatable);

    int32_t binding() const;
    void setBinding(int32_t binding);

    String declaration(const String& descriptor) const;
    void notify() const;

private:
    String _name;
    String _declared_type;
    Type _type;
    size_t _component_size;
    uint32_t _length;
    sp<Input> _input;
    int32_t _binding;
};

}

#endif
