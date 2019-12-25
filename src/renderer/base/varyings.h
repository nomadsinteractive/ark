#ifndef ARK_RENDERER_BASE_VARYINGS_H_
#define ARK_RENDERER_BASE_VARYINGS_H_

#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/array.h"
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
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        ByteArray::Borrowed _memory;
    };

public:
//[[script::bindings::auto]]
    Varyings();

    virtual void traverse(const Visitor& visitor) override;

    bool update(uint64_t timestamp) const;

//[[script::bindings::auto]]
    void set(const String& name, const sp<Numeric>& var);
//[[script::bindings::auto]]
    void set(const String& name, const sp<Vec2>& var);
//[[script::bindings::auto]]
    void set(const String& name, const sp<Vec3>& var);
//[[script::bindings::auto]]
    void set(const String& name, const sp<Vec4>& var);

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
        std::vector<VaryingBuilder> _varying_builders;
    };

private:
    void setVarying(const String& name, sp<Flatable> flatable);

private:
    sp<PipelineInput> _pipeline_input;
    std::map<String, Varying> _varyings;
    uint32_t _size;

    friend class BUILDER;
};

}

#endif
