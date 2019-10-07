#include "core/impl/dictionary/yaml_string_bundle.h"

#include <yaml.h>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/inf/readable.h"
#include "core/util/documents.h"
#include "core/types/null.h"

#include "platform/platform.h"

namespace ark {

static int _yaml_read_handler(void* data, unsigned char* buffer, size_t size, size_t* size_read)
{
    Readable* readable = reinterpret_cast<Readable*>(data);
    *size_read = readable->read(buffer, size);
    return 1;
}

YAMLStringBundle::YAMLStringBundle(const sp<AssetBundle>& resource)
    : _resource(resource)
{
    DASSERT(_resource);
}

sp<String> YAMLStringBundle::get(const String& name)
{
    String package, nodename;
    Strings::cut(name, package, nodename, '/');
    DCHECK(package && nodename, "Illegal string bundle \"%s\"", name.c_str());
    const auto iter = _bundle.find(package);
    if(iter == _bundle.end())
        loadBundle(package);

    std::map<String, sp<Item>>& bundle = _bundle[package];
    String arrayname;
    int32_t arrayindex;
    do {
        if(Strings::parseArrayAndIndex(nodename, arrayname, arrayindex))
        {
            const auto iter1 = bundle.find(arrayname);

            if(iter1 == bundle.end())
            {
                DWARN(false, "YAML node \"%s\" not found", arrayname.c_str());
                break;
            }
            if(!iter1->second->isSequence())
            {
                DWARN(false, "YAML node \"%s\" found, but it's not an array", arrayname.c_str());
                break;
            }
            if(arrayindex < 0 || static_cast<size_t>(arrayindex) >= iter1->second->sequence()->size())
            {
                DWARN(false , "YAML node \"%s\" found, but index out of range", arrayname.c_str());
                break;
            }
            return iter1->second->sequence()->at(static_cast<size_t>(arrayindex));
        }
        const auto iter1 = bundle.find(nodename);
        if(iter1 == bundle.end())
        {
            DWARN(false, "YAML node \"%s\" not found", nodename.c_str());
            break;
        }
        return iter1->second->value();
    } while(false);
    return sp<String>::make("[" + name + "]");
}

void YAMLStringBundle::loadBundle(const String& name)
{
    const sp<Asset> asset = _resource->get(name + ".yaml");
    DCHECK(asset, "Unable to load %s.yaml", name.c_str());
    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser))
        FATAL("Failed to initialize parser");

    const sp<Readable> readable = asset->open();
    yaml_parser_set_input(&parser, _yaml_read_handler, readable.get());
    yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);

    yaml_event_t event;
    yaml_event_type_t eventType;

    std::map<String, sp<Item>>& bundle = _bundle[name];
    std::vector<String> keys;
    sp<Item> value;
    do {
        if (!yaml_parser_parse(&parser, &event))
            FATAL("YAML parser error %d\n", parser.error);

        eventType = event.type;
        switch(eventType)
        {
        case YAML_MAPPING_START_EVENT:
            keys.push_back("");
            value = nullptr;
            break;
        case YAML_MAPPING_END_EVENT:
            keys.pop_back();
            break;
        case YAML_SEQUENCE_START_EVENT:
            DCHECK(value, "Unable to start YAML sequence");
            value->makeSequence();
            break;
        case YAML_SEQUENCE_END_EVENT:
            makeKey(bundle, keys) = std::move(value);
            break;
        case YAML_SCALAR_EVENT: {
            DCHECK(keys.size() > 0, "Illegal state: mapping event not start.");
            String scalar = reinterpret_cast<const char*>(event.data.scalar.value);
            if(value == nullptr)
            {
                keys.back() = std::move(scalar);
                value = sp<Item>::make();
            }
            else
            {
                if(value->isSequence())
                    value->addSequenceValue(std::move(scalar));
                else
                {
                    value->setValue(std::move(scalar));
                    makeKey(bundle, keys) = std::move(value);
                }
            }
        }
            break;
        default:
            break;
        }
        yaml_event_delete(&event);
    } while(eventType != YAML_STREAM_END_EVENT);
    yaml_parser_delete(&parser);
}

sp<YAMLStringBundle::Item>& YAMLStringBundle::makeKey(std::map<String, sp<Item>>& bundle, const std::vector<String>& keys) const
{
    StringBuffer sb;
    bool dirty = false;
    for(const String& i : keys)
    {
        if(dirty)
            sb << '/';
        else
            dirty = true;
        sb << i;

    }
    return bundle[sb.str()];
}

YAMLStringBundle::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC))
{
}

sp<StringBundle> YAMLStringBundle::BUILDER::build(const Scope& args)
{
    return sp<YAMLStringBundle>::make(Ark::instance().getAssetBundle(_src->build(args)));
}

void YAMLStringBundle::Item::setValue(String value)
{
    _value = sp<String>::make(std::move(value));
}

void YAMLStringBundle::Item::addSequenceValue(String value)
{
    DASSERT(_sequence);
    _sequence->push_back(sp<String>::make(std::move(value)));
}

void YAMLStringBundle::Item::makeSequence()
{
    _sequence = sp<std::vector<sp<String>>>::make();
}

bool YAMLStringBundle::Item::isSequence() const
{
    return static_cast<bool>(_sequence);
}

const sp<String>& YAMLStringBundle::Item::value() const
{
    return _value;
}

const sp<std::vector<sp<String>>>& YAMLStringBundle::Item::sequence() const
{
    return _sequence;
}

}
