#ifndef ARK_CORE_IMPL_INPUT_INPUT_IMPL_H_
#define ARK_CORE_IMPL_INPUT_INPUT_IMPL_H_

#include <map>
#include <vector>

#include "core/types/shared_ptr.h"
#include "core/inf/input.h"

namespace ark {

class InputImpl : public Input {
public:
    InputImpl(size_t size);
    InputImpl(const std::map<size_t, sp<Input>>& inputs, size_t size = 0);

    virtual bool update(uint64_t timestamp) override;

    virtual void upload(Writable& writable) override;

    void addInput(size_t offset, sp<Input> input);
    void removeInput(size_t offset);

private:
    struct InputStub {
        InputStub(size_t offset, sp<Input> input);

        size_t _offset;
        sp<Input> _input;
        bool _dirty;
        bool _fresh;
    };

    std::vector<InputStub> makeInputs(const std::map<size_t, sp<Input>>& inputs) const;
    size_t calculateUploaderSize() const;

    static bool _input_stub_comp(size_t offset, const InputStub& inputStub);

private:
    std::vector<InputStub> _inputs;
    std::vector<uint8_t> _buf;
};

}

#endif
