#include "renderer/base/graphics_buffer_allocator.h"

#include "core/inf/uploader.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class PageUploader final : public Uploader {
public:
    PageUploader(size_t size)
        : Uploader(size) {
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    void upload(Writable& buf) override {
    }
};

}

GraphicsBufferAllocator::GraphicsBufferAllocator(RenderController& renderController)
    : _render_controller(renderController)
{
}

std::pair<sp<GraphicsBufferAllocator::Page>, uint32_t> GraphicsBufferAllocator::ensurePage(uint32_t size)
{
    for(const sp<Page>& page : _pages)
        if(const Optional<uint32_t> optPtr = page->_heap.allocate(size))
            return {page, optPtr.value()};

    const sp<Page>& page = newPage();
    const Optional<uint32_t> optPtr = page->_heap.allocate(size);
    CHECK(optPtr, "Allocate failed, size: %d", size);
    return {page, optPtr.value()};
}

GraphicsBufferAllocator::Page::Page(GraphicsBufferAllocator& gba, Buffer buffer, uint32_t size)
    : _gba(gba), _buffer(std::move(buffer)), _size(size), _heap(Memory(_size))
{
}

GraphicsBufferAllocator::Memory::Memory(uint32_t units)
    : _size(units)
{
}

uint32_t GraphicsBufferAllocator::Memory::begin() const
{
    return 0;
}

uint32_t GraphicsBufferAllocator::Memory::end() const
{
    return _size;
}

uint32_t GraphicsBufferAllocator::Page::available() const
{
    return _heap.available();
}

void GraphicsBufferAllocator::Page::acquireStrideStrategy(uint32_t stride)
{
    if(const auto iter = _stride_strategies.find(stride); iter == _stride_strategies.end())
    {
        sp<HeapType::Strategy> strategy = sp<HeapType::StrategyFixSize>::make(stride);
        _stride_strategies.insert(std::make_pair(stride, std::make_pair(1, strategy)));
        _heap.addStrategy(std::move(strategy));
    }
    else
        (iter->second.first) ++;
}

void GraphicsBufferAllocator::Page::releaseStrideStrategy(uint32_t stride)
{
    const auto iter = _stride_strategies.find(stride);
    DASSERT(iter != _stride_strategies.end());
    if(-- (iter->second.first) == 0)
    {
        _heap.removeStrategy(iter->second.second);
        _stride_strategies.erase(iter);
    }
}

GraphicsBufferAllocator::Strips::Strips(sp<Page> page, uint32_t stride)
    : _page(std::move(page)), _heap_strategy_fix_size(stride), _size(0)
{
    _page->acquireStrideStrategy(_heap_strategy_fix_size);
}

GraphicsBufferAllocator::Strips::~Strips()
{
    _page->releaseStrideStrategy(_heap_strategy_fix_size);
}

const Buffer& GraphicsBufferAllocator::Strips::buffer() const
{
    return _page->_buffer;
}

element_index_t GraphicsBufferAllocator::Strips::allocate(uint32_t unitVertexCount)
{
    const uint32_t sizeNeedAllocate = unitVertexCount * _heap_strategy_fix_size;
    if(Optional<uint32_t> ptr = _page->_heap.allocate(sizeNeedAllocate, _heap_strategy_fix_size))
    {
        const element_index_t idx = ptr.value() / _heap_strategy_fix_size;
        _allocations.insert(idx);
        _size += sizeNeedAllocate;
        return idx;
    }
    FATAL("Unimplemented: Available %d", _page->available());
    return 0;
}

void GraphicsBufferAllocator::Strips::free(element_index_t idx)
{
    const auto iter = _allocations.find(idx);
    DCHECK(iter != _allocations.end(), "Unallocated index %d is being freed", idx);
    _allocations.erase(iter);
    _size -= _page->_heap.free(idx * _heap_strategy_fix_size);
}

void GraphicsBufferAllocator::Strips::dispose()
{
    for(const element_index_t i : _allocations)
        _page->_heap.free(i * _heap_strategy_fix_size);
    _allocations.clear();
    _size = 0;
}

const sp<GraphicsBufferAllocator::Page>& GraphicsBufferAllocator::newPage()
{
    constexpr uint32_t pageSize = 65536 * 32;
    _pages.push_front(sp<Page>::make(*this, _render_controller.makeVertexBuffer(Buffer::USAGE_BIT_DYNAMIC, sp<PageUploader>::make(pageSize)), pageSize));
    return _pages.front();
}

sp<GraphicsBufferAllocator::Strips> GraphicsBufferAllocator::makeStrips(uint32_t stride)
{
    return sp<Strips>::make(_pages.empty() ? newPage() :_pages.front(), stride);
}

}
