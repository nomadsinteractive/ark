#ifndef ARK_CORE_TYPES_CONVERSION_H_
#define ARK_CORE_TYPES_CONVERSION_H_

namespace ark {

class Conversions {
public:
    template<typename F, typename T> static T to(const F& instance);
};

}

#endif
