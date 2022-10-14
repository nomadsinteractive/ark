#ifndef ARK_RENDERER_UTIL_UPLOADER_TYPE_H_
#define ARK_RENDERER_UTIL_UPLOADER_TYPE_H_

#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::class("Uploader")]]
class ARK_API UploaderType {
public:
//[[script::bindings::constructor]]
    static sp<Uploader> create(sp<Input> value, size_t size = 0);
//[[script::bindings::constructor]]
    static sp<Uploader> create(std::map<size_t, sp<Input>> inputs, size_t size = 0);

//  [[script::bindings::property]]
    static size_t size(const sp<Uploader>& self);

//  [[script::bindings::auto]]
    static bytearray toBytes(const sp<Uploader>& self);

//  [[script::bindings::auto]]
    static void set(const sp<Uploader>& self, sp<Uploader> delegate);
//  [[script::bindings::auto]]
    static sp<Uploader> wrap(sp<Uploader> self);

//  [[script::bindings::auto]]
    static sp<Uploader> record(const sp<Uploader>& self);

//  [[script::bindings::auto]]
    static void addInput(const sp<Uploader>& self, size_t offset, sp<Input> input);
//  [[script::bindings::auto]]
    static void removeInput(const sp<Uploader>& self, size_t offset);

//  [[plugin::builder]]
    class BUILDER : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Input>> _input;
        sp<Builder<Integer>> _length;
        SafePtr<Builder<Integer>> _stride;
        std::vector<sp<Builder<Input>>> _attributes;
    };

};

}

#endif
