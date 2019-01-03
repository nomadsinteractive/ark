#ifndef ARK_RENDERER_BASE_UBO_H_
#define ARK_RENDERER_BASE_UBO_H_

#include <vector>

#include "graphics/base/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/uniform.h"

namespace ark {

class UBO {
public:
    UBO(std::vector<sp<Uniform>> uniforms);

    Layer::UBOSnapshot snapshot(MemoryPool& memoryPool) const;

    int32_t binding() const;
    size_t size() const;

    const std::vector<sp<Uniform>>& uniforms() const;
    const std::vector<std::pair<uintptr_t, size_t>>& slots() const;

private:
    bool doSnapshot() const;

private:
    std::vector<sp<Uniform>> _uniforms;
    int32_t _binding;

    std::vector<std::pair<uintptr_t, size_t>> _slots;

    bytearray _dirty_flags;
    bytearray _buffer;
};

}

#endif
