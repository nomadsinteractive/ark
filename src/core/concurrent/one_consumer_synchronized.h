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
    void push(const T& data) {
        _pending.push(obtain(data));
    }

    Container& synchronized() {
        if(_pending.head())
            synchronize();
        return _synchronized;
    }

    Container clear() {
        Container cleared = std::move(synchronized());
        _synchronized = Container();
        _recycler.clear();
        return cleared;
    }

private:
    Node* obtain(const T& data) {
        Node* pooled = _recycler.obtain();
        if(pooled)
            return new(pooled) Node(data, nullptr);
        return Node::alloc(data);
    }

    void synchronize() {
        Node* head = _pending.release();
        Synchronizer::synchronize(_synchronized, head);
        while(head) {
            Node* next = head->next();
            _recycler.put(head);
            head = next;
        }
    }

protected:
    Container _synchronized;
    _Stack<Node> _pending;
    _Recycler<Node> _recycler;
};

template<typename T> class _QueueSynchronizer {
public:
    static void synchronize(std::queue<T>& queue, _Node<T>* head) {
        std::list<T> l;
        while(head) {
            l.push_front(head->data());
            head = head->next();
        }
        for(const T& i : l)
            queue.push(i);
    }
};

template<typename T> class _UnorderedSetSynchronizer {
public:
    static void synchronize(std::unordered_set<T>& unorderedSet, _Node<T>* head) {
        std::list<T> l;
        while(head) {
            l.push_front(head->data());
            head = head->next();
        }
        for(const T& i : l)
            unorderedSet.insert(i);
    }
};

}
}

template<typename T> class OCSQueue : public internal::concurrent::_OneConsumerSynchronized<T, std::queue<T>, internal::concurrent::_QueueSynchronizer<T>> {
public:
    bool pop(T& data) {
        std::queue<T>& queue = this->synchronized();
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
        const auto iter = this->_synchronized.find(data);
        if(iter != this->_synchronized.end())
            return true;
        this->push(data);
        return false;
    }
};

}

#endif
