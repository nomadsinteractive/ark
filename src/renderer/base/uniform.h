#pragma once

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
        TYPE_IMAGE2D,
        TYPE_UIMAGE2D,
        TYPE_IIMAGE2D,
        TYPE_STRUCT,
        TYPE_COUNT
    };

    Uniform(String name, String declaredType, Type type, size_t size, uint32_t length, sp<Uploader> uploader);
    Uniform(String name, String type, uint32_t length, sp<Uploader> uploader);
    Uniform(String name, Type type, uint32_t length, sp<Uploader> uploader);
    DEFAULT_COPY_AND_ASSIGN(Uniform);

    const String& name() const;
    Type type() const;
    uint32_t length() const;

    size_t size() const;

    static Type toType(const String& declaredType);
    static uint32_t getComponentSize(Type type);

    const String& declaredType() const;

    const sp<Uploader>& uploader() const;
    void setUploader(sp<Uploader> uploader);

    String declaration(const String& descriptor) const;
    void notify() const;

private:
    String _name;
    String _declared_type;
    Type _type;
    size_t _component_size;
    uint32_t _length;
    sp<Uploader> _uploader;
};

}
