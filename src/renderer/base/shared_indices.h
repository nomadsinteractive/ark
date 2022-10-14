#ifndef ARK_RENDERER_BASE_SHARED_INDICES_H_
#define ARK_RENDERER_BASE_SHARED_INDICES_H_

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class SharedIndices {
public:
    SharedIndices(Buffer buffer, std::vector<element_index_t> boilerPlate, size_t vertexCount, bool degenerate);

    Buffer::Snapshot snapshot(RenderController& renderController, size_t primitiveCount, size_t reservedIfInsufficient = 0);

private:

    class Concat : public Uploader {
    public:
        Concat(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices);

        virtual void upload(Writable& uploader) override;

    private:
        size_t _primitive_count;
        size_t _vertex_count;
        std::vector<element_index_t> _indices;
    };

    class Degenerate : public Uploader {
    public:
        Degenerate(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices);

        virtual void upload(Writable& uploader) override;

    private:
        size_t _primitive_count;
        size_t _vertex_count;
        std::vector<element_index_t> _indices;
    };

private:
    Buffer _buffer;

    std::vector<element_index_t> _boiler_plate;
    size_t _vertex_count;
    bool _degenerate;

    size_t _primitive_count;
};

}

#endif
