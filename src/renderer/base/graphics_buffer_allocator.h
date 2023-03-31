#pragma once

#include <map>

#include "core/base/heap.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_input.h"

namespace ark {

class GraphicsBufferAllocator {
public:
    struct Memory {
        Memory(uint32_t size);

        uint32_t begin() const;
        uint32_t end() const;

        uint32_t _size;
    };

    typedef Heap<Memory, uint32_t, 16> HeapType;

    class Page {
    public:
        Page(GraphicsBufferAllocator& gba, Buffer buffer, uint32_t size);

        uint32_t available() const;

        void acquireStrideStrategy(uint32_t stride);
        void releaseStrideStrategy(uint32_t stride);

        GraphicsBufferAllocator& _gba;
        Buffer _buffer;
        uint32_t _size;

        HeapType _heap;
        std::map<uint32_t, std::pair<uint32_t, sp<HeapType::Strategy>>> _stride_strategies;

        friend class Strips;
    };

public:
    GraphicsBufferAllocator(RenderController& renderController);

    class Strips {
    public:
        Strips(sp<Page> page, uint32_t stride, element_index_t unitVertexCount);
        ~Strips();

        const Buffer& buffer() const;

        element_index_t allocate(uint32_t unitVertexCount);
        void free(element_index_t idx);

        void dispose();

    private:
        sp<Page> _page;
        uint32_t _heap_strategy_fix_size;
        uint32_t _size;

        std::unordered_set<element_index_t> _allocations;
    };

    std::pair<sp<Page>, uint32_t> ensurePage(uint32_t size);

    sp<Strips> makeStrips(uint32_t stride, uint32_t unitVertexCount);

private:
    const sp<GraphicsBufferAllocator::Page>& newPage();

private:
    RenderController& _render_controller;

    std::list<sp<Page>> _pages;
};

}
