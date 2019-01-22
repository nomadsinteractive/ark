#include "core/util/documents.h"

#include <fstream>
#include <tinyxml2.h>

#include "core/base/string_buffer.h"
#include "core/inf/readable.h"
#include "core/util/strings.h"

using namespace tinyxml2;

namespace ark {

namespace {

attribute _tinyattribute2attribute(const XMLAttribute* tinyattr)
{
    return attribute::make(tinyattr->Name(), tinyattr->Value());
}

document _tinyelement2document(const XMLElement* tinyelement)
{
    const document element = document::make(tinyelement->Name());
    const char* text = tinyelement->GetText();
    if(text)
        element->setValue(text);

    for(const XMLAttribute* tinyattr = tinyelement->FirstAttribute();
             tinyattr;
             tinyattr = tinyattr->Next())
    {
        attribute attr = _tinyattribute2attribute(tinyattr);
        element->addAttribute(attr);
    }
    for(const XMLElement* tinynode = tinyelement->FirstChildElement();
             tinynode;
             tinynode = tinynode->NextSiblingElement())
    {
        document node = _tinyelement2document(tinynode);
        element->addChild(node);
    }
    return element;
}

}

document Documents::parse(const String& xml)
{
    XMLDocument tinydoc;
    tinydoc.Parse(xml.c_str());
    DCHECK(tinydoc.RootElement(), "XML document parse failed.");
    return _tinyelement2document(tinydoc.RootElement());
}

document Documents::loadFromFile(const String& file_path)
{
    XMLDocument tinydoc;
    tinydoc.LoadFile(file_path.c_str());
    return _tinyelement2document(tinydoc.RootElement());
}

void Documents::saveToFile(const document& doc, const String& file_path)
{
    std::ofstream os(file_path.c_str());
    os << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << std::endl;
    os << toString(doc).c_str() << std::endl;
}

document Documents::loadFromReadable(const sp<Readable>& readable)
{
    return parse(Strings::loadFromReadable(readable));
}

document Documents::fromProperties(const String& str)
{
    std::map<String, String> properties = Strings::parseProperties(str);
    return fromProperties(properties);
}

document Documents::fromProperties(const std::map<String, String>& properties)
{
    document doc = document::make("root");
    for(auto entry : properties)
        doc->addAttribute(attribute::make(entry.first, entry.second));
    return doc;
}

void Documents::print(const document& doc, StringBuffer& sb, const String& indent, uint16_t indent_count)
{
    for(uint16_t i = 0; i < indent_count; ++i)
        sb << indent;
    sb << '<' << doc->name();

    for(const attribute& attr : doc->attributes()) {
        sb << ' ' << attr->name() << "=\"" << attr->value() << '"';
    }

    StringBuffer content;
    bool dirty = false;

    for(const document& node : doc->children()) {
        Documents::print(node, content, indent, indent_count + 1);
        content << std::endl;
        dirty = true;
    }

    if(doc->value() || dirty)
    {
        sb << '>' << doc->value();
        if(dirty)
        {
            sb << std::endl;
            sb << content.str();
            for(uint16_t i = 0; i < indent_count; ++i)
                sb << indent;
        }
        sb << "</" << doc->name() << '>';
    }
    else
        sb << "/>";
}

String Documents::toString(const document& doc, const String& indent, uint16_t indent_count)
{
    StringBuffer sb;
    print(doc, sb, indent, indent_count);
    return sb.str();
}

const String Documents::getId(const document& doc, const String& defValue)
{
    const String id = getAttribute(doc, Constants::Attributes::ID, defValue);
    return id ? Strings::stripReference(id) : "";
}

template<> ARK_API String Strings::toString<document>(const document& doc)
{
    return Documents::toString(doc);
}

const String Documents::getAttribute(const document& doc, const String& name, const String& defValue)
{
    const attribute& attr = doc->getAttribute(name);
    return attr ? attr->value() : defValue;
}

const String& Documents::ensureAttribute(const document& doc, const String& name)
{
    const attribute& attr = doc->getAttribute(name);
    DCHECK(attr, "Document '%s' has no attribute '%s'", toString(doc).c_str(), name.c_str());
    return attr->value();
}

String Documents::getAttributeValue(const document& doc, const String& path, const String& defValue)
{
    const attribute attr = findAttribute(doc, path.split('/'));
    return attr ? attr->value() : defValue;
}

attribute Documents::findAttribute(const document& doc, const std::vector<String>& paths)
{
    DASSERT(doc);
    DCHECK(paths.size()> 0, "Empty path");
    document node = doc;
    for(size_t i = 0; i < paths.size(); ++i)
    {
        const String& name = paths.at(i);
        document nextNode = node->getChild(name);
        if(nextNode)
            node = std::move(nextNode);
        else
        {
            if(i == paths.size() - 1)
                return node->getAttribute(name);
            return nullptr;
        }
    }
    return node;
}

}
