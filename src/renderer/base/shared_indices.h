#ifndef ARK_RENDERER_BASE_SHARED_INDICES_H_
#define ARK_RENDERER_BASE_SHARED_INDICES_H_

#include <functional>

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class SharedIndices {
public:
    typedef std::function<sp<Uploader>(size_t)> MakerFunc;

public:
    SharedIndices(Buffer buffer, MakerFunc maker, std::function<size_t(size_t)> sizeCalculator, std::vector<element_index_t> boilerPlate, size_t vertexCount, bool degenerate);

[[deprecated]]
    Buffer::Snapshot snapshot(RenderController& renderController, size_t objectCount, size_t reservedIfInsufficient = 0);
    Buffer::Snapshot snapshot(const RenderRequest& renderRequest, size_t objectCount, size_t reservedIfInsufficient = 0);

public:

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
    MakerFunc _maker;
    std::function<size_t(size_t)> _size_calculator;

    std::vector<element_index_t> _boiler_plate;
    size_t _vertex_count;
    bool _degenerate;

    size_t _primitive_count;
};

}

#endif
