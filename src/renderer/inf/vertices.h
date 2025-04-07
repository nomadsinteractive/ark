#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Vertices {
public:
    Vertices(size_t length);
    virtual ~Vertices() = default;

    virtual void write(VertexWriter& buf, const V3& size) = 0;

//  [[script::bindings::property]]
    size_t length() const;

//  [[script::bindings::classmethod]]
    static sp<Uploader> makeUploader(sp<Vertices> self, const Shader& shader, const V3& bounds);

private:
    size_t _length;
};

}
