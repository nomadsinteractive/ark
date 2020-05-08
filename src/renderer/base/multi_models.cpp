#include "renderer/base/multi_models.h"

#include "renderer/inf/vertices.h"

namespace ark {

void MultiModels::addModel(int32_t type, const Model& model)
{
    _models[type] = ModelInfo(model, _vertex_length, _index_length);
    _vertex_length += model.vertices()->length();
    _index_length += model.indices()->length();
}

Model MultiModels::load(int32_t type) const
{
    const auto iter = _models.find(type);
    DCHECK(iter != _models.end(), "Model not found, type: %d", type);
    return iter->second._model;
}

const std::unordered_map<int32_t, MultiModels::ModelInfo>& MultiModels::models() const
{
    return _models;
}

size_t MultiModels::vertexLength() const
{
    return _vertex_length;
}

size_t MultiModels::indexLength() const
{
    return _index_length;
}

MultiModels::ModelInfo::ModelInfo(const Model& model, size_t vertexOffset, size_t indexOffset)
    : _model(model), _vertex_offset(vertexOffset), _index_offset(indexOffset)
{
}

}
