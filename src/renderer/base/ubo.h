#ifndef ARK_RENDERER_BASE_UBO_H_
#define ARK_RENDERER_BASE_UBO_H_

#include <vector>

#include "graphics/base/layer.h"

#include "renderer/base/uniform.h"

namespace ark {

class UBO {
public:
    UBO(std::vector<Uniform> uniforms);

    Layer::UBOSnapshot snapshot(MemoryPool& memoryPool, const Camera& camera) const;

    const std::vector<Uniform>& uniforms() const;

    std::vector<Uniform> _uniforms;
    std::vector<std::pair<uintptr_t, size_t>> _slots;
    bytearray _dirty_flags;
    bytearray _buffer;

private:
    bool doSnapshot() const;
};

}

#endif
