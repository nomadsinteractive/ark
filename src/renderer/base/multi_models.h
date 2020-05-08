#ifndef ARK_RENDERER_BASE_MULTI_MODELS_H_
#define ARK_RENDERER_BASE_MULTI_MODELS_H_

#include <unordered_map>

#include "renderer/base/model.h"

namespace ark {

class ARK_API MultiModels {
public:
    struct ModelInfo {
        ModelInfo() = default;
        ModelInfo(const Model& model, size_t vertexOffset, size_t indexOffset);

        Model _model;
        size_t _vertex_offset;
        size_t _index_offset;
    };

    void addModel(int32_t type, const Model& model);

    Model load(int32_t type) const;

    const std::unordered_map<int32_t, ModelInfo>& models() const;

    size_t vertexLength() const;
    size_t indexLength() const;

private:
    std::unordered_map<int32_t, ModelInfo> _models;

    size_t _vertex_length;
    size_t _index_length;
};

}

#endif
