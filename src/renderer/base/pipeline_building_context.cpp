#include "renderer/base/pipeline_building_context.h"

#include <regex>

#include "core/base/string_buffer.h"
#include "core/epi/notifier.h"
#include "core/inf/flatable.h"
#include "core/util/strings.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_context.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"

namespace ark {

namespace {

class AlignedFlatable : public Flatable {
public:
    AlignedFlatable(const sp<Flatable>& delegate, size_t alignedSize)
        : _delegate(delegate), _aligned_size(alignedSize) {
        DCHECK(_delegate->size() <= _aligned_size, "Alignment is lesser than delegate's size(%d)", _delegate->size());
    }

    virtual void flat(void* buf) override {
        _delegate->flat(buf);
    }

    virtual uint32_t size() override {
        return _aligned_size;
    }

private:
    sp<Flatable> _delegate;

    size_t _aligned_size;
};

}

PipelineBuildingContext::PipelineBuildingContext(const sp<PipelineFactory>& pipelineFactory, const String& vertex, const String& fragment)
    : _pipeline_factory(pipelineFactory), _input(sp<PipelineInput>::make()), _vertex(ShaderPreprocessor::SHADER_TYPE_VERTEX), _fragment(ShaderPreprocessor::SHADER_TYPE_FRAGMENT)
{
    _vertex.initialize(vertex, *this);
    _fragment.initialize(fragment, *this);
}

PipelineBuildingContext::PipelineBuildingContext(const sp<PipelineFactory>& pipelineFactory, const String& vertex, const String& fragment, BeanFactory& factory, const Scope& args, const document& manifest)
    : _pipeline_factory(pipelineFactory), _input(sp<PipelineInput>::make()), _vertex(ShaderPreprocessor::SHADER_TYPE_VERTEX), _fragment(ShaderPreprocessor::SHADER_TYPE_FRAGMENT)
{
    loadPredefinedParam(factory, args, manifest);

    _vertex.initialize(vertex, *this);
    _fragment.initialize(fragment, *this);
}

void PipelineBuildingContext::loadPredefinedParam(BeanFactory& factory, const Scope& args, const document& manifest)
{
    loadPredefinedUniform(factory, args, manifest);
    loadPredefinedSampler(factory, args, manifest);
    loadPredefinedAttribute(manifest);
}

void PipelineBuildingContext::initialize()
{
    for(const auto& i : _vertex._ins.vars().values())
        if(_vert_in_declared.find(i.name()) == _vert_in_declared.end())
        {
            _vert_in_declared[i.name()] = i.type();
            addAttribute(i.name(), i.type());
        }

    std::set<String> fragmentUsedVars;
    static const std::regex varPattern("\\bv_([\\w\\d_]+)\\b");
    _fragment._main.search(varPattern, [&fragmentUsedVars](const std::smatch& m)->bool {
        fragmentUsedVars.insert(m[1].str());
        return true;
    });

    for(const auto& i : _vertex_in)
        _vertex._ins.declare(i.first, "a_", Strings::capitalFirst(i.second));
    for(const auto& i : _fragment_in)
    {
        const String n = Strings::capitalFirst(i.second);
        fragmentUsedVars.insert(n);
        _fragment._ins.declare(i.first, "v_", n);
    }

    Table<String, String> attributes;
    for(const auto& i : _fragment._ins.vars().values())
        if(!attributes.has(i.name()))
            attributes.push_back(i.name(), i.type());

    for(const auto& i : _attributes)
        if(!attributes.has(i.first))
            attributes.push_back(i.first, i.second.declareType());

    std::vector<String> generated;
    for(const auto& i : attributes)
    {
        if(!_vertex._ins.has(i.first)
                && !_vertex._outs.has(i.first)
                && !_vertex._main_block->hasOutParam(i.first))
        {
            generated.push_back(i.first);
            addAttribute(i.first, i.second);
        }
    }

    for(auto iter : _input->streams())
        iter.second.align();

    for(const auto& i : attributes)
    {
        if(fragmentUsedVars.find(i.first) != fragmentUsedVars.end())
            _fragment._ins.declare(i.second, "v_", i.first);
    }

    for(const String& i : generated)
    {
        _vertex._ins.declare(attributes.at(i), "a_", i);
        if(fragmentUsedVars.find(i) != fragmentUsedVars.end())
        {
            _vertex._outs.declare(attributes.at(i), "v_", i);
            _vertex.addPreMainSource(Strings::sprintf("v_%s = a_%s;", i.c_str(), i.c_str()));
        }
    }

    for(const auto& i : _vertex_out)
        _vertex._outs.declare(i.first, "", i.second);
}

void PipelineBuildingContext::setupUniforms()
{
    int32_t binding = 0;
    _vertex.setupUniforms(_uniforms, binding);
    _fragment.setupUniforms(_uniforms, binding);
}

void PipelineBuildingContext::addAttribute(const String& name, const String& type)
{
    Attribute& attr = addPredefinedAttribute(name, type, 0);
    _input->addAttribute(name, attr);
}

void PipelineBuildingContext::addSnippet(const sp<Snippet>& snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::adopt(new SnippetLinkedChain(_snippet, snippet)) : snippet;
}

void PipelineBuildingContext::addUniform(const String& name, Uniform::Type type, uint32_t length, const sp<Flatable>& flatable, const sp<Notifier>& notifier, int32_t binding)
{
    _uniforms.push_back(name, sp<Uniform>::make(name, type, length, flatable, notifier, binding));
}

void PipelineBuildingContext::addUniform(const sp<Uniform>& uniform)
{
    _uniforms.push_back(uniform->name(), uniform);
}

Attribute& PipelineBuildingContext::addPredefinedAttribute(const String& name, const String& type, uint32_t scopes)
{
    if(_attributes.find(name) == _attributes.end())
        _attributes[name] = makePredefinedAttribute(name, type);

    if(scopes == ShaderPreprocessor::SHADER_TYPE_FRAGMENT)
        _fragment_in.push_back(std::pair<String, String>(type, name));

    return _attributes[name];
}

void PipelineBuildingContext::loadPredefinedAttribute(const document& manifest)
{
    for(const document& i : manifest->children("attribute"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        DCHECK(!name.empty(), "Empty name");
        DWARN(isupper(name[0]) || name.startsWith("a_"), "GLAttribute name \"%s\" should be capital first or started with a_", name.c_str());
        const String attrName = name.startsWith("a_") ? name.substr(2) : name;
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        uint32_t divisor = Documents::getAttribute<uint32_t>(i, "divisor", 0);
        addPredefinedAttribute(attrName, type, 0).setDivisor(divisor);
    }
}

void PipelineBuildingContext::loadPredefinedUniform(BeanFactory& factory, const Scope& args, const document& manifest)
{
    for(const document& i : manifest->children("uniform"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        const String& type = Documents::ensureAttribute(i, Constants::Attributes::TYPE);
        const String& value = Documents::ensureAttribute(i, Constants::Attributes::VALUE);
        int32_t binding = Documents::getAttribute<int32_t>(i, Constants::Attributes::BINDING, -1);
        const sp<Flatable> flatable = factory.ensure<Flatable>(type, value, args);
        const uint32_t size = flatable->size();
        Uniform::Type uType = Uniform::TYPE_F1;
        switch (size) {
        case 4:
            if(type[0] == 'f')
                uType = Uniform::TYPE_F1;
            else if(type[0] == 'i')
                uType = Uniform::TYPE_I1;
            else
                FATAL("Unknow type \"%s\"", type.c_str());
            break;
        case 8:
            uType = Uniform::TYPE_F2;
            break;
        case 12:
            uType = Uniform::TYPE_F3;
            break;
        case 16:
            uType = Uniform::TYPE_F4;
            break;
        case 64:
            uType = Uniform::TYPE_MAT4;
            break;
        default:
            FATAL("Unknow type \"%s\"", type.c_str());
        }
        addUniform(name, uType, 1, uType == Uniform::TYPE_F3 ? sp<Flatable>::adopt(new AlignedFlatable(flatable, 16)) : flatable, flatable.as<Notifier>(), binding);
    }
}

void PipelineBuildingContext::loadPredefinedSampler(BeanFactory& factory, const Scope& args, const document& manifest)
{
    uint32_t binding = 0;

    for(const document& i : manifest->children("sampler"))
    {
        String name = Documents::getAttribute(i, Constants::Attributes::NAME);
        const sp<Texture> texture = factory.ensure<Texture>(i, args);
        if(!name)
            name = Strings::sprintf("u_Texture%d", binding);
        DCHECK(!_samplers.has(name), "Sampler \"%s\" redefined", name.c_str());
        _samplers.push_back(name, texture);
        binding++;
    }
}

Attribute PipelineBuildingContext::makePredefinedAttribute(const String& name, const String& type)
{
    if(type == "vec3")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 3, false);
    if(name == "TexCoordinate")
        return Attribute("a_TexCoordinate", Attribute::TYPE_USHORT, type, 2, true);
    if(type == "vec2")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 2, false);
    if(name == "Position")
        return Attribute("a_Position", Attribute::TYPE_FLOAT, type, 3, false);
    if(type == "float")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 1, false);
    if(type == "int")
        return Attribute("a_" + name, Attribute::TYPE_INTEGER, type, 1, false);
    if(type == "vec4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 4, false);
    if(type == "vec4b")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 4, true);
    if(type == "vec3b")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 3, true);
    if(type == "uint8")
        return Attribute("a_" + name, Attribute::TYPE_UBYTE, type, 1, false);
    if(type == "mat4")
        return Attribute("a_" + name, Attribute::TYPE_FLOAT, type, 16, false);
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return Attribute();
}

}
