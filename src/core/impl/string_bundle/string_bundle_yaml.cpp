#include "core/impl/string_bundle/string_bundle_yaml.h"

#include <yaml.h>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/inf/readable.h"

namespace ark {

static int _yaml_read_handler(void* data, unsigned char* buffer, size_t size, size_t* size_read)
{
    Readable* readable = reinterpret_cast<Readable*>(data);
    *size_read = readable->read(buffer, size);
    return 1;
}

StringBundleYAML::StringBundleYAML(sp<AssetBundle> assetBundle)
    : _root(std::move(assetBundle))
{
    DASSERT(_root._asset_bundle);
}

sp<String> StringBundleYAML::getString(const String& resid)
{
    sp<Node> node = _root.findNode(resid);
    CHECK_WARN(node, "YAML node \"%s\" not found", resid.c_str());
    return node ? node->value() : nullptr;
}

std::vector<String> StringBundleYAML::getStringArray(const String& resid)
{
    sp<Node> node = _root.findNode(resid);
    if(node)
        return node->isSequence() ? node->sequence() : std::vector<String>({node->value()});
    return {};
}

sp<StringBundleYAML::Directory> StringBundleYAML::loadAssetDirectory(Asset& asset, sp<AssetBundle> assetBundle)
{
    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser))
        FATAL("Failed to initialize parser");

    const sp<Readable> readable = asset.open();
    yaml_parser_set_input(&parser, _yaml_read_handler, readable.get());
    yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);

    yaml_event_t event;
    yaml_event_type_t eventType;

    std::vector<String> keys;
    sp<Directory> directory = sp<Directory>::make(std::move(assetBundle));
    sp<Node> value;
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
            directory->setNode(keys, std::move(value));
            break;
        case YAML_SCALAR_EVENT: {
            DCHECK(keys.size() > 0, "Illegal state: mapping event not start.");
            String scalar = reinterpret_cast<const char*>(event.data.scalar.value);
            if(value == nullptr)
            {
                keys.back() = std::move(scalar);
                value = sp<Node>::make();
            }
            else
            {
                if(value->isSequence())
                    value->addSequenceValue(std::move(scalar));
                else
                {
                    value->setValue(std::move(scalar));
                    directory->setNode(keys, std::move(value));
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

    return directory;
}

StringBundleYAML::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, constants::SRC))
{
}

sp<StringBundle> StringBundleYAML::BUILDER::build(const Scope& args)
{
    return sp<StringBundleYAML>::make(Ark::instance().getAssetBundle(_src->build(args)));
}

void StringBundleYAML::Node::setValue(String value)
{
    _value = sp<String>::make(std::move(value));
}

void StringBundleYAML::Node::addSequenceValue(String value)
{
    DASSERT(_sequence);
    _sequence->push_back(std::move(value));
}

void StringBundleYAML::Node::makeSequence()
{
    _sequence = sp<std::vector<String>>::make();
}

bool StringBundleYAML::Node::isSequence() const
{
    return static_cast<bool>(_sequence);
}

const sp<String>& StringBundleYAML::Node::value() const
{
    return _value;
}

const sp<std::vector<String> >& StringBundleYAML::Node::sequence() const
{
    return _sequence;
}

StringBundleYAML::Directory::Directory(sp<AssetBundle> assetBundle)
    : _asset_bundle(std::move(assetBundle))
{
}

sp<StringBundleYAML::Node> StringBundleYAML::Directory::findNode(const String& resid)
{
    const auto [dirname, nodeNameOpt] = resid.cut('/');
    if(nodeNameOpt)
    {
        const auto iter = _sub_directories.find(dirname);
        sp<StringBundleYAML::Directory> subdir;
        if(iter == _sub_directories.end())
        {
            if(!_asset_bundle)
                return nullptr;

            sp<Asset> asset = _asset_bundle->getAsset(dirname + ".yaml");
            sp<AssetBundle> subBundle = _asset_bundle->getBundle(dirname);
            subdir = asset ? loadAssetDirectory(asset, std::move(subBundle)) : sp<StringBundleYAML::Directory>::make(std::move(subBundle));
            _sub_directories.insert(std::make_pair(dirname, subdir));
        }
        else
            subdir = iter->second;

        return subdir->findNode(nodeNameOpt.value());
    }
    else if(dirname)
    {
        const auto iter = _nodes.find(dirname);
        return iter != _nodes.end() ? iter->second : nullptr;
    }
    return nullptr;
}

void StringBundleYAML::Directory::setNode(const std::vector<String>& keys, sp<Node> node)
{
    DASSERT(keys.size() > 0);

    Directory* iter = this;
    for(size_t i = 0; i < keys.size() - 1; ++i)
    {
        sp<Directory>& dir = iter->_sub_directories[keys.at(i)];
        if(!dir)
            dir = sp<Directory>::make();
        iter = dir.get();
    }

    iter->_nodes.insert_or_assign(keys.back(), std::move(node));
}

}
