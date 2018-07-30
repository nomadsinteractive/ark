#ifndef ARK_RENDERER_BASE_VARYINGS_H_
#define ARK_RENDERER_BASE_VARYINGS_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Varyings {
private:
    class Varying {
    public:
        Varying(uint16_t offset, const sp<Flatable>& flatable);
        Varying();
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Varying);

        void apply(uint8_t* ptr) const;

    private:
        uint16_t _offset;
        sp<Flatable> _flatable;
    };

public:
    class Snapshot {
    public:
        Snapshot() = default;
        Snapshot(const bytearray& bytes);
        DEFAULT_COPY_AND_ASSIGN(Snapshot);

        bytearray _bytes;
    };

public:
//[[script::bindings::auto]]
    Varyings(const GLShader& shader);
    Varyings();

    void addVarying(const String& name, const sp<Flatable>& flatable);

//[[script::bindings::auto]]
    void add(const String& name, const sp<Numeric>& var);

    Snapshot snapshot(MemoryPool& memoryPool) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Varyings> {
    private:
        class VaryingBuilder {
        public:
            VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable);
            VaryingBuilder(const VaryingBuilder& other) = default;
            VaryingBuilder(VaryingBuilder&& other) = default;

            String _name;
            sp<Builder<Flatable>> _flatable;
        };

    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Varyings> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        sp<Builder<GLShader>> _shader;
        std::vector<VaryingBuilder> _varying_builders;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Varyings> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Varyings> build(const sp<Scope>& args) override;

    private:
        sp<BUILDER> _delegate;
    };

private:
    sp<GLShaderSource> _shader_source;
    std::unordered_map<String, Varying> _varyings;
    size_t _size;
};

}

#endif
