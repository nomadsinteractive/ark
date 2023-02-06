#include "renderer/base/shared_indices.h"

#include "core/impl/writable/writable_memory.h"
#include "core/inf/input.h"

#include "renderer/base/render_controller.h"

namespace ark {

namespace {

class Concat : public Input {
public:
    Concat(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices)
        : Input(primitiveCount * indices.size() * sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices)) {
    }

    virtual void upload(Writable& uploader) override {
        size_t length = _indices.size();
        size_t size = length * sizeof(element_index_t);
        size_t offset = 0;
        std::vector<element_index_t> indices(_indices);
        element_index_t* buf = indices.data();

        for(size_t i = 0; i < _primitive_count; ++i, offset += size)
        {
            if(i != 0)
                for(size_t j = 0; j < length; ++j)
                    buf[j] += static_cast<element_index_t>(_vertex_count);
            uploader.write(buf, static_cast<uint32_t>(size), static_cast<uint32_t>(offset));
        }
    }


private:
    size_t _primitive_count;
    size_t _vertex_count;
    std::vector<element_index_t> _indices;
};

class Degenerate : public Input {
public:
    Degenerate(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices)
        : Input(((indices.size() + 2) * primitiveCount - 2) *  sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices)) {
    }

    virtual void upload(Writable& uploader) override {
        size_t length = _indices.size();
        size_t size = length * sizeof(element_index_t);
        uint32_t offset = 0;
        std::vector<element_index_t> indices(length + 2);
        element_index_t* buf = indices.data();
        memcpy(buf, _indices.data(), size);

        for(size_t i = 0; i < _primitive_count; ++i, offset += (size + 2 * sizeof(element_index_t)))
        {
            if(i == _primitive_count - 1)
                uploader.write(buf, static_cast<uint32_t>(size), offset);
            else
            {
                buf[length] = buf[length - 1];
                buf[length + 1] = static_cast<element_index_t>(buf[0] + _vertex_count);
                uploader.write(buf, static_cast<uint32_t>(size + 2 * sizeof(element_index_t)), offset);
                for(size_t j = 0; j < length; ++j)
                    buf[j] += static_cast<element_index_t>(_vertex_count);
            }
        }
    }

private:
    size_t _primitive_count;
    size_t _vertex_count;
    std::vector<element_index_t> _indices;
};

}

SharedIndices::SharedIndices::SharedIndices(Buffer buffer, std::vector<element_index_t> boilerPlate, size_t vertexCount, bool degenerate)
    : _buffer(std::move(buffer)), _boiler_plate(std::move(boilerPlate)), _vertex_count(vertexCount), _degenerate(degenerate), _primitive_count(0)
{
}

Buffer::Snapshot SharedIndices::snapshot(RenderController& renderController, size_t primitiveCount, size_t reservedIfInsufficient)
{
    const size_t warningLimit = 20000;
    DCHECK_WARN(primitiveCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", primitiveCount, warningLimit);
    size_t size = (_degenerate ? (_boiler_plate.size() + 2) * primitiveCount - 2 : _boiler_plate.size() * primitiveCount) * sizeof(element_index_t);
    if(_primitive_count < primitiveCount)
    {
        _primitive_count = primitiveCount + reservedIfInsufficient;
        sp<Input> input = _degenerate ? sp<Input>::make<Degenerate>(_primitive_count, _vertex_count, _boiler_plate) : sp<Input>::make<Concat>(_primitive_count, _vertex_count, _boiler_plate);
        DCHECK(input && input->size() >= size, "Making Input failed, primitive-count: %d, input-size: %d, required-size: %d", _primitive_count, input ? input->size() : 0, size);
        renderController.uploadBuffer(_buffer, std::move(input), RenderController::US_RELOAD);
    }
    return _buffer.snapshot(size);
}

}
