#pragma once

#include <execution>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <unordered_set>

#include "core/collection/bitwise_trie.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"
#include "core/util/math.h"

namespace ark {

template<typename MemoryType, typename SizeType, size_t kAlignment = sizeof(void*)> class Heap {
public:
    enum FragmentState {
        FRAGMENT_STATE_UNUSED,
        FRAGMENT_STATE_ALLOCATED,
        FRAGMENT_STATE_RESERVED
    };

public:
    typedef decltype(std::declval<MemoryType>().begin()) PtrType;

    struct Fragment {
        Fragment(FragmentState state, SizeType offset, SizeType size)
            : _state(state), _offset(offset), _size(size) {
        }
        Fragment(const Fragment& other) = default;

        void merge(const Fragment& other) {
            ASSERT(_state == FRAGMENT_STATE_UNUSED && other._state == FRAGMENT_STATE_UNUSED);
            CHECK(_offset + _size == other._offset, "Cannot merge non-adjacent fragments: size: %d, offset: %d, next-fragment-offset: %d", _size, _offset, other._offset);
            _size += other._size;
        }

        FragmentState _state;
        SizeType _offset;
        SizeType _size;
    };

    struct FragmentQueue {
        SizeType _size;
        std::queue<WeakPtr<Fragment>> _queue;

        FragmentQueue()
            : _size(0) {
        }

        bool empty() const {
            return _queue.empty();
        }

        sp<Fragment> pop() {
            const sp<Fragment> fragment = _queue.front().lock();
            _queue.pop();
            return fragment;
        }

        void push(sp<Fragment> fragment) {
            CHECK(fragment->_size >= _size, "Allocator won't accept fragment(%d) which is lesser than its size(%d)", fragment->_size, _size);
            _queue.push(std::move(fragment));
        }
    };

    typedef BitwiseTrie<SizeType, FragmentQueue> FragmentTrie;

    class Strategy {
    public:
        virtual ~Strategy() = default;

        virtual Optional<Fragment> allocate(Heap& heap, SizeType size, SizeType alignment) = 0;
        virtual Optional<SizeType> free(Heap& heap, SizeType offset) = 0;
        virtual void dispose(Heap& heap) = 0;
    };

private:
    sp<Fragment> doSplit(const sp<Fragment>& fragment, SizeType position, SizeType size) {
        DCHECK(fragment->_size >= position + size && fragment->_state == FRAGMENT_STATE_UNUSED, "Heap corrupted: size: %d, position: %d, fragment-state: %d, fragment-size: %d", size, position, fragment->_state, fragment->_size);

        const auto iter = _fragments.find(fragment->_offset);
        DCHECK(iter != _fragments.end(), "Heap corrupted: Fragment with offset(%d) not found", fragment->_offset);

        SizeType remaining = (fragment->_size - size - position);
        if(remaining != 0) {
            const auto next = std::next(iter);
            if(next == _fragments.end() || next->second->_state != FRAGMENT_STATE_UNUSED)
                addFragment(FRAGMENT_STATE_UNUSED, fragment->_offset + position + size, remaining);
            else {
                const sp<Fragment>& nextFragment = next->second;
                addFragment(FRAGMENT_STATE_UNUSED, fragment->_offset + position + size, remaining + nextFragment->_size);
                _fragments.erase(next);
            }
        }

        if(position == 0) {
            fragment->_size = size;
            fragment->_state = FRAGMENT_STATE_ALLOCATED;
            return fragment;
        } else {
            const auto prev = iter != _fragments.begin() ? std::prev(iter) : _fragments.end();
            if(prev == _fragments.end() || prev->second->_state != FRAGMENT_STATE_UNUSED) {
                fragment->_size = position;
            }
            else {
                const sp<Fragment>& prevFragment = prev->second;
                fragment->_size = position + prevFragment->_size;
                fragment->_offset = prevFragment->_offset;
                _fragments.erase(prev);
                _fragments[fragment->_offset] = fragment;
            }
            ensureFragmentQueue(fragment->_size).push(fragment);
            return addFragment(FRAGMENT_STATE_ALLOCATED, fragment->_offset + position, size);
        }
    }

    sp<Fragment> doAllocate(SizeType size, SizeType alignment) {
        SizeType sizeNeeded = size + (alignment > kAlignment ? alignment : 0);
        FragmentQueue* fragmentQueue = _fragment_trie.find(sizeNeeded);
        while(fragmentQueue) {
            sp<Fragment> fragment;
            while(!fragmentQueue->empty()) {
                fragment = fragmentQueue->pop();
                if(fragment && fragment->_state == FRAGMENT_STATE_UNUSED) {
                    return doSplit(fragment, findOptimizedPosition(fragment, size, alignment), size);
                }
            }
            _fragment_trie.remove(fragmentQueue->_size);
            fragmentQueue = _fragment_trie.find(sizeNeeded);
        }
        CHECK_WARN(checkFragments(sizeNeeded), "Unallocated candidate block which's in not in Allocator but found in fragments, something might go wrong");
        return nullptr;
    }

    SizeType doFree(SizeType offset) {
        auto iter = _fragments.find(offset);
        CHECK(iter != _fragments.end(), "Invalid offset(%d) being freed", offset);

        sp<Fragment> fragment = iter->second;
        SizeType freed = fragment->_size;
        ASSERT(fragment->_state == FRAGMENT_STATE_ALLOCATED);
        fragment->_state = FRAGMENT_STATE_UNUSED;

        const auto previter = iter != _fragments.begin() ? std::prev(iter) : iter;
        if(iter != _fragments.begin() && previter->second->_state == FRAGMENT_STATE_UNUSED) {
            previter->second->merge(fragment);
            fragment = previter->second;
            iter = _fragments.erase(iter);
        }
        else
            ++iter;

        if(iter != _fragments.end() && iter->second->_state == FRAGMENT_STATE_UNUSED) {
            fragment->merge(iter->second);
            _fragments.erase(iter);
        }

        addFragment(FRAGMENT_STATE_UNUSED, fragment->_offset, fragment->_size);
        return freed;
    }

private:
    bool checkFragments(SizeType size) {
        SizeType offset = 0;
        bool r = true;
        for(const auto& i : _fragments) {
            DCHECK(offset == i.first, "Fragments are not continuous");
            r = r && !(size <= i.second->_size && i.second->_state == FRAGMENT_STATE_UNUSED);
            offset += i.second->_size;
        }
        return r;
    }

    SizeType findOptimizedPosition(const Fragment& fragment, SizeType size, SizeType alignment) const {
        if(size * 8 < fragment._size) {
            SizeType p = align(fragment._offset + size * 3, alignment) - fragment._offset;
            if(fragment._size - p >= size)
                return p;
        }
        return align(fragment._offset, alignment) - fragment._offset;
    }

    sp<Fragment> addFragment(FragmentState state, SizeType offset, SizeType size) {
        sp<Fragment> fragment = sp<Fragment>::make(state, offset, size);
        if(state == FRAGMENT_STATE_UNUSED)
            ensureFragmentQueue(size).push(fragment);
        _fragments[offset] = fragment;
        return fragment;
    }

    FragmentQueue& ensureFragmentQueue(SizeType size) {
        FragmentQueue* fragementQueue = _fragment_trie.ensure(size);
        DCHECK(fragementQueue->_size == size || fragementQueue->_size == 0, "Bad FragementQueue, size = %d, required-size: %d", fragementQueue->_size, size);
        fragementQueue->_size = size;
        return *fragementQueue;
    }

    class StrategyDefault : public Strategy {
    public:
        virtual Optional<Fragment> allocate(Heap& heap, SizeType size, SizeType alignment) {
            sp<Fragment> fragment = heap.doAllocate(align(size, alignment), alignment);
            return fragment ? Optional<Fragment>(*fragment) : Optional<Fragment>();
        }

        virtual Optional<SizeType> free(Heap& heap, SizeType offset) {
            return heap.doFree(offset);
        }

        virtual void dispose(Heap& /*heap*/) {
        }
    };

public:

    class StrategyFixSize : public Strategy {
    public:
        StrategyFixSize(SizeType sizeRequired)
            : _size_required(sizeRequired), _allocation_units(32), _allocation_units_max(_allocation_units << 4) {
        }

        virtual Optional<Fragment> allocate(Heap& heap, SizeType size, SizeType alignment) {
            if(size != _size_required)
                return Optional<Fragment>();

            SizeType offset;
            const auto iter = _recycled.begin();
            if(iter == _recycled.end()) {
                if(!_allocated.empty() && _allocation_units < _allocation_units_max)
                    _allocation_units *= 2;
                sp<Fragment> fragment = heap.doAllocate(_allocation_units * size, alignment);
                offset = fragment->_offset;
                for(SizeType i = 1; i < _allocation_units; ++i)
                    _recycled.insert(offset + i * _size_required);
                _fragments.push_back(std::move(fragment));
            } else {
                offset = *iter;
                _recycled.erase(iter);
            }
            _allocated.insert(offset);
            return Fragment(FRAGMENT_STATE_ALLOCATED, offset, _size_required);
        }

        virtual Optional<SizeType> free(Heap& /*heap*/, SizeType offset) {
            const auto iter = _allocated.find(offset);
            if(iter != _allocated.end()) {
                _allocated.erase(iter);
                _recycled.insert(offset);
                return _size_required;
            }
            return Optional<SizeType>();
        }

        virtual void dispose(Heap& heap) {
            for(const Fragment& i : _fragments)
                heap.doFree(i._offset);
            _allocated.clear();
            _recycled.clear();
        }

    private:
        SizeType _size_required;

        SizeType _allocation_units;
        SizeType _allocation_units_max;

        std::unordered_set<SizeType> _allocated;
        std::unordered_set<SizeType> _recycled;

        std::vector<sp<Fragment>> _fragments;
    };

public:
    Heap(MemoryType memory)
        : _memory(std::move(memory)), _strategies{sp<StrategyDefault>::make()}, _size(static_cast<SizeType>(_memory.end() - _memory.begin())), _allocated(0) {
        addFragment(FRAGMENT_STATE_UNUSED, 0, _size);
    }

    SizeType allocated() const {
        return _allocated + (_next ? _next->allocated() : 0);
    }

    SizeType available() const {
        return _size - _allocated + (_next ? _next->available() : 0);
    }

    [[nodiscard]]
    Optional<PtrType> allocate(SizeType size, SizeType alignment = kAlignment) {
        DCHECK(alignment != 0 && ((alignment % kAlignment) == 0 || (kAlignment % alignment) == 0), "Illegal alignment %d", alignment);
        for(Strategy& i : _strategies) {
            const Optional<Fragment> fragmentOpt = i.allocate(*this, size, alignment);
            if(fragmentOpt) {
                const Fragment& fragment = fragmentOpt.value();
                _allocated += fragment._size;
                return _memory.begin() + fragment._offset;
            }
        }
        return _next ? _next->allocate(size, alignment) : Optional<PtrType>();
    }

    SizeType free(PtrType ptr) {
        if(ptr >= _memory.begin() && ptr < _memory.end()) {
            SizeType offset = ptr - _memory.begin();
            for(Strategy& i : _strategies) {
                const Optional<SizeType> freedOpt = i.free(*this, offset);
                if(freedOpt) {
                    _allocated -= freedOpt.value();
                    return freedOpt.value();
                }
            }
        }
        DCHECK(_next, "We go throught all heaps but find no way to free it");
        return _next->free(ptr);
    }

    void addStrategy(sp<Strategy> strategy) {
        _strategies.push_front(std::move(strategy));
    }

    void removeStrategy(const sp<Strategy>& strategy) {
        strategy->dispose(*this);
        _strategies.erase(std::remove(_strategies.begin(), _strategies.end(), strategy));
    }

    void extend(sp<Heap> other) {
        if(_next)
            _next->extend(std::move(other));
        else
            _next = std::move(other);
    }

private:
    static SizeType align(SizeType value, SizeType alignment) {
        SizeType m = value % alignment;
        return m ? value + (alignment - m) : value;
    }

private:
    MemoryType _memory;
    FragmentTrie _fragment_trie;
    std::map<SizeType, sp<Fragment>> _fragments;
    std::list<sp<Strategy>> _strategies;

    SizeType _size;
    SizeType _allocated;

    sp<Heap> _next;

    friend class Strategy;
};

}
