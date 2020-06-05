#ifndef ARK_RENDERER_BASE_MODEL_BUNDLE_H_
#define ARK_RENDERER_BASE_MODEL_BUNDLE_H_

#include "core/collection/table.h"
#include "core/inf/builder.h"

#include "renderer/base/model.h"

namespace ark {

class ARK_API ModelBundle {
public:
    enum MappingType {
        MAPPING_TYPE_ALBEDO,
        MAPPING_TYPE_NORMAL,
        MAPPING_TYPE_ROUGHNESS,
        MAPPING_TYPE_METALIC
    };

    struct ModelInfo {
        Model _model;
        size_t _vertex_offset;
        size_t _index_offset;
    };

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        virtual Model import(const String& src, const Rect& bounds) = 0;
    };

public:
    ModelBundle(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest, sp<Atlas> atlas, Importer& importer);

    const ModelInfo& ensure(int32_t type) const;
    Model load(int32_t type) const;

    const Table<int32_t, ModelInfo>& models() const;

    size_t vertexLength() const;
    size_t indexLength() const;

private:
    ModelInfo& addModel(int32_t type, const Model& model);

private:
    sp<Atlas> _atlas;

    Table<int32_t, ModelInfo> _models;
    size_t _vertex_length;
    size_t _index_length;
};

}

#endif
