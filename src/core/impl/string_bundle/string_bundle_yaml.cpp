#include "core/impl/string_bundle/string_bundle_yaml.h"

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

StringBundleYAML::StringBundleYAML(const sp<AssetBundle>& resource)
    : _resource(resource)
{
    DASSERT(_resource);
}

sp<String> StringBundleYAML::getString(const String& resid)
{
    String nodename, arrayname;
    int32_t arrayindex;
    const std::map<String, sp<Item>>& bundle = getPackageBundle(resid, nodename);
    do {
        if(Strings::parseArrayAndIndex(nodename, arrayname, arrayindex))
        {
            const auto iter = bundle.find(arrayname);

            if(iter == bundle.end())
            {
                DWARN(false, "YAML node \"%s\" not found", arrayname.c_str());
                break;
            }
            if(!iter->second->isSequence())
            {
                DWARN(false, "YAML node \"%s\" found, but it's not an array", arrayname.c_str());
                break;
            }
            if(arrayindex < 0 || static_cast<size_t>(arrayindex) >= iter->second->sequence()->size())
            {
                DWARN(false , "YAML node \"%s\" found, but index out of range", arrayname.c_str());
                break;
            }
            return sp<String>::make(iter->second->sequence()->at(static_cast<size_t>(arrayindex)));
        }
        const auto iter = bundle.find(nodename);
        if(iter == bundle.end())
        {
            DWARN(false, "YAML node \"%s\" not found", nodename.c_str());
            break;
        }
        return iter->second->value();
    } while(false);
    return sp<String>::make("[" + resid + "]");
}

std::vector<String> StringBundleYAML::getStringArray(const String& resid)
{
    String nodename, arrayname;
    const std::map<String, sp<Item>>& bundle = getPackageBundle(resid, nodename);
    do {
        const auto iter = bundle.find(nodename);
        if(iter == bundle.end())
        {
            DWARN(false, "YAML node \"%s\" not found", nodename.c_str());
            break;
        }
        return iter->second->isSequence() ? iter->second->sequence() : std::vector<String>({*iter->second->value()});
    } while(false);
    return {};
}

void StringBundleYAML::loadBundle(const String& name)
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

const std::map<String, sp<StringBundleYAML::Item>>& StringBundleYAML::getPackageBundle(const String& resid, String& nodename)
{
    String package;
    Strings::cut(resid, package, nodename, '/');
    DCHECK(package && nodename, "Illegal string bundle \"%s\"", resid.c_str());
    const auto iter = _bundle.find(package);
    if(iter == _bundle.end())
        loadBundle(package);

    return _bundle[package];
}

sp<StringBundleYAML::Item>& StringBundleYAML::makeKey(std::map<String, sp<Item>>& bundle, const std::vector<String>& keys) const
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

StringBundleYAML::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC))
{
}

sp<StringBundle> StringBundleYAML::BUILDER::build(const Scope& args)
{
    return sp<StringBundleYAML>::make(Ark::instance().getAssetBundle(_src->build(args)));
}

void StringBundleYAML::Item::setValue(String value)
{
    _value = sp<String>::make(std::move(value));
}

void StringBundleYAML::Item::addSequenceValue(String value)
{
    DASSERT(_sequence);
    _sequence->push_back(std::move(value));
}

void StringBundleYAML::Item::makeSequence()
{
    _sequence = sp<std::vector<String>>::make();
}

bool StringBundleYAML::Item::isSequence() const
{
    return static_cast<bool>(_sequence);
}

const sp<String>& StringBundleYAML::Item::value() const
{
    return _value;
}

const sp<std::vector<String> >& StringBundleYAML::Item::sequence() const
{
    return _sequence;
}

}
