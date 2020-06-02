#include "renderer/base/model_bundle.h"

#include "renderer/inf/vertices.h"

namespace ark {

void ModelBundle::addModel(int32_t type, const Model& model)
{
    _models[type] = ModelInfo(model, _vertex_length, _index_length);
    _vertex_length += model.vertexLength();
    _index_length += model.indexLength();
}

const ModelBundle::ModelInfo& ModelBundle::ensure(int32_t type) const
{
    const auto iter = _models.find(type);
    DCHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second;
}

Model ModelBundle::load(int32_t type) const
{
    return ensure(type)._model;
}

const Table<int32_t, ModelBundle::ModelInfo>& ModelBundle::models() const
{
    return _models;
}

size_t ModelBundle::vertexLength() const
{
    return _vertex_length;
}

size_t ModelBundle::indexLength() const
{
    return _index_length;
}

ModelBundle::ModelInfo::ModelInfo(const Model& model, size_t vertexOffset, size_t indexOffset)
    : _model(model), _vertex_offset(vertexOffset), _index_offset(indexOffset)
{
}

}
