#ifndef ARK_CORE_BASE_BEAN_FACTORY_WEAK_REF_H_
#define ARK_CORE_BASE_BEAN_FACTORY_WEAK_REF_H_

#include "core/types/shared_ptr.h"

namespace ark {

class BeanFactoryWeakRef {
public:
    BeanFactoryWeakRef(const BeanFactory& beanFactory);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(BeanFactoryWeakRef);

    BeanFactory ensure() const;

private:
    struct WeakRef;

    sp<WeakRef> _weak_ref;
};

}

#endif
