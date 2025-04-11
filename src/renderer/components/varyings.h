#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/timestamp.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/impl/uploader/uploader_of_variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Varyings {
private:
    struct SlotSnapshot {
        SlotSnapshot(void* content, uint32_t offset, uint32_t size);

        void* _content;
        uint32_t _offset;
        uint32_t _size;
        SlotSnapshot* _next;
    };

    struct Slot {
        sp<Uploader> _uploader;
        uint32_t _divisor = 0;
        int32_t _offset = -1;
    };

public:
    struct Divided {
        Divided();
        Divided(uint32_t divisor, ByteArray::Borrowed content);

        explicit operator bool() const;

        void apply(const SlotSnapshot* slots = nullptr);
        void addSnapshot(Allocator& allocator, const Slot& slot);

        void addSlotSnapshot(SlotSnapshot* slotSnapshot);

        uint32_t _divisor;

        ByteArray::Borrowed _content;
        SlotSnapshot* _slot_snapshot;
    };

    struct Snapshot {
        Snapshot() = default;
        Snapshot(const Array<Divided>::Borrowed& buffers);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        void apply(const Snapshot* defaults = nullptr);

        Array<Divided>::Borrowed _buffers;
        Map<HashId, Snapshot> _sub_properties;

        explicit operator bool() const;

        Varyings::Divided getDivided(uint32_t divisor) const;
        void snapshotSubProperties(const Map<String, sp<Varyings>>& subProperties, const PipelineLayout& pipelineInput, Allocator& allocator);
    };

public:
//  [[script::bindings::auto]]
    Varyings() = default;
    Varyings(const PipelineLayout& pipelineLayout);

    bool update(uint64_t timestamp);

//  [[script::bindings::getprop]]
    Box getProperty(const String& name) const;

//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Integer> var);
//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Numeric> var);
//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec2> var);
//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec3> var);
//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Vec4> var);
//  [[script::bindings::setprop]]
    void setProperty(const String& name, sp<Mat4> var);

//  [[script::bindings::map(get)]]
    sp<Varyings> subscribe(const String& name);

    Snapshot snapshot(const PipelineLayout& pipelineLayout, Allocator& allocator);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Varyings> {
    private:
        class UploaderBuilder {
        public:
            UploaderBuilder(BeanFactory& factory, const document& manifest);
            DEFAULT_COPY_AND_ASSIGN(UploaderBuilder);

            String _name;
            sp<Builder<Uploader>> _input;
        };

    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Varyings> build(const Scope& args) override;

    private:
        Vector<UploaderBuilder> _uploader_builders;
    };

private:
    template<typename T> void setProperty(const String& name, sp<Variable<T>> var) {
        String cname = Strings::capitalizeFirst(name);
        _properties[cname] = Box(var);
        setSlotUploader(std::move(cname), sp<Uploader>::make<UploaderOfVariable<T>>(std::move(var)));
        _timestamp.markDirty();
    }

    void setSlotUploader(const String& name, sp<Uploader> uploader);

private:
    Map<String, Box> _properties;
    Map<String, Slot> _slots;
    Map<uint32_t, uint32_t> _slot_strides;

    Map<String, sp<Varyings>> _sub_properties;

    Timestamp _timestamp;

    friend class BUILDER;
};

}
