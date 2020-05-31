#ifndef ARK_RENDERER_BASE_MODEL_BUNDLE_H_
#define ARK_RENDERER_BASE_MODEL_BUNDLE_H_

#include "core/collection/table.h"

#include "renderer/base/model.h"

namespace ark {

class ARK_API ModelBundle {
public:
    struct ModelInfo {
        ModelInfo() = default;
        ModelInfo(const Model& model, size_t vertexOffset, size_t indexOffset);

        Model _model;
        size_t _vertex_offset;
        size_t _index_offset;
    };

    void addModel(int32_t type, const Model& model);

    const ModelInfo& ensure(int32_t type) const;
    Model load(int32_t type) const;

    const Table<int32_t, ModelInfo>& models() const;

    size_t vertexLength() const;
    size_t indexLength() const;

private:
    Table<int32_t, ModelInfo> _models;

    size_t _vertex_length;
    size_t _index_length;
};

}

#endif
