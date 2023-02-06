#ifndef ARK_CORE_BASE_HEAP_H_
#define ARK_CORE_BASE_HEAP_H_

#include <limits>
#include <map>
#include <queue>

#include "core/collection/bitwise_trie.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"
#include "core/util/math.h"

namespace ark {

template<typename MemoryType, typename SizeType, size_t kAlignment = sizeof(void*)> class Heap {
public:
    typedef decltype(std::declval<MemoryType>().begin()) PtrType;

    static constexpr SizeType npos = std::numeric_limits<SizeType>::max();

    class Allocator {
    public:
        virtual ~Allocator() = default;

        virtual void initialize(SizeType size) = 0;
        virtual std::pair<SizeType, SizeType> allocate(SizeType size, SizeType alignment) = 0;
        virtual SizeType free(SizeType offset) = 0;
    };

    class L1 : public Allocator {
    private:
        typedef SizeType Chunk;

    public:
        L1(SizeType maxChunkLength)
            : _max_chunk_length(maxChunkLength) {
            DCHECK_WARN(Math::isPOT(maxChunkLength), "maxChunkLength(%d) should be POT", _max_chunk_length);
        }

        virtual void initialize(SizeType size) override {
            _trie.clear();
            _trie.resize(getLevel(_max_chunk_length) + 1);

            _chunks.clear();

            _length = size / kAlignment;
            _allocated = 0;

        }

        virtual std::pair<SizeType, SizeType> allocate(SizeType size, SizeType alignment) override {
            SizeType length = size / kAlignment;
            if(alignment > kAlignment || length > _max_chunk_length)
                return std::make_pair(npos, 0);

            Chunk chunk;
            uint32_t level = getLevel(length);
            uint32_t aquired = level;

            length = static_cast<SizeType>(1) << level;

            if(aquire(aquired, chunk)) {
                if(aquired != level)
                    split(chunk, aquired, level);
                return std::make_pair(chunk * kAlignment, (static_cast<SizeType>(1) << level) * kAlignment);
            }
            else {
                if(_length > (_allocated + length)) {
                    _chunks[_allocated] = true;

                    SizeType offset = _allocated * kAlignment;
                    _allocated += length;
                    return std::make_pair(offset, length * kAlignment);
                }
            }
            return std::make_pair(npos, 0);
        }

        virtual SizeType free(SizeType ptr) override {
            SizeType offset = ptr / kAlignment;
            auto iter = _chunks.find(offset);
            DCHECK(iter != _chunks.end() && iter->second, "Invalid offset(%d) being freed", offset);
            const auto next = std::next(iter);

            SizeType freed = (next == _chunks.end() ? _allocated : next->first)- iter->first;
            uint32_t level = getLevel(freed);
            iter->second = false;
            push(level, offset);
            return freed * kAlignment;
        }

    private:
        bool aquire(uint32_t& level, Chunk& chunk) {
            for(; level < _trie.size(); ++level) {
                std::queue<Chunk>& queue = _trie[level];
                if(!queue.empty()) {
                    chunk = queue.front();
                    queue.pop();

                    auto iter = _chunks.find(chunk);
                    DCHECK(iter != _chunks.end() && !iter->second, "Invalid chunk(%d)", chunk);
                    iter->second = true;
                    return true;
                }
            }
            return false;
        }

        void split(const Chunk& chunk, uint32_t chunkLevel, uint32_t toLevel) {
            for(uint32_t i = chunkLevel; i != toLevel; --i) {
                SizeType offset = chunk + (static_cast<SizeType>(1) << (i - 1));
                push(i - 1, offset);
                _chunks[offset] = false;
            }
        }

        uint32_t getLevel(SizeType length) const {
            for(uint32_t i = 0; i < sizeof(SizeType) * 8; ++i)
                if(length <= (static_cast<SizeType>(1) << i))
                    return i;
            return 0;
        }

        void push(uint32_t level, const Chunk& chunk) {
            DCHECK(level < _trie.size(), "Illegal level(%d)", level);
            _trie[level].push(chunk);
        }

    private:
        SizeType _max_chunk_length;
        SizeType _length;

        SizeType _allocated;

        std::vector<std::queue<Chunk>> _trie;
        std::map<SizeType, bool> _chunks;
    };

    class L2 : public Allocator {
    private:
        enum FragmentState {
            FRAGMENT_STATE_UNUSED,
            FRAGMENT_STATE_ALLOCATED,
            FRAGMENT_STATE_FREEZED,
            FRAGMENT_STATE_DELETED
        };

        struct Fragment {
            Fragment(FragmentState state, SizeType offset, SizeType size)
                : _state(state), _offset(offset), _size(size) {
            }
            Fragment(const Fragment& other) = default;

            Fragment split(const sp<Fragment>& self, SizeType position, SizeType size, L2& l2) {
                DCHECK(_size >= position + size && _state == FRAGMENT_STATE_UNUSED, "Heap corrupted: size: %d, position: %d, fragment-state: %d, fragment-size: %d", size, position, _state, _size);

                const auto iter = l2._fragments.find(_offset);
                DCHECK(iter != l2._fragments.end(), "Heap corrupted: Fragment with offset(%d) not found", _offset);

                SizeType remaining = (_size - size - position);
                if(remaining != 0) {
                    const auto next = std::next(iter);
                    if(next == l2._fragments.end() || next->second->_state != FRAGMENT_STATE_UNUSED)
                        l2.addFragment(FRAGMENT_STATE_UNUSED, _offset + position + size, remaining);
                    else {
                        const sp<Fragment>& nextFragment = next->second;
                        nextFragment->_state = FRAGMENT_STATE_DELETED;
                        l2.addFragment(FRAGMENT_STATE_UNUSED, _offset + position + size, remaining + nextFragment->_size);
                        l2._fragments.erase(next);
                    }
                }

                if(position == 0) {
                    _size = size;
                    _state = FRAGMENT_STATE_ALLOCATED;
                    return *this;
                } else {
                    const auto prev = iter != l2._fragments.begin() ? std::prev(iter) : l2._fragments.end();
                    if(prev == l2._fragments.end() || prev->second->_state != FRAGMENT_STATE_UNUSED) {
                        _size = position;
                    }
                    else {
                        const sp<Fragment>& prevFragment = prev->second;
                        prevFragment->_state = FRAGMENT_STATE_DELETED;
                        _size = position + prevFragment->_size;
                        _offset = prevFragment->_offset;
                        l2._fragments[_offset] = self;
                    }
                    l2.ensureAllocator(_size).push(self);
                    return *l2.addFragment(FRAGMENT_STATE_ALLOCATED, _offset + position, size);
                }
            }

            void merge(const sp<Fragment>& other) {
                DASSERT(_state == FRAGMENT_STATE_UNUSED && other->_state == FRAGMENT_STATE_UNUSED);
                DCHECK(_offset + _size == other->_offset, "Cannot merge non-adjacent fragments: size: %d, offset: %d, next-fragment-offset: %d", _size, _offset, other->_offset);
                other->_state = FRAGMENT_STATE_DELETED;
                _size += other->_size;
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
                DCHECK(fragment->_size >= _size, "Allocator won't accept fragment(%d) which is lesser than its size(%d)", fragment->_size, _size);
                _queue.push(std::move(fragment));
            }
        };

        typedef BitwiseTrie<SizeType, FragmentQueue> FragmentTrie;

    public:

        virtual void initialize(SizeType size) override {
            _fragments.clear();
            _fragment_trie.clear();
            addFragment(FRAGMENT_STATE_UNUSED, 0, size);
        }

        virtual std::pair<SizeType, SizeType> allocate(SizeType size, SizeType alignment) override {
            SizeType sizeNeeded = size + (alignment > kAlignment ? alignment : 0);
            FragmentQueue* allocator = _fragment_trie.find(sizeNeeded);
            while(allocator) {
                sp<Fragment> fragment;
                while(!allocator->empty()) {
                    fragment = allocator->pop();
                    if(fragment && fragment->_state == FRAGMENT_STATE_UNUSED) {
                        Fragment splitted = fragment->split(fragment, findOptimizedPosition(fragment, size, alignment), size, *this);
                        return std::make_pair(splitted._offset, splitted._size);
                    }
                }
                _fragment_trie.remove(allocator->_size);
                allocator = _fragment_trie.find(sizeNeeded);
            }
            DCHECK_WARN(checkFragments(sizeNeeded), "Unallocated candidate block which's in not in Allocator but found in fragments, something might go wrong");
            return std::make_pair(npos, 0);
        }

        virtual SizeType free(SizeType offset) override {
            auto iter = _fragments.find(offset);
            DCHECK(iter != _fragments.end(), "Invalid offset(%d) being freed", offset);

            sp<Fragment> fragment = iter->second;
            SizeType freed = fragment->_size;
            DASSERT(fragment->_state == FRAGMENT_STATE_ALLOCATED);
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
                ensureAllocator(size).push(fragment);
            _fragments[offset] = fragment;
            return fragment;
        }

        FragmentQueue& ensureAllocator(SizeType size) {
            FragmentQueue* allocator = _fragment_trie.ensure(size);
            DCHECK(allocator->_size == size || allocator->_size == 0, "Bad Allocator, size = %d, required-size: %d", allocator->_size, size);
            allocator->_size = size;
            return *allocator;
        }

    private:
        FragmentTrie _fragment_trie;
        std::map<SizeType, sp<Fragment>> _fragments;
    };

private:
    struct Stub {
        Stub(MemoryType memory, sp<Allocator> allocator)
            : _memory(std::move(memory)), _size(static_cast<SizeType>(_memory.end() - _memory.begin())), _allocated(0), _allocator(std::move(allocator)) {
            _allocator->initialize(_size);
        }

        SizeType allocated() const {
            return _allocated + (_next ? _next->allocated() : 0);
        }

        SizeType available() const {
            return _size - _allocated + (_next ? _next->available() : 0);
        }

        Optional<PtrType> allocate(SizeType size, SizeType alignment) {
            const auto [offset, allocated] = _allocator->allocate(align(size, kAlignment), alignment);
            if(offset != npos) {
                _allocated += allocated;
                return _memory.begin() + offset;
            }
            return _next ? _next->allocate(size, alignment) : Optional<PtrType>();
        }

        void free(PtrType ptr) {
            if(ptr >= _memory.begin() && ptr < _memory.end()) {
                SizeType offset = ptr - _memory.begin();
                SizeType freed = _allocator->free(offset);
                _allocated -= freed;
            }
            else {
                DCHECK(_next, "We go throught all heaps but find no way to free it");
                _next->free(ptr);
            }
        }

        void extend(sp<Stub> other) {
            if(_next)
                _next->extend(std::move(other));
            else
                _next = std::move(other);
        }

        MemoryType _memory;

        SizeType _size;
        SizeType _allocated;

        sp<Allocator> _allocator;

        sp<Stub> _next;
    };

public:
    Heap() = default;
    Heap(MemoryType memory, sp<Allocator> allocator)
        : _stub(sp<Stub>::make(std::move(memory), std::move(allocator))) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Heap);

    explicit operator bool() const {
        return static_cast<bool>(_stub);
    }

    SizeType allocated() const {
        DASSERT(_stub);
        return _stub->allocated();
    }

    SizeType available() const {
        DASSERT(_stub);
        return _stub->available();
    }

    Optional<PtrType> allocate(SizeType size, SizeType alignment = kAlignment) {
        DASSERT(_stub);
        DCHECK(alignment != 0 && ((alignment % kAlignment) == 0 || (kAlignment % alignment) == 0), "Illegal alignment %d", alignment);
        return _stub->allocate(size, alignment);
    }

    void free(PtrType ptr) {
        DASSERT(_stub);
        _stub->free(ptr);
    }

    void extend(const Heap& other) {
        DASSERT(_stub);
        _stub->extend(other._stub);
    }

    void extend(MemoryType memory, sp<Allocator> allocator) {
        sp<Stub> stub = sp<Stub>::make(std::move(memory), std::move(allocator));
        if(_stub)
            _stub->extend(std::move(stub));
        else
            _stub = std::move(stub);
    }

private:
    static SizeType align(SizeType value, SizeType alignment) {
        SizeType m = value % alignment;
        return m ? value + (alignment - m) : value;
    }

private:
    sp<Stub> _stub;
};

}

#endif
