#ifndef ARK_RENDERER_IMPL_VARYING_VARYING_ALPHA_H_
#define ARK_RENDERER_IMPL_VARYING_VARYING_ALPHA_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/varying.h"

namespace ark {

class VaryingNumeric : public Varying {
public:
    VaryingNumeric(sp<Numeric> value);

    virtual void bind(const PipelineInput& pipelineInput) override;
    virtual ByteArray::Borrowed snapshot(const RenderRequest& renderRequest) override;
    virtual void apply(Writable& writer, const ByteArray::Borrowed& snapshot, uint32_t vertexId) override;

//  [[plugin::builder("float")]]
    class BUILDER : public Builder<Varying> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Varying> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

private:
    sp<Numeric> _value;

};

}

#endif
