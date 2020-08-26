#ifndef ARK_CORE_UTIL_DOCUMENTS_H_
#define ARK_CORE_UTIL_DOCUMENTS_H_

#include <vector>

#include "core/base/api.h"
#include "core/dom/dom_document.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

namespace ark {

class ARK_API Documents {
public:
    static document parse(const String& xml);
    static document parseFull(const String& xml);
    static document loadFromFile(const String& file_path);
    static void saveToFile(const document& doc, const String& file_path);
    static document loadFromReadable(const sp<Readable>& readable);

    static document fromProperties(const String& str);
    static document fromProperties(const std::map<String, String>& properties);

    static void print(const document& doc, StringBuffer& sb, const String& indent = "\t", uint16_t indent_count = 0);
    static String toString(const document& doc, const String& indent = "\t", uint16_t indent_count = 0);

    static const String getId(const document& doc, const String& defValue = String());
    static const String getAttribute(const document& doc, const String& name, const String& defValue = String());
    static const String& ensureAttribute(const document& doc, const String& name);

    template<typename T> static T getAttribute(const document& doc, const String& name, const T& defValue) {
        const attribute& attr = doc->getAttribute(name);
        return attr ? Strings::parse<T>(attr->value()) : defValue;
    }
    template<typename T> static T ensureAttribute(const document& doc, const String& name) {
        return Strings::parse<T>(ensureAttribute(doc, name));
    }

    template<typename T> static std::vector<T> ensureAttributeList(const document& doc, const String& name, const String& childName = "") {
        std::vector<T> list;
        for(const document& i : childName ? doc->children(childName) : doc->children())
            list.push_back(ensureAttribute<T>(i, name));
        return list;
    }

    template<typename T, typename U> static T getSystemSpecificList(const U& iterable, const String& valuename) {
        T list;
        for(const document& i : iterable) {
            const String& system = getAttribute(i, "for");
            if(system.empty() || isPlatformSpecific(system))
                list.push_back(ensureAttribute(i, valuename));
        }
        return list;
    }

    template<typename T> static T getAttributeValue(const document& doc, const String& path, const T& defValue) {
        const attribute& attr = findAttribute(doc, path.split('/'));
        return attr ? Strings::parse<T>(attr->value()) : defValue;
    }

    static String getAttributeValue(const document& doc, const String& path, const String& defValue = String());

    static attribute findAttribute(const document& doc, const std::vector<String>& paths);

private:
    static bool isPlatformSpecific(const String& platform);

    Documents();
};

}

#endif
