#include "renderer/base/shared_indices.h"

#include "core/impl/writable/writable_memory.h"

#include "renderer/base/render_controller.h"

namespace ark {

SharedIndices::SharedIndices::SharedIndices(Buffer buffer, SharedIndices::MakerFunc maker, std::function<size_t(size_t)> sizeCalculator, std::vector<element_index_t> boilerPlate, size_t vertexCount, bool degenerate)
    : _buffer(std::move(buffer)), _maker(std::move(maker)), _size_calculator(std::move(sizeCalculator)), _boiler_plate(std::move(boilerPlate)), _vertex_count(vertexCount), _degenerate(degenerate), _primitive_count(0)
{
}

Buffer::Snapshot SharedIndices::SharedIndices::snapshot(RenderController& renderController, size_t primitiveCount, size_t reservedIfInsufficient)
{
    const size_t warningLimit = 20000;
    DWARN(primitiveCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", primitiveCount, warningLimit);
    size_t size = _size_calculator(primitiveCount);
    if(_primitive_count < primitiveCount)
    {
        _primitive_count = primitiveCount + reservedIfInsufficient;
        sp<Uploader> uploader = _maker(_primitive_count);
        DCHECK(uploader && uploader->size() >= size, "Making Uploader failed, primitive-count: %d, uploader-size: %d, required-size: %d", _primitive_count, uploader ? uploader->size() : 0, size);
        renderController.uploadBuffer(_buffer, std::move(uploader), RenderController::US_RELOAD);
    }
    return _buffer.snapshot(size);
}

Buffer::Snapshot SharedIndices::snapshot(const RenderRequest& renderRequest, size_t primitiveCount, size_t reservedIfInsufficient)
{
    const size_t warningLimit = 20000;
    DWARN(primitiveCount < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you're sure what you're doing", primitiveCount, warningLimit);
    size_t size = (_degenerate ? (_boiler_plate.size() + 2) * primitiveCount - 2 : _boiler_plate.size() * primitiveCount) * sizeof(element_index_t);
    if(_primitive_count < primitiveCount || _buffer.size() < size)
    {
        _primitive_count = primitiveCount + reservedIfInsufficient;
        ByteArray::Borrowed content = renderRequest.allocator().sbrk(size);
        WritableMemory writer(content.buf());
        if(_degenerate)
        {
            Degenerate degenerate(_primitive_count, _vertex_count, _boiler_plate);
            degenerate.upload(writer);
            return _buffer.snapshot(content);
        }
        else
        {
            Concat concat(_primitive_count, _vertex_count, _boiler_plate);
            concat.upload(writer);
            return _buffer.snapshot(content);
        }
    }
    return _buffer.snapshot(size);
}

SharedIndices::Concat::Concat(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices)
    : Uploader(primitiveCount * indices.size() * sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices))
{
}

void SharedIndices::Concat::upload(Writable& uploader)
{
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

SharedIndices::Degenerate::Degenerate(size_t primitiveCount, size_t vertexCount, std::vector<element_index_t> indices)
    : Uploader((primitiveCount * indices.size() + 2 * (primitiveCount - 1)) *  sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices))
{
}

void SharedIndices::Degenerate::upload(Writable& uploader)
{
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

}
