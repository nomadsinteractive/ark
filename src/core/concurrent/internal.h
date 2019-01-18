#ifndef ARK_CORE_CONCURRENT_INTERNAL_H_
#define ARK_CORE_CONCURRENT_INTERNAL_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <stdlib.h>

#include "core/base/api.h"

namespace ark {
namespace internal {
namespace concurrent {

template<typename T> class _Node {
public:
    _Node(_Node<T>* next)
        : _next(next) {
    }
    _Node(T data, _Node<T>* next)
        : _data(std::move(data)), _next(next) {
    }

    const T& data() const {
        return _data;
    }

    T& data() {
        return _data;
    }

    _Node<T>* next() {
        return _next;
    }

    void link(_Node<T>* next) {
        _next = next;
    }

    _Node<T>* recycle() {
        this->~_Node();
        return this;
    }

    static _Node<T>* alloc(T data, _Node<T>* next = nullptr) {
        void* ptr = malloc(sizeof(_Node));
        return new(ptr) _Node(std::move(data), next);
    }

    _Node<T>* dispose() {
        this->~_Node();
        return dealloc();
    }

    _Node<T>* dealloc() {
        _Node* next = _next;
        free(this);
        return next;
    }

private:
    T _data;
    _Node<T>* _next;

};

template<typename Node, typename T> class _iterator {
public:
    _iterator(Node* node)
        : _node(node) {
    }

    T& operator *() {
        return _node->data();
    }

    const _iterator<Node, T>& operator ++ () {
        _node = _node->next();
        return *this;
    }

    const _iterator<Node, T> operator ++ (int) {
        Node* n = _node;
        _node = _node->next();
        return n;
    }

    bool operator != (const _iterator<Node, T>& other) const {
        return _node != other._node;
    }

    bool operator == (const _iterator<Node, T>& other) const {
        return _node == other._node;
    }

private:
    Node* _node;
};

template<typename Node, typename T> class _const_iterator {
public:
    _const_iterator(const Node* node)
        : _node(node) {
    }

    const T& operator *() const {
        return _node->data();
    }

    const _const_iterator<Node, T>& operator ++ () {
        _node = _node->next();
        return *this;
    }

    const _const_iterator<Node, T> operator ++ (int) {
        Node* n = _node;
        _node = _node->next();
        return n;
    }

    bool operator != (const _const_iterator<Node, T>& other) const {
        return _node != other._node;
    }

    bool operator == (const _const_iterator<Node, T>& other) const {
        return _node == other._node;
    }

private:
    const Node* _node;
};


template<typename Node> class _Stack {
public:
    _Stack()
        : _head(nullptr) {
    }

    void push(Node* node) {
//
//      node->_next = _head.load(std::memory_order_relaxed);
//      while(!_head.compare_exchange_weak(node->_next, node));
//
// http://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange
// Note: the above use is not thread-safe in at least
// GCC prior to 4.8.3 (bug 60272), clang prior to 2014-05-05 (bug 18899)
// MSVC prior to 2014-03-17 (bug 819819). The following is a workaround:
        Node* expected = _head.load(std::memory_order_relaxed);
        do {
            node->link(expected);
        } while(!_head.compare_exchange_weak(expected, node, std::memory_order_release, std::memory_order_relaxed));
    }

    Node* pop() {
        Node* popped;
        Node* expected = _head.load(std::memory_order_relaxed);
        do {
            popped = expected;
        } while(popped && !_head.compare_exchange_weak(expected, popped->next(), std::memory_order_release, std::memory_order_relaxed));
        if(popped)
            popped->link(nullptr);
        return popped;
    }

    Node* head() const {
        return _head.load(std::memory_order_relaxed);
    }

    Node* reset(Node* newHead) {
        Node* head;
        Node* expected = _head.load(std::memory_order_relaxed);
        do {
            head = expected;
        } while(!_head.compare_exchange_weak(expected, newHead, std::memory_order_release, std::memory_order_relaxed));
        return head;
    }

    Node* release() {
        return reset(nullptr);
    }

private:
    std::atomic<Node*> _head;

    DISALLOW_COPY_AND_ASSIGN(_Stack);

};

template<typename Node> class _Recycler {
public:
    _Recycler(uint64_t recyclingCoolDown = 8)
        : _last_recycle_ms(0), _recycling_cool_down_ms(recyclingCoolDown) {
    }
    ~_Recycler() {
        clear();
    }

    void put(Node* node) {
        _trash_bin.push(node->recycle());
    }

    Node* obtain() {
        Node* node = _recycler.pop();
        if(!node) {
            recycle();
            return _recycler.pop();
        }
        return node;
    }

    Node* release() {
        doRecycle();
        return _recycler.release();
    }

    void clear() {
        Node* iter = release();
        while(iter)
            iter = iter->dealloc();
    }

private:
    void recycle() {
        uint64_t tick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        uint64_t lastRecycled, expected = _last_recycle_ms.load(std::memory_order_relaxed);
        if((tick - expected) > _recycling_cool_down_ms) {
            do {
                lastRecycled = expected;
            } while(_last_recycle_ms.compare_exchange_weak(expected, tick, std::memory_order_release, std::memory_order_relaxed));
            if((tick - lastRecycled) > _recycling_cool_down_ms)
                doRecycle();
        }
    }

    void doRecycle() {
        Node* iter = _trash_bin.release();
        while(iter) {
            Node* next = iter->next();
            _recycler.push(iter);
            iter = next;
        }
    }

private:
    _Stack<Node> _trash_bin;
    _Stack<Node> _recycler;
    std::atomic<uint64_t> _last_recycle_ms;
    uint64_t _recycling_cool_down_ms;

    DISALLOW_COPY_AND_ASSIGN(_Recycler);
};


template<typename Node, typename T> class _ClearIterable {
private:
    class Deleter {
    public:
        Deleter(Node* node, Node* recycler)
            : _node(node), _recycler(recycler) {
        }

        ~Deleter() {
            Node* iter = _node;
            while(iter)
                iter = iter->dispose();
            iter = _recycler;
            while(iter)
                iter = iter->dealloc();
        }

    private:
        Node* _node;
        Node* _recycler;
    };

public:
    _ClearIterable(Node* node, Node* recycler)
        : _node(node), _deleter(std::make_shared<Deleter>(node, recycler)) {
    }
    _ClearIterable(const _ClearIterable& other) = default;

    class iterator {
    public:
        iterator(Node* node, std::shared_ptr<Deleter>& deleter)
            : _node(node), _deleter(deleter) {
        }

        iterator(const iterator& other)
            : _node(other._node), _deleter(other._deleter) {
        }

        T& operator *() {
            return _node->data();
        }

        const iterator& operator ++ () {
            _node = _node->next();
            return *this;
        }

        const iterator operator ++ (int) {
            FATAL("Illegal operator called");
            return nullptr;
        }

        bool operator != (const iterator& other) const {
            return _node != other._node;
        }

        bool operator == (const iterator& other) const {
            return _node == other._node;
        }

    private:
        Node* _node;
        std::shared_ptr<Deleter> _deleter;
    };

    class const_iterator {
    public:
        const_iterator(const Node* node, std::shared_ptr<Deleter>& deleter)
            : _node(node), _deleter(deleter) {
        }

        const_iterator(const const_iterator& other)
            : _node(other._node), _deleter(other._deleter) {
        }

        const T& operator *() const {
            return _node->_data;
        }

        const const_iterator& operator ++ () {
            _node = _node->_next;
            return *this;
        }

        const const_iterator operator ++ (int) {
            FATAL("Illegal operator called");
            return nullptr;
        }

        bool operator != (const const_iterator& other) const {
            return _node != other._node;
        }

        bool operator == (const const_iterator& other) const {
            return _node == other._node;
        }

    private:
        const Node* _node;
        std::shared_ptr<Deleter> _deleter;
    };

    iterator begin() {
        return iterator(_node, _deleter);
    }

    iterator end() {
        return iterator(nullptr, _deleter);
    }

    const_iterator begin() const {
        return const_iterator(_node, _deleter);
    }

    const_iterator end() const {
        return const_iterator(nullptr, _deleter);
    }

private:
    Node* _node;
    std::shared_ptr<Deleter> _deleter;
};

}
}
}

#endif
