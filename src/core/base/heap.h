#ifndef ARK_CORE_BASE_HEAP_H_
#define ARK_CORE_BASE_HEAP_H_

#include <limits>
#include <map>
#include <queue>

#include "core/base/object_pool.h"
#include "core/collection/bitwise_trie.h"
#include "core/types/shared_ptr.h"


namespace ark {

template<typename SizeType, SizeType kAlignment = sizeof(size_t)> class Heap {
private:
    enum {
        npos = std::numeric_limits<SizeType>::max()
    };

    enum FragmentState {
        FRAGMENT_STATE_UNUSED,
        FRAGMENT_STATE_ALLOCATED,
        FRAGMENT_STATE_FREEZED,
        FRAGMENT_STATE_DELETED
    };

    struct Stub;

    struct L1 {

    };

    struct L2 {
        L2(const Stub& stub, SizeType offset, SizeType size)
            : _object_pool(stub._object_pool), _offset(offset), _size(size) {
            addFragment(FRAGMENT_STATE_UNUSED, 0, size);
        }

        struct Fragment {
            Fragment(FragmentState state, SizeType offset, SizeType size)
                : _state(state), _offset(offset), _size(size) {
            }
            Fragment(const Fragment& other) = default;

            Fragment split(SizeType position, SizeType size, struct L2& l2) {
                DCHECK(_size >= position + size && _state == FRAGMENT_STATE_UNUSED, "Heap corrupted: size: %d, position: %d, fragment-state: %d, fragment-size: %d", size, position, _state, _size);

                SizeType remaining = (_size - size - position);
                if(remaining != 0)
                    l2.addFragment(FRAGMENT_STATE_UNUSED, _offset + position + size, remaining);

                if(position == 0) {
                    _size = size;
                    _state = FRAGMENT_STATE_ALLOCATED;
                    return *this;
                } else {
                    _size = position;
                    return *l2.addFragment(FRAGMENT_STATE_ALLOCATED, _offset + position, size);
                }
            }

            FragmentState _state;
            SizeType _offset;
            SizeType _size;
        };

        typedef std::queue<sp<Fragment>> AllocatorQueue;
        typedef BitwiseTrie<SizeType, AllocatorQueue> AllocatorTrie;

        SizeType allocate(SizeType size) {
            std::queue<sp<Fragment>>* queue = _allocators.find(size);
            if(queue) {
                sp<Fragment> fragment;
                while(!queue->empty()) {
                    fragment = queue->front();
                    queue->pop();
                    if(fragment->_state == FRAGMENT_STATE_UNUSED)
                        break;
                }
                if(fragment)
                    return fragment->split(0, size, *this)._offset + _offset;
            }
            return npos;
        }

        sp<Fragment> addFragment(FragmentState state, SizeType offset, SizeType size) {
            DCHECK((size % kAlignment) == 0 && (offset % kAlignment) == 0, "Illegal fragment size: %d, offset: %d, aligment(%d) unsatisfied", size, offset, kAlignment);

            sp<Fragment> fragment = _object_pool->obtain<Fragment>(state, offset, size);
            if(state == FRAGMENT_STATE_UNUSED)
                ensureQueue(size).push(fragment);
            _fragments.insert(std::make_pair(offset, fragment));
            return fragment;
        }

        AllocatorQueue& ensureQueue(SizeType size) {
            AllocatorQueue* queue = _allocators.find(size);
            if(queue)
                return *queue;
            return *_allocators.put(size, {});
        }

        sp<ObjectPool> _object_pool;
        SizeType _offset;
        SizeType _size;

        AllocatorTrie _allocators;
        std::map<SizeType, sp<Fragment>> _fragments;
    };

    struct L3 {

    };

    struct Stub {
        Stub(SizeType size)
            : _size(size), _object_pool(sp<ObjectPool>::make()), _l2(*this, 0, size) {
        }

        SizeType _size;
        sp<ObjectPool> _object_pool;

        L1 _l1;
        L2 _l2;
        L3 _l3;
    };

public:
    Heap(SizeType size)
        : _stub(sp<Stub>::make(size)) {
    }

    SizeType allocate(SizeType size) {
        return _stub->_l2.allocate(align(size));
    }

    static SizeType align(SizeType size) {
        SizeType m = size % kAlignment;
        return m ? size + (kAlignment - m) : size;
    }

private:
    sp<Stub> _stub;
};

}

#endif
