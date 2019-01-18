#ifndef ARK_CORE_CONCURRENT_LF_STACK_H_
#define ARK_CORE_CONCURRENT_LF_STACK_H_

#include "core/concurrent/internal.h"
#include "core/forwarding.h"

namespace ark {

template<typename T> class LFStack {
private:
    typedef internal::concurrent::_Node<T> Node;
    typedef internal::concurrent::_iterator<Node, T> iterator;
    typedef internal::concurrent::_const_iterator<Node, T> const_iterator;
    typedef internal::concurrent::_ClearIterable<Node, T> ClearIterable;

public:
    LFStack()
        : _owned_recycler(new internal::concurrent::_Recycler<Node>()), _recycler(_owned_recycler) {
    }
    LFStack(internal::concurrent::_Recycler<Node>* recycler)
        : _owned_recycler(nullptr), _recycler(recycler) {
    }
    ~LFStack() {
        clear();
        delete _owned_recycler;
    }

    ClearIterable clear() {
        return ClearIterable(_delegate.release(), _recycler->release());
    }

//[[deprecated]]
    bool empty() const {
        return _delegate.head() == nullptr;
    }

    iterator begin() {
        return _delegate.head();
    }

    iterator end() {
        return nullptr;
    }

    const_iterator begin() const {
        return _delegate.head();
    }

    const_iterator end() const {
        return nullptr;
    }

//[[ark::threadsafe]]
    void push(T data) {
        _delegate.push(obtain(std::move(data)));
    }

//[[ark::threadsafe]]
    bool pop(T& data) {
        Node* head = _delegate.pop();
        if(head) {
            data = head->data();
            _recycler->put(head);
            return true;
        }
        return false;
    }

    template<typename U> ClearIterable assign(const U& other) {
        Node* iter = nullptr;
        Node* head = nullptr;
        for(const T& i : other) {
            Node* node = obtain(i);
            if(iter)
                iter->link(node);
            else
                head = node;
            iter = node;
        }
        return ClearIterable(_delegate.reset(head), nullptr);
    }

private:
    Node* obtain(T data) {
        Node* pooled = _recycler->obtain();
        if(pooled)
            return new(pooled) Node(std::move(data), nullptr);
        return Node::alloc(std::move(data));
    }

private:
    internal::concurrent::_Stack<Node> _delegate;

    internal::concurrent::_Recycler<Node>* _owned_recycler;
    internal::concurrent::_Recycler<Node>* _recycler;

    friend class LFQueue<T>;

    DISALLOW_COPY_AND_ASSIGN(LFStack);
};

}

#endif
