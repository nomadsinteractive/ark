#ifndef ARK_RENDERER_BASE_GL_VARIABLES_H_
#define ARK_RENDERER_BASE_GL_VARIABLES_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLVariables {
public:
    class Varying {
    public:
        Varying(uint16_t offset, const sp<Flatable>& flatable);
        Varying(const Varying& other) = default;
        Varying(Varying&& other) = default;

        void settle(uint8_t* ptr) const;

    private:
        uint16_t _offset;
        sp<Flatable> _flatable;
    };

public:
//[[script::bindings::auto]]
    GLVariables(const sp<Numeric>& alpha = nullptr);

    void addVarying(const Varying& varying);
    void setVaryings(void *buf, uint32_t stride, uint32_t count) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<GLVariables> {
    private:
        class VaryingBuilder {
        public:
            VaryingBuilder(const String& name, const sp<Builder<Flatable>>& flatable);
            VaryingBuilder(const VaryingBuilder& other);
            VaryingBuilder(VaryingBuilder&& other);

            Varying build(const sp<Scope> &args) const;

            String _name;
            sp<Builder<Flatable>> _flatable;

            uint16_t _offset;
        };

    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLVariables> build(const sp<Scope>& args) override;

    private:
        void initVaryings(const sp<GLShader>& shader, const sp<Scope>& args);

    private:
        BeanFactory _factory;
        document _manifest;

        sp<Builder<GLShader>> _shader;
        sp<Builder<Numeric>> _alpha;

        List<VaryingBuilder> _varying_builders;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<GLVariables> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<GLVariables> build(const sp<Scope>& args) override;

    private:
        sp<BUILDER> _delegate;
    };

private:
    sp<Numeric> _alpha;

    List<Varying> _varyings;
};

}

#endif
