#ifndef ARK_CORE_IMPL_INPUT_INPUT_SHAPSHOT_H_
#define ARK_CORE_IMPL_INPUT_INPUT_SHAPSHOT_H_

#include <vector>

#include "core/inf/input.h"

namespace ark {

class InputSnapshot : public Input {
public:
    InputSnapshot(Input& delegate);

    virtual void upload(Writable& writable) override;

private:
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;
};

}

#endif
