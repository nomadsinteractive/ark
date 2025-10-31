#pragma once

#include "core/forwarding.h"

#include "core/concurrent/lf_stack.h"

namespace ark {

template<typename T> class LFQueue {
public:
    LFQueue()
        : _ins(&_recycler), _outs(&_recycler) {
    }

//  [[ark::threadsafe]]
    void push(T data) {
        _ins.push(std::move(data));
    }

//  [[ark::threadsafe]]
    Optional<T> pop() {
        if(Optional<T> opt = _outs.pop())
            return opt;
        for(T& i : _ins.clear())
            _outs.push(std::move(i));
        return _outs.pop();
    }

private:
    typedef internal::concurrent::_Node<T> Node;

    internal::concurrent::_Recycler<Node> _recycler;

    LFStack<T> _ins;
    LFStack<T> _outs;

    DISALLOW_COPY_AND_ASSIGN(LFQueue);
};

}
