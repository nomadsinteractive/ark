#ifndef ARK_CORE_CONCURRENT_LF_QUEUE_H_
#define ARK_CORE_CONCURRENT_LF_QUEUE_H_

#include "core/forwarding.h"

#include "core/concurrent/lf_stack.h"

namespace ark {

template<typename T> class LFQueue {
public:
    LFQueue()
        : _ins(&_recycler), _outs(&_recycler) {
    }

//[[ark::threadsafe]]
    void push(T data) {
        _ins.push(std::move(data));
    }

//[[ark::threadsafe]]
    bool pop(T& data) {
        if(_outs.pop(data))
            return true;
        for(const T& i : _ins.clear())
            _outs.push(i);
        return _outs.pop(data);
    }

private:
    typedef internal::concurrent::_Node<T> Node;

    internal::concurrent::_Recycler<Node> _recycler;

    LFStack<T> _ins;
    LFStack<T> _outs;

    DISALLOW_COPY_AND_ASSIGN(LFQueue);
};

}

#endif
