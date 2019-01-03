#ifndef ARK_CORE_CONCURRENT_ONE_CONSUMER_SYNCHRONIZED_H_
#define ARK_CORE_CONCURRENT_ONE_CONSUMER_SYNCHRONIZED_H_

#include <list>
#include <queue>
#include <unordered_set>

#include "core/concurrent/internal.h"

namespace ark {
namespace internal {
namespace concurrent {


template<typename T, typename Container, typename Synchronizer> class _OneConsumerSynchronized {
private:
    typedef _Node<T> Node;

public:
    void add(const T& data) {
        _pending.push(obtain(data));
    }

    size_t size() const {
        return _collection.size();
    }

    Container& synchronize() {
        if(_pending.head()) {
            Node* head = _pending.release();
            Synchronizer::append(_collection, head);
            while(head) {
                Node* next = head->next();
                _recycler.put(head);
                head = next;
            }
        }
        return _collection;
    }

private:
    Node* obtain(const T& data) {
        Node* pooled = _recycler.obtain();
        if(pooled)
            return new(pooled) Node(data, nullptr);
        return Node::alloc(data);
    }

private:
    Container _collection;

    _Stack<Node> _pending;
    _Recycler<Node> _recycler;
};

template<typename T> class _QueueSynchronizer {
public:
    static void append(std::queue<T>& queue, _Node<T>* head) {
        std::list<T> list;
        while(head) {
            list.push_front(head->data());
            head = head->next();
        }
        for(const T& i : list)
            queue.push(i);
    }
};

template<typename T> class _UnorderedSetSynchronizer {
public:
    static void append(std::unordered_set<T>& unorderedSet, _Node<T>* head) {
        while(head) {
            unorderedSet.insert(head->data());
            head = head->next();
        }
    }
};

}
}

template<typename T> class OCSQueue : public internal::concurrent::_OneConsumerSynchronized<T, std::queue<T>, internal::concurrent::_QueueSynchronizer<T>> {
public:
    bool pop(T& data) {
        std::queue<T>& queue = this->synchronize();
        if(queue.empty())
            return false;
        data = queue.front();
        queue.pop();
        return true;
    }
};

template<typename T> class OCSUnorderedSet : public internal::concurrent::_OneConsumerSynchronized<T, std::unordered_set<T>, internal::concurrent::_UnorderedSetSynchronizer<T>> {
public:
    bool containsOrInsert(const T& data) {
        const auto iter = this->_collection->find(data);
        if(iter != this->_collection->end())
            return true;
        this->add(data);
        return false;
    }
};

}

#endif
