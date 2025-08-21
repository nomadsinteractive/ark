#pragma once

#include <list>
#include <queue>

#include "core/concurrent/internal.h"

namespace ark {

namespace internal::concurrent {

template<typename T, typename Container, typename Synchronizer> class _OneConsumerSynchronized {
private:
    typedef _Node<T> Node;

public:
    void add(T data) {
        _pending.push(obtain(std::move(data)));
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
    Node* obtain(T data) {
        Node* pooled = _recycler.obtain();
        if(pooled)
            return new(pooled) Node(std::move(data), nullptr);
        return Node::alloc(std::move(data));
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

}
