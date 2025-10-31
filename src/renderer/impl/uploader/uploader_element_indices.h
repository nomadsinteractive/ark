#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/inf/uploader.h"

namespace ark {

class UploaderElementIndices : public Uploader {
public:
    UploaderElementIndices(std::vector<element_index_t> elementIndices, std::vector<element_index_t> vertexIndices);

    virtual bool update(uint32_t tick) override;
    virtual void upload(Writable& writable) override;

private:
    std::vector<element_index_t> _element_indices;
    std::vector<element_index_t> _vertex_indices;

};

}
