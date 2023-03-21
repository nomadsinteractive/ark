#include "renderer/impl/uploader/uploader_element_indices.h"

#include "core/inf/writable.h"

namespace ark {

UploaderElementIndices::UploaderElementIndices(std::vector<element_index_t> elementIndices, std::vector<element_index_t> vertexIndices)
    : Uploader(elementIndices.size() * vertexIndices.size() * sizeof(element_index_t)), _element_indices(std::move(elementIndices)), _vertex_indices(std::move(vertexIndices))
{
}

bool UploaderElementIndices::update(uint64_t /*timestamp*/)
{
    return false;
}

void UploaderElementIndices::upload(Writable& writable)
{
    size_t offset = 0;
    std::vector<element_index_t> indices(_element_indices.size() * _vertex_indices.size());
    for(element_index_t i : _vertex_indices)
    {
        element_index_t* buf = indices.data() + offset;
        memcpy(buf, _element_indices.data(), sizeof(element_index_t) * _element_indices.size());
        for(size_t j = 0; j < _element_indices.size(); ++j)
            buf[j] += i;
        offset += _element_indices.size();
    }
    writable.write(indices.data(), sizeof(element_index_t) * indices.size(), 0);
}

}
