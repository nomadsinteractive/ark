#ifndef ARK_RENDERER_BASE_VARYINGS_H_
#define ARK_RENDERER_BASE_VARYINGS_H_

#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/inf/input.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Varyings : public Holder {
private:
    class Slot {
    public:
        Slot(sp<Input> input, int32_t offset = -1);
        Slot();
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Slot);

        void apply(uint8_t* ptr) const;

    private:
        sp<Input> _input;
        int32_t _offset;

        friend class Varyings;
    };

public:
    struct Snapshot {
        Snapshot();
        Snapshot(ByteArray::Borrowed memory);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        ByteArray::Borrowed _memory;

        explicit operator bool() const;
    };

public:
//[[script::bindings::auto]]
    Varyings();

    virtual void traverse(const Visitor& visitor) override;

    bool update(uint64_t timestamp) const;

//[[script::bindings::getprop]]
    Box getProperty(const String& name) const;

//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Numeric> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec2> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec3> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec4> var);

    Snapshot snapshot(const PipelineInput& pipelineInput, Allocator& allocator);

//  [[plugin::builder]]
    class BUILDER : public Builder<Varyings> {
    private:
        class VaryingBuilder {
        public:
            VaryingBuilder(String name, sp<Builder<Input> > input);
            DEFAULT_COPY_AND_ASSIGN(VaryingBuilder);

            String _name;
            sp<Builder<Input>> _input;
        };

    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Varyings> build(const Scope& args) override;

    private:
        std::vector<VaryingBuilder> _varying_builders;
    };

private:
    void setVarying(const String& name, sp<Input> input);

private:
    std::map<String, Box> _properties;
    sp<PipelineInput> _pipeline_input;
    std::map<String, Slot> _varyings;
    uint32_t _size;

    friend class BUILDER;
};

}

#endif
