#ifndef ARK_CORE_CORE_CONCURRENT_LOCK_FREE_STACK_H_
#define ARK_CORE_CORE_CONCURRENT_LOCK_FREE_STACK_H_

#include "core/concurrent/internal.h"
#include "core/forwarding.h"

namespace ark {

template<typename T> class LockFreeStack {
private:

    typedef internal::concurrent::_Node<T> Node;
    typedef internal::concurrent::_iterator<Node, T> iterator;
    typedef internal::concurrent::_const_iterator<Node, T> const_iterator;

    class ClearIterable {
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
        ClearIterable(Node* node, Node* recycler)
            : _node(node), _deleter(std::make_shared<Deleter>(node, recycler)) {
        }
        ClearIterable(const ClearIterable& other)
            : _node(other._node), _deleter(other._deleter) {
        }

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

public:
    LockFreeStack()
        : _recycler() {
    }
    ~LockFreeStack() {
        clear();
    }

    ClearIterable clear() {
        return ClearIterable(_delegate.release(), _recycler.release());
    }

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
    void push(const T& data) {
        _delegate.push(obtain(data));
    }
//[[ark::threadsafe]]
    bool pop(T& data) {
        Node* head = _delegate.pop();
        if(head) {
            data = head->data();
            _recycler.put(head);
            return true;
        }
        return false;
    }
//[[ark::threadsafe]]
    void pop() {
        Node* node = _delegate.pop();
        if(node)
            _recycler.put(node);
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
    Node* obtain(const T& data) {
        Node* pooled = _recycler.obtain();
        if(pooled)
            return new(pooled) Node(data, nullptr);
        return Node::alloc(data);
    }

private:
    internal::concurrent::_Stack<Node> _delegate;
    internal::concurrent::_Recycler<Node> _recycler;

    DISALLOW_COPY_AND_ASSIGN(LockFreeStack);
};

}

#endif
