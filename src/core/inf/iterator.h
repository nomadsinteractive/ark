#ifndef ARK_CORE_INF_ITERATOR_H_
#define ARK_CORE_INF_ITERATOR_H_

namespace ark {

template<typename T> class Iterator {
public:
    virtual ~Iterator() = default;

    virtual bool hasNext() = 0;
    virtual T next() = 0;
};

}

#endif
