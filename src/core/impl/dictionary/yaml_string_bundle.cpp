#include "core/impl/dictionary/yaml_string_bundle.h"

#include <yaml.h>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/asset.h"
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

YAMLStringBundle::YAMLStringBundle(const sp<Asset>& resource)
    : _resource(resource)
{
    NOT_NULL(_resource);
}

sp<String> YAMLStringBundle::get(const String& name)
{
    String package, nodename;
    Strings::cut(name, package, nodename, '/');
    DCHECK(package && nodename, "Illegal string bundle \"%s\"", name.c_str());
    const auto iter = _bundle.find(package);
    if(iter == _bundle.end())
        loadBundle(package);

    std::map<String, sp<String>>& bundle = _bundle[package];
    const auto iter1 = bundle.find(nodename);
    DCHECK(iter1 != bundle.end(), "\"%s\" not found", nodename.c_str());
    return iter1->second;
}

void YAMLStringBundle::loadBundle(const String& name)
{
    const sp<Readable> readable = _resource->get(name + ".yaml");
    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser))
        FATAL("Failed to initialize parser");

    yaml_parser_set_input(&parser, _yaml_read_handler, readable.get());
    yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);

    yaml_event_t event;
    yaml_event_type_t eventType;

    std::map<String, sp<String>>& bundle = _bundle[name];
    String key, value;
    String* scalar = nullptr;
    int32_t seqIndex = -1;
    do {
        if (!yaml_parser_parse(&parser, &event))
            FATAL("YAML parser error %d\n", parser.error);

        eventType = event.type;
        switch(eventType)
        {
        case YAML_MAPPING_START_EVENT:
            scalar = &key;
            break;
        case YAML_MAPPING_END_EVENT:
            scalar = nullptr;
            break;
        case YAML_SEQUENCE_START_EVENT:
            seqIndex = 0;
            break;
        case YAML_SEQUENCE_END_EVENT:
            seqIndex = -1;
            scalar = &key;
            break;
        case YAML_SCALAR_EVENT:
            DCHECK(scalar, "Illegal state: mapping event not start.");
            *scalar = reinterpret_cast<const char*>(event.data.scalar.value);
            if(seqIndex != -1)
                bundle[Strings::sprintf("%s[%d]", key.c_str(), seqIndex++)] = sp<String>::make(value);
            else if(scalar == &key)
                scalar = &value;
            else
            {
                bundle[key] = sp<String>::make(value);
                scalar = &key;
            }
            break;
        default:
            break;
        }
        yaml_event_delete(&event);
    } while(eventType != YAML_STREAM_END_EVENT);
    yaml_parser_delete(&parser);
}

YAMLStringBundle::BUILDER::BUILDER(BeanFactory& /*parent*/, const document& doc)
    : _path(Strings::load(doc, Constants::Attributes::PATH))
{
}

sp<StringBundle> YAMLStringBundle::BUILDER::build(const sp<Scope>& args)
{
    return sp<YAMLStringBundle>::make(Ark::instance().getAsset(_path->build(args)));
}

}
