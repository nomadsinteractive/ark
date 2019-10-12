#ifndef ARK_RENDERER_BASE_VARYINGS_H_
#define ARK_RENDERER_BASE_VARYINGS_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Varyings : public Holder {
private:
    class Varying {
    public:
        Varying(const sp<Flatable>& flatable, int32_t offset = -1);
        Varying();
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Varying);

        void apply(uint8_t* ptr) const;

    private:
        sp<Flatable> _flatable;
        int32_t _offset;

        friend class Varyings;
    };

public:
    struct Snapshot {
        Snapshot();
        Snapshot(ByteArray::Borrowed memory);
        DEFAULT_COPY_AND_ASSIGN(Snapshot);

        ByteArray::Borrowed _memory;
    };

public:
//[[script::bindings::auto]]
    Varyings();

    virtual void traverse(const Visitor& visitor) override;

//[[script::bindings::auto]]
    void add(const String& name, const sp<Numeric>& var);

    Snapshot snapshot(const PipelineInput& pipelineInput, Allocator& allocator);

//  [[plugin::builder]]
    class BUILDER : public Builder<Varyings> {
    private:
        class VaryingBuilder {
        public:
            VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable);
            DEFAULT_COPY_AND_ASSIGN(VaryingBuilder);

            String _name;
            sp<Builder<Flatable>> _flatable;
        };

    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Varyings> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        std::vector<VaryingBuilder> _varying_builders;
    };

private:
    void addVarying(const String& name, const sp<Flatable>& flatable);

private:
    sp<PipelineInput> _pipeline_input;
    std::unordered_map<String, Varying> _varyings;
    uint32_t _size;

    friend class BUILDER;
};

}

#endif
