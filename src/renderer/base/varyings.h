#ifndef ARK_RENDERER_BASE_VARYINGS_H_
#define ARK_RENDERER_BASE_VARYINGS_H_

#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/impl/input/input_variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Varyings : public Holder {
private:
    class Slot {
    public:
        Slot(sp<Input> input = nullptr, uint32_t divisor = 0, int32_t offset = -1);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Slot);

        void apply(uint8_t* ptr) const;

    private:
        sp<Input> _input;
        uint32_t _divisor;
        int32_t _offset;

        friend class Varyings;
    };

    struct Divided {
        Divided(uint32_t divisor, ByteArray::Borrowed content);

        uint32_t _divisor;
        ByteArray::Borrowed _content;
    };

public:
    struct Snapshot {
        Snapshot() = default;
        Snapshot(Array<Divided>::Borrowed buffers);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        Array<Divided>::Borrowed _buffers;
        std::map<size_t, Snapshot> _sub_properties;

        explicit operator bool() const;

        ByteArray::Borrowed getDivided(uint32_t divisor) const;
        void snapshotSubProperties(const std::map<String, sp<Varyings>>& subProperties, const PipelineInput& pipelineInput, Allocator& allocator);
    };

public:
//[[script::bindings::auto]]
    Varyings();

    virtual void traverse(const Visitor& visitor) override;

    bool update(uint64_t timestamp) const;

//[[script::bindings::getprop]]
    Box getProperty(const String& name) const;

//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Integer> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Numeric> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec2> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec3> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec4> var);
//[[script::bindings::setprop]]
    void setProperty(const String& name, sp<Mat4> var);

//[[script::bindings::map([])]]
    sp<Varyings> subscribe(const String& name);

    Snapshot snapshot(const PipelineInput& pipelineInput, Allocator& allocator);

//  [[plugin::builder]]
    class BUILDER : public Builder<Varyings> {
    private:
        class VaryingBuilder {
        public:
            VaryingBuilder(String name, sp<Builder<Input>> input);
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
    template<typename T> void setProperty(const String& name, sp<Variable<T>> var) {
        String cname = Strings::capitalizeFirst(name);
        _properties[cname] = var;
        setSlotInput(std::move(cname), sp<Input>::make<InputVariable<T>>(std::move(var)));
    }

    void setSlotInput(const String& name, sp<Input> input);

private:
    std::map<String, Box> _properties;
    std::map<String, Slot> _slots;
    std::map<uint32_t, uint32_t> _slot_strides;

    std::map<String, sp<Varyings>> _sub_properties;

    friend class BUILDER;
};

}

#endif
