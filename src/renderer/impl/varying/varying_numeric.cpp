#include "renderer/impl/varying/varying_numeric.h"

#include "core/base/bean_factory.h"

#include "graphics/base/render_request.h"

namespace ark {

VaryingNumeric::VaryingNumeric(String name, sp<Numeric> value)
    : _name(std::move(name)), _value(std::move(value))
{
}

void VaryingNumeric::bind(const PipelineInput& pipelineInput)
{
}

ByteArray::Borrowed VaryingNumeric::snapshot(const RenderRequest& renderRequest)
{
    ByteArray::Borrowed snapshot = renderRequest.allocator().sbrk(sizeof(float));
    return snapshot;
}

void VaryingNumeric::apply(Writable& writer, const ByteArray::Borrowed& snapshot, uint32_t vertexId)
{
}

VaryingNumeric::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _name(factory.ensureBuilder<String>(manifest, Constants::Attributes::NAME)), _value(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::VALUE))
{
}

sp<Varying> VaryingNumeric::BUILDER::build(const Scope& args)
{
    return sp<VaryingNumeric>::make(_name->build(args), _value->build(args));
}

}
